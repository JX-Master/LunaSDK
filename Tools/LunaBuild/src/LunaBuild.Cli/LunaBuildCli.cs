using LunaBuild.Core;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Cli;

public static class LunaBuildCli
{
    public static int Run(string[] args)
    {
        try
        {
            if(args.Length == 0 || IsHelp(args[0]))
            {
                PrintUsage();
                return 0;
            }

            var command = args[0];
            var options = CommandLineOptions.Parse(args.Skip(1).ToArray());
            return command switch
            {
                "inspect" => Inspect(options),
                "generate" => Generate(options),
                "build" => Build(options),
                "clean" => Clean(options),
                "install" => Install(options),
                _ => UnknownCommand(command),
            };
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine($"lunabuild: {ex.Message}");
            return 1;
        }
    }

    private static int Inspect(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);

        Console.WriteLine($"Workspace: {workspace.RootDirectory}");
        Console.WriteLine($"Mode: {buildOptions.Mode}, Platform: {buildOptions.Platform}, Arch: {buildOptions.Architecture}, RHI: {buildOptions.RhiApi}");
        Console.WriteLine($"Targets: {targets.Count}");

        foreach(var target in targets)
        {
            var deps = target.Dependencies.Count == 0
                ? "-"
                : string.Join(", ", target.Dependencies);
            Console.WriteLine($"  {target.Name} [{target.Category}] -> {deps}");
        }
        return 0;
    }

    private static int Generate(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var format = ResolveOutputFormat(options);
        var categoryFilter = TargetCategoryFilter(options);
        var effectiveAllTargets = options.AllTargets || categoryFilter is not null || (IsIdeOutputFormat(format) && string.IsNullOrWhiteSpace(options.TargetName));
        var graph = GenerateGraph(
            workspace,
            buildOptions,
            targets,
            options.TargetName,
            effectiveAllTargets,
            categoryFilter);
        var outputTargets = SelectTargetsForOutput(
            targets,
            buildOptions,
            options.TargetName,
            effectiveAllTargets,
            categoryFilter,
            includeAllDiscoveredWhenUnfiltered: IsIdeOutputFormat(format) && categoryFilter is null && string.IsNullOrWhiteSpace(options.TargetName));
        var outputPath = ResolveOutputPath(workspace, options, format);

        WriteOutput(workspace, buildOptions, graph, outputTargets, outputPath, format);
        Console.WriteLine($"Generated {FormatName(format)}: {Path.GetFullPath(outputPath)}");
        Console.WriteLine($"Nodes: {graph.Nodes.Count}, Targets: {graph.Targets.Count}");
        return 0;
    }

    private static int Build(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var categoryFilter = TargetCategoryFilter(options);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);

        if(options.OutputPath is not null)
        {
            var format = ResolveOutputFormat(options);
            var outputPath = ResolveOutputPath(workspace, options, format);
            var outputTargets = SelectTargetsForOutput(targets, buildOptions, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);
            WriteOutput(workspace, buildOptions, graph, outputTargets, outputPath, format);
            Console.WriteLine($"Dumped build graph ({format.ToString().ToLowerInvariant()}): {Path.GetFullPath(outputPath)}");
        }

        try
        {
            var makeSystem = new MakeSystemBackend(new IMakeActionExecutor[]
            {
                new CppActionExecutor(),
                new LunaMetaActionExecutor(),
                new CppslShaderActionExecutor(),
                new FileCopyActionExecutor(),
                new BinaryEmbedHeaderActionExecutor(),
                new DotNetBuildActionExecutor(),
                new AggregateActionExecutor(),
            });
            var result = makeSystem.BuildAsync(
                workspace,
                graph,
                options: new MakeSystemBuildOptions(options.ForceRebuild)).GetAwaiter().GetResult();
            Console.WriteLine(result.UpToDate
                ? $"Up to date. Nodes: {result.NodesVisited}"
                : $"Build finished. Nodes: {result.NodesVisited}, Actions: {result.ActionsExecuted}");
            return 0;
        }
        catch(MissingMakeActionExecutorException ex)
        {
            Console.Error.WriteLine(ex.Message);
            Console.Error.WriteLine("C# MakeSystem backend is wired, but the C++ action executors are not implemented yet.");
            return 2;
        }
    }

    private static int Clean(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        if(options.FullClean)
        {
            if(Directory.Exists(workspace.BuildDirectory))
            {
                Directory.Delete(workspace.BuildDirectory, recursive: true);
            }
            Console.WriteLine($"Removed LunaBuild directory: {workspace.BuildDirectory}");
            return 0;
        }

        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var categoryFilter = TargetCategoryFilter(options);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);
        var makeSystem = new MakeSystemBackend(Array.Empty<IMakeActionExecutor>());
        var result = makeSystem.Clean(workspace, graph);
        Console.WriteLine($"Clean finished. Nodes: {result.NodesVisited}, Files: {result.FilesDeleted}, Cache records: {result.CacheRecordsRemoved}");
        return 0;
    }

    private static int Install(CommandLineOptions options)
    {
        if(string.IsNullOrWhiteSpace(options.OutputPath))
        {
            throw new ArgumentException("install requires --output <dir>.");
        }

        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var categoryFilter = TargetCategoryFilter(options);
        var effectiveAllTargets = options.AllTargets || categoryFilter is not null || string.IsNullOrWhiteSpace(options.TargetName);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName, effectiveAllTargets, categoryFilter);
        var outputTargets = SelectTargetsForOutput(targets, buildOptions, options.TargetName, effectiveAllTargets, categoryFilter);
        var result = InstallWriter.Install(workspace, graph, outputTargets, options.OutputPath);
        Console.WriteLine($"Install finished. Files: {result.FilesCopied}, Output: {Path.GetFullPath(options.OutputPath)}");
        return 0;
    }

    private static IReadOnlyList<BuildTargetDefinition> SelectTargetsForOutput(
        IReadOnlyList<BuildTargetDefinition> targets,
        BuildOptions options,
        string? targetName,
        bool allTargets,
        IReadOnlySet<BuildTargetCategory>? categoryFilter,
        bool includeAllDiscoveredWhenUnfiltered = false)
    {
        if(allTargets || string.IsNullOrWhiteSpace(targetName))
        {
            if(categoryFilter is { Count: > 0 })
            {
                return targets
                    .Where(target => categoryFilter.Contains(target.Category))
                    .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase)
                    .ToArray();
            }
            if(includeAllDiscoveredWhenUnfiltered)
            {
                return targets;
            }
            return targets
                .Where(target => BuildTargetCategoryPolicy.IsDefaultEnabled(target.Category))
                .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase)
                .ToArray();
        }

        var targetMap = targets.ToDictionary(target => target.Name, StringComparer.OrdinalIgnoreCase);
        var selected = new List<BuildTargetDefinition>();
        var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        Visit(targetName);
        return selected.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase).ToArray();

        void Visit(string name)
        {
            if(!visited.Add(name) || !targetMap.TryGetValue(name, out var target))
            {
                return;
            }
            foreach(var dependency in target.Dependencies.Order(StringComparer.OrdinalIgnoreCase))
            {
                Visit(dependency);
            }
            selected.Add(target);
        }
    }

    private static BuildGraph GenerateGraph(
        BuildWorkspace workspace,
        BuildOptions buildOptions,
        IReadOnlyList<BuildTargetDefinition> targets,
        string? targetName,
        bool allTargets,
        IReadOnlySet<BuildTargetCategory>? categoryFilter = null)
    {
        if(allTargets)
        {
            return new CppTargetGraphGenerator().GenerateAll(workspace, buildOptions, targets, categoryFilter);
        }
        if(!string.IsNullOrWhiteSpace(targetName))
        {
            return new CppTargetGraphGenerator().Generate(workspace, buildOptions, targets, targetName);
        }
        return new BuildGraphGenerator().GenerateTargetInspectionGraph(workspace, buildOptions, targets);
    }

    private static IReadOnlySet<BuildTargetCategory>? TargetCategoryFilter(CommandLineOptions options)
    {
        return options.TargetCategories.Count == 0
            ? null
            : options.TargetCategories.ToHashSet();
    }

    private static BuildGraphOutputFormat ResolveOutputFormat(CommandLineOptions options)
    {
        if(options.OutputFormat is not null)
        {
            return options.OutputFormat.Value;
        }

        var extension = Path.GetExtension(options.OutputPath ?? string.Empty);
        if(extension.Equals(".json", StringComparison.OrdinalIgnoreCase))
        {
            return BuildGraphOutputFormat.Json;
        }
        if(extension.Equals(".xcodeproj", StringComparison.OrdinalIgnoreCase))
        {
            return BuildGraphOutputFormat.Xcode;
        }
        return BuildGraphOutputFormat.Rules;
    }

    private static string ResolveOutputPath(
        BuildWorkspace workspace,
        CommandLineOptions options,
        BuildGraphOutputFormat format)
    {
        if(options.OutputPath is not null)
        {
            return options.OutputPath;
        }

        return format switch
        {
            BuildGraphOutputFormat.Json => Path.Combine(workspace.BuildDirectory, "graph.json"),
            BuildGraphOutputFormat.Rules => Path.Combine(workspace.BuildDirectory, "graph.lunarules"),
            BuildGraphOutputFormat.CompileCommands => Path.Combine(workspace.BuildDirectory, "compile_commands.json"),
            BuildGraphOutputFormat.Vs2022 => Path.Combine(workspace.BuildDirectory, "VS2022"),
            BuildGraphOutputFormat.Vscode => Path.Combine(workspace.RootDirectory, ".vscode"),
            BuildGraphOutputFormat.Xcode => Path.Combine(workspace.BuildDirectory, "Xcode"),
            _ => throw new ArgumentOutOfRangeException(nameof(format), format, null),
        };
    }

    private static void WriteGraph(BuildGraph graph, string outputPath, BuildGraphOutputFormat format)
    {
        if(format == BuildGraphOutputFormat.Json)
        {
            BuildGraphWriter.WriteJson(graph, outputPath);
            return;
        }
        BuildRuleFileWriter.Write(graph, outputPath);
    }

    private static void WriteOutput(
        BuildWorkspace workspace,
        BuildOptions buildOptions,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputPath,
        BuildGraphOutputFormat format)
    {
        switch(format)
        {
            case BuildGraphOutputFormat.Json:
            case BuildGraphOutputFormat.Rules:
                WriteGraph(graph, outputPath, format);
                break;
            case BuildGraphOutputFormat.CompileCommands:
                CompileCommandsWriter.Write(workspace, graph, outputPath);
                break;
            case BuildGraphOutputFormat.Vs2022:
                VisualStudioSolutionWriter.Write(workspace, buildOptions, graph, targets, outputPath);
                break;
            case BuildGraphOutputFormat.Vscode:
                VSCodeWorkspaceWriter.Write(workspace, buildOptions, graph, targets, outputPath);
                break;
            case BuildGraphOutputFormat.Xcode:
                XcodeProjectWriter.Write(workspace, buildOptions, graph, targets, outputPath);
                break;
            default:
                throw new ArgumentOutOfRangeException(nameof(format), format, null);
        }
    }

    private static int UnknownCommand(string command)
    {
        Console.Error.WriteLine($"Unknown command: {command}");
        PrintUsage();
        return 1;
    }

    private static bool IsHelp(string value) => value is "-h" or "--help" or "help";

    private static bool IsIdeOutputFormat(BuildGraphOutputFormat format)
    {
        return format is BuildGraphOutputFormat.CompileCommands or BuildGraphOutputFormat.Vs2022 or BuildGraphOutputFormat.Vscode or BuildGraphOutputFormat.Xcode;
    }

    private static string FormatName(BuildGraphOutputFormat format)
    {
        return format switch
        {
            BuildGraphOutputFormat.CompileCommands => "compile_commands",
            BuildGraphOutputFormat.Vs2022 => "vs2022 project",
            BuildGraphOutputFormat.Vscode => "vscode workspace files",
            BuildGraphOutputFormat.Xcode => "xcode project",
            _ => $"build graph ({format.ToString().ToLowerInvariant()})",
        };
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: lunabuild <inspect|generate|build|clean|install> [options]");
        Console.WriteLine();
        Console.WriteLine("Options:");
        Console.WriteLine("  --root <path>       LunaSDK repository root. Defaults to auto-discovery.");
        Console.WriteLine("  --output <path>     Graph output path for generate, or optional debug dump for build.");
        Console.WriteLine("  --format <name>     rules, json, compile_commands, vs2022, vscode, or xcode.");
        Console.WriteLine("  --target <name>     Generate/build one pure C++ target graph.");
        Console.WriteLine("  --all               Generate/build all discovered targets as a pure C++ graph.");
        Console.WriteLine("  --category <name>   Filter all-target operations by Engine, Tests, or Tools. Can repeat or use commas.");
        Console.WriteLine("  --force             Force build actions to run even when up to date.");
        Console.WriteLine("  --full              Clean the whole build/LunaBuild directory.");
        Console.WriteLine("  --mode <name>       Debug, Profile, or Release. Default: Debug.");
        Console.WriteLine("  --platform <name>   Windows, MacOS, Linux, Android, or IOS. Default: host.");
        Console.WriteLine("  --arch <name>       Architecture string. Default: host architecture.");
        Console.WriteLine("  --rhi <name>        D3D12, Vulkan, or Metal. Default: platform default.");
        Console.WriteLine("  --static            Generate static target configuration.");
    }
}

