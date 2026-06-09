using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace LunaBuild.Core;

public static class VSCodeWorkspaceWriter
{
    private static readonly JsonDocumentOptions JsonDocumentOptions = new()
    {
        AllowTrailingCommas = true,
        CommentHandling = JsonCommentHandling.Skip,
    };

    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
    };

    public static void Write(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputDirectory,
        VSCodeLaunchOptions? launchOptions = null)
    {
        launchOptions ??= VSCodeLaunchOptions.Default(options.Platform);
        Directory.CreateDirectory(outputDirectory);
        var compileCommandsPath = Path.Combine(workspace.BuildDirectory, "compile_commands.json");
        CompileCommandsWriter.Write(workspace, graph, compileCommandsPath);
        WriteTasks(workspace, options, targets, Path.Combine(outputDirectory, "tasks.json"));
        WriteLaunch(workspace, options, graph, targets, Path.Combine(outputDirectory, "launch.json"), launchOptions);
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
            if(target.Kind is BuildTargetKind.Executable or BuildTargetKind.DotNetProject)
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
        string path,
        VSCodeLaunchOptions launchOptions)
    {
        var root = ReadObject(path);
        root["version"] = "0.2.0";
        var configurations = PreserveArray(root["configurations"], "name");
        var executableOutputs = IdeProjectModel.FindExecutableOutputs(workspace, graph)
            .ToDictionary(pair => pair.TargetName, pair => pair.OutputPath, StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets
            .Where(target => target.Kind == BuildTargetKind.Executable && executableOutputs.ContainsKey(target.Name))
            .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            var program = executableOutputs[target.Name];
            foreach(var debugger in launchOptions.Debuggers)
            {
                configurations.Add(CreateLaunchConfiguration(
                    workspace,
                    options,
                    target.Name,
                    program,
                    debugger,
                    launchOptions.Debuggers.Count > 1));
            }
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

    private static JsonObject CreateLaunchConfiguration(
        BuildWorkspace workspace,
        BuildOptions options,
        string targetName,
        string program,
        VSCodeDebuggerType debugger,
        bool includeDebuggerInName)
    {
        var debuggerName = debugger.ToString();
        var configuration = new JsonObject
        {
            ["name"] = includeDebuggerInName
                ? $"LunaBuild: launch {targetName} ({debuggerName})"
                : $"LunaBuild: launch {targetName}",
            ["type"] = debuggerName,
            ["request"] = "launch",
            ["program"] = ToWorkspaceVariablePath(workspace, program),
            ["args"] = new JsonArray(),
            ["cwd"] = ToWorkspaceVariablePath(workspace, Path.GetDirectoryName(program)!),
            ["preLaunchTask"] = $"LunaBuild: build {targetName}",
        };

        switch(debugger.Kind)
        {
            case VSCodeDebuggerKind.CppVsDbg:
                configuration["stopAtEntry"] = false;
                configuration["environment"] = new JsonArray();
                configuration["console"] = "integratedTerminal";
                break;
            case VSCodeDebuggerKind.CppDbg:
                configuration["stopAtEntry"] = false;
                configuration["environment"] = new JsonArray();
                configuration["externalConsole"] = false;
                AddCppDbgMiMode(configuration, options.Platform);
                break;
            case VSCodeDebuggerKind.LldbDap:
                configuration["stopOnEntry"] = false;
                break;
            default:
                throw new ArgumentOutOfRangeException(nameof(debugger), debugger.Kind, null);
        }

        return configuration;
    }

    private static void AddCppDbgMiMode(JsonObject configuration, BuildPlatform platform)
    {
        if(platform == BuildPlatform.MacOS)
        {
            configuration["MIMode"] = "lldb";
        }
        else if(platform is BuildPlatform.Linux or BuildPlatform.Android)
        {
            configuration["MIMode"] = "gdb";
        }
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
        try
        {
            var node = JsonNode.Parse(File.ReadAllText(path), documentOptions: JsonDocumentOptions);
            return node as JsonObject ?? new JsonObject();
        }
        catch(JsonException ex)
        {
            throw new InvalidOperationException($"Failed to parse VSCode JSON file '{path}': {ex.Message}", ex);
        }
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

public sealed record VSCodeLaunchOptions(IReadOnlyList<VSCodeDebuggerType> Debuggers)
{
    public static VSCodeLaunchOptions Default(BuildPlatform platform)
    {
        return new VSCodeLaunchOptions(new[]
        {
            platform == BuildPlatform.Windows
                ? VSCodeDebuggerType.CppVsDbg
                : VSCodeDebuggerType.CppDbg,
        });
    }
}

public readonly record struct VSCodeDebuggerType(VSCodeDebuggerKind Kind)
{
    public static VSCodeDebuggerType CppVsDbg { get; } = new(VSCodeDebuggerKind.CppVsDbg);

    public static VSCodeDebuggerType CppDbg { get; } = new(VSCodeDebuggerKind.CppDbg);

    public static VSCodeDebuggerType LldbDap { get; } = new(VSCodeDebuggerKind.LldbDap);

    public static VSCodeDebuggerType Parse(string value)
    {
        return value.Replace("_", "-", StringComparison.Ordinal).ToLowerInvariant() switch
        {
            "cppvsdbg" => CppVsDbg,
            "cppdbg" => CppDbg,
            "lldb-dap" => LldbDap,
            _ => throw new ArgumentException($"Unsupported VSCode debugger type: {value}. Expected cppvsdbg, cppdbg, or lldb-dap."),
        };
    }

    public override string ToString()
    {
        return Kind switch
        {
            VSCodeDebuggerKind.CppVsDbg => "cppvsdbg",
            VSCodeDebuggerKind.CppDbg => "cppdbg",
            VSCodeDebuggerKind.LldbDap => "lldb-dap",
            _ => throw new ArgumentOutOfRangeException(nameof(Kind), Kind, null),
        };
    }
}

public enum VSCodeDebuggerKind
{
    CppVsDbg,
    CppDbg,
    LldbDap,
}
