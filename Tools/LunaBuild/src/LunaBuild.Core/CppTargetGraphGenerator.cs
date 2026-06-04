namespace LunaBuild.Core;

public sealed class CppTargetGraphGenerator
{
    public BuildGraph Generate(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets,
        string targetName)
    {
        var targetMap = targets.ToDictionary(target => target.Name, StringComparer.OrdinalIgnoreCase);
        if(!targetMap.TryGetValue(targetName, out var target))
        {
            throw new ArgumentException($"Target does not exist: {targetName}");
        }

        var builder = new CppTargetGraphBuilder(workspace, options, targetMap);
        builder.AddTarget(target.Name);

        return new BuildGraph(
            Version: 1,
            Options: options,
            Nodes: builder.Nodes,
            Targets: new[] { BuildGraphIds.Target(target.Name) });
    }

    public BuildGraph GenerateAll(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets,
        IReadOnlySet<BuildTargetCategory>? categoryFilter = null)
    {
        var rootTargets = targets
            .Where(target => ShouldIncludeRootTarget(target, categoryFilter))
            .ToArray();
        var targetMap = targets.ToDictionary(target => target.Name, StringComparer.OrdinalIgnoreCase);
        var builder = new CppTargetGraphBuilder(workspace, options, targetMap);
        foreach(var target in rootTargets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            builder.AddTarget(target.Name);
        }

        var targetIds = rootTargets
            .Select(target => BuildGraphIds.Target(target.Name))
            .Order(StringComparer.OrdinalIgnoreCase)
            .ToArray();
        builder.AddNode(new BuildGraphNode(
            Id: BuildGraphIds.AllTargets,
            Kind: BuildGraphNodeKind.Phony,
            Path: null,
            Command: null,
            Dependencies: targetIds,
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>()));

        return new BuildGraph(
            Version: 1,
            Options: options,
            Nodes: builder.Nodes,
            Targets: new[] { BuildGraphIds.AllTargets });
    }

    private static bool ShouldIncludeRootTarget(
        BuildTargetDefinition target,
        IReadOnlySet<BuildTargetCategory>? categoryFilter)
    {
        return categoryFilter is { Count: > 0 }
            ? categoryFilter.Contains(target.Category)
            : BuildTargetCategoryPolicy.IsDefaultEnabled(target.Category);
    }

    private sealed class CppTargetGraphBuilder
    {
        private readonly BuildWorkspace _workspace;
        private readonly BuildOptions _options;
        private readonly IReadOnlyDictionary<string, BuildTargetDefinition> _targetMap;
        private readonly Dictionary<string, BuildGraphNode> _nodesById = new(StringComparer.Ordinal);
        private readonly HashSet<string> _visitedTargets = new(StringComparer.OrdinalIgnoreCase);
        private readonly HashSet<string> _visitingTargets = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, TargetBuildOutputs> _outputsByTarget = new(StringComparer.OrdinalIgnoreCase);

        public CppTargetGraphBuilder(
            BuildWorkspace workspace,
            BuildOptions options,
            IReadOnlyDictionary<string, BuildTargetDefinition> targetMap)
        {
            _workspace = workspace;
            _options = options;
            _targetMap = targetMap;
        }

        public IReadOnlyList<BuildGraphNode> Nodes => _nodesById.Values.ToArray();

