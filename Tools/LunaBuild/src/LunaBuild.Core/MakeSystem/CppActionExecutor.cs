namespace LunaBuild.Core.MakeSystem;

using System.Text.Json;

public sealed class CppActionExecutor : IMakeActionExecutor
{
    private readonly Lazy<MsvcToolchain> _msvcToolchain = new(MsvcToolchainLocator.Locate);
    private readonly Lazy<AppleClangToolchain> _appleToolchain = new(AppleClangToolchainLocator.LocateMacOS);
    private readonly TimeSpan _actionTimeout;

    public CppActionExecutor(TimeSpan? actionTimeout = null)
    {
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

    private Task CompileAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return context.Graph.Options.Platform switch
        {
            BuildPlatform.Windows => CompileMsvcAsync(context, cancellationToken),
            BuildPlatform.MacOS => CompileAppleAsync(context, cancellationToken),
            _ => throw new MakeSystemException($"C++ compile is not implemented for platform {context.Graph.Options.Platform}."),
        };
    }

    private Task LinkSharedAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return context.Graph.Options.Platform switch
        {
            BuildPlatform.Windows => LinkMsvcSharedAsync(context, cancellationToken),
            BuildPlatform.MacOS => LinkAppleSharedAsync(context, cancellationToken),
            _ => throw new MakeSystemException($"C++ shared linking is not implemented for platform {context.Graph.Options.Platform}."),
        };
    }

    private Task LinkStaticAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return context.Graph.Options.Platform switch
        {
            BuildPlatform.Windows => LinkMsvcStaticAsync(context, cancellationToken),
            BuildPlatform.MacOS => LinkAppleStaticAsync(context, cancellationToken),
            _ => throw new MakeSystemException($"C++ static linking is not implemented for platform {context.Graph.Options.Platform}."),
        };
    }

    private Task LinkExecutableAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        return context.Graph.Options.Platform switch
        {
            BuildPlatform.Windows => LinkMsvcExecutableAsync(context, cancellationToken),
            BuildPlatform.MacOS => LinkAppleExecutableAsync(context, cancellationToken),
            _ => throw new MakeSystemException($"C++ executable linking is not implemented for platform {context.Graph.Options.Platform}."),
        };
    }

    private async Task CompileMsvcAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        if(context.Workspace.OptionsHostPlatform() != BuildPlatform.Windows)
        {
            throw new MakeSystemException("Windows/MSVC C++ compilation requires a Windows host.");
        }

        var payload = ActionPayload.Parse(context.ActionPayload);
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = context.Workspace.ResolveRepositoryPath(payload.Required("depfile"));
        var sourceDependencies = depfile + ".json";
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);
        Directory.CreateDirectory(Path.GetDirectoryName(depfile)!);

        var args = CppCommandLineBuilder.BuildMsvcCompileArguments(context.Workspace, context.Graph.Options, payload);
        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "cl", args);
        var result = await RunVsToolAsync(context.Workspace, _msvcToolchain.Value.ClExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ compile failed for {source}:{Environment.NewLine}{result.Output}");
        }

        WriteDepfile(depfile, output, source, ReadSourceDependencies(sourceDependencies));
    }

    private async Task CompileAppleAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        if(context.Workspace.OptionsHostPlatform() != BuildPlatform.MacOS)
        {
            throw new MakeSystemException("macOS/Apple clang C++ compilation requires a macOS host.");
        }

        var payload = ActionPayload.Parse(context.ActionPayload);
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = context.Workspace.ResolveRepositoryPath(payload.Required("depfile"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);
        Directory.CreateDirectory(Path.GetDirectoryName(depfile)!);

        var language = payload.Required("language");
        var args = CppCommandLineBuilder.BuildAppleCompileArguments(context.Workspace, payload, _appleToolchain.Value.SdkPath);
        var compiler = CppCommandLineBuilder.UsesCxxCompiler(language) ? _appleToolchain.Value.ClangCxx : _appleToolchain.Value.Clang;
        var result = await ProcessRunner.RunAsync(compiler, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ compile failed for {source}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task CompileResourceAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        if(context.Graph.Options.Platform != BuildPlatform.Windows || context.Workspace.OptionsHostPlatform() != BuildPlatform.Windows)
        {
            throw new MakeSystemException("Windows resource compilation requires a Windows/MSVC host and target.");
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

    private async Task LinkMsvcSharedAsync(MakeActionContext context, CancellationToken cancellationToken)
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

        args.AddRange(LinkInputPaths(context).Select(Quote));
        args.AddRange(payload.All("library"));
        args.Add($"/implib:{Quote(importLib)}");

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "link", args);
        var result = await RunVsToolAsync(context.Workspace, _msvcToolchain.Value.LinkExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkMsvcStaticAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = new List<string>
        {
            "/nologo",
            $"/out:{Quote(output)}",
        };
        args.AddRange(LinkInputPaths(context).Select(Quote));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "lib", args);
        var result = await RunVsToolAsync(context.Workspace, _msvcToolchain.Value.LibExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ static link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkMsvcExecutableAsync(MakeActionContext context, CancellationToken cancellationToken)
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

        args.AddRange(LinkInputPaths(context).Select(Quote));
        args.AddRange(payload.All("library"));

        var rsp = WriteResponseFile(context.Workspace, context.Node.Id, "link", args);
        var result = await RunVsToolAsync(context.Workspace, _msvcToolchain.Value.LinkExe, rsp, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ executable link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkAppleSharedAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = AppleLinkArgs(context, payload, output);
        args.Insert(0, "-dynamiclib");
        args.Add("-install_name");
        args.Add("@rpath/" + Path.GetFileName(output));

        var result = await ProcessRunner.RunAsync(_appleToolchain.Value.ClangCxx, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkAppleStaticAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = new List<string>
        {
            "-static",
            "-o",
            output,
        };
        args.AddRange(LinkInputPaths(context).Where(path => path.EndsWith(".o", StringComparison.OrdinalIgnoreCase) || path.EndsWith(".a", StringComparison.OrdinalIgnoreCase)));

        var result = await ProcessRunner.RunAsync(_appleToolchain.Value.Libtool, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ static link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task LinkAppleExecutableAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var output = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        Directory.CreateDirectory(Path.GetDirectoryName(output)!);

        var args = AppleLinkArgs(context, payload, output);
        var result = await ProcessRunner.RunAsync(_appleToolchain.Value.ClangCxx, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"C++ executable link failed for {output}:{Environment.NewLine}{result.Output}");
        }
    }

    private List<string> AppleLinkArgs(MakeActionContext context, ActionPayload payload, string output)
    {
        var args = new List<string>
        {
            "-arch",
            AppleArchitecture(payload.Required("arch")),
            "-isysroot",
            _appleToolchain.Value.SdkPath,
            "-o",
            output,
            "-Wl,-rpath,@loader_path",
        };
        args.AddRange(LinkInputPaths(context));
        foreach(var library in payload.All("library"))
        {
            AddAppleLibrary(args, library);
        }
        foreach(var framework in payload.All("framework"))
        {
            args.Add("-framework");
            args.Add(framework);
        }
        return args;
    }

    private IEnumerable<string> LinkInputPaths(MakeActionContext context)
    {
        return context.Dependencies
            .Where(node => node.Kind == BuildGraphNodeKind.File && node.Path is not null && IsLinkInput(node.Path))
            .Select(node => context.Workspace.ResolveRepositoryPath(node.Path!));
    }

    private async Task<ProcessRunResult> RunVsToolAsync(BuildWorkspace workspace, string tool, string responseFile, CancellationToken cancellationToken)
    {
        var command = $"call {Quote(_msvcToolchain.Value.VcVarsBat)} >nul && {Quote(tool)} @{Quote(responseFile)}";
        return await ProcessRunner.RunAsync("cmd.exe", $"/d /s /c \"{command}\"", workspace.RootDirectory, _actionTimeout, cancellationToken);
    }

    private static void AddAppleLanguageArgs(List<string> args, string language)
    {
        switch(language.ToLowerInvariant())
        {
            case "c":
                args.Add("-std=c17");
                break;
            case "c++20":
                args.Add("-std=c++20");
                break;
            case "objective-c++20":
                args.Add("-x");
                args.Add("objective-c++");
                args.Add("-std=c++20");
                args.Add("-fobjc-arc");
                break;
            case "objective-c":
                args.Add("-fobjc-arc");
                break;
            case "assembler":
            case "assembler-with-cpp":
                break;
            default:
                throw new MakeSystemException($"Unsupported Apple clang source language: {language}");
        }
    }

    private static void AddAppleModeArgs(List<string> args, string mode)
    {
        if(mode.Equals("Release", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("-O2");
            args.Add("-DNDEBUG");
            return;
        }
        if(mode.Equals("Profile", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("-O2");
            args.Add("-g");
            return;
        }
        args.Add("-O0");
        args.Add("-g");
    }

    private static void AddAppleLibrary(List<string> args, string library)
    {
        if(library.StartsWith("-", StringComparison.Ordinal) || Path.IsPathRooted(library))
        {
            args.Add(library);
            return;
        }

        var name = library;
        if(name.EndsWith(".lib", StringComparison.OrdinalIgnoreCase) ||
            name.EndsWith(".a", StringComparison.OrdinalIgnoreCase) ||
            name.EndsWith(".dylib", StringComparison.OrdinalIgnoreCase))
        {
            name = Path.GetFileNameWithoutExtension(name);
        }
        if(name.StartsWith("lib", StringComparison.OrdinalIgnoreCase))
        {
            name = name[3..];
        }
        args.Add("-l" + name);
    }

    private static bool UsesCxxCompiler(string language)
    {
        return language.Equals("c++20", StringComparison.OrdinalIgnoreCase) ||
            language.Equals("objective-c++20", StringComparison.OrdinalIgnoreCase);
    }

    private static string AppleArchitecture(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "arm64" or "aarch64" => "arm64",
            "x64" or "x86_64" => "x86_64",
            _ => throw new MakeSystemException($"Unsupported macOS architecture: {architecture}"),
        };
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

    private static string MsvcOptimizationFlag(string mode)
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
            path.EndsWith(".o", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".res", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".lib", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".a", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".dylib", StringComparison.OrdinalIgnoreCase) ||
            path.EndsWith(".so", StringComparison.OrdinalIgnoreCase);
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

internal sealed record AppleClangToolchain(string Clang, string ClangCxx, string Libtool, string SdkPath);

internal static class AppleClangToolchainLocator
{
    public static AppleClangToolchain LocateMacOS()
    {
        if(!OperatingSystem.IsMacOS())
        {
            throw new MakeSystemException("Apple clang toolchain lookup requires a macOS host.");
        }

        return new AppleClangToolchain(
            Clang: Xcrun("-sdk", "macosx", "-find", "clang"),
            ClangCxx: Xcrun("-sdk", "macosx", "-find", "clang++"),
            Libtool: Xcrun("-sdk", "macosx", "-find", "libtool"),
            SdkPath: Xcrun("-sdk", "macosx", "--show-sdk-path"));
    }

    private static string Xcrun(params string[] arguments)
    {
        var result = ProcessRunner.RunAsync("xcrun", arguments, Directory.GetCurrentDirectory(), TimeSpan.FromSeconds(30), CancellationToken.None)
            .GetAwaiter()
            .GetResult();
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"xcrun {string.Join(' ', arguments)} failed:{Environment.NewLine}{result.Output}");
        }

        var value = result.Output
            .Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries)
            .Select(line => line.Trim())
            .LastOrDefault(line => Path.IsPathFullyQualified(line));
        if(string.IsNullOrWhiteSpace(value))
        {
            throw new MakeSystemException($"xcrun {string.Join(' ', arguments)} returned an empty path.");
        }
        return value;
    }
}
