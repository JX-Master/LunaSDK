using System.Security.Cryptography;
using System.Text;

namespace LunaBuild.Core;

public sealed class Project
{
    private readonly BuildSession _session;
    private readonly List<Project> _imports = new();
    private readonly List<ActionConfigurationImport> _actionConfigurationImports = new();
    private BuildOptions _primaryOptions;
    private string _buildDirectory;
    private BuildWorkspace _workspace;
    private ProjectRules _rules;
    private ProjectTargetRulesProvider _rulesProvider;
    private bool _configurationFrozen;

    internal Project(
        BuildSession session,
        Project? importer,
        BuildWorkspace workspace,
        ProjectRules rules,
        ProjectTargetRulesProvider rulesProvider,
        BuildProjectDefinition definition,
        bool isHost)
    {
        _session = session;
        Importer = importer;
        _workspace = workspace;
        _rules = rules;
        _rulesProvider = rulesProvider;
        Definition = definition;
        IsHost = isHost;
        _buildDirectory = workspace.BuildDirectory;
        _primaryOptions = BuildOptions.HostDefault() with
        {
            Properties = definition.ResolveProperties(new Dictionary<string, string?>()),
        };
    }

    public string Name => Definition.Name;

    public string RootDirectory => Workspace.RootDirectory;

    public string BuildDirectory
    {
        get => _buildDirectory;
        set
        {
            EnsureMutable(nameof(BuildDirectory));
            if(!Path.IsPathFullyQualified(value))
            {
                throw new ArgumentException("An imported project build directory must be an absolute path.", nameof(value));
            }
            _session.UpdateBuildDirectory(this, Path.GetFullPath(value));
        }
    }

    public BuildOptions DefaultBuildOptions => BuildOptions.HostDefault() with
    {
        Properties = Definition.ResolveProperties(new Dictionary<string, string?>()),
    };

    public BuildOptions PrimaryOptions
    {
        get => _primaryOptions;
        set
        {
            EnsureMutable(nameof(PrimaryOptions));
            _primaryOptions = value ?? throw new ArgumentNullException(nameof(value));
        }
    }

    public BuildProjectDefinition Definition { get; }

    public bool IsHost { get; }

    public Project? Importer { get; }

    public IReadOnlyList<Project> ImportedProjects => _imports;

    internal BuildWorkspace Workspace => _workspace;

    internal ProjectRules Rules => _rules;

    internal ProjectTargetRulesProvider RulesProvider => _rulesProvider;

    public string ConfigurationId => ProjectConfigurationIdentity.Create(_primaryOptions);

    internal string ImportChain => Importer is null ? Name : Importer.ImportChain + " -> " + Name;

    public Project ImportProject(string path)
    {
        return _session.ImportProject(this, path);
    }

    public BuildProperties ResolveProperties(IReadOnlyDictionary<string, string?> overrides)
    {
        return Definition.ResolveProperties(overrides);
    }

    public void UseActionConfiguration(Project provider, string name)
    {
        _session.UseActionConfiguration(this, provider, name);
    }

    internal void AddImport(Project project)
    {
        _imports.Add(project);
    }

    internal void AddActionConfigurationImport(Project provider, string name)
    {
        _actionConfigurationImports.Add(new ActionConfigurationImport(provider, name));
    }

    internal IReadOnlyList<ActionConfigurationImport> ActionConfigurationImports => _actionConfigurationImports;

    internal void FreezeConfiguration()
    {
        _configurationFrozen = true;
    }

    internal void SetConfiguredOptions(BuildOptions options)
    {
        _primaryOptions = options;
    }

    internal void SetWorkspace(
        BuildWorkspace workspace,
        ProjectRules rules,
        ProjectTargetRulesProvider rulesProvider)
    {
        _workspace = workspace;
        _rules = rules;
        _rulesProvider = rulesProvider;
        _buildDirectory = workspace.BuildDirectory;
    }

    private void EnsureMutable(string propertyName)
    {
        if(_configurationFrozen)
        {
            throw new InvalidOperationException($"Project `{Name}` configuration is frozen; `{propertyName}` can no longer be changed.");
        }
    }

    internal sealed record ActionConfigurationImport(Project Provider, string Name);
}

public sealed class BuildSession
{
    private readonly Dictionary<string, Project> _projectsByRoot = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, Project> _projectsByName = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<Project> _projects = new();
    private Project? _configuringProject;

    private BuildSession(BuildWorkspace hostWorkspace)
    {
        HostWorkspace = hostWorkspace;
    }

    public BuildWorkspace HostWorkspace { get; }

    public Project HostProject { get; private set; } = null!;

