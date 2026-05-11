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
        var targets = new Dictionary<string, BuildTargetDefinition>(StringComparer.OrdinalIgnoreCase);

        foreach(var provider in _rulesProviders)
        {
            foreach(var rules in provider.GetTargetRules(workspace))
            {
                if(!rules.SupportsPlatform(options.Platform))
                {
                    continue;
                }
                targets[rules.Name] = rules.ToDefinition(workspace, options);
            }
        }

        IEnumerable<BuildTargetDefinition> discoveredTargets = targets.Values;
        if(!options.BuildTests)
        {
            discoveredTargets = discoveredTargets.Where(target => !target.IsTest);
        }

        return discoveredTargets
            .OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }
}
