using System.Diagnostics;
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
            var commandArguments = args.Skip(1).ToArray();
            var runArguments = Array.Empty<string>();
            if(string.Equals(command, "run", StringComparison.OrdinalIgnoreCase))
            {
                (commandArguments, runArguments) = SplitRunArguments(commandArguments);
                commandArguments = NormalizeRunTargetArgument(commandArguments);
            }
            else if(string.Equals(command, "package", StringComparison.OrdinalIgnoreCase))
            {
                commandArguments = NormalizeRunTargetArgument(commandArguments);
            }
            var options = CommandLineOptions.Parse(commandArguments);
            return command switch
            {
                "inspect" => Inspect(options),
                "generate" => Generate(options),
                "build" => Build(options),
                "clean" => Clean(options),
                "install" => Install(options),
                "run" => RunTarget(options, runArguments),
                "package" => Package(options),
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
        var context = CreateBuildContext(options);
        var workspace = context.Workspace;
        var buildOptions = context.BuildOptions;
        var targets = context.Targets;

        Console.WriteLine($"Workspace: {workspace.RootDirectory}");
        Console.WriteLine($"Mode: {buildOptions.Mode}, Platform: {buildOptions.Platform}, Arch: {buildOptions.Architecture}, RHI: {buildOptions.RhiApi}");
        Console.WriteLine($"Options: shared={buildOptions.Shared}");
        if(buildOptions.Properties.Values.Count > 0)
        {
            Console.WriteLine("Project properties:");
            foreach(var property in buildOptions.Properties.Values)
            {
                Console.WriteLine($"  {property.Name}={property.Value}");
            }
        }
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
        var context = CreateBuildContext(options);
        var workspace = context.Workspace;
        var buildOptions = context.BuildOptions;
        var targets = context.Targets;
        var format = ResolveOutputFormat(options);
        var categoryFilter = TargetCategoryFilter(options);
        var graphCategoryFilter = categoryFilter ??
            (IsIdeOutputFormat(format) && string.IsNullOrWhiteSpace(options.TargetName)
                ? AllTargetCategories()
                : null);
        var effectiveAllTargets = options.AllTargets || categoryFilter is not null || (IsIdeOutputFormat(format) && string.IsNullOrWhiteSpace(options.TargetName));
        var graph = GenerateGraph(
            workspace,
            buildOptions,
            targets,
            options.TargetName,
            effectiveAllTargets,
            graphCategoryFilter);
        var outputTargets = SelectTargetsForOutput(
            targets,
            buildOptions,
            options.TargetName,
            effectiveAllTargets,
            categoryFilter,
            includeAllDiscoveredWhenUnfiltered: IsIdeOutputFormat(format) && categoryFilter is null && string.IsNullOrWhiteSpace(options.TargetName));
        var outputPath = ResolveOutputPath(workspace, options, format);

        WriteOutput(workspace, buildOptions, graph, outputTargets, outputPath, format, options);
        Console.WriteLine($"Generated {FormatName(format)}: {Path.GetFullPath(outputPath)}");
        Console.WriteLine($"Nodes: {graph.Nodes.Count}, Targets: {graph.Targets.Count}");
        return 0;
    }

    private static int Build(CommandLineOptions options)
    {
        var context = CreateBuildContext(options);
        var workspace = context.Workspace;
        var buildOptions = context.BuildOptions;
        var targets = context.Targets;
        var categoryFilter = TargetCategoryFilter(options);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);

        if(options.OutputPath is not null)
        {
            var format = ResolveOutputFormat(options);
            var outputPath = ResolveOutputPath(workspace, options, format);
            var outputTargets = SelectTargetsForOutput(targets, buildOptions, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);
            WriteOutput(workspace, buildOptions, graph, outputTargets, outputPath, format, options);
            Console.WriteLine($"Dumped build graph ({format.ToString().ToLowerInvariant()}): {Path.GetFullPath(outputPath)}");
        }

        try
        {
            var result = ExecuteBuild(workspace, graph, options.ForceRebuild);
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

    private static int RunTarget(CommandLineOptions options, IReadOnlyList<string> arguments)
    {
        if(string.IsNullOrWhiteSpace(options.TargetName))
        {
            throw new ArgumentException("run requires --target <name> or a target name.");
        }
        if(options.AllTargets)
        {
            throw new ArgumentException("run cannot be combined with --all.");
        }
        if(options.TargetCategories.Count > 0)
        {
            throw new ArgumentException("run cannot be combined with --category.");
        }

        var context = CreateBuildContext(options);
        var workspace = context.Workspace;
        var target = context.Targets.FirstOrDefault(target => string.Equals(target.Name, options.TargetName, StringComparison.OrdinalIgnoreCase))
            ?? throw new ArgumentException($"Unknown target: {options.TargetName}");
        if(target.Kind is not (BuildTargetKind.Executable or BuildTargetKind.DotNetProject))
        {
            throw new ArgumentException($"Target `{target.Name}` is {target.Kind} and cannot be run.");
        }

        var graph = GenerateGraph(workspace, context.BuildOptions, context.Targets, target.Name, allTargets: false);
        var result = ExecuteBuild(workspace, graph, options.ForceRebuild);
        Console.WriteLine(result.UpToDate
            ? $"Up to date. Nodes: {result.NodesVisited}"
            : $"Build finished. Nodes: {result.NodesVisited}, Actions: {result.ActionsExecuted}");

        var executable = BuildGraphQueries.FindRunnableOutput(workspace, graph, target.Name)
            ?? throw new InvalidOperationException($"Target `{target.Name}` did not produce a runnable output.");
        if(!File.Exists(executable))
        {
            throw new FileNotFoundException($"Runnable output is missing after build: {executable}", executable);
        }

        Console.WriteLine($"Running {target.Name}: {executable}");
        return RunProcess(executable, arguments);
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

        var context = CreateBuildContext(options, workspace);
        var buildOptions = context.BuildOptions;
        var targets = context.Targets;
        var categoryFilter = TargetCategoryFilter(options);
        var graph = GenerateGraph(workspace, buildOptions, targets, options.TargetName, options.AllTargets || categoryFilter is not null, categoryFilter);
        var makeSystem = new MakeSystemBackend(Array.Empty<IMakeActionExecutor>());
        var result = makeSystem.Clean(workspace, graph);
        Console.WriteLine($"Clean finished. Nodes: {result.NodesVisited}, Files: {result.FilesDeleted}, Cache records: {result.CacheRecordsRemoved}");
        return 0;
    }

    private static int Package(CommandLineOptions options)
    {
        if(string.IsNullOrWhiteSpace(options.TargetName))
        {
            throw new ArgumentException("package requires --target <name> or a target name.");
        }
        if(options.AllTargets)
        {
            throw new ArgumentException("package cannot be combined with --all.");
        }
        if(options.TargetCategories.Count > 0)
        {
            throw new ArgumentException("package cannot be combined with --category.");
        }

        var context = CreateBuildContext(options);
        if(context.BuildOptions.Platform != BuildPlatform.Android)
        {
            throw new ArgumentException("package currently supports Android targets only. Pass --platform Android.");
        }

        var target = context.Targets.FirstOrDefault(target => string.Equals(target.Name, options.TargetName, StringComparison.OrdinalIgnoreCase))
            ?? throw new ArgumentException($"Unknown target: {options.TargetName}");
        if(target.Kind != BuildTargetKind.Executable)
        {
            throw new ArgumentException($"Target `{target.Name}` is {target.Kind} and cannot be packaged as an Android application.");
        }

        var graph = GenerateGraph(context.Workspace, context.BuildOptions, context.Targets, target.Name, allTargets: false);
        var result = ExecuteBuild(context.Workspace, graph, options.ForceRebuild);
        Console.WriteLine(result.UpToDate
            ? $"Up to date. Nodes: {result.NodesVisited}"
            : $"Build finished. Nodes: {result.NodesVisited}, Actions: {result.ActionsExecuted}");

        var packageResult = PackageAndroid(context.Workspace, context.BuildOptions, graph, target, options.OutputPath);
        Console.WriteLine($"Android package finished. Native libraries: {packageResult.NativeLibrariesCopied}, APK: {packageResult.ApkPath}");
        return 0;
    }

    private static int Install(CommandLineOptions options)
    {
        if(string.IsNullOrWhiteSpace(options.OutputPath))
        {
            throw new ArgumentException("install requires --output <dir>.");
        }

        var context = CreateBuildContext(options);
        var workspace = context.Workspace;
        var buildOptions = context.BuildOptions;
        var targets = context.Targets;
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

    private static BuildContext CreateBuildContext(CommandLineOptions options, BuildWorkspace? existingWorkspace = null)
    {
        var workspace = existingWorkspace ?? BuildWorkspace.Discover(options.RootDirectory);
        var provider = new ProjectTargetRulesProvider();
        var projectRules = provider.GetProjectRules(workspace);
        var projectDefinition = MergeProjectDefinitions(projectRules.Select(rules => rules.ToDefinition(workspace)).ToArray());
        var buildOptions = options.ToBuildOptions(projectDefinition);
        foreach(var rules in projectRules)
        {
            buildOptions = rules.ConfigureBuildOptions(workspace, buildOptions);
        }
        var targets = new TargetDiscovery(new ITargetRulesProvider[] { provider }).DiscoverTargets(workspace, buildOptions);
        return new BuildContext(workspace, buildOptions, targets, projectDefinition);
    }

    private static BuildProjectDefinition MergeProjectDefinitions(IReadOnlyList<BuildProjectDefinition> definitions)
    {
        if(definitions.Count == 0)
        {
            return BuildProjectDefinition.Empty;
        }

        return new BuildProjectDefinition(
            string.Join("+", definitions.Select(definition => definition.Name).Order(StringComparer.OrdinalIgnoreCase)),
            definitions
                .SelectMany(definition => definition.Properties)
                .OrderBy(property => property.Name, StringComparer.OrdinalIgnoreCase)
                .ToArray());
    }

    private static IReadOnlySet<BuildTargetCategory>? TargetCategoryFilter(CommandLineOptions options)
    {
        return options.TargetCategories.Count == 0
            ? null
            : options.TargetCategories.ToHashSet();
    }

    private static IReadOnlySet<BuildTargetCategory> AllTargetCategories()
    {
        return Enum.GetValues<BuildTargetCategory>().ToHashSet();
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
        BuildGraphOutputFormat format,
        CommandLineOptions commandLineOptions)
    {
        if(format != BuildGraphOutputFormat.Vscode && commandLineOptions.VSCodeDebuggerTypes.Count > 0)
        {
            throw new ArgumentException("--vscode-debugger can only be used with --format vscode.");
        }

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
                VSCodeWorkspaceWriter.Write(workspace, buildOptions, graph, targets, outputPath, commandLineOptions.ToVSCodeLaunchOptions());
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

    private static MakeSystemResult ExecuteBuild(BuildWorkspace workspace, BuildGraph graph, bool forceRebuild)
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
        return makeSystem.BuildAsync(
            workspace,
            graph,
            options: new MakeSystemBuildOptions(forceRebuild)).GetAwaiter().GetResult();
    }

    private static int RunProcess(string executable, IReadOnlyList<string> arguments)
    {
        using var process = new Process();
        process.StartInfo = new ProcessStartInfo
        {
            FileName = executable,
            WorkingDirectory = Path.GetDirectoryName(executable) ?? Environment.CurrentDirectory,
            UseShellExecute = false,
        };
        foreach(var argument in arguments)
        {
            process.StartInfo.ArgumentList.Add(argument);
        }

        process.Start();
        process.WaitForExit();
        return process.ExitCode;
    }

    private static AndroidPackageResult PackageAndroid(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        BuildTargetDefinition target,
        string? outputPath)
    {
        var androidProject = Path.Combine(target.Directory, "AndroidProject");
        if(!File.Exists(Path.Combine(androidProject, "settings.gradle")))
        {
            throw new DirectoryNotFoundException($"Android project was not found for target `{target.Name}`: {androidProject}");
        }

        var abi = AndroidAbi(options.Architecture);
        var jniLibsDirectory = Path.Combine(androidProject, "app", "src", "main", "jniLibs", abi);
        if(Directory.Exists(jniLibsDirectory))
        {
            Directory.Delete(jniLibsDirectory, recursive: true);
        }
        Directory.CreateDirectory(jniLibsDirectory);

        var sharedLibraries = FindAndroidSharedLibraries(workspace, graph).ToArray();
        var targetLibraryName = $"lib{target.Name}.so";
        if(!sharedLibraries.Any(path => Path.GetFileName(path).Equals(targetLibraryName, StringComparison.OrdinalIgnoreCase)))
        {
            throw new FileNotFoundException($"Android target `{target.Name}` did not produce `{targetLibraryName}`.");
        }

        foreach(var library in sharedLibraries)
        {
            File.Copy(library, Path.Combine(jniLibsDirectory, Path.GetFileName(library)), overwrite: true);
        }
        var cxxRuntime = AndroidNdkToolchainLocator.CxxSharedRuntime(AndroidNdkToolchainLocator.Locate(), options.Architecture);
        File.Copy(cxxRuntime, Path.Combine(jniLibsDirectory, Path.GetFileName(cxxRuntime)), overwrite: true);

        var buildType = options.Mode == BuildMode.Release ? "Release" : "Debug";
        var gradleTask = $":app:assemble{buildType}";
        RunGradle(workspace, androidProject, gradleTask);

        var apkDirectory = Path.Combine(androidProject, "app", "build", "outputs", "apk", buildType.ToLowerInvariant());
        var apks = Directory.Exists(apkDirectory)
            ? Directory.GetFiles(apkDirectory, "*.apk").Order(StringComparer.OrdinalIgnoreCase).ToArray()
            : Array.Empty<string>();
        if(apks.Length == 0)
        {
            throw new FileNotFoundException($"Gradle did not produce an APK under: {apkDirectory}");
        }

        var apk = apks[0];
        if(!string.IsNullOrWhiteSpace(outputPath))
        {
            var fullOutput = Path.GetFullPath(outputPath);
            if(Path.GetExtension(fullOutput).Equals(".apk", StringComparison.OrdinalIgnoreCase))
            {
                Directory.CreateDirectory(Path.GetDirectoryName(fullOutput)!);
                File.Copy(apk, fullOutput, overwrite: true);
                apk = fullOutput;
            }
            else
            {
                Directory.CreateDirectory(fullOutput);
                var copyPath = Path.Combine(fullOutput, Path.GetFileName(apk));
                File.Copy(apk, copyPath, overwrite: true);
                apk = copyPath;
            }
        }

        return new AndroidPackageResult(sharedLibraries.Length + 1, apk);
    }

    private static IEnumerable<string> FindAndroidSharedLibraries(BuildWorkspace workspace, BuildGraph graph)
    {
        return graph.Nodes
            .Where(node => node.Kind == BuildGraphNodeKind.File &&
                node.Path is not null &&
                node.Command is not null &&
                node.Path.EndsWith(".so", StringComparison.OrdinalIgnoreCase) &&
                BuildActionKind.Extract(node.Command) == "cpp.link.shared")
            .Select(node => workspace.ResolveRepositoryPath(node.Path!))
            .Where(File.Exists)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase);
    }

    private static void RunGradle(BuildWorkspace workspace, string androidProject, string task)
    {
        var gradlew = Path.Combine(androidProject, OperatingSystem.IsWindows() ? "gradlew.bat" : "gradlew");
        if(!File.Exists(gradlew))
        {
            throw new FileNotFoundException($"Gradle wrapper was not found: {gradlew}", gradlew);
        }

        var gradleArguments = new[] { task, "--no-daemon", "--stacktrace", "--console=plain" };
        var startInfo = new ProcessStartInfo
        {
            WorkingDirectory = androidProject,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };
        if(OperatingSystem.IsWindows())
        {
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = $"/d /s /c \"\"{gradlew}\" {string.Join(" ", gradleArguments.Select(QuoteCommandArgument))}\"";
        }
        else
        {
            startInfo.FileName = gradlew;
            foreach(var argument in gradleArguments)
            {
                startInfo.ArgumentList.Add(argument);
            }
        }
        ConfigureAndroidGradleEnvironment(startInfo);
        var gradleUserHome = Path.Combine(workspace.BuildDirectory, "GradleUserHome");
        Directory.CreateDirectory(gradleUserHome);
        startInfo.Environment["GRADLE_USER_HOME"] = gradleUserHome;
        var androidUserHome = Path.Combine(workspace.BuildDirectory, "AndroidUserHome");
        Directory.CreateDirectory(androidUserHome);
        startInfo.Environment["ANDROID_USER_HOME"] = androidUserHome;
        RemoveEnvironmentVariable(startInfo, "ANDROID_SDK_HOME");

        var output = new System.Text.StringBuilder();
        var outputLock = new object();
        void RecordOutput(string line, bool isError)
        {
            lock(outputLock)
            {
                output.AppendLine(line);
            }
            if(isError)
            {
                Console.Error.WriteLine(line);
            }
            else
            {
                Console.WriteLine(line);
            }
        }

        Console.WriteLine($"Running Gradle: {Path.GetFileName(gradlew)} {string.Join(" ", gradleArguments)}");
        using var process = new Process();
        process.StartInfo = startInfo;
        process.OutputDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                RecordOutput(e.Data, isError: false);
            }
        };
        process.ErrorDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                RecordOutput(e.Data, isError: true);
            }
        };
        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();
        if(!process.WaitForExit((int)TimeSpan.FromMinutes(5).TotalMilliseconds))
        {
            TryKill(process);
            throw new TimeoutException($"Gradle timed out after 300s: {task}{Environment.NewLine}{LastLines(output.ToString(), 80)}");
        }
        process.WaitForExit();

        if(process.ExitCode != 0)
        {
            throw new InvalidOperationException($"Gradle task failed with exit code {process.ExitCode}: {task}{Environment.NewLine}{LastLines(output.ToString(), 80)}");
        }
    }

    private static void ConfigureAndroidGradleEnvironment(ProcessStartInfo startInfo)
    {
        var javaHome = Environment.GetEnvironmentVariable("JAVA_HOME");
        if(string.IsNullOrWhiteSpace(javaHome) || !File.Exists(Path.Combine(javaHome, "bin", OperatingSystem.IsWindows() ? "java.exe" : "java")))
        {
            javaHome = LocateJavaHome();
            if(javaHome is not null)
            {
                startInfo.Environment["JAVA_HOME"] = javaHome;
                PrependPath(startInfo, Path.Combine(javaHome, "bin"));
            }
        }

        var androidSdk = Environment.GetEnvironmentVariable("ANDROID_HOME") ?? Environment.GetEnvironmentVariable("ANDROID_SDK_ROOT");
        if(string.IsNullOrWhiteSpace(androidSdk) || !Directory.Exists(androidSdk))
        {
            androidSdk = LocateAndroidSdk();
            if(androidSdk is not null)
            {
                startInfo.Environment["ANDROID_HOME"] = androidSdk;
                startInfo.Environment["ANDROID_SDK_ROOT"] = androidSdk;
            }
        }
    }

    private static string? LocateJavaHome()
    {
        var candidates = new List<string>();
        if(OperatingSystem.IsWindows())
        {
            var programFiles = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
            if(!string.IsNullOrWhiteSpace(programFiles))
            {
                candidates.Add(Path.Combine(programFiles, "Android", "Android Studio", "jbr"));
                candidates.AddRange(Directory.Exists(Path.Combine(programFiles, "Java"))
                    ? Directory.GetDirectories(Path.Combine(programFiles, "Java")).OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
                    : Array.Empty<string>());
                candidates.AddRange(Directory.Exists(Path.Combine(programFiles, "Eclipse Adoptium"))
                    ? Directory.GetDirectories(Path.Combine(programFiles, "Eclipse Adoptium")).OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
                    : Array.Empty<string>());
            }
        }
        else if(OperatingSystem.IsMacOS())
        {
            candidates.Add("/Applications/Android Studio.app/Contents/jbr/Contents/Home");
            candidates.AddRange(Directory.Exists("/Library/Java/JavaVirtualMachines")
                ? Directory.GetDirectories("/Library/Java/JavaVirtualMachines")
                    .OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
                    .Select(path => Path.Combine(path, "Contents", "Home"))
                : Array.Empty<string>());
        }
        else
        {
            candidates.Add("/usr/lib/jvm/default-java");
            candidates.AddRange(Directory.Exists("/usr/lib/jvm")
                ? Directory.GetDirectories("/usr/lib/jvm").OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
                : Array.Empty<string>());
        }

        var javaName = OperatingSystem.IsWindows() ? "java.exe" : "java";
        return candidates.FirstOrDefault(path => File.Exists(Path.Combine(path, "bin", javaName)));
    }

    private static string? LocateAndroidSdk()
    {
        foreach(var name in new[] { "ANDROID_HOME", "ANDROID_SDK_ROOT" })
        {
            var value = Environment.GetEnvironmentVariable(name);
            if(!string.IsNullOrWhiteSpace(value) && Directory.Exists(value))
            {
                return Path.GetFullPath(value);
            }
        }

        if(OperatingSystem.IsWindows())
        {
            var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            if(!string.IsNullOrWhiteSpace(localAppData))
            {
                var sdk = Path.Combine(localAppData, "Android", "Sdk");
                if(Directory.Exists(sdk))
                {
                    return sdk;
                }
            }
        }
        else if(OperatingSystem.IsMacOS())
        {
            var sdk = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), "Library", "Android", "sdk");
            if(Directory.Exists(sdk))
            {
                return sdk;
            }
        }
        else
        {
            var sdk = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), "Android", "Sdk");
            if(Directory.Exists(sdk))
            {
                return sdk;
            }
        }
        return null;
    }

    private static void PrependPath(ProcessStartInfo startInfo, string path)
    {
        if(!Directory.Exists(path))
        {
            return;
        }

        var key = startInfo.Environment.Keys.FirstOrDefault(key => key.Equals("PATH", StringComparison.OrdinalIgnoreCase)) ?? "PATH";
        var existing = startInfo.Environment.TryGetValue(key, out var value) ? value : string.Empty;
        startInfo.Environment[key] = string.IsNullOrWhiteSpace(existing)
            ? path
            : path + Path.PathSeparator + existing;
    }

    private static void RemoveEnvironmentVariable(ProcessStartInfo startInfo, string name)
    {
        var key = startInfo.Environment.Keys.FirstOrDefault(key => key.Equals(name, StringComparison.OrdinalIgnoreCase));
        if(key is not null)
        {
            startInfo.Environment.Remove(key);
        }
    }

    private static string QuoteCommandArgument(string argument)
    {
        if(argument.Length == 0)
        {
            return "\"\"";
        }
        return argument.Any(char.IsWhiteSpace) || argument.Contains('"')
            ? "\"" + argument.Replace("\"", "\\\"") + "\""
            : argument;
    }

    private static string LastLines(string text, int maxLines)
    {
        var lines = text.Replace("\r\n", "\n", StringComparison.Ordinal).Split('\n');
        if(lines.Length <= maxLines)
        {
            return text.TrimEnd();
        }
        return string.Join(Environment.NewLine, lines.Skip(lines.Length - maxLines)).TrimEnd();
    }

    private static void TryKill(Process process)
    {
        try
        {
            if(!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
            }
        }
        catch
        {
            // Best effort cleanup after timeout.
        }
    }

    private static string AndroidAbi(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "arm64" or "aarch64" or "arm64-v8a" => "arm64-v8a",
            "x64" or "x86_64" => "x86_64",
            "x86" or "i386" => "x86",
            "arm" or "armv7" or "armeabi-v7a" => "armeabi-v7a",
            _ => throw new ArgumentException($"Unsupported Android architecture: {architecture}"),
        };
    }

    private static (string[] Options, string[] Arguments) SplitRunArguments(string[] args)
    {
        var separatorIndex = Array.IndexOf(args, "--");
        if(separatorIndex < 0)
        {
            return (args, Array.Empty<string>());
        }
        return (args[..separatorIndex], args[(separatorIndex + 1)..]);
    }

    private static string[] NormalizeRunTargetArgument(string[] args)
    {
        if(args.Length == 0 || args[0].StartsWith("--", StringComparison.Ordinal))
        {
            return args;
        }

        return new[] { "--target", args[0] }.Concat(args.Skip(1)).ToArray();
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: lunabuild <inspect|generate|build|clean|install|run|package> [options]");
        Console.WriteLine();
        Console.WriteLine("Options:");
        Console.WriteLine("  --root <path>       LunaSDK repository root. Defaults to auto-discovery.");
        Console.WriteLine("  --output <path>     Graph output path for generate, or optional debug dump for build.");
        Console.WriteLine("  --format <name>     rules, json, compile_commands, vs2022, vscode, or xcode.");
        Console.WriteLine("  --vscode-debugger <name>");
        Console.WriteLine("                       VSCode launch debugger: cppvsdbg, cppdbg, or lldb-dap. Can repeat or use commas.");
        Console.WriteLine("  --target <name>     Select one target.");
        Console.WriteLine("  --all               Select all discovered targets.");
        Console.WriteLine("  --category <name>   Filter all-target operations by Engine, Tests, or Tools. Can repeat or use commas.");
        Console.WriteLine("  --force             Force build actions to run even when up to date.");
        Console.WriteLine("  --full              Clean the whole build/LunaBuild directory.");
        Console.WriteLine("  --mode <name>       Debug, Profile, or Release. Default: Debug.");
        Console.WriteLine("  --platform <name>   Windows, MacOS, Linux, Android, or IOS. Default: host.");
        Console.WriteLine("  --arch <name>       Architecture string. Default: host architecture.");
        Console.WriteLine("  --rhi <name>        D3D12, Vulkan, or Metal. Default: platform default.");
        Console.WriteLine("  --static            Generate static target configuration.");
        Console.WriteLine("  --property <k=v>    Set one project-defined build property.");
        Console.WriteLine("  --<property>        Set one project-defined boolean property to true.");
        Console.WriteLine();
        Console.WriteLine("Run:");
        Console.WriteLine("  lunabuild run --target <name> -- [program arguments]");
        Console.WriteLine("  lunabuild run <name> -- [program arguments]");
        Console.WriteLine();
        Console.WriteLine("Package:");
        Console.WriteLine("  lunabuild package <name> --platform Android --arch arm64-v8a [--output <apk-or-dir>]");
    }

    private sealed record AndroidPackageResult(int NativeLibrariesCopied, string ApkPath);

    private sealed record BuildContext(
        BuildWorkspace Workspace,
        BuildOptions BuildOptions,
        IReadOnlyList<BuildTargetDefinition> Targets,
        BuildProjectDefinition Project);
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

    public IReadOnlyList<VSCodeDebuggerType> VSCodeDebuggerTypes { get; private init; } = Array.Empty<VSCodeDebuggerType>();

    public IReadOnlyDictionary<string, string?> ProjectPropertyOverrides { get; private init; } = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

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
                case "--vscode-debugger":
                case "--vscode-debuggers":
                    foreach(var debugger in ParseVSCodeDebuggers(RequireValue(args, ref i, args[i])))
                    {
                        options.VSCodeDebuggerTypes.Add(debugger);
                    }
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
                case "--property":
                    AddProjectProperty(options.ProjectPropertyOverrides, RequireValue(args, ref i, "--property"));
                    break;
                default:
                    if(args[i].StartsWith("--", StringComparison.Ordinal))
                    {
                        AddProjectProperty(options.ProjectPropertyOverrides, ParseProjectPropertyOption(args, ref i));
                        break;
                    }
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

    public BuildOptions ToBuildOptions(BuildProjectDefinition projectDefinition)
    {
        var defaults = BuildOptions.HostDefault();
        var platform = Platform ?? defaults.Platform;
        var architecture = Architecture ?? defaults.Architecture;
        ValidatePlatformArchitecture(platform, architecture);
        var properties = projectDefinition.ResolveProperties(ProjectPropertyOverrides);
        return defaults with
        {
            Mode = Mode ?? defaults.Mode,
            Platform = platform,
            Architecture = architecture,
            Shared = Shared ?? defaults.Shared,
            RhiApi = RhiApi ?? DefaultRhiApi(platform),
            Properties = properties,
        };
    }

    public VSCodeLaunchOptions? ToVSCodeLaunchOptions()
    {
        return VSCodeDebuggerTypes.Count == 0
            ? null
            : new VSCodeLaunchOptions(VSCodeDebuggerTypes);
    }

    private static ProjectPropertyOverride ParseProjectPropertyOption(string[] args, ref int index)
    {
        var token = args[index][2..];
        var equalsIndex = token.IndexOf('=');
        if(equalsIndex >= 0)
        {
            return new ProjectPropertyOverride(token[..equalsIndex], token[(equalsIndex + 1)..]);
        }
        if(index + 1 < args.Length && !args[index + 1].StartsWith("--", StringComparison.Ordinal))
        {
            ++index;
            return new ProjectPropertyOverride(token, args[index]);
        }
        return new ProjectPropertyOverride(token, null);
    }

    private static void AddProjectProperty(IDictionary<string, string?> properties, string value)
    {
        var equalsIndex = value.IndexOf('=');
        if(equalsIndex < 0)
        {
            properties[value] = null;
            return;
        }
        properties[value[..equalsIndex]] = value[(equalsIndex + 1)..];
    }

    private static void AddProjectProperty(IDictionary<string, string?> properties, ProjectPropertyOverride property)
    {
        properties[property.Name] = property.Value;
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

    private static void ValidatePlatformArchitecture(BuildPlatform platform, string architecture)
    {
        if(platform == BuildPlatform.MacOS && !IsMacOsSupportedArchitecture(architecture))
        {
            throw new ArgumentException($"Unsupported macOS architecture: {architecture}. LunaSDK supports macOS arm64 only.");
        }
    }

    private static bool IsMacOsSupportedArchitecture(string architecture)
    {
        return architecture.Equals("arm64", StringComparison.OrdinalIgnoreCase) ||
            architecture.Equals("aarch64", StringComparison.OrdinalIgnoreCase);
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

    private static IEnumerable<VSCodeDebuggerType> ParseVSCodeDebuggers(string value)
    {
        foreach(var part in value.Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            yield return VSCodeDebuggerType.Parse(part);
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
        public List<VSCodeDebuggerType> VSCodeDebuggerTypes { get; } = new();
        public Dictionary<string, string?> ProjectPropertyOverrides { get; } = new(StringComparer.OrdinalIgnoreCase);

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
                VSCodeDebuggerTypes = VSCodeDebuggerTypes.Distinct().ToArray(),
                ProjectPropertyOverrides = new Dictionary<string, string?>(ProjectPropertyOverrides, StringComparer.OrdinalIgnoreCase),
            };
        }
    }

    private sealed record ProjectPropertyOverride(string Name, string? Value);
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
