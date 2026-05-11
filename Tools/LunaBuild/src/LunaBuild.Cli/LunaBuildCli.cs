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
            Console.WriteLine($"  {target.Name} -> {deps}");
        }
        return 0;
    }

    private static int Generate(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName);
        var format = ResolveOutputFormat(options);
        var outputPath = ResolveOutputPath(workspace, options, format);

        WriteGraph(graph, outputPath, format);
        Console.WriteLine($"Generated build graph ({format.ToString().ToLowerInvariant()}): {Path.GetFullPath(outputPath)}");
        Console.WriteLine($"Nodes: {graph.Nodes.Count}, Targets: {graph.Targets.Count}");
        return 0;
    }

    private static int Build(CommandLineOptions options)
    {
        var workspace = BuildWorkspace.Discover(options.RootDirectory);
        var buildOptions = options.ToBuildOptions();
        var targets = new TargetDiscovery().DiscoverTargets(workspace, buildOptions);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName);

        if(options.OutputPath is not null)
        {
            var format = ResolveOutputFormat(options);
            var outputPath = ResolveOutputPath(workspace, options, format);
            WriteGraph(graph, outputPath, format);
            Console.WriteLine($"Dumped build graph ({format.ToString().ToLowerInvariant()}): {Path.GetFullPath(outputPath)}");
        }

        try
        {
            var makeSystem = new MakeSystemBackend(new IMakeActionExecutor[]
            {
                new CppActionExecutor(),
                new CppslShaderActionExecutor(),
                new FileCopyActionExecutor(),
                new BinaryEmbedHeaderActionExecutor(),
                new DotNetBuildActionExecutor(),
                new AggregateActionExecutor(),
            });
            var result = makeSystem.BuildAsync(workspace, graph).GetAwaiter().GetResult();
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

    private static BuildGraph GenerateGraph(
        BuildWorkspace workspace,
        BuildOptions buildOptions,
        IReadOnlyList<BuildTargetDefinition> targets,
        string? targetName)
    {
        if(!string.IsNullOrWhiteSpace(targetName))
        {
            return new CppTargetGraphGenerator().Generate(workspace, buildOptions, targets, targetName);
        }
        return new BuildGraphGenerator().GenerateTargetInspectionGraph(workspace, buildOptions, targets);
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

        var extension = format == BuildGraphOutputFormat.Json ? ".json" : ".lunarules";
        return Path.Combine(workspace.BuildDirectory, $"graph{extension}");
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

    private static int UnknownCommand(string command)
    {
        Console.Error.WriteLine($"Unknown command: {command}");
        PrintUsage();
        return 1;
    }

    private static bool IsHelp(string value) => value is "-h" or "--help" or "help";

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: lunabuild <inspect|generate|build> [options]");
        Console.WriteLine();
        Console.WriteLine("Options:");
        Console.WriteLine("  --root <path>       LunaSDK repository root. Defaults to auto-discovery.");
        Console.WriteLine("  --output <path>     Graph output path for generate, or optional debug dump for build.");
        Console.WriteLine("  --format <name>     rules or json. Defaults to rules, or json for .json output.");
        Console.WriteLine("  --target <name>     Generate/build one pure C++ target graph.");
        Console.WriteLine("  --mode <name>       Debug, Profile, or Release. Default: Debug.");
        Console.WriteLine("  --platform <name>   Windows, MacOS, Linux, Android, or IOS. Default: host.");
        Console.WriteLine("  --arch <name>       Architecture string. Default: x64.");
        Console.WriteLine("  --rhi <name>        D3D12, Vulkan, or Metal. Default: platform default.");
        Console.WriteLine("  --static            Generate static target configuration.");
        Console.WriteLine("  --no-tests          Disable tests in generated options.");
    }
}

internal sealed class CommandLineOptions
{
    public string? RootDirectory { get; private init; }

    public string? OutputPath { get; private init; }

    public string? TargetName { get; private init; }

    public BuildGraphOutputFormat? OutputFormat { get; private init; }

    public BuildMode? Mode { get; private init; }

    public BuildPlatform? Platform { get; private init; }

    public string? Architecture { get; private init; }

    public RhiApi? RhiApi { get; private init; }

    public bool? Shared { get; private init; }

    public bool? BuildTests { get; private init; }

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
                    options.OutputFormat = ParseEnum<BuildGraphOutputFormat>(RequireValue(args, ref i, "--format"), "--format");
                    break;
                case "--target":
                    options.TargetName = RequireValue(args, ref i, "--target");
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
                case "--no-tests":
                    options.BuildTests = false;
                    break;
                case "--tests":
                    options.BuildTests = true;
                    break;
                default:
                    throw new ArgumentException($"Unknown option: {args[i]}");
            }
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
            BuildTests = BuildTests ?? defaults.BuildTests,
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

    private sealed class MutableOptions
    {
        public string? RootDirectory { get; set; }
        public string? OutputPath { get; set; }
        public string? TargetName { get; set; }
        public BuildGraphOutputFormat? OutputFormat { get; set; }
        public BuildMode? Mode { get; set; }
        public BuildPlatform? Platform { get; set; }
        public string? Architecture { get; set; }
        public RhiApi? RhiApi { get; set; }
        public bool? Shared { get; set; }
        public bool? BuildTests { get; set; }

        public CommandLineOptions ToImmutable()
        {
            return new CommandLineOptions
            {
                RootDirectory = RootDirectory,
                OutputPath = OutputPath,
                TargetName = TargetName,
                OutputFormat = OutputFormat,
                Mode = Mode,
                Platform = Platform,
                Architecture = Architecture,
                RhiApi = RhiApi,
                Shared = Shared,
                BuildTests = BuildTests,
            };
        }
    }
}

internal enum BuildGraphOutputFormat
{
    Rules,
    Json,
}
