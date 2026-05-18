namespace LunaBuild.Core.MakeSystem;

public sealed class AggregateActionExecutor : IMakeActionExecutor
{
    public bool CanExecute(string actionKind)
    {
        return actionKind is "target.cpp" or "target.dotnet" or "target.external" or "target.inspect";
    }

    public Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return Task.CompletedTask;
    }
}
