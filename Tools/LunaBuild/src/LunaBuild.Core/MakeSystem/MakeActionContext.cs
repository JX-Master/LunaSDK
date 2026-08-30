namespace LunaBuild.Core.MakeSystem;

public sealed record MakeActionContext(
    BuildWorkspace Workspace,
    BuildGraph Graph,
    BuildGraphNode Node,
    string ActionKind,
    string ActionPayload,
    IReadOnlyList<BuildGraphNode> Dependencies,
    IReadOnlyList<BuildGraphNode> Outputs,
    IReadOnlyList<BuildGraphNode> Depfiles)
{
    public BuildOptions Options => Node.Options ?? Graph.Options;
}

public interface IMakeActionExecutor
{
    bool CanExecute(string actionKind);

    string? GetDescription(MakeActionContext context) => context.ActionKind;

    Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken);
}
