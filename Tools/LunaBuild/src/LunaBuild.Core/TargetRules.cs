namespace LunaBuild.Core;

public abstract class TargetRules
{
    private readonly List<string> _dependencies = new();
    private readonly List<string> _sourcePatterns = new();
    private readonly List<string> _excludedSourcePatterns = new();
    private readonly List<string> _headerPatterns = new();
    private readonly List<string> _includeDirectories = new();
    private readonly List<string> _defines = new();
    private readonly List<string> _undefines = new();
    private readonly List<string> _packageNames = new();
    private readonly List<string> _linkLibraryFiles = new();
    private readonly List<string> _systemLibraries = new();
    private readonly List<string> _runtimeFilePatterns = new();
    private readonly List<EmbeddedHeaderRule> _embeddedHeaders = new();
    private readonly List<ShaderRule> _shaders = new();
    private readonly HashSet<BuildPlatform> _supportedPlatforms = new();
    private BuildWorkspace? _currentWorkspace;
    private BuildOptions? _currentOptions;
    private string? _msvcRuntimeLibrary;
    private DotNetProjectRule? _dotNetProject;

    protected TargetRules(string name, string targetDirectory, string rulesPath)
    {
        Name = name;
        TargetDirectory = targetDirectory;
        RulesPath = rulesPath;
    }

    public string Name { get; }

    public string TargetDirectory { get; }

    public string RulesPath { get; }

    public BuildTargetKind Kind { get; protected set; } = BuildTargetKind.SharedLibrary;

    public bool IsTest { get; protected set; }

    protected BuildWorkspace Workspace => _currentWorkspace ?? throw new InvalidOperationException("TargetRules workspace is only available during Configure/ToDefinition.");

    protected BuildOptions Options => _currentOptions ?? throw new InvalidOperationException("TargetRules options are only available during Configure/ToDefinition.");

    protected BuildPlatform Platform => Options.Platform;

    protected string Architecture => Options.Architecture;

    protected RhiApi RhiApi => Options.RhiApi;

    public bool SupportsPlatform(BuildPlatform platform)
    {
        return _supportedPlatforms.Count == 0 || _supportedPlatforms.Contains(platform);
    }

    protected void SupportedPlatforms(params BuildPlatform[] platforms)
    {
        foreach(var platform in platforms)
        {
            _supportedPlatforms.Add(platform);
        }
    }

    protected void DependsOn(params string[] targetNames)
    {
        _dependencies.AddRange(targetNames);
    }

    protected void Sources(params string[] patterns)
    {
        _sourcePatterns.AddRange(patterns);
    }

    protected void ExcludeSources(params string[] patterns)
    {
        _excludedSourcePatterns.AddRange(patterns);
    }

    protected void Headers(params string[] patterns)
    {
        _headerPatterns.AddRange(patterns);
    }

    protected void IncludeDirectories(params string[] directories)
    {
        _includeDirectories.AddRange(directories);
    }

    protected void Defines(params string[] defines)
    {
        _defines.AddRange(defines);
    }

    protected void Undefines(params string[] undefines)
    {
        _undefines.AddRange(undefines);
    }

    protected void Packages(params string[] packageNames)
    {
        _packageNames.AddRange(packageNames);
    }

    protected void LinkLibraryFiles(params string[] files)
    {
        _linkLibraryFiles.AddRange(files);
    }

    protected void SystemLibraries(params string[] libraries)
    {
        _systemLibraries.AddRange(libraries);
    }

    protected void MsvcRuntimeLibrary(string runtimeLibrary)
    {
        _msvcRuntimeLibrary = runtimeLibrary;
    }

    protected void DotNetProject(string projectFile, string outputFile)
    {
        _dotNetProject = new DotNetProjectRule(projectFile, outputFile);
    }

    protected void RuntimeFiles(params string[] patterns)
    {
        _runtimeFilePatterns.AddRange(patterns);
    }

    protected void EmbeddedHeader(string sourceFile, string headerFile, string dataSymbol, string sizeSymbol)
    {
        _embeddedHeaders.Add(new EmbeddedHeaderRule(sourceFile, headerFile, dataSymbol, sizeSymbol));
    }

    protected void Shader(string sourceFile, string stage, string entryPoint)
    {
        _shaders.Add(new ShaderRule(sourceFile, stage, entryPoint));
    }

    protected virtual void Configure(BuildWorkspace workspace, BuildOptions options)
    {
    }

