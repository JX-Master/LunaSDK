namespace LunaBuild.Core.MakeSystem;

using System.Text.Json;

public sealed class CppActionExecutor : IMakeActionExecutor
{
    private readonly MsvcToolchain _toolchain;
    private readonly TimeSpan _actionTimeout;

    public CppActionExecutor(TimeSpan? actionTimeout = null)
    {
        _toolchain = MsvcToolchainLocator.Locate();
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public bool CanExecute(string actionKind)
    {
        return actionKind is "cpp.compile" or "rc.compile" or "cpp.link.shared" or "cpp.link.static" or "cpp.link.executable";
    }

    public Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return context.ActionKind switch
        {
            "cpp.compile" => CompileAsync(context, cancellationToken),
            "rc.compile" => CompileResourceAsync(context, cancellationToken),
            "cpp.link.shared" => LinkSharedAsync(context, cancellationToken),
            "cpp.link.static" => LinkStaticAsync(context, cancellationToken),
            "cpp.link.executable" => LinkExecutableAsync(context, cancellationToken),
            _ => throw new MissingMakeActionExecutorException(context.ActionKind, context.Node.Id),
        };
    }

    private async Task CompileAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        if(context.Workspace.OptionsHostPlatform() != BuildPlatform.Windows)
        {
            throw new MakeSystemException("The initial C++ executor only supports Windows/MSVC.");
        }

