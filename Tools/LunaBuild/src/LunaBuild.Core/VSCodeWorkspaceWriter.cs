using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Core;

public static class VSCodeWorkspaceWriter
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
    };

    public static void Write(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputDirectory)
    {
        Directory.CreateDirectory(outputDirectory);
        var compileCommandsPath = Path.Combine(workspace.BuildDirectory, "compile_commands.json");
        CompileCommandsWriter.Write(workspace, graph, compileCommandsPath);
        WriteTasks(workspace, options, targets, Path.Combine(outputDirectory, "tasks.json"));
        WriteLaunch(workspace, options, graph, targets, Path.Combine(outputDirectory, "launch.json"));
        WriteSettings(workspace, compileCommandsPath, Path.Combine(outputDirectory, "settings.json"));
    }

    private static void WriteTasks(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets,
        string path)
    {
        var root = ReadObject(path);
        root["version"] = "2.0.0";
        var tasks = PreserveArray(root["tasks"], "label");

        tasks.Add(CreateTask("LunaBuild: build all", CommandArgs(workspace, options, "build", targetName: null, all: true, force: false)));
        tasks.Add(CreateTask("LunaBuild: rebuild all", CommandArgs(workspace, options, "build", targetName: null, all: true, force: true)));
        tasks.Add(CreateTask("LunaBuild: clean all", CommandArgs(workspace, options, "clean", targetName: null, all: true, force: false)));
        tasks.Add(CreateTask("LunaBuild: clean full", new[]
        {
            "run", "--no-restore", "--project", "${workspaceFolder}/Tools/LunaBuild/src/LunaBuild.Cli/LunaBuild.Cli.csproj",
            "--", "clean", "--root", "${workspaceFolder}", "--full",
        }));

        foreach(var target in targets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            tasks.Add(CreateTask($"LunaBuild: build {target.Name}", CommandArgs(workspace, options, "build", target.Name, all: false, force: false)));
            tasks.Add(CreateTask($"LunaBuild: rebuild {target.Name}", CommandArgs(workspace, options, "build", target.Name, all: false, force: true)));
            tasks.Add(CreateTask($"LunaBuild: clean {target.Name}", CommandArgs(workspace, options, "clean", target.Name, all: false, force: false)));
        }

        root["tasks"] = tasks;
        WriteObject(path, root);
    }

    private static void WriteLaunch(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string path)
    {
        var root = ReadObject(path);
        root["version"] = "0.2.0";
        var configurations = PreserveArray(root["configurations"], "name");
        var executableOutputs = FindExecutableOutputs(workspace, graph)
            .ToDictionary(pair => pair.TargetName, pair => pair.OutputPath, StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets
            .Where(target => target.Kind == BuildTargetKind.Executable && executableOutputs.ContainsKey(target.Name))
            .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            var program = executableOutputs[target.Name];
            var configuration = new JsonObject
            {
                ["name"] = $"LunaBuild: launch {target.Name}",
                ["type"] = options.Platform == BuildPlatform.Windows ? "cppvsdbg" : "cppdbg",
                ["request"] = "launch",
                ["program"] = ToWorkspaceVariablePath(workspace, program),
                ["args"] = new JsonArray(),
                ["stopAtEntry"] = false,
                ["cwd"] = ToWorkspaceVariablePath(workspace, Path.GetDirectoryName(program)!),
                ["environment"] = new JsonArray(),
                ["preLaunchTask"] = $"LunaBuild: build {target.Name}",
            };
            if(options.Platform == BuildPlatform.MacOS)
            {
                configuration["MIMode"] = "lldb";
            }
            else if(options.Platform == BuildPlatform.Linux)
            {
                configuration["MIMode"] = "gdb";
            }
            configurations.Add(configuration);
        }

        root["configurations"] = configurations;
        WriteObject(path, root);
    }

    private static void WriteSettings(BuildWorkspace workspace, string compileCommandsPath, string path)
    {
        var root = ReadObject(path);
        root["C_Cpp.default.compileCommands"] = ToWorkspaceVariablePath(workspace, compileCommandsPath);
        WriteObject(path, root);
    }

    private static JsonObject CreateTask(string label, IReadOnlyList<string> args)
    {
        var task = new JsonObject
        {
            ["label"] = label,
            ["type"] = "process",
            ["command"] = "dotnet",
            ["args"] = new JsonArray(args.Select(arg => JsonValue.Create(arg)).ToArray<JsonNode?>()),
            ["options"] = new JsonObject
            {
                ["cwd"] = "${workspaceFolder}",
            },
            ["problemMatcher"] = new JsonArray(),
        };
        if(label.Contains("build", StringComparison.OrdinalIgnoreCase))
        {
            task["group"] = "build";
        }
        return task;
    }

    private static IReadOnlyList<string> CommandArgs(
        BuildWorkspace workspace,
        BuildOptions options,
        string command,
        string? targetName,
        bool all,
        bool force)
    {
        var args = new List<string>
        {
            "run",
            "--no-restore",
            "--project",
            "${workspaceFolder}/Tools/LunaBuild/src/LunaBuild.Cli/LunaBuild.Cli.csproj",
            "--",
            command,
            "--root",
            "${workspaceFolder}",
        };
        if(all)
        {
            args.Add("--all");
        }
        else if(targetName is not null)
        {
            args.Add("--target");
            args.Add(targetName);
        }
        args.AddRange(new[]
        {
            "--mode", options.Mode.ToString(),
            "--platform", options.Platform.ToString(),
            "--arch", options.Architecture,
            "--rhi", options.RhiApi.ToString(),
            options.Shared ? "--shared" : "--static",
        });
        if(!options.BuildTests)
        {
            args.Add("--no-tests");
        }
        if(force)
        {
            args.Add("--force");
        }
        return args;
    }

    private static IEnumerable<(string TargetName, string OutputPath)> FindExecutableOutputs(BuildWorkspace workspace, BuildGraph graph)
    {
        foreach(var node in graph.Nodes)
        {
            if(node.Command is null || BuildActionKind.Extract(node.Command) != "cpp.link.executable" || node.Path is null)
            {
                continue;
            }
            var payload = ActionPayload.Parse(node.Command);
            yield return (payload.Required("target"), workspace.ResolveRepositoryPath(node.Path));
        }
    }

    private static JsonArray PreserveArray(JsonNode? node, string key)
    {
        var result = new JsonArray();
        if(node is not JsonArray existing)
        {
            return result;
        }
        foreach(var item in existing)
        {
            if(item is not JsonObject obj)
            {
                continue;
            }
            var value = obj[key]?.GetValue<string>();
            if(value is null || !value.StartsWith("LunaBuild:", StringComparison.Ordinal))
            {
                result.Add(obj.DeepClone());
            }
        }
        return result;
    }

    private static JsonObject ReadObject(string path)
    {
        if(!File.Exists(path))
        {
            return new JsonObject();
        }
        var node = JsonNode.Parse(File.ReadAllText(path));
        return node as JsonObject ?? new JsonObject();
    }

    private static void WriteObject(string path, JsonObject root)
    {
        var directory = Path.GetDirectoryName(Path.GetFullPath(path));
        if(!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }
        File.WriteAllText(path, root.ToJsonString(JsonOptions) + Environment.NewLine, new UTF8Encoding(false));
    }

    private static string ToWorkspaceVariablePath(BuildWorkspace workspace, string path)
    {
        return "${workspaceFolder}/" + workspace.ToRepositoryRelativePath(path);
    }
}