internal sealed class CommandLineOptions
{
    public string? RootDirectory { get; private init; }

    public string? OutputPath { get; private init; }

    public string? TargetName { get; private init; }

    public bool AllTargets { get; private init; }

    public IReadOnlyList<BuildTargetCategory> TargetCategories { get; private init; } = Array.Empty<BuildTargetCategory>();

    public bool ForceRebuild { get; private init; }

    public bool FullClean { get; private init; }

    public BuildGraphOutputFormat? OutputFormat { get; private init; }

    public BuildMode? Mode { get; private init; }

    public BuildPlatform? Platform { get; private init; }

    public string? Architecture { get; private init; }

    public RhiApi? RhiApi { get; private init; }

    public bool? Shared { get; private init; }

    public static CommandLineOptions Parse(string[] args)
    {
        var options = new MutableOptions();
        for(var i = 0; i < args.Length; ++i)
        {
            switch(args[i])
            {
                case "--root":
                    options.RootDirectory = RequireValue(args, ref i, "--root");
                    break;
                case "--output":
                    options.OutputPath = RequireValue(args, ref i, "--output");
                    break;
                case "--format":
                    options.OutputFormat = ParseOutputFormat(RequireValue(args, ref i, "--format"));
                    break;
                case "--target":
                    options.TargetName = RequireValue(args, ref i, "--target");
                    break;
                case "--all":
                    options.AllTargets = true;
                    break;
                case "--category":
                case "--categories":
                    foreach(var category in ParseCategories(RequireValue(args, ref i, args[i])))
                    {
                        options.TargetCategories.Add(category);
                    }
                    break;
                case "--force":
                    options.ForceRebuild = true;
                    break;
                case "--full":
                    options.FullClean = true;
                    break;
                case "--mode":
                    options.Mode = ParseEnum<BuildMode>(RequireValue(args, ref i, "--mode"), "--mode");
                    break;
                case "--platform":
                    options.Platform = ParseEnum<BuildPlatform>(RequireValue(args, ref i, "--platform"), "--platform");
                    break;
                case "--arch":
                    options.Architecture = RequireValue(args, ref i, "--arch");
                    break;
                case "--rhi":
                    options.RhiApi = ParseEnum<RhiApi>(RequireValue(args, ref i, "--rhi"), "--rhi");
                    break;
                case "--static":
                    options.Shared = false;
                    break;
                case "--shared":
                    options.Shared = true;
                    break;
                default:
                    throw new ArgumentException($"Unknown option: {args[i]}");
            }
        }
        if(options.AllTargets && !string.IsNullOrWhiteSpace(options.TargetName))
        {
            throw new ArgumentException("--all cannot be combined with --target.");
        }
        if(options.TargetCategories.Count > 0 && !string.IsNullOrWhiteSpace(options.TargetName))
        {
            throw new ArgumentException("--category cannot be combined with --target.");
        }
        return options.ToImmutable();
    }

