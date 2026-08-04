namespace LunaBuild.Core;

public enum BuildGraphNodeKind
{
    File,
    Phony,
    Virtual,
}

public sealed record BuildGraphNode(
    string Id,
    BuildGraphNodeKind Kind,
    string? Path,
    string? Command,
    IReadOnlyList<string> Dependencies,
    IReadOnlyList<string> OrderOnlyDependencies,
    IReadOnlyList<string> Outputs,
    IReadOnlyList<string> Depfiles)
{
    public string ProjectName { get; init; } = string.Empty;

    public string ConfigurationId { get; init; } = string.Empty;

    public BuildOptions? Options { get; init; }
}

public sealed record BuildGraph(
    int Version,
    BuildOptions Options,
    IReadOnlyList<BuildGraphNode> Nodes,
    IReadOnlyList<string> Targets)
{
    public IReadOnlyList<BuildGraphProject> Projects { get; init; } = Array.Empty<BuildGraphProject>();

    public IReadOnlyList<BuildGraphConfiguration> Configurations { get; init; } = Array.Empty<BuildGraphConfiguration>();
}

public sealed record BuildGraphProject(
    string Name,
    string RootDirectory,
    string BuildDirectory,
    bool IsHost);

public sealed record BuildGraphConfiguration(
    string Id,
    string ProjectName,
    BuildOptions Options);

internal static class BuildGraphMetadata
{
    public static BuildGraph AddMetadata(this BuildGraph graph, IReadOnlyList<BuildTargetDefinition> targets)
    {
        return graph with
        {
            Projects = targets
                .GroupBy(target => target.ProjectName, StringComparer.OrdinalIgnoreCase)
                .Select(group => group.First())
                .Select(target => new BuildGraphProject(
                    target.ProjectName,
                    target.ProjectRootDirectory,
                    target.ProjectBuildDirectory,
                    target.IsHostProject))
                .OrderBy(project => project.Name, StringComparer.OrdinalIgnoreCase)
                .ToArray(),
            Configurations = targets
                .GroupBy(target => (target.ProjectName, target.ConfigurationId))
                .Select(group => group.First())
                .Select(target => new BuildGraphConfiguration(
                    target.ConfigurationId,
                    target.ProjectName,
                    target.Options))
                .OrderBy(configuration => configuration.ProjectName, StringComparer.OrdinalIgnoreCase)
                .ThenBy(configuration => configuration.Id, StringComparer.Ordinal)
                .ToArray(),
        };
    }

    public static BuildGraph MergeMetadata(
        this BuildGraph graph,
        IReadOnlyList<BuildGraphProject> projects,
        IReadOnlyList<BuildGraphConfiguration> configurations)
    {
        return graph with
        {
            Projects = graph.Projects
                .Concat(projects)
                .GroupBy(project => project.Name, StringComparer.OrdinalIgnoreCase)
                .Select(group => group.OrderByDescending(project => project.IsHost).First())
                .OrderBy(project => project.Name, StringComparer.OrdinalIgnoreCase)
                .ToArray(),
            Configurations = graph.Configurations
                .Concat(configurations)
                .GroupBy(configuration => (configuration.ProjectName, configuration.Id))
                .Select(group => group.First())
                .OrderBy(configuration => configuration.ProjectName, StringComparer.OrdinalIgnoreCase)
                .ThenBy(configuration => configuration.Id, StringComparer.Ordinal)
                .ToArray(),
        };
    }
}
