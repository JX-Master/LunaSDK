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
    IReadOnlyList<string> Depfiles);

public sealed record BuildGraph(
    int Version,
    BuildOptions Options,
    IReadOnlyList<BuildGraphNode> Nodes,
    IReadOnlyList<string> Targets);
