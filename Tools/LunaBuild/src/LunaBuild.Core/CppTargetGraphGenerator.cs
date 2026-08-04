using System.Security.Cryptography;
using System.Text;

namespace LunaBuild.Core;

public sealed class CppTargetGraphGenerator
{
    public BuildGraph Generate(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets,
        string targetName)
    {
        var targetMap = targets.ToDictionary(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase);
        var target = ResolveTarget(targets, targetName);
        if(target is null)
        {
            throw new ArgumentException($"Target does not exist: {targetName}");
        }

        var builder = new CppTargetGraphBuilder(workspace, options, targetMap);
        builder.AddTarget(target.QualifiedName);

        return new BuildGraph(
            Version: 2,
            Options: options,
            Nodes: builder.Nodes,
            Targets: new[] { BuildGraphIds.Target(target.QualifiedName) })
            .AddMetadata(targets)
            .MergeMetadata(builder.AdditionalProjects, builder.AdditionalConfigurations);
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
        var targetMap = targets.ToDictionary(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase);
        var builder = new CppTargetGraphBuilder(workspace, options, targetMap);
        foreach(var target in rootTargets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            builder.AddTarget(target.QualifiedName);
        }

        var targetIds = rootTargets
            .Select(target => BuildGraphIds.Target(target.QualifiedName))
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
            Version: 2,
            Options: options,
            Nodes: builder.Nodes,
            Targets: new[] { BuildGraphIds.AllTargets })
            .AddMetadata(targets)
            .MergeMetadata(builder.AdditionalProjects, builder.AdditionalConfigurations);
    }

    private static bool ShouldIncludeRootTarget(
        BuildTargetDefinition target,
        IReadOnlySet<BuildTargetCategory>? categoryFilter)
    {
        return target.IsHostProject && (categoryFilter is { Count: > 0 }
            ? categoryFilter.Contains(target.Category)
            : BuildTargetCategoryPolicy.IsDefaultEnabled(target.Category));
    }

    private static BuildTargetDefinition? ResolveTarget(IReadOnlyList<BuildTargetDefinition> targets, string name)
    {
        return name.Contains('.', StringComparison.Ordinal)
            ? targets.FirstOrDefault(target => target.QualifiedName.Equals(name, StringComparison.OrdinalIgnoreCase))
            : targets.FirstOrDefault(target => target.IsHostProject && target.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
    }

    private sealed class CppTargetGraphBuilder
    {
        private readonly BuildWorkspace _workspace;
        private BuildOptions _options;
        private readonly IReadOnlyDictionary<string, BuildTargetDefinition> _targetMap;
        private readonly Dictionary<string, BuildGraphNode> _nodesById = new(StringComparer.Ordinal);
        private readonly HashSet<string> _visitedTargets = new(StringComparer.OrdinalIgnoreCase);
        private readonly HashSet<string> _visitingTargets = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, TargetBuildOutputs> _outputsByTarget = new(StringComparer.OrdinalIgnoreCase);
        private readonly List<BuildGraphProject> _additionalProjects = new();
        private readonly List<BuildGraphConfiguration> _additionalConfigurations = new();
        private string _activeProjectName = string.Empty;
        private string _activeConfigurationId = string.Empty;

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

        public IReadOnlyList<BuildGraphProject> AdditionalProjects => _additionalProjects;

        public IReadOnlyList<BuildGraphConfiguration> AdditionalConfigurations => _additionalConfigurations;

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

            var previousOptions = _options;
            var previousProjectName = _activeProjectName;
            var previousConfigurationId = _activeConfigurationId;
            _options = target.Options;
            _activeProjectName = target.ProjectName;
            _activeConfigurationId = target.ConfigurationId;
            try
            {

            var dependencyOutputs = target.Dependencies
                .Select(AddTarget)
                .OrderBy(output => output.TargetId, StringComparer.OrdinalIgnoreCase)
                .ToArray();
            ValidateLinkCompatibility(target, dependencyOutputs);

            if(target.Kind is BuildTargetKind.External or BuildTargetKind.HeaderOnly)
            {
                var externalOutputs = AddExternalTarget(target, dependencyOutputs);
                _outputsByTarget[target.QualifiedName] = externalOutputs;
                _visitedTargets.Add(targetName);
                _visitingTargets.Remove(targetName);
                return externalOutputs;
            }

            if(target.Kind == BuildTargetKind.DotNetProject)
            {
                var dotNetOutputs = AddDotNetTarget(target, dependencyOutputs);
                _outputsByTarget[target.QualifiedName] = dotNetOutputs;
                _visitedTargets.Add(targetName);
                _visitingTargets.Remove(targetName);
                return dotNetOutputs;
            }

            var discoveredSourceFiles = target.SourceFiles
                .Where(IsBuildSource)
                .Where(source => IsSourceEnabledForPlatform(_options.Platform, source))
                .ToArray();
            if(discoveredSourceFiles.Length == 0)
            {
                throw new InvalidOperationException($"Target {target.Name} has no C/C++ source files discovered from rules.");
            }

            var metaToolOutputs = AddLunaMetaToolTargetForMeta(target);
            var metaOutputs = AddMetaNodes(target, dependencyOutputs, metaToolOutputs);
            var sourceFiles = discoveredSourceFiles
                .Concat(metaOutputs.GeneratedSourceFile is null ? Array.Empty<string>() : new[] { metaOutputs.GeneratedSourceFile })
                .ToArray();
            var dependencyMetaIds = dependencyOutputs
                .SelectMany(output => output.PublicMetaDependencyIds)
                .Distinct(StringComparer.Ordinal)
                .ToArray();
            var cppslToolOutputs = AddCppslToolTargetsForShaders(target);
            var shaderHeaderIds = AddShaderNodes(target, cppslToolOutputs);
            var embeddedHeaderIds = AddEmbeddedHeaderNodes(target);
            var generatedHeaderIds = shaderHeaderIds
                .Concat(embeddedHeaderIds)
                .Concat(metaOutputs.GeneratedHeaderIds)
                .Concat(metaOutputs.StampId is null ? Array.Empty<string>() : new[] { metaOutputs.StampId })
                .Concat(dependencyMetaIds)
                .ToArray();
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
            var linkInputId = target.Kind != BuildTargetKind.Executable && _options.Shared && _options.Platform == BuildPlatform.Windows
                ? BuildGraphIds.File(_workspace.ToRepositoryRelativePath(Path.ChangeExtension(binaryPath, ".lib")))
                : binaryId;
            var targetId = BuildGraphIds.Target(target.QualifiedName);
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
                target.Options.GlobalIncludeDirectories
                    .Concat(target.PublicIncludeDirectories)
                    .Concat(metaOutputs.PublicIncludeDirectories)
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                (metaOutputs.StampId is null ? Array.Empty<string>() : new[] { metaOutputs.StampId })
                    .Concat(dependencyMetaIds)
                    .Distinct(StringComparer.Ordinal)
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
                (target.Kind != BuildTargetKind.Executable && _options.Shared
                    ? new[] { binaryPath }
                    : Array.Empty<string>())
                    .Concat(target.RuntimeFiles)
                    .Concat(dependencyOutputs.SelectMany(output => output.RuntimeFiles))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray());
            _outputsByTarget[target.QualifiedName] = outputs;
            _visitedTargets.Add(targetName);
            _visitingTargets.Remove(targetName);
            return outputs;
            }
            finally
            {
                _options = previousOptions;
                _activeProjectName = previousProjectName;
                _activeConfigurationId = previousConfigurationId;
            }
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

            var dotNetRoot = Path.Combine(GetTargetConfigurationDirectory(target, _options), "dotnet", target.Name);
            var artifactsDirectory = Path.Combine(dotNetRoot, "artifacts");
            var dotNetProjectName = Path.GetFileNameWithoutExtension(target.DotNetProjectFile);
            var outputPath = Path.Combine(
                artifactsDirectory,
                "bin",
                dotNetProjectName,
                _options.Mode.ToString().ToLowerInvariant(),
                Path.GetFileName(target.DotNetOutputFile));
            var outputId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(outputPath));
            var dependencyTargetIds = dependencyOutputs.Select(output => output.TargetId).ToArray();
            AddNode(new BuildGraphNode(
                Id: outputId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(outputPath),
                Command: BuildDotNetBuildCommandDescription(
                    _workspace,
                    _options,
                    target,
                    outputPath,
                    artifactsDirectory),
                Dependencies: sourceIds,
                OrderOnlyDependencies: dependencyTargetIds,
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            var targetId = BuildGraphIds.Target(target.QualifiedName);
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
                target.Options.GlobalIncludeDirectories
                    .Concat(target.PublicIncludeDirectories)
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                dependencyOutputs
                    .SelectMany(output => output.PublicMetaDependencyIds)
                    .Distinct(StringComparer.Ordinal)
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
            var targetId = BuildGraphIds.Target(target.QualifiedName);
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
                target.Options.GlobalIncludeDirectories
                    .Concat(target.PublicIncludeDirectories)
                    .Concat(dependencyOutputs.SelectMany(output => output.PublicIncludeDirectories))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                dependencyOutputs
                    .SelectMany(output => output.PublicMetaDependencyIds)
                    .Distinct(StringComparer.Ordinal)
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

        private TargetBuildOutputs? AddLunaMetaToolTargetForMeta(BuildTargetDefinition target)
        {
            if(target.MetaHeaderFiles.Count == 0)
            {
                return null;
            }
            var configuration = RequireActionConfiguration(target, "luna.meta");
            var localToolTarget = RequireBinding(configuration.Targets, "tool", configuration.Name);
            var metaTargetName = QualifyTarget(configuration.ProviderProjectName, localToolTarget);
            if(target.QualifiedName.Equals(metaTargetName, StringComparison.OrdinalIgnoreCase))
            {
                return null;
            }
            if(IsCurrentBuildHostBuild())
            {
                if(!_targetMap.ContainsKey(metaTargetName))
                {
                    throw new InvalidOperationException(
                        $"Target `{target.QualifiedName}` uses MetaHeaders(...), but action configuration `{configuration.Name}` references unavailable target `{metaTargetName}`.");
                }
                return AddTarget(metaTargetName);
            }
            return AddHostLunaMetaToolTargetForMeta(target);
        }

        private CppslToolBuildOutputs? AddCppslToolTargetsForShaders(BuildTargetDefinition target)
        {
            if(!target.Shaders.Any(shader => IsShaderEnabledForPlatform(_options.Platform, shader.SourceFile)))
            {
                return null;
            }
            var configuration = RequireActionConfiguration(target, "cppsl.shader");
            var compilerTarget = RequireBinding(configuration.Targets, "compiler", configuration.Name);
            var nativeExtractorTarget = RequireBinding(configuration.Targets, "native_extractor", configuration.Name);
            var cppslTargetName = QualifyTarget(configuration.ProviderProjectName, compilerTarget);
            var nativeExtractorTargetName = QualifyTarget(configuration.ProviderProjectName, nativeExtractorTarget);
            if(target.QualifiedName.Equals(cppslTargetName, StringComparison.OrdinalIgnoreCase) ||
                target.QualifiedName.Equals(nativeExtractorTargetName, StringComparison.OrdinalIgnoreCase))
            {
                return null;
            }

            if(IsCurrentBuildHostBuild())
            {
                if(!_targetMap.ContainsKey(cppslTargetName) ||
                    !_targetMap.ContainsKey(nativeExtractorTargetName))
                {
                    throw new InvalidOperationException(
                        $"Target {target.Name} uses Shader(...), but host CPPSL tool targets are not available.");
                }

                var cppslOutputs = AddTarget(cppslTargetName);
                var nativeExtractorOutputs = AddTarget(nativeExtractorTargetName);
                return new CppslToolBuildOutputs(cppslOutputs.BinaryId, nativeExtractorOutputs.BinaryId);
            }

            return AddHostCppslToolTargetsForShaders(target);
        }

        private CppslToolBuildOutputs AddHostCppslToolTargetsForShaders(BuildTargetDefinition target)
        {
            var configuration = RequireActionConfiguration(target, "cppsl.shader");
            var compilerTarget = RequireBinding(configuration.Targets, "compiler", configuration.Name);
            var nativeExtractorTarget = RequireBinding(configuration.Targets, "native_extractor", configuration.Name);
            var hostOptions = BuildOptions.HostDefault() with
            {
                Properties = configuration.ProviderProperties,
            };
            var compilerQualifiedName = QualifyTarget(configuration.ProviderProjectName, compilerTarget);
            var nativeExtractorQualifiedName = QualifyTarget(configuration.ProviderProjectName, nativeExtractorTarget);
            var hostTargets = DiscoverHostToolTargets(configuration, hostOptions);
            if(!hostTargets.Any(hostTarget => hostTarget.Name.Equals(compilerTarget, StringComparison.OrdinalIgnoreCase)) ||
                !hostTargets.Any(hostTarget => hostTarget.Name.Equals(nativeExtractorTarget, StringComparison.OrdinalIgnoreCase)))
            {
                throw new InvalidOperationException(
                    $"Target {target.Name} uses Shader(...), but host CPPSL tool targets are not available for {hostOptions.Platform} {hostOptions.Architecture}.");
            }

            var hostGraph = new CppTargetGraphGenerator().Generate(_workspace, hostOptions, hostTargets, compilerQualifiedName);
            MergeHostGraph(hostGraph);

            return new CppslToolBuildOutputs(
                FindTargetBinaryId(hostGraph, compilerQualifiedName, OperatingSystem.IsWindows() ? "cppslc.exe" : "cppslc"),
                FindTargetBinaryId(hostGraph, nativeExtractorQualifiedName, OperatingSystem.IsWindows() ? "cppsl-native-extractor.exe" : "cppsl-native-extractor"));
        }

        private TargetBuildOutputs? AddHostLunaMetaToolTargetForMeta(BuildTargetDefinition target)
        {
            var configuration = RequireActionConfiguration(target, "luna.meta");
            var toolTarget = RequireBinding(configuration.Targets, "tool", configuration.Name);
            var qualifiedToolTarget = QualifyTarget(configuration.ProviderProjectName, toolTarget);
            var hostOptions = BuildOptions.HostDefault() with
            {
                Properties = configuration.ProviderProperties,
            };
            var hostTargets = DiscoverHostToolTargets(configuration, hostOptions);
            if(!hostTargets.Any(hostTarget => hostTarget.Name.Equals(toolTarget, StringComparison.OrdinalIgnoreCase)))
            {
                throw new InvalidOperationException(
                    $"Target `{target.QualifiedName}` uses MetaHeaders(...), but host tool target `{qualifiedToolTarget}` is unavailable for {hostOptions.Platform} {hostOptions.Architecture}.");
            }

            var hostGraph = new CppTargetGraphGenerator().Generate(_workspace, hostOptions, hostTargets, qualifiedToolTarget);
            MergeHostGraph(hostGraph);

            var targetId = BuildGraphIds.Target(qualifiedToolTarget);
            var binaryId = FindTargetBinaryId(hostGraph, qualifiedToolTarget, Path.GetFileNameWithoutExtension(toolTarget) + (OperatingSystem.IsWindows() ? ".exe" : string.Empty));
            return new TargetBuildOutputs(
                targetId,
                binaryId,
                Array.Empty<string>(),
                Array.Empty<string>(),
                Array.Empty<string>(),
                Array.Empty<string>(),
                Array.Empty<string>(),
                Array.Empty<string>(),
                Array.Empty<string>());
        }

        private IReadOnlyList<BuildTargetDefinition> DiscoverHostToolTargets(
            BuildActionConfigurationDefinition configuration,
            BuildOptions hostOptions)
        {
            var workspace = new BuildWorkspace(
                configuration.ProviderProjectRootDirectory,
                configuration.ProviderProjectBuildDirectory,
                _workspace.RunnerProjectPath);
            var provider = new ProjectTargetRulesProvider();
            var projectRules = provider.GetProjectRules(workspace);
            if(projectRules.Count != 1)
            {
                throw new InvalidOperationException(
                    $"Project `{configuration.ProviderProjectName}` must provide exactly one ProjectRules type to configure host tools.");
            }
            var configuredOptions = projectRules[0].ConfigureBuildOptions(workspace, hostOptions);
            var configurationId = ProjectConfigurationIdentity.Create(configuredOptions);
            return new TargetDiscovery(new ITargetRulesProvider[] { provider })
                .DiscoverTargets(workspace, configuredOptions)
                .Select(hostTarget => hostTarget with
                {
                    ProjectName = configuration.ProviderProjectName,
                    QualifiedName = configuration.ProviderProjectName + "." + hostTarget.Name,
                    ProjectRootDirectory = configuration.ProviderProjectRootDirectory,
                    ProjectBuildDirectory = configuration.ProviderProjectBuildDirectory,
                    ConfigurationId = configurationId,
                    Options = configuredOptions,
                    IsHostProject = false,
                    Dependencies = hostTarget.Dependencies.Select(dependency => configuration.ProviderProjectName + "." + dependency).ToArray(),
                })
                .ToArray();
        }

        private void MergeHostGraph(BuildGraph graph)
        {
            foreach(var node in graph.Nodes)
            {
                AddNode(node);
            }
            _additionalProjects.AddRange(graph.Projects);
            _additionalConfigurations.AddRange(graph.Configurations);
        }

        private bool IsCurrentBuildHostBuild()
        {
            var hostOptions = BuildOptions.HostDefault();
            return _options.Platform == hostOptions.Platform &&
                _options.Architecture.Equals(hostOptions.Architecture, StringComparison.OrdinalIgnoreCase);
        }

        private static string FindTargetBinaryId(BuildGraph graph, string targetName, string executableName)
        {
            var targetId = BuildGraphIds.Target(targetName);
            var targetNode = graph.Nodes.FirstOrDefault(node => node.Id.Equals(targetId, StringComparison.OrdinalIgnoreCase))
                ?? throw new InvalidOperationException($"Host {targetName} graph did not contain target node `{targetId}`.");
            var binaryId = targetNode.Dependencies.FirstOrDefault(id =>
                TryGetFileNameFromFileId(id).Equals(executableName, StringComparison.OrdinalIgnoreCase));
            if(string.IsNullOrWhiteSpace(binaryId))
            {
                throw new InvalidOperationException(
                    $"Host {targetName} target did not expose `{executableName}` as an executable dependency. " +
                    $"Dependencies: {string.Join(", ", targetNode.Dependencies)}");
            }
            return binaryId;
        }

        private static string TryGetFileNameFromFileId(string id)
        {
            const string filePrefix = "file://";
            if(!id.StartsWith(filePrefix, StringComparison.OrdinalIgnoreCase))
            {
                return string.Empty;
            }
            return Path.GetFileName(id[filePrefix.Length..].Replace('\\', '/'));
        }

        private static string QualifyTarget(BuildTargetDefinition owner, string targetName)
        {
            return string.IsNullOrWhiteSpace(owner.ProjectName) ? targetName : owner.ProjectName + "." + targetName;
        }

        private static string QualifyTarget(string projectName, string targetName)
        {
            return string.IsNullOrWhiteSpace(projectName) ? targetName : projectName + "." + targetName;
        }

        private static BuildActionConfigurationDefinition RequireActionConfiguration(
            BuildTargetDefinition target,
            string name)
        {
            return target.Options.FindActionConfiguration(name) ?? throw new InvalidOperationException(
                $"Target `{target.QualifiedName}` requires project action configuration `{name}`.");
        }

        private static string RequireBinding(
            IReadOnlyDictionary<string, string> bindings,
            string name,
            string configurationName)
        {
            return bindings.TryGetValue(name, out var value) && !string.IsNullOrWhiteSpace(value)
                ? value
                : throw new InvalidOperationException(
                    $"Action configuration `{configurationName}` does not define required binding `{name}`.");
        }

        private void ValidateLinkCompatibility(
            BuildTargetDefinition consumer,
            IReadOnlyList<TargetBuildOutputs> dependencies)
        {
            if(consumer.Kind is BuildTargetKind.External or BuildTargetKind.HeaderOnly or BuildTargetKind.DotNetProject)
            {
                return;
            }

            foreach(var dependencyOutput in dependencies)
            {
                const string prefix = "target://";
                var qualifiedName = dependencyOutput.TargetId.StartsWith(prefix, StringComparison.Ordinal)
                    ? dependencyOutput.TargetId[prefix.Length..]
                    : dependencyOutput.TargetId;
                if(!_targetMap.TryGetValue(qualifiedName, out var dependency) ||
                    dependency.Kind is BuildTargetKind.External or BuildTargetKind.HeaderOnly or BuildTargetKind.DotNetProject)
                {
                    continue;
                }

                var mismatches = new List<string>();
                AddMismatch("platform", consumer.Options.Platform, dependency.Options.Platform);
                AddMismatch("architecture", consumer.Options.Architecture, dependency.Options.Architecture);
                AddMismatch("mode", consumer.Options.Mode, dependency.Options.Mode);
                AddMismatch("linkage", consumer.Options.Shared ? "shared" : "static", dependency.Options.Shared ? "shared" : "static");
                AddMismatch("RHI", consumer.Options.RhiApi, dependency.Options.RhiApi);
                if(mismatches.Count > 0)
                {
                    throw new InvalidOperationException(
                        $"Native link configuration mismatch: `{consumer.QualifiedName}` cannot link `{dependency.QualifiedName}`. " +
                        string.Join(", ", mismatches));
                }

                void AddMismatch<T>(string name, T consumerValue, T dependencyValue)
                {
                    if(!EqualityComparer<T>.Default.Equals(consumerValue, dependencyValue))
                    {
                        mismatches.Add($"{name}: consumer={consumerValue}, dependency={dependencyValue}");
                    }
                }
            }
        }

        private LunaMetaBuildOutputs AddMetaNodes(
            BuildTargetDefinition target,
            IReadOnlyList<TargetBuildOutputs> dependencyOutputs,
            TargetBuildOutputs? metaToolOutputs)
        {
            if(target.MetaHeaderFiles.Count == 0)
            {
                return new LunaMetaBuildOutputs(null, Array.Empty<string>(), Array.Empty<string>(), null);
            }
            var actionConfiguration = RequireActionConfiguration(target, "luna.meta");
            var actionConfigurationInputIds = AddActionConfigurationInputNodes(actionConfiguration);

            var generatedDirectory = GetGeneratedMetaHeaderDirectory(_workspace, _options, target);
            var generatedByName = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach(var header in target.MetaHeaderFiles)
            {
                var generatedName = Path.GetFileNameWithoutExtension(header) + ".generated.hpp";
                if(generatedByName.TryGetValue(generatedName, out var existing))
                {
                    throw new InvalidOperationException(
                        $"Target {target.Name} has multiple meta headers that generate `{generatedName}`: {existing} and {header}");
                }
                generatedByName.Add(generatedName, header);
            }

            var sourceHeaderIds = new List<string>();
            foreach(var header in target.MetaHeaderFiles)
            {
                var sourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(header));
                AddNode(new BuildGraphNode(
                    Id: sourceId,
                    Kind: BuildGraphNodeKind.File,
                    Path: _workspace.ToRepositoryRelativePath(header),
                    Command: null,
                    Dependencies: Array.Empty<string>(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));
                sourceHeaderIds.Add(sourceId);
            }

            var generatedHeaderIds = target.MetaHeaderFiles
                .Select(header =>
                {
                    var headerPath = GetMetaGeneratedHeaderPath(_workspace, _options, target, header);
                    var headerId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(headerPath));
                    AddNode(new BuildGraphNode(
                        Id: headerId,
                        Kind: BuildGraphNodeKind.File,
                        Path: _workspace.ToRepositoryRelativePath(headerPath),
                        Command: null,
                        Dependencies: Array.Empty<string>(),
                        OrderOnlyDependencies: Array.Empty<string>(),
                        Outputs: Array.Empty<string>(),
                        Depfiles: Array.Empty<string>()));
                    return headerId;
                })
                .ToList();
            var registrationHeaderPath = GetMetaRegistrationHeaderPath(_workspace, _options, target);
            var registrationHeaderId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(registrationHeaderPath));
            AddNode(new BuildGraphNode(
                Id: registrationHeaderId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(registrationHeaderPath),
                Command: null,
                Dependencies: Array.Empty<string>(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));
            generatedHeaderIds.Add(registrationHeaderId);

            var registrationSourcePath = GetMetaRegistrationSourcePath(_workspace, _options, target);
            var registrationSourceId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(registrationSourcePath));
            AddNode(new BuildGraphNode(
                Id: registrationSourceId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(registrationSourcePath),
                Command: null,
                Dependencies: Array.Empty<string>(),
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));

            var stampPath = GetMetaStampPath(_workspace, _options, target);
            var stampId = BuildGraphIds.File(_workspace.ToRepositoryRelativePath(stampPath));
            var depfilePath = Path.ChangeExtension(stampPath, ".d");
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

            var dependencyMetaIds = dependencyOutputs
                .SelectMany(output => output.PublicMetaDependencyIds)
                .Distinct(StringComparer.Ordinal)
                .ToArray();
            var toolDependencyIds = metaToolOutputs is null ? Array.Empty<string>() : new[] { metaToolOutputs.BinaryId };
            var toolOrderOnlyDependencyIds = metaToolOutputs is null ? Array.Empty<string>() : new[] { metaToolOutputs.TargetId };
            AddNode(new BuildGraphNode(
                Id: stampId,
                Kind: BuildGraphNodeKind.File,
                Path: _workspace.ToRepositoryRelativePath(stampPath),
                Command: BuildMetaCommandDescription(
                    _workspace,
                    _options,
                    target,
                    dependencyOutputs,
                    generatedDirectory,
                    stampPath,
                    depfilePath,
                    metaToolOutputs?.BinaryId),
                Dependencies: sourceHeaderIds
                    .Concat(dependencyMetaIds)
                    .Concat(toolDependencyIds)
                    .Concat(actionConfigurationInputIds)
                    .Distinct(StringComparer.Ordinal)
                    .ToArray(),
                OrderOnlyDependencies: toolOrderOnlyDependencyIds,
                Outputs: generatedHeaderIds.Concat(new[] { registrationSourceId }).ToArray(),
                Depfiles: new[] { depfileId }));
            return new LunaMetaBuildOutputs(stampId, generatedHeaderIds, new[] { generatedDirectory }, registrationSourcePath);
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
                if(!_nodesById.ContainsKey(sourceId))
                {
                    AddNode(new BuildGraphNode(
                        Id: sourceId,
                        Kind: BuildGraphNodeKind.File,
                        Path: _workspace.ToRepositoryRelativePath(runtimeFile),
                        Command: null,
                        Dependencies: Array.Empty<string>(),
                        OrderOnlyDependencies: Array.Empty<string>(),
                        Outputs: Array.Empty<string>(),
                        Depfiles: Array.Empty<string>()));
                }

                var outputPath = Path.Combine(runtimeDirectory, Path.GetFileName(runtimeFile));
                if(Path.GetFullPath(runtimeFile).Equals(Path.GetFullPath(outputPath), StringComparison.OrdinalIgnoreCase))
                {
                    outputIds.Add(sourceId);
                    continue;
                }
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

        private IReadOnlyList<string> AddShaderNodes(BuildTargetDefinition target, CppslToolBuildOutputs? cppslToolOutputs)
        {
            var shaderHeaderIds = new List<string>();
            var actionConfigurationInputIds = target.Shaders.Any(shader => IsShaderEnabledForPlatform(_options.Platform, shader.SourceFile))
                ? AddActionConfigurationInputNodes(RequireActionConfiguration(target, "cppsl.shader"))
                : Array.Empty<string>();
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
                    Command: BuildShaderCommandDescription(_workspace, _options, target, shader, headerPath, cppslToolOutputs),
                    Dependencies: new[] { sourceId }
                        .Concat(cppslToolOutputs?.DependencyIds ?? Array.Empty<string>())
                        .Concat(actionConfigurationInputIds)
                        .Distinct(StringComparer.Ordinal)
                        .ToArray(),
                    OrderOnlyDependencies: Array.Empty<string>(),
                    Outputs: Array.Empty<string>(),
                    Depfiles: Array.Empty<string>()));
                shaderHeaderIds.Add(headerId);
            }
            return shaderHeaderIds;
        }

        private IReadOnlyList<string> AddActionConfigurationInputNodes(
            BuildActionConfigurationDefinition configuration)
        {
            var files = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach(var (name, path) in configuration.Files)
            {
                if(!File.Exists(path))
                {
                    throw new FileNotFoundException(
                        $"Action configuration `{configuration.Name}` file binding `{name}` does not exist: {path}",
                        path);
                }
                files.Add(path);
            }
            foreach(var (name, path) in configuration.Directories)
            {
                if(!Directory.Exists(path))
                {
                    throw new DirectoryNotFoundException(
                        $"Action configuration `{configuration.Name}` directory binding `{name}` does not exist: {path}");
                }
                foreach(var file in Directory.EnumerateFiles(path, "*", SearchOption.AllDirectories))
                {
                    files.Add(file);
                }
            }
            return files.Select(path => AddFileReferenceNode(path)).ToArray();
        }

        public void AddNode(BuildGraphNode node)
        {
            var configuredNode = node.Options is null
                ? node with
                {
                    Options = _options,
                    ProjectName = _activeProjectName,
                    ConfigurationId = _activeConfigurationId,
                }
                : node;
            if(_nodesById.TryGetValue(configuredNode.Id, out var existing))
            {
                if(!NodesEquivalent(existing, configuredNode))
                {
                    throw new InvalidOperationException(
                        $"Build graph node `{configuredNode.Id}` is produced by multiple non-identical declarations." +
                        $"{Environment.NewLine}Existing command: {existing.Command ?? "<none>"}" +
                        $"{Environment.NewLine}Conflicting command: {configuredNode.Command ?? "<none>"}");
                }
                return;
            }
            _nodesById.Add(configuredNode.Id, configuredNode);
        }

        private static bool NodesEquivalent(BuildGraphNode left, BuildGraphNode right)
        {
            return left.Kind == right.Kind && left.Path == right.Path && left.Command == right.Command &&
                left.Dependencies.SequenceEqual(right.Dependencies, StringComparer.Ordinal) &&
                left.OrderOnlyDependencies.SequenceEqual(right.OrderOnlyDependencies, StringComparer.Ordinal) &&
                left.Outputs.SequenceEqual(right.Outputs, StringComparer.Ordinal) &&
                left.Depfiles.SequenceEqual(right.Depfiles, StringComparer.Ordinal);
        }
    }

    private sealed record TargetBuildOutputs(
        string TargetId,
        string BinaryId,
        IReadOnlyList<string> LinkInputIds,
        IReadOnlyList<string> PublicIncludeDirectories,
        IReadOnlyList<string> PublicMetaDependencyIds,
        IReadOnlyList<string> PublicDefines,
        IReadOnlyList<string> PublicUndefines,
        IReadOnlyList<string> Frameworks,
        IReadOnlyList<string> RuntimeFiles);

    private sealed record LunaMetaBuildOutputs(
        string? StampId,
        IReadOnlyList<string> GeneratedHeaderIds,
        IReadOnlyList<string> PublicIncludeDirectories,
        string? GeneratedSourceFile);

    private sealed record CppslToolBuildOutputs(
        string CppslcId,
        string NativeExtractorId)
    {
        public IReadOnlyList<string> DependencyIds => new[] { CppslcId, NativeExtractorId }
            .Distinct(StringComparer.Ordinal)
            .ToArray();
    }

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
        var relativeSource = Path.GetRelativePath(target.ProjectRootDirectory, sourceFile).Replace('\\', '/');
        var sourceHash = StableHash(relativeSource)[..10];
        var baseName = Path.GetFileNameWithoutExtension(sourceFile);
        var extension = IsResourceSource(sourceFile)
            ? ".res"
            : options.Platform == BuildPlatform.Windows ? ".obj" : ".o";
        return Path.Combine(GetTargetObjectDirectory(workspace, options, target), $"{baseName}-{sourceHash}{extension}");
    }

    private static string GetTargetObjectDirectory(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(
            GetTargetConfigurationDirectory(target, options),
            "obj",
            target.Name);
    }

    private static string GetTargetBinaryPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        var fileName = IsAndroidNativeActivityLibrary(options, target)
            ? $"lib{target.Name}.so"
            : target.Kind == BuildTargetKind.Executable
            ? $"{target.Name}{ExecutableExtension(options.Platform)}"
            : $"{options.LibraryPrefix}{target.Name}{(options.Shared ? SharedLibraryExtension(options.Platform) : StaticLibraryExtension(options.Platform))}";
        return Path.Combine(
            GetTargetConfigurationDirectory(target, options),
            "bin",
            fileName);
    }

    private static string GetGeneratedShaderHeaderDirectory(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(
            GetTargetConfigurationDirectory(target, options),
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
            GetTargetConfigurationDirectory(target, options),
            "generated",
            target.Name,
            "embedded",
            headerFile);
    }

    private static string GetGeneratedMetaHeaderDirectory(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(
            GetTargetConfigurationDirectory(target, options),
            "generated",
            target.Name,
            "meta");
    }

    private static string GetMetaGeneratedHeaderPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target, string headerFile)
    {
        return Path.Combine(GetGeneratedMetaHeaderDirectory(workspace, options, target), Path.GetFileNameWithoutExtension(headerFile) + ".generated.hpp");
    }

    private static string GetMetaRegistrationHeaderPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(GetGeneratedMetaHeaderDirectory(workspace, options, target), target.Name + ".meta.generated.hpp");
    }

    private static string GetMetaRegistrationSourcePath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        var extension = target.MetaHeaderFiles.Any(path => MetaHeaderLanguage(path) == "objective-c++20") ? ".mm" : ".cpp";
        return Path.Combine(GetGeneratedMetaHeaderDirectory(workspace, options, target), target.Name + ".meta.generated" + extension);
    }

    private static string GetMetaStampPath(BuildWorkspace workspace, BuildOptions options, BuildTargetDefinition target)
    {
        return Path.Combine(GetGeneratedMetaHeaderDirectory(workspace, options, target), ".luna_meta.stamp");
    }

    private static string GetTargetConfigurationDirectory(BuildTargetDefinition target, BuildOptions options)
    {
        var directory = Path.Combine(
            target.ProjectBuildDirectory,
            options.Platform.ToString(),
            options.Architecture,
            options.Mode.ToString());
        return target.IsHostProject ? directory : Path.Combine(directory, target.ConfigurationId);
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
            $"target={target.QualifiedName}",
            $"source={workspace.ToRepositoryRelativePath(sourceFile)}",
            $"object={workspace.ToRepositoryRelativePath(objectPath)}",
            $"depfile={workspace.ToRepositoryRelativePath(depfilePath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"language={SourceLanguage(sourceFile)}",
            "exceptions=none",
            "define=LUNA_MANUAL_CONFIG_DEBUG_LEVEL",
            $"define=LUNA_DEBUG_LEVEL={DebugLevel(options.Mode)}",
            options.Shared ? "define=LUNA_BUILD_SHARED_LIB" : "linkage=static",
        };
        lines.AddRange(options.GlobalDefines.Select(define => $"define={define}"));
        lines.AddRange(options.GlobalIncludeDirectories.Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        if(!target.EnableRtti)
        {
            lines.Add("rtti=none");
        }
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
        if(target.MetaHeaderFiles.Count > 0)
        {
            lines.Add($"include={workspace.ToRepositoryRelativePath(GetGeneratedMetaHeaderDirectory(workspace, options, target))}");
        }
        lines.AddRange(target.IncludeDirectories.Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicIncludeDirectories)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase)
            .Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        return string.Join('\n', lines);
    }

    private static string BuildMetaCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        IReadOnlyList<TargetBuildOutputs> dependencyOutputs,
        string generatedDirectory,
        string stampPath,
        string depfilePath,
        string? toolId)
    {
        var lines = new List<string>
        {
            "kind=luna.meta",
            $"target={target.QualifiedName}",
            $"target_name={target.Name}",
            $"output_dir={workspace.ToRepositoryRelativePath(generatedDirectory)}",
            $"stamp={workspace.ToRepositoryRelativePath(stampPath)}",
            $"depfile={workspace.ToRepositoryRelativePath(depfilePath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            "define=LUNA_MANUAL_CONFIG_DEBUG_LEVEL",
            $"define=LUNA_DEBUG_LEVEL={DebugLevel(options.Mode)}",
            options.Shared ? "define=LUNA_BUILD_SHARED_LIB" : "linkage=static",
        };
        if(!string.IsNullOrWhiteSpace(toolId))
        {
            lines.Add($"tool={FileIdToRepositoryRelativePath(toolId)}");
        }
        var actionConfiguration = options.FindActionConfiguration("luna.meta");
        if(actionConfiguration is not null && actionConfiguration.Directories.TryGetValue("clang_resource", out var resourceDirectory))
        {
            lines.Add($"resource_dir={workspace.ToRepositoryRelativePath(resourceDirectory)}");
        }
        if(target.MsvcRuntimeLibrary is not null)
        {
            lines.Add($"runtime={target.MsvcRuntimeLibrary}");
        }
        lines.AddRange(options.GlobalDefines.Select(define => $"define={define}"));
        lines.AddRange(options.GlobalUndefines.Select(undefine => $"undefine={undefine}"));
        lines.AddRange(options.GlobalIncludeDirectories.Select(path => $"include={workspace.ToRepositoryRelativePath(path)}"));
        var metaHeaders = target.MetaHeaderFiles
            .Order(StringComparer.OrdinalIgnoreCase)
            .ToArray();
        lines.AddRange(metaHeaders.Select(path => $"header={workspace.ToRepositoryRelativePath(path)}"));
        lines.AddRange(metaHeaders.Select(path => $"header_language={MetaHeaderLanguage(path)}"));
        lines.AddRange(target.Defines.Select(define => $"define={define}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicDefines)
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal)
            .Select(define => $"define={define}"));
        lines.AddRange(target.Undefines.Select(undefine => $"undefine={undefine}"));
        lines.AddRange(dependencyOutputs
            .SelectMany(output => output.PublicUndefines)
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal)
            .Select(undefine => $"undefine={undefine}"));
        lines.Add($"include={workspace.ToRepositoryRelativePath(generatedDirectory)}");
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
        string headerPath,
        CppslToolBuildOutputs? cppslToolOutputs)
    {
        var lines = new List<string>
        {
            "kind=cppsl.shader",
            $"target={target.QualifiedName}",
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
        if(cppslToolOutputs is not null)
        {
            lines.Add($"cppslc={FileIdToRepositoryRelativePath(cppslToolOutputs.CppslcId)}");
            lines.Add($"native_extractor={FileIdToRepositoryRelativePath(cppslToolOutputs.NativeExtractorId)}");
        }
        var actionConfiguration = options.FindActionConfiguration("cppsl.shader") ?? throw new InvalidOperationException(
            $"Target `{target.QualifiedName}` requires project action configuration `cppsl.shader`.");
        foreach(var (name, path) in actionConfiguration.Files.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase))
        {
            lines.Add($"{name}={workspace.ToRepositoryRelativePath(path)}");
        }
        foreach(var (name, path) in actionConfiguration.Directories.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase))
        {
            lines.Add($"{name}={workspace.ToRepositoryRelativePath(path)}");
        }
        foreach(var (name, value) in actionConfiguration.Values.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase))
        {
            lines.Add($"{name}={value}");
        }
        return string.Join('\n', lines);
    }

    private static string FileIdToRepositoryRelativePath(string fileId)
    {
        const string prefix = "file://";
        if(!fileId.StartsWith(prefix, StringComparison.Ordinal))
        {
            throw new InvalidOperationException($"Expected file node id, got `{fileId}`.");
        }
        return fileId[prefix.Length..];
    }

    private static string BuildCopyCommandDescription(
        BuildWorkspace workspace,
        BuildTargetDefinition target,
        string sourcePath,
        string outputPath)
    {
        return string.Join('\n',
            "kind=file.copy",
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
            $"target={target.QualifiedName}",
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

    private static string MetaHeaderLanguage(string path)
    {
        if(Path.GetExtension(path).Equals(".h", StringComparison.OrdinalIgnoreCase) &&
            path.Replace('\\', '/').Contains("/Metal/", StringComparison.Ordinal))
        {
            return "objective-c++20";
        }
        return LooksLikeObjectiveCxxHeader(path) ? "objective-c++20" : "c++20";
    }

    private static bool LooksLikeObjectiveCxxHeader(string path)
    {
        var extension = Path.GetExtension(path);
        if(!extension.Equals(".h", StringComparison.OrdinalIgnoreCase) &&
            !extension.Equals(".hpp", StringComparison.OrdinalIgnoreCase) &&
            !extension.Equals(".hh", StringComparison.OrdinalIgnoreCase) &&
            !extension.Equals(".hxx", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }
        if(!File.Exists(path))
        {
            return false;
        }

        var text = File.ReadAllText(path);
        return text.Contains("#import", StringComparison.Ordinal) ||
            text.Contains("@class", StringComparison.Ordinal) ||
            text.Contains("@interface", StringComparison.Ordinal) ||
            text.Contains("@protocol", StringComparison.Ordinal) ||
            text.Contains("id<", StringComparison.Ordinal) ||
            text.Contains("NS_", StringComparison.Ordinal) ||
            text.Contains("NSView", StringComparison.Ordinal) ||
            text.Contains("NSWindow", StringComparison.Ordinal) ||
            text.Contains("UIView", StringComparison.Ordinal) ||
            text.Contains("UIWindow", StringComparison.Ordinal) ||
            text.Contains("MTL", StringComparison.Ordinal);
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
            $"target={target.QualifiedName}",
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
        var actionKind = target.Kind == BuildTargetKind.Executable
            ? IsAndroidNativeActivityLibrary(options, target) ? "cpp.link.shared" : "cpp.link.executable"
            : options.Shared ? "cpp.link.shared" : "cpp.link.static";
        return string.Join('\n',
            $"kind={actionKind}",
            $"target={target.QualifiedName}",
            $"output={workspace.ToRepositoryRelativePath(binaryPath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}")
            + string.Concat(inputIds.OrderBy(id => id, StringComparer.OrdinalIgnoreCase).Select(id => $"\ninput={id}"))
            + string.Concat(target.SystemLibraries.OrderBy(id => id, StringComparer.OrdinalIgnoreCase).Select(library => $"\nlibrary={library}"))
            + string.Concat(target.Frameworks
                .Concat(dependencyFrameworks)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .Select(framework => $"\nframework={framework}"));
    }

    private static bool IsAndroidNativeActivityLibrary(BuildOptions options, BuildTargetDefinition target)
    {
        return options.Platform == BuildPlatform.Android && target.Kind == BuildTargetKind.Executable;
    }

    private static string BuildTargetCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target,
        int sourceCount)
    {
        return string.Join('\n',
            "kind=target.cpp",
            $"name={target.QualifiedName}",
            $"rules={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"directory={workspace.ToRepositoryRelativePath(target.Directory)}",
            $"source_count={sourceCount}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"shared={options.Shared}",
            $"rhi={options.RhiApi}",
            $"runtime_file_count={target.RuntimeFiles.Count}",
            $"meta_header_count={target.MetaHeaderFiles.Count}",
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
            $"name={target.QualifiedName}",
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
        BuildTargetDefinition target,
        string outputPath,
        string artifactsDirectory)
    {
        return string.Join('\n',
            "kind=dotnet.build",
            $"name={target.QualifiedName}",
            $"project={workspace.ToRepositoryRelativePath(target.DotNetProjectFile!)}",
            $"output={workspace.ToRepositoryRelativePath(outputPath)}",
            $"artifacts_dir={workspace.ToRepositoryRelativePath(artifactsDirectory)}",
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
            $"name={target.QualifiedName}",
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

    private static string StableHash(string value)
    {
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(value))).ToLowerInvariant();
    }
}
