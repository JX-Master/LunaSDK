namespace LunaBuild.Core.Targets;

public abstract class ManagedSampleTargetRules : TargetRules
{
    protected ManagedSampleTargetRules(string name, string directory, params string[] dependencies)
        : base(name, directory, "Samples/ManagedSamples.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Engine;
        Kind = BuildTargetKind.ManagedExecutable;
        DotNetSettings("net10.0");
        Sources("*.cs");
        DependsOn(dependencies);
    }
}

public sealed class HelloWindowTargetRules : ManagedSampleTargetRules
{
    public HelloWindowTargetRules() : base("HelloWindow", "Samples/HelloWindow", "Luna.Window") { }
}

public sealed class HelloRhiTargetRules : ManagedSampleTargetRules
{
    public HelloRhiTargetRules() : base("HelloRHI", "Samples/HelloRHI", "Luna.Window", "Luna.RHI") { }
}

public sealed class ManagedHostAppTargetRules : ManagedSampleTargetRules
{
    public ManagedHostAppTargetRules() : base("ManagedHostApp", "Samples/ManagedHostApp", "Luna.Window", "Luna.RHI") { }
}

public sealed class NativeManagedHostTargetRules : TargetRules
{
    public NativeManagedHostTargetRules()
        : base("NativeManagedHost", "Samples/NativeManagedHost", "Samples/ManagedSamples.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Engine;
        Kind = BuildTargetKind.Executable;
        Sources("Source/*.cpp");
        DependsOn("Window", "ManagedHostApp");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var nativeDirectory = DotNetHostNativeDirectory(options);
        IncludeDirectories(nativeDirectory);
        LinkLibraryFiles(DotNetHostLinkLibrary(options, nativeDirectory));
        var runtimeLibrary = DotNetHostRuntimeLibrary(options, nativeDirectory);
        if(runtimeLibrary is not null)
        {
            RuntimeFiles(runtimeLibrary);
        }
    }

    private static string DotNetHostNativeDirectory(BuildOptions options)
    {
        var rid = DotNetRid(options);
        var roots = new List<string>();
        var dotnetRoot = Environment.GetEnvironmentVariable("DOTNET_ROOT");
        if(!string.IsNullOrWhiteSpace(dotnetRoot))
        {
            roots.Add(dotnetRoot);
        }
        if(options.Platform == BuildPlatform.Windows)
        {
            var programFiles = Environment.GetEnvironmentVariable("ProgramFiles") ?? "C:\\Program Files";
            roots.Add(Path.Combine(programFiles, "dotnet"));
        }
        else
        {
            roots.Add("/usr/local/share/dotnet");
            roots.Add("/opt/homebrew/share/dotnet");
            var home = Environment.GetEnvironmentVariable("HOME");
            if(!string.IsNullOrWhiteSpace(home))
            {
                roots.Add(Path.Combine(home, ".dotnet"));
            }
        }

        foreach(var root in roots.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            var packDirectory = Path.Combine(root, "packs", "Microsoft.NETCore.App.Host." + rid);
            if(!Directory.Exists(packDirectory))
            {
                continue;
            }
            var versions = Directory.EnumerateDirectories(packDirectory)
                .OrderBy(VersionSortKey, StringComparer.OrdinalIgnoreCase)
                .ToArray();
            if(versions.Length == 0)
            {
                continue;
            }
            var nativeDirectory = Path.Combine(versions[^1], "runtimes", rid, "native");
            if(Directory.Exists(nativeDirectory))
            {
                return nativeDirectory;
            }
        }
        throw new DirectoryNotFoundException($"Cannot find .NET host native pack for RID `{rid}`. Install the .NET SDK or set DOTNET_ROOT.");
    }

    private static string DotNetRid(BuildOptions options)
    {
        return options.Platform switch
        {
            BuildPlatform.Windows when options.Architecture is "arm64" or "aarch64" => "win-arm64",
            BuildPlatform.Windows => "win-x64",
            BuildPlatform.MacOS when options.Architecture is "arm64" or "aarch64" => "osx-arm64",
            BuildPlatform.MacOS => "osx-x64",
            BuildPlatform.Linux when options.Architecture is "arm64" or "aarch64" => "linux-arm64",
            BuildPlatform.Linux => "linux-x64",
            _ => throw new NotSupportedException($"NativeManagedHost does not support platform {options.Platform}."),
        };
    }

    private static string DotNetHostLinkLibrary(BuildOptions options, string nativeDirectory)
    {
        return options.Platform switch
        {
            BuildPlatform.Windows => Path.Combine(nativeDirectory, "nethost.lib"),
            BuildPlatform.MacOS => Path.Combine(nativeDirectory, "libnethost.dylib"),
            _ => Path.Combine(nativeDirectory, "libnethost.so"),
        };
    }

    private static string? DotNetHostRuntimeLibrary(BuildOptions options, string nativeDirectory)
    {
        return options.Platform switch
        {
            BuildPlatform.Windows => Path.Combine(nativeDirectory, "nethost.dll"),
            BuildPlatform.MacOS => Path.Combine(nativeDirectory, "libnethost.dylib"),
            BuildPlatform.Linux => Path.Combine(nativeDirectory, "libnethost.so"),
            _ => null,
        };
    }

    private static string VersionSortKey(string path)
    {
        var fileName = Path.GetFileName(path);
        var parts = fileName.Split('.', StringSplitOptions.RemoveEmptyEntries);
        return string.Join('.', parts.Select(part => int.TryParse(part, out var value) ? value.ToString("D8") : part));
    }
}
