using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

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
            "run", "--no-restore", "--project", "${workspaceFolder}/LunaBuild.csproj",
            "--", "clean", "--root", "${workspaceFolder}", "--full",
        }));

        foreach(var target in targets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            tasks.Add(CreateTask($"LunaBuild: build {target.Name}", CommandArgs(workspace, options, "build", target.Name, all: false, force: false)));
            tasks.Add(CreateTask($"LunaBuild: rebuild {target.Name}", CommandArgs(workspace, options, "build", target.Name, all: false, force: true)));
            tasks.Add(CreateTask($"LunaBuild: clean {target.Name}", CommandArgs(workspace, options, "clean", target.Name, all: false, force: false)));
            if(target.Kind.ProducesNativeExecutable() || target.Kind == BuildTargetKind.DotNetProject)
            {
                tasks.Add(CreateTask($"LunaBuild: run {target.Name}", CommandArgs(workspace, options, "run", target.Name, all: false, force: false)));
            }
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
        var executableOutputs = IdeProjectModel.FindExecutableOutputs(workspace, graph)
            .ToDictionary(pair => pair.TargetName, pair => pair.OutputPath, StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets
            .Where(target => target.Kind.ProducesNativeExecutable() && executableOutputs.ContainsKey(target.Name))
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
        return IdeProjectModel.LunaBuildArguments(
            workspace,
            options,
            command,
            targetName,
            all,
            force,
            "${workspaceFolder}/LunaBuild.csproj",
            "${workspaceFolder}");
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
