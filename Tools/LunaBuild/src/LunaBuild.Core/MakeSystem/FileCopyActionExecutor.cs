namespace LunaBuild.Core.MakeSystem;

public sealed class FileCopyActionExecutor : KnownActionExecutor
{
    public FileCopyActionExecutor()
        : base("file.copy")
    {
    }

    public override Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));

        cancellationToken.ThrowIfCancellationRequested();
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);
        File.Copy(source, output, overwrite: true);
        File.SetLastWriteTimeUtc(output, File.GetLastWriteTimeUtc(source));
        return Task.CompletedTask;
    }
}
