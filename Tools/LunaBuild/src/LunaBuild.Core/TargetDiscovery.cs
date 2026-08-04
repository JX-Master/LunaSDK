namespace LunaBuild.Core;

public sealed class TargetDiscovery
{
    private readonly IReadOnlyList<ITargetRulesProvider> _rulesProviders;

    public TargetDiscovery()
        : this(new ITargetRulesProvider[] { new ProjectTargetRulesProvider() })
    {
    }

    public TargetDiscovery(IReadOnlyList<ITargetRulesProvider> rulesProviders)
    {
        _rulesProviders = rulesProviders;
    }

    public IReadOnlyList<BuildTargetDefinition> DiscoverTargets(BuildWorkspace workspace, BuildOptions options)
    {
        return DiscoverTargets(workspace, options, projectName: null, configurationId: null, isHostProject: true);
    }

    public IReadOnlyList<BuildTargetDefinition> DiscoverTargets(Project project)
    {
        return DiscoverTargets(
            project.Workspace,
            project.PrimaryOptions,
            project.Name,
            project.ConfigurationId,
            project.IsHost);
    }

    private IReadOnlyList<BuildTargetDefinition> DiscoverTargets(
        BuildWorkspace workspace,
        BuildOptions options,
        string? projectName,
        string? configurationId,
        bool isHostProject)
    {
        var targets = new Dictionary<string, BuildTargetDefinition>(StringComparer.OrdinalIgnoreCase);

        foreach(var provider in _rulesProviders)
        {
            foreach(var rules in provider.GetTargetRules(workspace))
            {
                if(!rules.SupportsPlatform(options.Platform))
                {
                    continue;
                }
                ValidateTargetName(rules.Name);
                if(targets.ContainsKey(rules.Name))
                {
                    throw new InvalidOperationException($"Project `{projectName ?? workspace.RootDirectory}` declares target `{rules.Name}` more than once.");
                }
                targets.Add(rules.Name, rules.ToDefinition(workspace, options, projectName, configurationId, isHostProject));
            }
        }

        return targets.Values
            .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private static void ValidateTargetName(string name)
    {
        if(string.IsNullOrWhiteSpace(name) || name.Contains('.', StringComparison.Ordinal) ||
            name.Contains('/') || name.Contains('\\') || name.Contains("..", StringComparison.Ordinal))
        {
            throw new InvalidOperationException(
                $"Invalid target name `{name}`. Names cannot be empty or contain `.`, `..`, or path separators.");
        }
    }
}
