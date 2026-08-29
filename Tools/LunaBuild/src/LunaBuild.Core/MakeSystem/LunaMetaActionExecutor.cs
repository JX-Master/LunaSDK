namespace LunaBuild.Core.MakeSystem;

public sealed class LunaMetaActionExecutor : KnownActionExecutor
{
    private readonly TimeSpan _actionTimeout;

    public LunaMetaActionExecutor(TimeSpan? actionTimeout = null)
        : base("luna.meta")
    {
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public override string GetDescription(MakeActionContext context)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        return $"generating metadata for {payload.Required("target")}";
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

        var tool = context.Workspace.ResolveRepositoryPath(payload.Required("tool"));
        if(!File.Exists(tool))
        {
            throw new MakeSystemException($"Metadata tool was not built or does not exist: {tool}");
        }
        var args = new List<string>
        {
            "--output-dir",
            outputDirectory,
            "--stamp",
            stamp,
            "--depfile",
            depfile,
            "--target",
            payload.Contains("target_name") ? payload.Required("target_name") : payload.Required("target"),
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
        foreach(var language in payload.All("header_language"))
        {
            args.Add("--header-language");
            args.Add(language);
        }
        if((context.Options.Platform is BuildPlatform.MacOS or BuildPlatform.IOS) && OperatingSystem.IsMacOS())
        {
            var appleToolchain = AppleClangToolchainLocator.Locate(CppCommandLineBuilder.AppleSdkName(context.Options));
            args.Add("--isysroot");
            args.Add(appleToolchain.SdkPath);
            args.Add("--resource-dir");
            args.Add(payload.Contains("resource_dir")
                ? context.Workspace.ResolveRepositoryPath(payload.Required("resource_dir"))
                : await LocateClangResourceDirectoryAsync(context.Workspace, appleToolchain.Clang, cancellationToken));
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

    private async Task<string> LocateClangResourceDirectoryAsync(
        BuildWorkspace workspace,
        string clang,
        CancellationToken cancellationToken)
    {
        var result = await ProcessRunner.RunAsync(
            clang,
            new[] { "-print-resource-dir" },
            workspace.RootDirectory,
            _actionTimeout,
            cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"Failed to locate clang resource directory:{Environment.NewLine}{result.Output}");
        }
        var path = result.Output
            .Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries)
            .Select(line => line.Trim())
            .LastOrDefault(line => Path.IsPathFullyQualified(line));
        if(string.IsNullOrWhiteSpace(path))
        {
            throw new MakeSystemException("clang -print-resource-dir returned an empty path.");
        }
        return path;
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