        public TargetBuildOutputs AddTarget(string targetName)
        {
            if(_outputsByTarget.TryGetValue(targetName, out var existing))
            {
                return existing;
            }
            if(!_targetMap.TryGetValue(targetName, out var target))
            {
                throw new ArgumentException($"Target does not exist: {targetName}");
            }
            if(!_visitingTargets.Add(targetName))
            {
                throw new InvalidOperationException($"Circular target dependency detected at target {targetName}.");
            }
            if(_visitedTargets.Contains(targetName))
            {
                _visitingTargets.Remove(targetName);
                return _outputsByTarget[targetName];
            }

            var dependencyOutputs = target.Dependencies
                .Where(_targetMap.ContainsKey)
                .Select(AddTarget)
                .OrderBy(output => output.TargetId, StringComparer.OrdinalIgnoreCase)
                .ToArray();

            if(target.Kind == BuildTargetKind.External)
            {
                var externalOutputs = AddExternalTarget(target, dependencyOutputs);
                _outputsByTarget[target.Name] = externalOutputs;
                _visitedTargets.Add(targetName);
                _visitingTargets.Remove(targetName);
                return externalOutputs;
            }

            if(target.Kind == BuildTargetKind.DotNetProject)
            {
                var dotNetOutputs = AddDotNetTarget(target, dependencyOutputs);
                _outputsByTarget[target.Name] = dotNetOutputs;
                _visitedTargets.Add(targetName);
                _visitingTargets.Remove(targetName);
                return dotNetOutputs;
            }

            var sourceFiles = target.SourceFiles
                .Where(IsBuildSource)
                .Where(source => IsSourceEnabledForPlatform(_options.Platform, source))
                .ToArray();
            if(sourceFiles.Length == 0)
            {
                throw new InvalidOperationException($"Target {target.Name} has no C/C++ source files discovered from rules.");
            }

            var shaderHeaderIds = AddShaderNodes(target);
            var embeddedHeaderIds = AddEmbeddedHeaderNodes(target);
            var generatedHeaderIds = shaderHeaderIds.Concat(embeddedHeaderIds).ToArray();
            var objectIds = new List<string>();
            foreach(var sourceFile in sourceFiles)
            {
                var sourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(sourceFile));
                AddNode(new BuildGraphNode(
                    Id: sourceId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(sourceFile),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));

                var objectPath = GetObjectPath(_workspace, _options, target, sourceFile);
                var objectId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(objectPath));
                var depfilePath = Path.ChangeExtension(objectPath, ".d");
                var depfileId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(depfilePath));

                AddNode(new BuildGraphNode(
                    Id: depfileId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(depfilePath),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));

                var isResource = IsResourceSource(sourceFile);
                AddNode(new BuildGraphNode(
                    Id: objectId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(objectPath),
                    Command: isResource
                        ? BuildResourceCommandDescription(_workspace, _options, target, sourceFile, objectPath)
                        : BuildCompileCommandDescription(_workspace, _options, target, dependencyOutputs, sourceFile, objectPath, depfilePath),
                    Dependencies: new[] { sourceId }.Concat(generatedHeaderIds).ToArray(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: isResource ? Array.Empty<string>() : new[] { depfileId }));

                objectIds.Add(objectId);
            }