    public IReadOnlyList<Project> Projects => _projects;

    public IReadOnlyList<BuildTargetDefinition> Targets { get; private set; } = Array.Empty<BuildTargetDefinition>();

    public static BuildSession Create(
        BuildWorkspace hostWorkspace,
        Func<BuildProjectDefinition, BuildOptions> resolveHostOptions)
    {
        var session = new BuildSession(hostWorkspace);
        var host = session.LoadProject(importer: null, hostWorkspace.RootDirectory, isHost: true, hostWorkspace);
        session.HostProject = host;
        host.PrimaryOptions = resolveHostOptions(host.Definition);
        session.ConfigureProjectTree(host);
        session.ValidateBuildDirectories();
        session.DiscoverTargets();
        return session;
    }

    public string ResolveTargetName(string targetName)
    {
        if(targetName.Contains('.', StringComparison.Ordinal))
        {
            if(Targets.Any(target => target.QualifiedName.Equals(targetName, StringComparison.OrdinalIgnoreCase)))
            {
                return Targets.First(target => target.QualifiedName.Equals(targetName, StringComparison.OrdinalIgnoreCase)).QualifiedName;
            }
            throw new ArgumentException($"Unknown target: {targetName}");
        }

        var qualifiedName = $"{HostProject.Name}.{targetName}";
        if(Targets.Any(target => target.QualifiedName.Equals(qualifiedName, StringComparison.OrdinalIgnoreCase)))
        {
            return qualifiedName;
        }
        throw new ArgumentException($"Unknown host target: {targetName}");
    }

    internal Project ImportProject(Project importer, string path)
    {
        if(!ReferenceEquals(_configuringProject, importer))
        {
            throw new InvalidOperationException($"Project `{importer.Name}` can import projects only while its ConfigureProject method is running.");
        }
        if(string.IsNullOrWhiteSpace(path))
        {
            throw new ArgumentException("Imported project path cannot be empty.", nameof(path));
        }

        var root = Path.IsPathFullyQualified(path)
            ? Path.GetFullPath(path)
            : Path.GetFullPath(Path.Combine(importer.RootDirectory, path));
        var project = LoadProject(importer, root, isHost: false, hostWorkspace: null);
        importer.AddImport(project);
        return project;
    }

    internal void UseActionConfiguration(Project consumer, Project provider, string name)
    {
        if(!ReferenceEquals(_configuringProject, consumer))
        {
            throw new InvalidOperationException(
                $"Project `{consumer.Name}` can adopt action configurations only while its ConfigureProject method is running.");
        }
        if(string.IsNullOrWhiteSpace(name))
        {
            throw new ArgumentException("Action configuration name cannot be empty.", nameof(name));
        }
        if(!IsVisible(consumer, provider))
        {
            throw new InvalidOperationException(
                $"Project `{consumer.Name}` cannot adopt action configuration `{name}` from non-visible project `{provider.Name}`.");
        }
        consumer.AddActionConfigurationImport(provider, name);
    }