    public BuildOptions ToBuildOptions()
    {
        var defaults = BuildOptions.HostDefault();
        var platform = Platform ?? defaults.Platform;
        return defaults with
        {
            Mode = Mode ?? defaults.Mode,
            Platform = platform,
            Architecture = Architecture ?? defaults.Architecture,
            Shared = Shared ?? defaults.Shared,
            RhiApi = RhiApi ?? DefaultRhiApi(platform),
        };
    }

    private static RhiApi DefaultRhiApi(BuildPlatform platform)
    {
        return platform switch
        {
            BuildPlatform.Windows => LunaBuild.Core.RhiApi.D3D12,
            BuildPlatform.MacOS or BuildPlatform.IOS => LunaBuild.Core.RhiApi.Metal,
            _ => LunaBuild.Core.RhiApi.Vulkan,
        };
    }

    private static string RequireValue(string[] args, ref int index, string optionName)
    {
        if(index + 1 >= args.Length)
        {
            throw new ArgumentException($"{optionName} requires a value.");
        }
        ++index;
        return args[index];
    }

    private static T ParseEnum<T>(string value, string optionName)
        where T : struct
    {
        if(Enum.TryParse<T>(value, ignoreCase: true, out var parsed))
        {
            return parsed;
        }
        throw new ArgumentException($"{optionName} has invalid value: {value}");
    }

