namespace LunaBuild.Core.MakeSystem;

public sealed class LunaMetaActionExecutor : KnownActionExecutor
{
    private readonly TimeSpan _actionTimeout;

    public LunaMetaActionExecutor(TimeSpan? actionTimeout = null)
        : base("luna.meta")
    {
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public override async Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var outputDirectory = context.Workspace.ResolveRepositoryPath(payload.Required("output_dir"));
        var stamp = context.Workspace.ResolveRepositoryPath(payload.Required("stamp"));
        var depfile = context.Workspace.ResolveRepositoryPath(payload.Required("depfile"));

        Directory.CreateDirectory(outputDirectory);
        Directory.CreateDirectory(Path.GetDirectoryName(stamp)!);
        Directory.CreateDirectory(Path.GetDirectoryName(depfile)!);

        var tool = LocateLunaMetaTool(context.Workspace, context.Graph.Options);
        var args = new List<string>
        {
            "--output-dir",
            outputDirectory,
            "--stamp",
            stamp,
            "--depfile",
            depfile,
            "--mode",
            payload.Required("mode"),
            "--platform",
            payload.Required("platform"),
            "--arch",
            payload.Required("arch"),
        };

        foreach(var header in payload.All("header"))
        {
            args.Add("--header");
            args.Add(context.Workspace.ResolveRepositoryPath(header));
        }
        foreach(var include in payload.All("include"))
        {
            args.Add("--include");
            args.Add(context.Workspace.ResolveRepositoryPath(include));
        }
        foreach(var define in payload.All("define"))
        {
            args.Add("--define");
            args.Add(define);
        }
        foreach(var undefine in payload.All("undefine"))
        {
            args.Add("--undefine");
            args.Add(undefine);
        }

        var result = await ProcessRunner.RunAsync(tool, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException(FormatToolFailure(
                "LunaMetaTool failed",
                context.Workspace.RootDirectory,
                tool,
                args,
                result.Output));
        }
    }

    private static string LocateLunaMetaTool(BuildWorkspace workspace, BuildOptions options)
    {
        var executable = OperatingSystem.IsWindows() ? "LunaMetaTool.exe" : "LunaMetaTool";
        var candidates = new[]
        {
            Path.Combine(workspace.RootDirectory, "SDKs", "LunaMetaTool", "macosx", "arm64", "bin", executable),
            Path.Combine(workspace.RootDirectory, "SDKs", "LunaMetaTool", "windows", "x64", "bin", executable),
            Path.Combine(workspace.RootDirectory, "Tools", "LunaMetaTool", "bin", executable),
            Path.Combine(workspace.BuildDirectory, options.Platform.ToString(), options.Architecture, options.Mode.ToString(), "bin", executable),
            Path.Combine(workspace.BuildDirectory, "MacOS", "arm64", "Debug", "bin", executable),
            Path.Combine(workspace.BuildDirectory, "Windows", "x64", "Debug", "bin", executable),
        };
        foreach(var candidate in candidates)
        {
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        throw new MakeSystemException(
            "LunaMetaTool is missing. Expected one of:" +
            string.Concat(candidates.Select(path => $"{Environment.NewLine}  {path}")));
    }

    private static string FormatToolFailure(
        string title,
        string workingDirectory,
        string tool,
        IReadOnlyList<string> arguments,
        string output)
    {
        return title + Environment.NewLine +
            $"  working directory: {workingDirectory}{Environment.NewLine}" +
            $"  tool: {tool}{Environment.NewLine}" +
            "  arguments:" + Environment.NewLine +
            string.Concat(arguments.Take(256).Select(argument => $"    {argument}{Environment.NewLine}")) +
            (arguments.Count > 256 ? $"    ... {arguments.Count - 256} more{Environment.NewLine}" : string.Empty) +
            "  process output:" + Environment.NewLine +
            (output.Length == 0 ? "<empty>" : output).TrimEnd();
    }
}