    internal void UpdateBuildDirectory(Project project, string buildDirectory)
    {
        var workspace = new BuildWorkspace(project.RootDirectory, buildDirectory, project.Workspace.RunnerProjectPath);
        if(project.BuildDirectory.Equals(workspace.BuildDirectory, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        var provider = new ProjectTargetRulesProvider();
        var projectRules = provider.GetProjectRules(workspace);
        if(projectRules.Count != 1 || !projectRules[0].Name.Equals(project.Name, StringComparison.OrdinalIgnoreCase))
        {
            throw new InvalidOperationException(
                $"Project `{project.Name}` could not reload its unique ProjectRules type under build directory `{buildDirectory}`.");
        }
        projectRules[0].ToDefinition(workspace);
        project.SetWorkspace(workspace, projectRules[0], provider);
    }

    private Project LoadProject(
        Project? importer,
        string root,
        bool isHost,
        BuildWorkspace? hostWorkspace)
    {
        var canonicalRoot = BuildWorkspace.CanonicalizeDirectory(root);
        if(_projectsByRoot.TryGetValue(canonicalRoot, out var existingRoot))
        {
            var requestedChain = importer is null ? canonicalRoot : importer.ImportChain + " -> " + canonicalRoot;
            throw new InvalidOperationException(
                $"Project root `{canonicalRoot}` is imported more than once. Existing chain: {existingRoot.ImportChain}. Requested chain: {requestedChain}.");
        }

        var rootHash = StableHash(canonicalRoot);
        var rootProjectFile = Directory.EnumerateFiles(canonicalRoot, "*.Project.cs", SearchOption.TopDirectoryOnly)
            .Order(StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
        var projectDirectoryName = rootProjectFile is null
            ? Path.GetFileName(canonicalRoot)
            : Path.GetFileName(rootProjectFile)[..^".Project.cs".Length];
        var workspace = hostWorkspace ?? new BuildWorkspace(
            canonicalRoot,
            Path.Combine(HostWorkspace.BuildDirectory, "Projects", Sanitize(projectDirectoryName) + "-" + rootHash),
            HostWorkspace.RunnerProjectPath);
        var provider = new ProjectTargetRulesProvider();
        var projectRules = provider.GetProjectRules(workspace);
        if(projectRules.Count != 1)
        {
            var ruleFiles = Directory.EnumerateFiles(canonicalRoot, "*.Project.cs", SearchOption.TopDirectoryOnly)
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray();
            throw new InvalidOperationException(
                $"Project root `{canonicalRoot}` must define exactly one root-level concrete ProjectRules type; found {projectRules.Count}. " +
                $"Root rule files: {(ruleFiles.Length == 0 ? "<none>" : string.Join(", ", ruleFiles))}");
        }

        var rules = projectRules[0];
        ValidateName(rules.Name, "project");
        if(_projectsByName.TryGetValue(rules.Name, out var existingName))
        {
            var requestedChain = importer is null ? rules.Name : importer.ImportChain + " -> " + rules.Name;
            throw new InvalidOperationException(
                $"Project name `{rules.Name}` is imported more than once. Existing chain: {existingName.ImportChain}. Requested chain: {requestedChain}.");
        }

        var definition = rules.ToDefinition(workspace);
        var project = new Project(this, importer, workspace, rules, provider, definition, isHost);
        _projectsByRoot.Add(canonicalRoot, project);
        _projectsByName.Add(project.Name, project);
        _projects.Add(project);
        return project;
    }

    private void ConfigureProjectTree(Project project)
    {
        project.FreezeConfiguration();
        _configuringProject = project;
        try
        {
            project.Rules.ConfigureProjectInstance(project);
        }
        finally
        {
            _configuringProject = null;
        }

        foreach(var importedProject in project.ImportedProjects)
        {
            ConfigureProjectTree(importedProject);
        }
    }

    private void DiscoverTargets()
    {
        foreach(var project in _projects)
        {
            var options = project.Rules.ConfigureBuildOptions(project.Workspace, project.PrimaryOptions);
            project.SetConfiguredOptions(options);
        }

        foreach(var project in _projects)
        {
            if(project.ActionConfigurationImports.Count == 0)
            {
                continue;
            }
            var configurations = project.PrimaryOptions.ActionConfigurations.ToList();
            foreach(var import in project.ActionConfigurationImports)
            {
                if(configurations.Any(configuration => configuration.Name.Equals(import.Name, StringComparison.OrdinalIgnoreCase)))
                {
                    throw new InvalidOperationException(
                        $"Project `{project.Name}` already defines action configuration `{import.Name}`.");
                }
                var configuration = import.Provider.PrimaryOptions.FindActionConfiguration(import.Name)
                    ?? throw new InvalidOperationException(
                        $"Project `{import.Provider.Name}` does not export action configuration `{import.Name}`.");
                configurations.Add(configuration);
            }
            project.SetConfiguredOptions(project.PrimaryOptions with { ActionConfigurations = configurations.ToArray() });
        }

        var discovered = new List<BuildTargetDefinition>();
        foreach(var project in _projects)
        {
            discovered.AddRange(new TargetDiscovery(new ITargetRulesProvider[] { project.RulesProvider }).DiscoverTargets(project));
        }

        var targetMap = discovered.ToDictionary(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase);
        Targets = discovered
            .Select(target => target with
            {
                Dependencies = target.Dependencies.Select(dependency => ResolveDependency(target, dependency, targetMap)).ToArray(),
            })
            .OrderBy(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private void ValidateBuildDirectories()
    {
        var duplicate = _projects
            .GroupBy(project => project.BuildDirectory, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault(group => group.Count() > 1);
        if(duplicate is not null)
        {
            throw new InvalidOperationException(
                $"Projects {string.Join(", ", duplicate.Select(project => $"`{project.Name}`"))} use the same build directory `{duplicate.Key}`.");
        }
    }

    private string ResolveDependency(
        BuildTargetDefinition target,
        string dependency,
        IReadOnlyDictionary<string, BuildTargetDefinition> targetMap)
    {
        var owner = _projectsByName[target.ProjectName];
        string qualifiedName;
        if(dependency.Contains('.', StringComparison.Ordinal))
        {
            var parts = dependency.Split('.');
            if(parts.Length != 2 || parts.Any(string.IsNullOrWhiteSpace))
            {
                throw new InvalidOperationException($"Target `{target.QualifiedName}` has invalid qualified dependency `{dependency}`.");
            }
            if(!_projectsByName.TryGetValue(parts[0], out var referencedProject) || !IsVisible(owner, referencedProject))
            {
                throw new InvalidOperationException(
                    $"Target `{target.QualifiedName}` cannot reference `{dependency}` because project `{parts[0]}` is not in `{owner.Name}`'s import subtree.");
            }
            qualifiedName = referencedProject.Name + "." + parts[1];
        }
        else
        {
            qualifiedName = owner.Name + "." + dependency;
        }

        if(!targetMap.ContainsKey(qualifiedName))
        {
            throw new InvalidOperationException($"Target `{target.QualifiedName}` depends on unknown or unsupported target `{qualifiedName}`.");
        }
        return qualifiedName;
    }

    private static bool IsVisible(Project owner, Project referencedProject)
    {
        if(ReferenceEquals(owner, referencedProject))
        {
            return true;
        }
        return owner.ImportedProjects.Any(imported => IsVisible(imported, referencedProject));
    }

    private static void ValidateName(string name, string kind)
    {
        if(string.IsNullOrWhiteSpace(name) || name.Contains('.', StringComparison.Ordinal) ||
            name.Contains('/') || name.Contains('\\') || name.Contains("..", StringComparison.Ordinal))
        {
            throw new InvalidOperationException(
                $"Invalid {kind} name `{name}`. Names cannot be empty or contain `.`, `..`, or path separators.");
        }
    }

    private static string StableHash(string value)
    {
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(value))).ToLowerInvariant()[..12];
    }

    private static string Sanitize(string value)
    {
        return string.Concat(value.Select(character => char.IsLetterOrDigit(character) || character is '-' or '_' ? character : '_'));
    }
}

internal static class ProjectConfigurationIdentity
{
    public static string Create(BuildOptions options)
    {
        var value = string.Join('\n', new[]
        {
            options.Platform.ToString(),
            options.Architecture,
            options.Mode.ToString(),
            options.Shared.ToString(),
            options.RhiApi.ToString(),
        }
        .Concat(AppleConfigurationIdentity(options))
        .Concat(options.Properties.Values.Select(property => $"property:{property.Name}={property.Value}"))
        .Concat(options.GlobalDefines.Select(define => "define:" + define))
        .Concat(options.GlobalUndefines.Select(undefine => "undefine:" + undefine))
        .Concat(options.GlobalIncludeDirectories.Select(include => "include:" + include))
        .Concat(new[] { "library_prefix:" + options.LibraryPrefix })
        .Concat(options.ActionConfigurations.Select(configuration =>
            "action:" + configuration.Name + ":provider=" + configuration.ProviderProjectName +
            ":root=" + configuration.ProviderProjectRootDirectory +
            ":build=" + configuration.ProviderProjectBuildDirectory + ":" +
            string.Join(',', configuration.ProviderProperties.Values.Select(
                property => property.Name + "=" + property.Value)) + ":" +
            string.Join(',', configuration.Targets.OrderBy(pair => pair.Key).Select(pair => pair.Key + "=" + pair.Value)) + ":" +
            string.Join(',', configuration.Files.OrderBy(pair => pair.Key).Select(pair => pair.Key + "=" + pair.Value)) + ":" +
            string.Join(',', configuration.Directories.OrderBy(pair => pair.Key).Select(pair => pair.Key + "=" + pair.Value)) + ":" +
            string.Join(',', configuration.Values.OrderBy(pair => pair.Key).Select(pair => pair.Key + "=" + pair.Value)))));
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(value))).ToLowerInvariant()[..12];
    }

    private static IEnumerable<string> AppleConfigurationIdentity(BuildOptions options)
    {
        if(options.Platform == BuildPlatform.MacOS)
        {
            yield return "apple_sdk:macosx";
            yield return "macos_deployment_target:" + options.Apple.MacOSDeploymentTarget;
        }
        else if(options.Platform == BuildPlatform.IOS)
        {
            yield return "apple_sdk:" + BuildOutputLayout.AppleSdkName(options);
            yield return "ios_deployment_target:" + options.Apple.IOSDeploymentTarget;
            yield return "ios_bundle_identifier:" + options.Apple.IOSBundleIdentifier;
            yield return "ios_codesign_identity:" + options.Apple.IOSCodeSignIdentity;
            yield return "ios_provisioning_profile:" + options.Apple.IOSProvisioningProfile;
        }
    }
}