        var payload = ActionPayload.Parse(context.ActionPayload);
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = context.Workspace.ResolveRepositoryPath(payload.Required("depfile"));
        var sourceDependencies = depfile + ".json";
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);
        Directory.CreateDirectory(Path.GetDirectoryName(depfile)!);

        var args = new List<string>
        {
            "/c",
            "/nologo",
            RuntimeFlag(payload.Contains("runtime") ? payload.Required("runtime") : payload.Required("mode")),
            "/Zi",
            "/FS",
            $"/Fd{Quote(Path.Combine(context.Workspace.BuildDirectory, context.OptionsDirectoryName(), "compile.LunaBuild.pdb"))}",
            OptimizationFlag(payload.Required("mode")),
            "/D_WINDOWS",
            "/DUNICODE",
            "/D_UNICODE",
            "/DNOMINMAX",
            "/D_CRT_SECURE_NO_WARNINGS",
            $"/sourceDependencies {Quote(sourceDependencies)}",
        };
        if(payload.Required("language").Equals("c", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("/TC");
        }
        else
        {
            args.Add("/TP");
            args.Add("/std:c++20");
        }

        foreach(var include in payload.All("include"))
        {
            args.Add($"/I{Quote(context.Workspace.ResolveRepositoryPath(include))}");
        }
        foreach(var define in payload.All("define"))
        {
            args.Add($"/D{define}");
        }
        foreach(var undefine in payload.All("undefine"))
        {
            args.Add($"/U{undefine}");
        }
        if(payload.Required("mode").Equals("Debug", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("/DLUNA_ENABLE_API_VALIDATION");
        }

        args.Add($"/Fo{Quote(output)}");
        args.Add(Quote(source));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "cl", args);
        var result = await RunVsToolAsync(context.Workspace, _toolchain.ClExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ compile failed for {source}:{Environment.NewLine}{result.Output}");
        }

        WriteDepfile(depfile, output, source, ReadSourceDependencies(sourceDependencies));
    }

    private async Task LinkSharedAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var importLib = Path.ChangeExtension(output, ".lib");
        var pdb = Path.ChangeExtension(output, ".pdb");

        var args = new List<string>
        {
            "/dll",
            "/nologo",
            "/machine:x64",
            "/debug",
            $"/pdb:{Quote(pdb)}",
            $"/out:{Quote(output)}",
        };

        args.AddRange(context.Dependencies
            .Where(node => node.Kind == BuildGraphNodeKind.File && node.Path is not null && IsLinkInput(node.Path))
            .Select(node => Quote(context.Workspace.ResolveRepositoryPath(node.Path!))));
        args.AddRange(payload.All("library"));
        args.Add($"/implib:{Quote(importLib)}");

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "link", args);
        var result = await RunVsToolAsync(context.Workspace, _toolchain.LinkExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task CompileResourceAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        if(context.Workspace.OptionsHostPlatform() != BuildPlatform.Windows)
        {
            throw new MakeSystemException("The initial resource executor only supports Windows/MSVC.");
        }

        var payload = ActionPayload.Parse(context.ActionPayload);
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = new List<string>
        {
            "/nologo",
            $"/fo{Quote(output)}",
        };
        foreach(var include in payload.All("include"))
        {
            args.Add($"/i{Quote(context.Workspace.ResolveRepositoryPath(include))}");
        }
        args.Add(Quote(source));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "rc", args);
        var result = await RunVsToolAsync(context.Workspace, "rc.exe", rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"Resource compile failed for {source}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkStaticAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = new List<string>
        {
            "/nologo",
            $"/out:{Quote(output)}",
        };
        args.AddRange(context.Dependencies
            .Where(node => node.Kind == BuildGraphNodeKind.File && node.Path is not null && IsLinkInput(node.Path))
            .Select(node => Quote(context.Workspace.ResolveRepositoryPath(node.Path!))));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "lib", args);
        var result = await RunVsToolAsync(context.Workspace, _toolchain.LibExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ static link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkExecutableAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var pdb = Path.ChangeExtension(output, ".pdb");
        var args = new List<string>
        {
            "/nologo",
            "/machine:x64",
            "/debug",
            $"/pdb:{Quote(pdb)}",
            $"/out:{Quote(output)}",
        };

        args.AddRange(context.Dependencies
            .Where(node => node.Kind == BuildGraphNodeKind.File && node.Path is not null && IsLinkInput(node.Path))
            .Select(node => Quote(context.Workspace.ResolveRepositoryPath(node.Path!))));
        args.AddRange(payload.All("library"));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "link", args);
        var result = await RunVsToolAsync(context.Workspace, _toolchain.LinkExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ executable link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task<ProcessRunResult> RunVsToolAsync(BuildWorkspace workspace, string tool, string responseFile, CancellationToken cancellationToken)
    {
        var command = $"call {Quote(_toolchain.VcVarsBat)} >nul && {Quote(tool)} @{Quote(responseFile)}";
        return await ProcessRunner.RunAsync("cmd.exe", $"/d /s /c \"{command}\"", workspace.RootDirectory, _actionTimeout, cancellationToken);
    }

    private static string WriteResponseFile(BuildWorkspace workspace, string nodeId, string toolName, IEnumerable<string> args)
    {
        var safeName = string.Concat(nodeId.Select(ch => char.IsLetterOrDigit(ch) ? ch : '_'));
        var directory = Path.Combine(workspace.BuildDirectory, "MakeSystem", "rsp");
        Directory.CreateDirectory(directory);
        var path = Path.Combine(directory, $"{safeName}.{toolName}.rsp");
        File.WriteAllLines(path, args);
        return path;
    }

    private static string Quote(string value)
    {
        return value.Contains(' ') || value.Contains('\t')
            ? $"\"{value}\""
            : value;
    }

    private static string RuntimeFlag(string value)
    {
        return value.ToUpperInvariant() switch
        {
            "DEBUG" or "MDD" => "/MDd",
            "PROFILE" or "RELEASE" or "MD" => "/MD",
            "MTD" => "/MTd",
            "MT" => "/MT",
            _ => throw new MakeSystemException($"Unsupported MSVC runtime library: {value}"),
        };
    }

    private static string OptimizationFlag(string mode)
    {
        return mode.Equals("Release", StringComparison.OrdinalIgnoreCase) ? "/O2" : "/Od";
    }

    private static void WriteDepfile(string depfile, string output, string source, IReadOnlyList<string> includedFiles)
    {
        var dependencies = new[] { source }
            .Concat(includedFiles)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Select(EscapeDepfilePath);
        File.WriteAllText(depfile, $"{EscapeDepfilePath(output)}: {string.Join(' ', dependencies)}{Environment.NewLine}");
    }

    private static string EscapeDepfilePath(string path)
    {
        return path.Replace("\\", "/").Replace(" ", "\\ ");
    }

    private static bool IsLinkInput(string path)
    {
        return path.EndsWith(".obj", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".res", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".lib", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".a", StringComparison.OrdinalIgnoreCase);
    }

    private static IReadOnlyList<string> ReadSourceDependencies(string path)
    {
        if(!File.Exists(path))
        {
            return Array.Empty<string>();
        }

        var ret = new List<string>();
        using var document = JsonDocument.Parse(File.ReadAllText(path));
        if(!document.RootElement.TryGetProperty("Data", out var data) ||
            !data.TryGetProperty("Includes", out var includes) ||
            includes.ValueKind != JsonValueKind.Array)
        {
            return ret;
        }

        foreach(var include in includes.EnumerateArray())
        {
            var value = include.GetString();
            if(!string.IsNullOrWhiteSpace(value))
            {
                ret.Add(value);
            }
        }
        return ret;
    }
}

internal static class BuildWorkspaceExtensions
{
    public static BuildPlatform OptionsHostPlatform(this BuildWorkspace _)
    {
        return OperatingSystem.IsWindows()
            ? BuildPlatform.Windows
            : OperatingSystem.IsMacOS()
                ? BuildPlatform.MacOS
                : BuildPlatform.Linux;
    }

    public static string OptionsDirectoryName(this MakeActionContext context)
    {
        return Path.Combine(
            context.Graph.Options.Platform.ToString(),
            context.Graph.Options.Architecture,
            context.Graph.Options.Mode.ToString());
    }
}
