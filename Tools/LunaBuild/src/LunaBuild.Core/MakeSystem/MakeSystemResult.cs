namespace LunaBuild.Core.MakeSystem;

public sealed record MakeSystemResult(
    int NodesVisited,
    int ActionsExecuted,
    bool UpToDate);

public class MakeSystemException : Exception
{
    public MakeSystemException(string message)
        : base(message)
    {
    }
}

public sealed class MissingMakeActionExecutorException : MakeSystemException
{
    public MissingMakeActionExecutorException(string actionKind, string nodeId)
        : base($"No MakeSystem action executor registered for `{actionKind}` on node `{nodeId}`.")
    {
        ActionKind = actionKind;
        NodeId = nodeId;
    }

    public string ActionKind { get; }

    public string NodeId { get; }
}
