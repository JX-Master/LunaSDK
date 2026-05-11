namespace LunaBuild.Core;

public sealed class BuildGraphGenerator
{
    public BuildGraph GenerateTargetInspectionGraph(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets)
    {
        var nodes = new List<BuildGraphNode>();
        var targetNames = targets.Select(target => target.Name).ToHashSet(StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets)
        {
            var dependencies = target.Dependencies
                .Where(targetNames.Contains)
                .Select(BuildGraphIds.Target)
                .OrderBy(id => id, StringComparer.OrdinalIgnoreCase)
                .ToArray();

            nodes.Add(new BuildGraphNode(
                Id: BuildGraphIds.Target(target.Name),
                Kind: BuildGraphNodeKind.Virtual,
                Path: workspace.ToRepositoryRelativePath(target.Directory),
                Command: BuildTargetCommandDescription(workspace, options, target),
                Dependencies: dependencies,
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>()));
        }

        nodes.Add(new BuildGraphNode(
            Id: BuildGraphIds.AllTargets,
            Kind: BuildGraphNodeKind.Phony,
            Path: null,
            Command: null,
            Dependencies: targets.Select(target => BuildGraphIds.Target(target.Name)).ToArray(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>()));

        return new BuildGraph(
            Version: 1,
            Options: options,
            Nodes: nodes,
            Targets: new[] { BuildGraphIds.AllTargets });
    }

    private static string BuildTargetCommandDescription(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildTargetDefinition target)
    {
        return string.Join('\n',
            "kind=target.inspect",
            $"name={target.Name}",
            $"script={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"mode={options.Mode}",
            $"platform={options.Platform}",
            $"arch={options.Architecture}",
            $"shared={options.Shared}",
            $"rhi={options.RhiApi}");
    }
}

public static class BuildGraphIds
{
    public const string AllTargets = "phony://targets/all";

    public static string Target(string name) => $"target://{name}";

    public static string File(string path) => $"file://{path.Replace('\\', '/')}";
}