    public virtual BuildTargetDefinition ToDefinition(BuildWorkspace workspace, BuildOptions options)
    {
        var state = CaptureState();
        _currentWorkspace = workspace;
        _currentOptions = options;
        try
        {
            Configure(workspace, options);

            var directory = workspace.ResolveRepositoryPath(TargetDirectory);
            var excludedSources = TargetPatternExpander.ExpandPatterns(directory, _excludedSourcePatterns)
                .ToHashSet(StringComparer.OrdinalIgnoreCase);
            return new BuildTargetDefinition(
                Name: Name,
                Directory: directory,
                ScriptPath: workspace.ResolveRepositoryPath(RulesPath),
                Dependencies: _dependencies.Distinct(StringComparer.OrdinalIgnoreCase).Order(StringComparer.OrdinalIgnoreCase).ToArray(),
                SourceFiles: TargetPatternExpander.ExpandPatterns(directory, _sourcePatterns)
                    .Where(source => !excludedSources.Contains(source))
                    .ToArray(),
            HeaderFiles: TargetPatternExpander.ExpandPatterns(directory, _headerPatterns),
            IncludeDirectories: _includeDirectories
                    .Select(includeDirectory => ResolveTargetPath(workspace, TargetDirectory, includeDirectory))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .Order(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                Defines: _defines
                    .Distinct(StringComparer.Ordinal)
                    .Order(StringComparer.Ordinal)
                    .ToArray(),
                Undefines: _undefines
                    .Distinct(StringComparer.Ordinal)
                    .Order(StringComparer.Ordinal)
                    .ToArray(),
                PackageNames: _packageNames
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .Order(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                LinkLibraryFiles: _linkLibraryFiles
                    .Select(file => ResolveTargetPath(workspace, TargetDirectory, file))
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .Order(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                SystemLibraries: _systemLibraries
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .Order(StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                RuntimeFiles: TargetPatternExpander.ExpandPatterns(directory, _runtimeFilePatterns),
                EmbeddedHeaders: _embeddedHeaders
                    .Select(header => new BuildEmbeddedHeaderDefinition(
                        workspace.ResolveRepositoryPath(Path.Combine(TargetDirectory, header.SourceFile)),
                        header.HeaderFile,
                        header.DataSymbol,
                        header.SizeSymbol))
                    .Distinct()
                    .OrderBy(header => header.HeaderFile, StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                Shaders: _shaders
                    .Select(shader => new BuildShaderDefinition(
                        workspace.ResolveRepositoryPath(Path.Combine(TargetDirectory, shader.SourceFile)),
                        shader.Stage,
                        shader.EntryPoint))
                    .Distinct()
                    .OrderBy(shader => shader.SourceFile, StringComparer.OrdinalIgnoreCase)
                    .ToArray(),
                Kind: Kind,
                IsTest: IsTest,
                MsvcRuntimeLibrary: _msvcRuntimeLibrary,
                DotNetProjectFile: _dotNetProject is null
                    ? null
                    : workspace.ResolveRepositoryPath(Path.Combine(TargetDirectory, _dotNetProject.ProjectFile)),
                DotNetOutputFile: _dotNetProject is null
                    ? null
                    : workspace.ResolveRepositoryPath(Path.Combine(TargetDirectory, _dotNetProject.OutputFile)));
        }
        finally
        {
            RestoreState(state);
            _currentWorkspace = null;
            _currentOptions = null;
        }
    }

    private static string ResolveTargetPath(BuildWorkspace workspace, string targetDirectory, string path)
    {
        return Path.IsPathRooted(path)
            ? Path.GetFullPath(path)
            : workspace.ResolveRepositoryPath(Path.Combine(targetDirectory, path));
    }

    private TargetRuleState CaptureState()
    {
        return new TargetRuleState(
            Dependencies: _dependencies.Count,
            SourcePatterns: _sourcePatterns.Count,
            ExcludedSourcePatterns: _excludedSourcePatterns.Count,
            HeaderPatterns: _headerPatterns.Count,
            IncludeDirectories: _includeDirectories.Count,
            Defines: _defines.Count,
            Undefines: _undefines.Count,
            PackageNames: _packageNames.Count,
            LinkLibraryFiles: _linkLibraryFiles.Count,
            SystemLibraries: _systemLibraries.Count,
            RuntimeFilePatterns: _runtimeFilePatterns.Count,
            EmbeddedHeaders: _embeddedHeaders.Count,
            Shaders: _shaders.Count,
            SupportedPlatforms: _supportedPlatforms.ToArray(),
            MsvcRuntimeLibrary: _msvcRuntimeLibrary,
            DotNetProject: _dotNetProject);
    }

    private void RestoreState(TargetRuleState state)
    {
        Truncate(_dependencies, state.Dependencies);
        Truncate(_sourcePatterns, state.SourcePatterns);
        Truncate(_excludedSourcePatterns, state.ExcludedSourcePatterns);
        Truncate(_headerPatterns, state.HeaderPatterns);
        Truncate(_includeDirectories, state.IncludeDirectories);
        Truncate(_defines, state.Defines);
        Truncate(_undefines, state.Undefines);
        Truncate(_packageNames, state.PackageNames);
        Truncate(_linkLibraryFiles, state.LinkLibraryFiles);
        Truncate(_systemLibraries, state.SystemLibraries);
        Truncate(_runtimeFilePatterns, state.RuntimeFilePatterns);
        Truncate(_embeddedHeaders, state.EmbeddedHeaders);
        Truncate(_shaders, state.Shaders);
        _supportedPlatforms.Clear();
        foreach(var platform in state.SupportedPlatforms)
        {
            _supportedPlatforms.Add(platform);
        }
        _msvcRuntimeLibrary = state.MsvcRuntimeLibrary;
        _dotNetProject = state.DotNetProject;
    }

    private static void Truncate<T>(List<T> list, int count)
    {
        if(list.Count > count)
        {
            list.RemoveRange(count, list.Count - count);
        }
    }

    private sealed record ShaderRule(string SourceFile, string Stage, string EntryPoint);

    private sealed record EmbeddedHeaderRule(string SourceFile, string HeaderFile, string DataSymbol, string SizeSymbol);

    private sealed record DotNetProjectRule(string ProjectFile, string OutputFile);

    private sealed record TargetRuleState(
        int Dependencies,
        int SourcePatterns,
        int ExcludedSourcePatterns,
        int HeaderPatterns,
        int IncludeDirectories,
        int Defines,
        int Undefines,
        int PackageNames,
        int LinkLibraryFiles,
        int SystemLibraries,
        int RuntimeFilePatterns,
        int EmbeddedHeaders,
        int Shaders,
        BuildPlatform[] SupportedPlatforms,
        string? MsvcRuntimeLibrary,
        DotNetProjectRule? DotNetProject);
}

public interface ITargetRulesProvider
{
    IReadOnlyList<TargetRules> GetTargetRules(BuildWorkspace workspace);
}
