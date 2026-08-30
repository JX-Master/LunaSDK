namespace LunaBuild.Core.MakeSystem;

public sealed class DotNetBuildActionExecutor : KnownActionExecutor
{
    private readonly TimeSpan _actionTimeout;

    public DotNetBuildActionExecutor(TimeSpan? actionTimeout = null)
        : base("dotnet.build")
    {
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public override string GetDescription(MakeActionContext context)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var project = context.Workspace.ResolveRepositoryPath(payload.Required("project"));
        return $"building {Path.GetFileName(project)}";
    }

    public override async Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var project = context.Workspace.ResolveRepositoryPath(payload.Required("project"));
        var expectedOutput = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        var artifactsDirectory = context.Workspace.ResolveRepositoryPath(payload.Required("artifacts_dir"));
        var configuration = payload.Required("mode");
        var dotnet = LocateDotnet();
        var args = new[]
        {
            "build",
            project,
            "--configuration",
            configuration,
            "--artifacts-path",
            artifactsDirectory,
            "-m:1",
            "/nr:false",
            "--nologo",
            "-p:UseSharedCompilation=false",
        };

        var result = await ProcessRunner.RunAsync(dotnet, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($".NET build failed for {project}:{Environment.NewLine}{result.Output}");
        }
        if(!File.Exists(expectedOutput))
        {
            throw new MakeSystemException($".NET build did not produce expected output: {expectedOutput}");
        }
    }

    private static string LocateDotnet()
    {
        var executable = OperatingSystem.IsWindows() ? "dotnet.exe" : "dotnet";
        var pathValue = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;
        foreach(var pathEntry in pathValue.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = Path.Combine(pathEntry, executable);
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        return "dotnet";
    }
}