    private static IEnumerable<BuildTargetCategory> ParseCategories(string value)
    {
        foreach(var part in value.Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            yield return ParseEnum<BuildTargetCategory>(part, "--category");
        }
    }

    private static BuildGraphOutputFormat ParseOutputFormat(string value)
    {
        return value.Replace("-", "_").ToLowerInvariant() switch
        {
            "rules" => BuildGraphOutputFormat.Rules,
            "json" => BuildGraphOutputFormat.Json,
            "compile_commands" => BuildGraphOutputFormat.CompileCommands,
            "compilecommands" => BuildGraphOutputFormat.CompileCommands,
            "vs2022" => BuildGraphOutputFormat.Vs2022,
            "vscode" => BuildGraphOutputFormat.Vscode,
            "xcode" => BuildGraphOutputFormat.Xcode,
            "xcodeproj" => BuildGraphOutputFormat.Xcode,
            _ => throw new ArgumentException($"--format has invalid value: {value}"),
        };
    }

    private sealed class MutableOptions
    {
        public string? RootDirectory { get; set; }
        public string? OutputPath { get; set; }
        public string? TargetName { get; set; }
        public bool AllTargets { get; set; }
        public List<BuildTargetCategory> TargetCategories { get; } = new();
        public bool ForceRebuild { get; set; }
        public bool FullClean { get; set; }
        public BuildGraphOutputFormat? OutputFormat { get; set; }
        public BuildMode? Mode { get; set; }
        public BuildPlatform? Platform { get; set; }
        public string? Architecture { get; set; }
        public RhiApi? RhiApi { get; set; }
        public bool? Shared { get; set; }

        public CommandLineOptions ToImmutable()
        {
            return new CommandLineOptions
            {
                RootDirectory = RootDirectory,
                OutputPath = OutputPath,
                TargetName = TargetName,
                AllTargets = AllTargets,
                TargetCategories = TargetCategories.Distinct().OrderBy(category => category.ToString(), StringComparer.Ordinal).ToArray(),
                ForceRebuild = ForceRebuild,
                FullClean = FullClean,
                OutputFormat = OutputFormat,
                Mode = Mode,
                Platform = Platform,
                Architecture = Architecture,
                RhiApi = RhiApi,
                Shared = Shared,
            };
        }
    }
}

internal enum BuildGraphOutputFormat
{
    Rules,
    Json,
    CompileCommands,
    Vs2022,
    Vscode,
    Xcode,
}
