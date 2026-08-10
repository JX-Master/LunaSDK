namespace LunaBuild.Core;

public sealed class BuildGraphGenerator
{
    public BuildGraph GenerateTargetInspectionGraph(
        BuildWorkspace workspace,
        BuildOptions options,
        IReadOnlyList<BuildTargetDefinition> targets)
    {
        var nodes = new List<BuildGraphNode>();
        var targetNames = targets.Select(target => target.QualifiedName).ToHashSet(StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets)
        {
            var dependencies = target.Dependencies
                .Where(targetNames.Contains)
                .Select(BuildGraphIds.Target)
                .OrderBy(id => id, StringComparer.OrdinalIgnoreCase)
                .ToArray();

            nodes.Add(new BuildGraphNode(
                Id: BuildGraphIds.Target(target.QualifiedName),
                Kind: BuildGraphNodeKind.Virtual,
                Path: workspace.ToRepositoryRelativePath(target.Directory),
                Command: BuildTargetCommandDescription(workspace, target),
                Dependencies: dependencies,
                OrderOnlyDependencies: Array.Empty<string>(),
                Outputs: Array.Empty<string>(),
                Depfiles: Array.Empty<string>())
            {
                ProjectName = target.ProjectName,
                ConfigurationId = target.ConfigurationId,
                Options = target.Options,
            });
        }

        nodes.Add(new BuildGraphNode(
            Id: BuildGraphIds.AllTargets,
            Kind: BuildGraphNodeKind.Phony,
            Path: null,
            Command: null,
            Dependencies: targets.Where(target => target.IsHostProject).Select(target => BuildGraphIds.Target(target.QualifiedName)).ToArray(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>()));

        return new BuildGraph(
            Version: 2,
            Options: options,
            Nodes: nodes,
            Targets: new[] { BuildGraphIds.AllTargets }).AddMetadata(targets);
    }

    private static string BuildTargetCommandDescription(
        BuildWorkspace workspace,
        BuildTargetDefinition target)
    {
        var lines = new List<string>
        {
            "kind=target.inspect",
            $"name={target.QualifiedName}",
            $"script={workspace.ToRepositoryRelativePath(target.ScriptPath)}",
            $"mode={target.Options.Mode}",
            $"platform={target.Options.Platform}",
            $"arch={target.Options.Architecture}",
            $"shared={target.Options.Shared}",
            $"category={target.Category}",
            $"rhi={target.Options.RhiApi}",
        };
        foreach(var property in target.Options.Properties.Values.OrderBy(property => property.Name, StringComparer.OrdinalIgnoreCase))
        {
            lines.Add($"project_property.{property.Name}={property.Value}");
        }
        if(target.Options.Platform == BuildPlatform.MacOS)
        {
            lines.Add($"macos_deployment_target={target.Options.Apple.MacOSDeploymentTarget}");
        }
        else if(target.Options.Platform == BuildPlatform.IOS)
        {
            lines.Add($"apple_sdk={BuildOutputLayout.AppleSdkName(target.Options)}");
            lines.Add($"ios_deployment_target={target.Options.Apple.IOSDeploymentTarget}");
        }
        return string.Join('\n', lines);
    }
}

public static class BuildGraphIds
{
    public const string AllTargets = "phony://targets/all";

    public static string Target(string name) => $"target://{name}";

    public static string File(string path) => $"file://{path.Replace('\\', '/')}";
}