            var binaryPath = GetTargetBinaryPath(_workspace, _options, target);
            var binaryId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(binaryPath));
            var explicitLinkInputIds = target.LinkLibraryFiles
                .Select(path => AddFileReferenceNode(path))
                .ToArray();
            var sideOutputIds = GetLinkSideOutputs(_workspace, _options, binaryPath)
                .Select(path =>
                {
                    var id = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(path));
                    AddNode(new BuildGraphNode(
                        Id: id,
                        Kind: BuildGraphNodeKind.File,
                        Path: _workspace.ToRepositoryRelativePath(path),
                        Command: null,
                        Dependencies: Array.Empty<string>(),
                        OrderOnlyDependencies: Array.Empty<string>(),
                        Outputs: Array.Empty<string>(),
                        Depfiles: Array.Empty<string>()));
                    return id;
                })
                .ToArray();

            var dependencyTargetIds = dependencyOutputs.Select(output => output.TargetId).ToArray();
            var dependencyLinkInputIds = dependencyOutputs
                .SelectMany(output => output.LinkInputIds)
                .Distinct(StringComparer.Ordinal)
                .ToArray();
            var dependencyFrameworks = dependencyOutputs
                .SelectMany(output => output.Frameworks)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray();
            var linkDependencyIds = objectIds.Concat(dependencyTargetIds).Concat(dependencyLinkInputIds).Concat(explicitLinkInputIds).ToArray();
            AddNode(new BuildGraphNode(
                Id: binaryId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(binaryPath),
                Command: BuildLinkCommandDescription(_workspace, _options, target, dependencyFrameworks, binaryPath, objectIds.Concat(dependencyLinkInputIds).Concat(explicitLinkInputIds).ToArray()),
                Dependencies: linkDependencyIds,
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: sideOutputIds,
                Depfiles: Array.Empty<string>()));

            var runtimeFileIds = AddRuntimeFileNodes(target, dependencyOutputs.SelectMany(output => output.RuntimeFiles), binaryPath);
            var linkInputId = !target.Kind.ProducesNativeExecutable() && _options.Shared && _options.Platform == BuildPlatform.Windows
                ? BuildGraphIds.File(_workspace.ToRepositoryRelativePath(Path.ChangeExtension(binaryPath, ".lib")))
                : binaryId;
            var targetId = BuildGraphIds.Target(target.Name);
            AddNode(new BuildGraphNode(
                Id: targetId,
                Kind: BuildGraphNodeKind.Virtual,
                Path: _workspace.ToRepositoryRelativePath(target.Directory),
                Command: BuildTargetCommandDescription(_workspace, _options, target, sourceFiles.Length),
                Dependencies: new[] { binaryId }.Concat(runtimeFileIds).ToArray(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            var outputs = new TargetBuildOutputs(
                targetId,
                binaryId,
                new[] { linkInputId }
                    .Concat(dependencyLinkInputIds)
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.PublicIncludeDirectories
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.PublicDefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicDefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.PublicUndefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicUndefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.Frameworks
                    .Concat(dependencyFrameworks)
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.RuntimeFiles
                    .Concat(dependencyOutputs.SelectMany(output => output.RuntimeFiles))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray());
            _outputsByTarget[target.Name] = outputs;
            _visitedTargets.Add(targetName);
            _visitingTargets.Remove(targetName);
            return outputs;
        }

        private TargetBuildOutputs AddDotNetTarget(BuildTargetDefinition target, IReadOnlyList<TargetBuildOutputs> dependencyOutputs)
        {
            if(string.IsNullOrWhiteSpace(target.DotNetProjectFile) || string.IsNullOrWhiteSpace(target.DotNetOutputFile))
            {
                throw new InvalidOperationException($"Target {target.Name} is a .NET project target but does not declare DotNetProject(...).");
            }

            var sourceIds = target.SourceFiles
                .Concat(new[] { target.DotNetProjectFile })
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .Select(path =>
                {
                    var id = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(path));
                    AddNode(new BuildGraphNode(
                        Id: id,
                        Kind: BuildGraphNodeKind.File,
                        Path: _workspace.ToRepositoryRelativePath(path),
                        Command: null,
                        Dependencies: Array.Empty<string>(),
                        OrderOnlyDependencies: Array.Empty<string>(),
                        Outputs: Array.Empty<string>(),
                        Depfiles: Array.Empty<string>()));
                    return id;
                })
                .ToArray();

            var outputId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(target.DotNetOutputFile));
            var dependencyTargetIds = dependencyOutputs.Select(output => output.TargetId).ToArray();
            AddNode(new BuildGraphNode(
                Id: outputId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(target.DotNetOutputFile),
                Command: BuildDotNetBuildCommandDescription(_workspace, _options, target),
                Dependencies: sourceIds,
                OrderOnlyDependencies: dependencyTargetIds,
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            var targetId = BuildGraphIds.Target(target.Name);
            AddNode(new BuildGraphNode(
                Id: targetId,
                Kind: BuildGraphNodeKind.Virtual,
                Path: _workspace.ToRepositoryRelativePath(target.Directory),
                Command: BuildDotNetTargetCommandDescription(_workspace, _options, target, target.SourceFiles.Count),
                Dependencies: new[] { outputId }.Concat(dependencyTargetIds).ToArray(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            return new TargetBuildOutputs(
                targetId,
                outputId,
                Array.Empty<string>(),
                target.PublicIncludeDirectories
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.PublicDefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicDefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.PublicUndefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicUndefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.Frameworks
                    .Concat(dependencyOutputs.SelectMany(output => output.Frameworks))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.RuntimeFiles
                    .Concat(dependencyOutputs.SelectMany(output => output.RuntimeFiles))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray());
        }

        private TargetBuildOutputs AddExternalTarget(BuildTargetDefinition target, IReadOnlyList<TargetBuildOutputs> dependencyOutputs)
        {
            foreach(var includeDirectory in target.PublicIncludeDirectories)
            {
                if(!Directory.Exists(includeDirectory))
                {
                    throw new DirectoryNotFoundException($"External target `{target.Name}` references missing include directory: {includeDirectory}");
                }
            }

            var requiredIds = target.RequiredFiles
                .Concat(target.LinkLibraryFiles)
                .Concat(target.RuntimeFiles)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .Select(path => AddFileReferenceNode(path, target.Name))
                .ToArray();
            var dependencyTargetIds = dependencyOutputs.Select(output => output.TargetId).ToArray();
            var targetId = BuildGraphIds.Target(target.Name);
            AddNode(new BuildGraphNode(
                Id: targetId,
                Kind: BuildGraphNodeKind.Virtual,
                Path: _workspace.ToRepositoryRelativePath(target.Directory),
                Command: BuildExternalTargetCommandDescription(_workspace, _options, target),
                Dependencies: requiredIds.Concat(dependencyTargetIds).ToArray(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            return new TargetBuildOutputs(
                targetId,
                string.Empty,
                target.LinkLibraryFiles
                    .Select(path => AddFileReferenceNode(path, target.Name))
                    .Concat(dependencyOutputs.SelectMany(output => output.LinkInputIds))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.PublicIncludeDirectories
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.PublicDefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicDefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.PublicUndefines
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicUndefines))
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                target.Frameworks
                    .Concat(dependencyOutputs.SelectMany(output => output.Frameworks))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                target.RuntimeFiles
                    .Concat(dependencyOutputs.SelectMany(output => output.RuntimeFiles))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray());
        }

        private IReadOnlyList<string> AddEmbeddedHeaderNodes(BuildTargetDefinition target)
        {
            var headerIds = new List<string>();
            foreach(var header in target.EmbeddedHeaders)
            {
                var sourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(header.SourceFile));
                AddNode(new BuildGraphNode(
                    Id: sourceId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(header.SourceFile),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));

                var headerPath = GetEmbeddedHeaderPath(_workspace, _options, target, header.HeaderFile);
                var headerId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(headerPath));
                AddNode(new BuildGraphNode(
                    Id: headerId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(headerPath),
                    Command: BuildEmbeddedHeaderCommandDescription(_workspace, target, header, headerPath),
                    Dependencies: new[] { sourceId },
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));
                headerIds.Add(headerId);
            }
            return headerIds;
        }

        private IReadOnlyList<string> AddRuntimeFileNodes(BuildTargetDefinition target, IEnumerable<string> dependencyRuntimeFiles, string binaryPath)
        {
            var outputIds = new List<string>();
            var runtimeFiles = target.RuntimeFiles
                .Concat(dependencyRuntimeFiles)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray();
            if(runtimeFiles.Length == 0)
            {
                return outputIds;
            }

            var runtimeDirectory = Path.GetDirectoryName(binaryPath)!;
            foreach(var runtimeFile in runtimeFiles)
            {
                var sourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(runtimeFile));
                AddNode(new BuildGraphNode(
                    Id: sourceId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(runtimeFile),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));

                var outputPath = Path.Combine(runtimeDirectory, Path.GetFileName(runtimeFile));
                var outputId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(outputPath));
                AddNode(new BuildGraphNode(
                    Id: outputId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(outputPath),
                    Command: BuildCopyCommandDescription(_workspace, target, runtimeFile, outputPath),
                    Dependencies: new[] { sourceId },
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));
                outputIds.Add(outputId);
            }
            return outputIds;
        }

        private string AddFileReferenceNode(string path, string? externalTargetName = null)
        {
            if(externalTargetName is not null && !File.Exists(path))
            {
                throw new FileNotFoundException($"External target `{externalTargetName}` references missing file: {path}", path);
            }

            var id = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(path));
            AddNode(new BuildGraphNode(
                Id: id,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(path),
                Command: null,
                Dependencies: Array.Empty<string>(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));
            return id;
        }

        private IReadOnlyList<string> AddShaderNodes(BuildTargetDefinition target)
        {
            var shaderHeaderIds = new List<string>();
            foreach(var shader in target.Shaders)
            {
                if(!IsShaderEnabledForPlatform(_options.Platform, shader.SourceFile))
                {
                    continue;
                }

                var sourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(shader.SourceFile));
                AddNode(new BuildGraphNode(
                    Id: sourceId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(shader.SourceFile),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));

                var headerPath = GetShaderHeaderPath(_workspace, _options, target, shader.SourceFile);
                var headerId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(headerPath));
                AddNode(new BuildGraphNode(
                    Id: headerId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(headerPath),
                    Command: BuildShaderCommandDescription(_workspace, _options, target, shader, headerPath),
                    Dependencies: new[] { sourceId },
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));
                shaderHeaderIds.Add(headerId);
            }
            return shaderHeaderIds;
        }

        public void AddNode(BuildGraphNode node)
        {
            if(!_nodesById.TryAdd(node.Id, node))
            {
                return;
            }
        }
    }

    private sealed record TargetBuildOutputs(
        string TargetId,
        string BinaryId,
        IReadOnlyList<string> LinkInputIds,
        IReadOnlyList<string> PublicIncludeDirectories,
        IReadOnlyList<string> PublicDefines,
        IReadOnlyList<string> PublicUndefines,
        IReadOnlyList<string> Frameworks,
        IReadOnlyList<string> RuntimeFiles);

    private static bool IsBuildSource(string path)
    {
        var extension = Path.GetExtension(path);
        return extension.Equals(".c", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cpp", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cc", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cxx", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".m", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".mm", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".s", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".S", StringComparison.Ordinal) ||
            extension.Equals(".rc", StringComparison.OrdinalIgnoreCase);
    }

    private static bool IsShaderEnabledForPlatform(BuildPlatform platform, string sourceFile)
    {
        return IsSourceEnabledForPlatform(platform, sourceFile);
    }

    private static bool IsSourceEnabledForPlatform(BuildPlatform platform, string sourceFile)
    {
        var normalized = sourceFile.Replace('\\', '/');
        if(platform == BuildPlatform.Windows)
        {
            return !normalized.Contains("/Source/Platform/POSIX/", StringComparison.OrdinalIgnoreCase);
        }
        return !normalized.Contains("/Source/Platform/Windows/", StringComparison.OrdinalIgnoreCase);
    }

    private static string GetObjectPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target, string sourceFile)
    {
        var relativeSource = workspace.ToRepositoryRelativePath(sourceFile)
            .Replace('/', '_')
            .Replace(':', '_');
        var extension = IsResourceSource(sourceFile)
            ? ".res"
            : options.Platform == BuildPlatform.Windows ? ".obj" : ".o";
        return Path.Combine(GetTargetObjectDirectory(workspace, options, target), $"{relativeSource}{extension}");
    }

    private static string GetTargetObjectDirectory(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(
            BuildOutputLayout.ConfigurationDirectory(workspace, options),
            "obj",
            target.Name);
    }

    private static string GetTargetBinaryPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        var fileName = IsAndroidNativeActivityLibrary(options, target)
            ? $"lib{target.Name}.so"
            : target.Kind.ProducesNativeExecutable()
            ? $"{target.Name}{ExecutableExtension(options.Platform)}"
            : $"Luna{target.Name}{(options.Shared ? SharedLibraryExtension(options.Platform) : StaticLibraryExtension(options.Platform))}";
        return Path.Combine(
            BuildOutputLayout.ConfigurationDirectory(workspace, options),
            "bin",
            fileName);
    }

    private static string GetGeneratedShaderHeaderDirectory(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(
            BuildOutputLayout.ConfigurationDirectory(workspace, options),
            "generated",
            target.Name,
            "shaders");
    }

    private static string GetShaderHeaderPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target, string sourceFile)
    {
        return Path.Combine(GetGeneratedShaderHeaderDirectory(workspace, options, target), Path.GetFileNameWithoutExtension(sourceFile) + ".hpp");
    }

    private static string GetEmbeddedHeaderPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target, string headerFile)
    {
        return Path.Combine(
            BuildOutputLayout.ConfigurationDirectory(workspace, options),
            "generated",
            target.Name,
            "embedded",
            headerFile);
    }

    private static IReadOnlyList<string> GetLinkSideOutputs(BuildWorkspace workspace, BuildOptions options, string binaryPath)
    {
        if(options.Shared && options.Platform == BuildPlatform.Windows && !binaryPath.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
        {
            return new[] { Path.ChangeExtension(binaryPath, ".lib") };
        }
        return Array.Empty<string>();
    }

    private static string ExecutableExtension(BuildPlatform platform)
    {
        return platform == BuildPlatform.Windows ? ".exe" : string.Empty;
    }

    private static string SharedLibraryExtension(BuildPlatform platform)
    {
        return platform switch
        {
            BuildPlatform.Windows => ".dll",
            BuildPlatform.MacOS or BuildPlatform.IOS => ".dylib",
            _ => ".so",
        };
    }

    private static string StaticLibraryExtension(BuildPlatform platform)
    {
        return platform switch
        {
            BuildPlatform.Windows => ".lib",
            _ => ".a",
        };
    }

    private static string BuildCompileCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        IReadOnlyList<TargetBuildOutputs> dependencyOutputs,
        string sourceFile,
        string objectPath,
        string depfilePath)
    {
        var lines = new List<string>
        {
            "kind=cpp.compile",
            $"target={target.Name}",
            $"source={workspace.ToRepositoryRelativePath(sourceFile)}",
            $"object={workspace.ToRepositoryRelativePath(objectPath)}",
            $"depfile={workspace.ToRepositoryRelativePath(depfilePath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"language={SourceLanguage(sourceFile)}",
            "exceptions=none",
            "include=Modules",
            "define=LUNA_MANUAL_CONFIG_DEBUG_LEVEL",
            $"define=LUNA_DEBUG_LEVEL={DebugLevel(options.Mode)}",
            options.Shared ? "define=LUNA_BUILD_SHARED_LIB" : "linkage=static",
        };
        lines.AddRange(options.GlobalDefines.Select(define => $"define={define}"));
        if(target.MsvcRuntimeLibrary is not null)
        {
            lines.Add($"runtime={target.MsvcRuntimeLibrary}");
        }
        lines.AddRange(target.Defines.Select(define => $"define={define}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicDefines)
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal)
            .Select(define => $"define={define}"));
        lines.AddRange(options.GlobalUndefines.Select(undefine => $"undefine={undefine}"));
        lines.AddRange(target.Undefines.Select(undefine => $"undefine={undefine}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicUndefines)
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal)
            .Select(undefine => $"undefine={undefine}"));
        if(target.Shaders.Count > 0)
        {
            lines.Add($"include={workspace.ToRepositoryRelativePath(GetGeneratedShaderHeaderDirectory(workspace, options, target))}");
        }
        if(target.EmbeddedHeaders.Count > 0)
        {
            lines.Add($"include={workspace.ToRepositoryRelativePath(Path.GetDirectoryName(GetEmbeddedHeaderPath(workspace, options, target, target.EmbeddedHeaders[0].HeaderFile))!)}");
        }
        lines.AddRange(target.IncludeDirectories.Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicIncludeDirectories)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase)
            .Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        return string.Join('\n', lines);
    }

    private static string BuildShaderCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        BuildShaderDefinition shader,
        string headerPath)
    {
        var lines = new List<string>
        {
            "kind=cppsl.shader",
            $"target={target.Name}",
            $"source={workspace.ToRepositoryRelativePath(shader.SourceFile)}",
            $"header={workspace.ToRepositoryRelativePath(headerPath)}",
            $"intermediate_dir={workspace.ToRepositoryRelativePath(Path.Combine(Path.GetDirectoryName(headerPath)!, ".cppsl", Path.GetFileNameWithoutExtension(shader.SourceFile)))}",
            $"stage={shader.Stage}",
            $"entry={shader.EntryPoint}",
            $"format={ShaderDataFormat(options.RhiApi)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
        };
        if(options.Platform == BuildPlatform.IOS)
        {
            lines.Add($"apple_sdk={BuildOutputLayout.AppleSdkName(options)}");
            lines.Add($"ios_deployment_target={IOSDeploymentTarget(options)}");
        }
        return string.Join('\n', lines);
    }

    private static string BuildCopyCommandDescription(
        BuildWorkspace workspace,
        BuildTargetDefinition target,
        string sourcePath,
        string outputPath)
    {
        return string.Join('\n',
            "kind=file.copy",
            $"target={target.Name}",
            $"source={workspace.ToRepositoryRelativePath(sourcePath)}",
            $"output={workspace.ToRepositoryRelativePath(outputPath)}");
    }

    private static string BuildEmbeddedHeaderCommandDescription(
        BuildWorkspace workspace,
        BuildTargetDefinition target,
        BuildEmbeddedHeaderDefinition header,
        string headerPath)
    {
        return string.Join('\n',
            "kind=bin.embed_header",
            $"target={target.Name}",
            $"source={workspace.ToRepositoryRelativePath(header.SourceFile)}",
            $"output={workspace.ToRepositoryRelativePath(headerPath)}",
            $"data_symbol={header.DataSymbol}",
            $"size_symbol={header.SizeSymbol}");
    }

    private static string ShaderDataFormat(RhiApi rhiApi)
    {
        return rhiApi switch
        {
            RhiApi.D3D12 => "dxil",
            RhiApi.Vulkan => "spir_v",
            RhiApi.Metal => "msl",
            _ => throw new ArgumentOutOfRangeException(nameof(rhiApi), rhiApi, null),
        };
    }

    private static string IOSDeploymentTarget(BuildOptions options)
    {
        return string.IsNullOrWhiteSpace(options.Apple.IOSDeploymentTarget)
            ? "13.0"
            : options.Apple.IOSDeploymentTarget;
    }

    private static string SourceLanguage(string path)
    {
        return Path.GetExtension(path) switch
        {
            var extension when extension.Equals(".c", StringComparison.OrdinalIgnoreCase) => "c",
            var extension when extension.Equals(".m", StringComparison.OrdinalIgnoreCase) => "objective-c",
            var extension when extension.Equals(".mm", StringComparison.OrdinalIgnoreCase) => "objective-c++20",
            var extension when extension.Equals(".s", StringComparison.OrdinalIgnoreCase) => "assembler",
            var extension when extension.Equals(".S", StringComparison.Ordinal) => "assembler-with-cpp",
            _ => "c++20",
        };
    }

    private static bool IsResourceSource(string path)
    {
        return Path.GetExtension(path).Equals(".rc", StringComparison.OrdinalIgnoreCase);
    }

    private static string BuildResourceCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        string sourceFile,
        string outputPath)
    {
        return string.Join('\n',
            "kind=rc.compile",
            $"target={target.Name}",
            $"source={workspace.ToRepositoryRelativePath(sourceFile)}",
            $"output={workspace.ToRepositoryRelativePath(outputPath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"include={workspace.ToRepositoryRelativePath(Path.GetDirectoryName(sourceFile)!)}");
    }

    private static string BuildLinkCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        IReadOnlyList<string> dependencyFrameworks,
        string binaryPath,
        IReadOnlyList<string> inputIds)
    {
        var actionKind = target.Kind.ProducesNativeExecutable()
            ? IsAndroidNativeActivityLibrary(options, target) ? "cpp.link.shared" : "cpp.link.executable"
            : options.Shared ? "cpp.link.shared" : "cpp.link.static";
        var lines = new List<string>
        {
            $"kind={actionKind}",
            $"target={target.Name}",
            $"output={workspace.ToRepositoryRelativePath(binaryPath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
        };
        if(target.Kind == BuildTargetKind.Application)
        {
            lines.Add("application=true");
        }
        lines.AddRange(inputIds.OrderBy(id => id, StringComparer.OrdinalIgnoreCase).Select(id => $"input={id}"));
        lines.AddRange(target.SystemLibraries.OrderBy(id => id, StringComparer.OrdinalIgnoreCase).Select(library => $"library={library}"));
        lines.AddRange(target.Frameworks
                .Concat(dependencyFrameworks)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .Select(framework => $"framework={framework}"));
        return string.Join('\n', lines);
    }

    private static bool IsAndroidNativeActivityLibrary(BuildOptions options, BuildTargetDefinition target)
    {
        return options.Platform == BuildPlatform.Android && target.Kind == BuildTargetKind.Application;
    }

    private static string BuildTargetCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        int sourceCount)
    {
        return string.Join('\n',
            "kind=target.cpp",
            $"name={target.Name}",
            $"rules={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"directory={workspace.ToRepositoryRelativePath(target.Directory)}",
            $"source_count={sourceCount}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"shared={options.Shared}",
            $"rhi={options.RhiApi}",
            $"runtime_file_count={target.RuntimeFiles.Count}",
            $"embedded_header_count={target.EmbeddedHeaders.Count}",
            $"framework_count={target.Frameworks.Count}");
    }

    private static string BuildExternalTargetCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target)
    {
        return string.Join('\n',
            "kind=target.external",
            $"name={target.Name}",
            $"rules={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"directory={workspace.ToRepositoryRelativePath(target.Directory)}",
            $"include_count={target.PublicIncludeDirectories.Count}",
            $"library_count={target.LinkLibraryFiles.Count}",
            $"framework_count={target.Frameworks.Count}",
            $"runtime_file_count={target.RuntimeFiles.Count}",
            $"required_file_count={target.RequiredFiles.Count}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}");
    }

    private static string BuildDotNetBuildCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target)
    {
        return string.Join('\n',
            "kind=dotnet.build",
            $"name={target.Name}",
            $"project={workspace.ToRepositoryRelativePath(target.DotNetProjectFile!)}",
            $"output={workspace.ToRepositoryRelativePath(target.DotNetOutputFile!)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}");
    }

    private static string BuildDotNetTargetCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        int sourceCount)
    {
        return string.Join('\n',
            "kind=target.dotnet",
            $"name={target.Name}",
            $"rules={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"directory={workspace.ToRepositoryRelativePath(target.Directory)}",
            $"project={workspace.ToRepositoryRelativePath(target.DotNetProjectFile!)}",
            $"source_count={sourceCount}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}");
    }

    private static int DebugLevel(BuildMode mode)
    {
        return mode switch
        {
            BuildMode.Debug => 2,
            BuildMode.Profile => 1,
            _ => 0,
        };
    }
}
