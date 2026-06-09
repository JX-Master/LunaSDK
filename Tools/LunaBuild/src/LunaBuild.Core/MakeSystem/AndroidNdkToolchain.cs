namespace LunaBuild.Core.MakeSystem;

public sealed record AndroidNdkToolchain(
    string NdkRoot,
    string ToolchainDirectory,
    string Clang,
    string ClangCxx,
    string Ar,
    string Sysroot,
    int ApiLevel);

public static class AndroidNdkToolchainLocator
{
    public const int DefaultApiLevel = 31;

    public static AndroidNdkToolchain Locate(int apiLevel = DefaultApiLevel)
    {
        foreach(var ndkRoot in CandidateNdkRoots().Distinct(StringComparer.OrdinalIgnoreCase))
        {
            var toolchainDirectory = Path.Combine(ndkRoot, "toolchains", "llvm", "prebuilt", HostTag());
            var binDirectory = Path.Combine(toolchainDirectory, "bin");
            var clang = Path.Combine(binDirectory, ToolName("clang"));
            var clangCxx = Path.Combine(binDirectory, ToolName("clang++"));
            var ar = Path.Combine(binDirectory, ToolName("llvm-ar"));
            var sysroot = Path.Combine(toolchainDirectory, "sysroot");
            if(File.Exists(clang) && File.Exists(clangCxx) && File.Exists(ar) && Directory.Exists(sysroot))
            {
                return new AndroidNdkToolchain(ndkRoot, toolchainDirectory, clang, clangCxx, ar, sysroot, apiLevel);
            }
        }

        throw new DirectoryNotFoundException(
            "Android NDK was not found. Set ANDROID_NDK_ROOT, ANDROID_NDK_HOME, NDK_ROOT, ANDROID_HOME, or ANDROID_SDK_ROOT.");
    }

    private static IEnumerable<string> CandidateNdkRoots()
    {
        foreach(var name in new[] { "ANDROID_NDK_ROOT", "ANDROID_NDK_HOME", "NDK_ROOT" })
        {
            var value = Environment.GetEnvironmentVariable(name);
            if(!string.IsNullOrWhiteSpace(value))
            {
                yield return Path.GetFullPath(value);
            }
        }

        foreach(var sdkRoot in CandidateSdkRoots())
        {
            var ndkDirectory = Path.Combine(sdkRoot, "ndk");
            if(Directory.Exists(ndkDirectory))
            {
                foreach(var versionDirectory in Directory.GetDirectories(ndkDirectory).OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase))
                {
                    yield return versionDirectory;
                }
            }
        }
    }

    private static IEnumerable<string> CandidateSdkRoots()
    {
        foreach(var name in new[] { "ANDROID_HOME", "ANDROID_SDK_ROOT" })
        {
            var value = Environment.GetEnvironmentVariable(name);
            if(!string.IsNullOrWhiteSpace(value))
            {
                yield return Path.GetFullPath(value);
            }
        }

        var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        if(!string.IsNullOrWhiteSpace(localAppData))
        {
            yield return Path.Combine(localAppData, "Android", "Sdk");
        }
    }

    private static string HostTag()
    {
        if(OperatingSystem.IsWindows())
        {
            return "windows-x86_64";
        }
        if(OperatingSystem.IsMacOS())
        {
            return "darwin-x86_64";
        }
        if(OperatingSystem.IsLinux())
        {
            return "linux-x86_64";
        }
        throw new PlatformNotSupportedException("Android NDK host toolchain is only supported on Windows, macOS, and Linux.");
    }

    private static string ToolName(string name)
    {
        return OperatingSystem.IsWindows() ? name + ".exe" : name;
    }

    public static string Abi(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "arm64" or "aarch64" or "arm64-v8a" => "arm64-v8a",
            "x64" or "x86_64" => "x86_64",
            "x86" or "i386" => "x86",
            "arm" or "armv7" or "armeabi-v7a" => "armeabi-v7a",
            _ => throw new MakeSystemException($"Unsupported Android architecture: {architecture}"),
        };
    }

    public static string TargetTriple(string architecture)
    {
        return Abi(architecture) switch
        {
            "arm64-v8a" => "aarch64-linux-android",
            "x86_64" => "x86_64-linux-android",
            "x86" => "i686-linux-android",
            "armeabi-v7a" => "armv7a-linux-androideabi",
            _ => throw new MakeSystemException($"Unsupported Android architecture: {architecture}"),
        };
    }

    public static string TargetTripleWithApi(string architecture, int apiLevel)
    {
        return TargetTriple(architecture) + apiLevel.ToString(System.Globalization.CultureInfo.InvariantCulture);
    }

    public static string CxxSharedRuntime(AndroidNdkToolchain toolchain, string architecture)
    {
        var path = Path.Combine(toolchain.Sysroot, "usr", "lib", TargetTriple(architecture), "libc++_shared.so");
        if(!File.Exists(path))
        {
            throw new FileNotFoundException($"Android NDK C++ shared runtime was not found: {path}", path);
        }
        return path;
    }
}
