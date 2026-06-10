namespace LunaBuild.Core.MakeSystem;

internal sealed record CppCommandLine(string Tool, IReadOnlyList<string> Arguments)
{
    public string ToShellCommand() => string.Join(" ", new[] { Tool }.Concat(Arguments).Select(CppCommandLineBuilder.Quote));
}

internal static class CppCommandLineBuilder
{
    public static CppCommandLine BuildCompileCommand(
        BuildWorkspace workspace,
        BuildOptions options,
        string actionPayload,
        string? msvcToolPath = null,
        string? appleClangPath = null,
        string? appleClangCxxPath = null,
        string? appleSdkPath = null,
        string? androidClangPath = null,
        string? androidClangCxxPath = null,
        string? androidSysroot = null,
        int androidApiLevel = AndroidNdkToolchainLocator.DefaultApiLevel)
    {
        var payload = ActionPayload.Parse(actionPayload);
        return options.Platform switch
        {
            BuildPlatform.Windows => new CppCommandLine(
                msvcToolPath ?? "cl.exe",
                BuildMsvcCompileArguments(workspace, options, payload)),
            BuildPlatform.MacOS => new CppCommandLine(
                UsesCxxCompiler(payload.Required("language")) ? appleClangCxxPath ?? "clang++" : appleClangPath ?? "clang",
                BuildAppleCompileArguments(workspace, payload, appleSdkPath)),
            BuildPlatform.Android => new CppCommandLine(
                UsesCxxCompiler(payload.Required("language")) ? androidClangCxxPath ?? "clang++" : androidClangPath ?? "clang",
                BuildAndroidCompileArguments(workspace, payload, androidSysroot, androidApiLevel)),
            _ => new CppCommandLine(
                UsesCxxCompiler(payload.Required("language")) ? "clang++" : "clang",
                BuildClangCompileArguments(workspace, payload)),
        };
    }

    public static IReadOnlyList<string> BuildMsvcCompileArguments(
        BuildWorkspace workspace,
        BuildOptions options,
        ActionPayload payload)
    {
        var source = workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = workspace.ResolveRepositoryPath(payload.Required("depfile"));
        var sourceDependencies = depfile + ".json";
        var args = new List<string>
        {
            "/c",
            "/nologo",
            RuntimeFlag(payload.Contains("runtime") ? payload.Required("runtime") : payload.Required("mode")),
            "/Zi",
            "/FS",
            $"/Fd{Quote(Path.ChangeExtension(output, ".pdb"))}",
            MsvcOptimizationFlag(payload.Required("mode")),
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
        if(payload.Contains("rtti") && payload.Required("rtti").Equals("none", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("/GR-");
        }

        foreach(var include in payload.All("include"))
        {
            args.Add($"/I{Quote(workspace.ResolveRepositoryPath(include))}");
        }
        foreach(var define in payload.All("define"))
        {
            args.Add($"/D{define}");
        }
        foreach(var undefine in payload.All("undefine"))
        {
            args.Add($"/U{undefine}");
        }
        args.Add($"/Fo{Quote(output)}");
        args.Add(Quote(source));
        return args;
    }

    public static IReadOnlyList<string> BuildAppleCompileArguments(
        BuildWorkspace workspace,
        ActionPayload payload,
        string? sdkPath)
    {
        var source = workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = workspace.ResolveRepositoryPath(payload.Required("depfile"));
        var args = new List<string>
        {
            "-c",
            "-o",
            output,
            "-MMD",
            "-MF",
            depfile,
            "-arch",
            AppleArchitecture(payload.Required("arch")),
            "-fPIC",
            "-fno-exceptions",
        };
        if(!string.IsNullOrWhiteSpace(sdkPath))
        {
            args.Add("-isysroot");
            args.Add(sdkPath);
        }
        AddRttiArgs(args, payload);
        AddAppleLanguageArgs(args, payload.Required("language"));
        AddAppleModeArgs(args, payload.Required("mode"));
        AddCommonClangArgs(workspace, payload, args);
        args.Add(source);
        return args;
    }

    private static IReadOnlyList<string> BuildClangCompileArguments(BuildWorkspace workspace, ActionPayload payload)
    {
        var source = workspace.ResolveRepositoryPath(payload.Required("source"));
        var output = workspace.ResolveRepositoryPath(payload.Required("object"));
        var depfile = workspace.ResolveRepositoryPath(payload.Required("depfile"));
        var args = new List<string>
        {
            "-c",
            "-o",
            output,
            "-MMD",
            "-MF",
            depfile,
            "-fPIC",
            "-fno-exceptions",
        };
        AddRttiArgs(args, payload);
        AddAppleLanguageArgs(args, payload.Required("language"));
        AddAppleModeArgs(args, payload.Required("mode"));
        AddCommonClangArgs(workspace, payload, args);
        args.Add(source);
        return args;
    }

    public static IReadOnlyList<string> BuildAndroidCompileArguments(
        BuildWorkspace workspace,
        ActionPayload payload,
        string? sysroot,
        int apiLevel)
    {
        var args = BuildClangCompileArguments(workspace, payload).ToList();
        args.Insert(0, "--target=" + AndroidNdkToolchainLocator.TargetTripleWithApi(payload.Required("arch"), apiLevel));
        if(!string.IsNullOrWhiteSpace(sysroot))
        {
            args.Insert(1, "--sysroot=" + sysroot);
        }
        args.Insert(Math.Max(0, args.Count - 1), "-DANDROID");
        return args;
    }

    public static string Quote(string value)
    {
        return value.Contains(' ') || value.Contains('\t') || value.Contains('"')
            ? $"\"{value.Replace("\"", "\\\"")}\""
            : value;
    }

    public static bool UsesCxxCompiler(string language)
    {
        return language.Equals("c++20", StringComparison.OrdinalIgnoreCase) ||
            language.Equals("objective-c++20", StringComparison.OrdinalIgnoreCase);
    }

    private static void AddCommonClangArgs(BuildWorkspace workspace, ActionPayload payload, List<string> args)
    {
        foreach(var include in payload.All("include"))
        {
            args.Add("-I" + workspace.ResolveRepositoryPath(include));
        }
        foreach(var define in payload.All("define"))
        {
            args.Add("-D" + define);
        }
        foreach(var undefine in payload.All("undefine"))
        {
            args.Add("-U" + undefine);
        }
    }

    private static void AddRttiArgs(List<string> args, ActionPayload payload)
    {
        if(payload.Contains("rtti") && payload.Required("rtti").Equals("none", StringComparison.OrdinalIgnoreCase))
        {
            args.Add("-fno-rtti");
        }
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
                args.Add("-Wno-unknown-attributes");
                args.Add("-Wno-ignored-attributes");
                break;
            case "objective-c++20":
                args.Add("-x");
                args.Add("objective-c++");
                args.Add("-std=c++20");
                args.Add("-Wno-unknown-attributes");
                args.Add("-Wno-ignored-attributes");
                args.Add("-fobjc-arc");
                break;
            case "objective-c":
                args.Add("-fobjc-arc");
                break;
            case "assembler":
            case "assembler-with-cpp":
                break;
            default:
                throw new MakeSystemException($"Unsupported clang source language: {language}");
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

    private static string AppleArchitecture(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "arm64" or "aarch64" => "arm64",
            _ => throw new MakeSystemException($"Unsupported clang architecture: {architecture}"),
        };
    }
}
