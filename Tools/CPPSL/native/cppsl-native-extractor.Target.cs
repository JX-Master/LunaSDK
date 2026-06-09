namespace LunaBuild.Core.Targets;

public sealed class CppslNativeExtractorTargetRules : TargetRules
{
    public CppslNativeExtractorTargetRules()
        : base(
            name: "cppsl-native-extractor",
            targetDirectory: "Tools/CPPSL/native",
            rulesPath: "Tools/CPPSL/native/cppsl-native-extractor.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.MacOS, BuildPlatform.Windows);
        Category = BuildTargetCategory.Tools;
        Kind = BuildTargetKind.Executable;
        Sources("src/main.cpp");
        Defines("NDEBUG");
        Rtti(false);
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(options.Platform == BuildPlatform.MacOS)
        {
            var llvmSdk = Path.Combine(workspace.RootDirectory, "SDKs", "llvm-21.1.1", "macosx", "arm64");
            if(!Directory.Exists(llvmSdk))
            {
                throw new DirectoryNotFoundException($"CPPSL native extractor requires LLVM SDK: {llvmSdk}");
            }

            IncludeDirectories(Path.Combine(llvmSdk, "include"));
            LinkLibraryFiles(Path.Combine(llvmSdk, "lib", "libclang-cpp.dylib"));
            RuntimeFiles(
                Path.Combine(llvmSdk, "lib", "libclang-cpp.dylib"),
                Path.Combine(llvmSdk, "lib", "libzstd.1.dylib"));
            Frameworks("CoreServices", "CoreFoundation");
        }
        else if(options.Platform == BuildPlatform.Windows)
        {
            var llvmSdk = Path.Combine(workspace.RootDirectory, "SDKs", "llvm-21.1.1", "windows", "x64");
            if(!Directory.Exists(llvmSdk))
            {
                throw new DirectoryNotFoundException($"CPPSL native extractor requires LLVM SDK: {llvmSdk}");
            }

            var llvmLibDirectory = Path.Combine(llvmSdk, "lib");
            IncludeDirectories(Path.Combine(llvmSdk, "include"));
            LinkLibraryFiles(Directory.EnumerateFiles(llvmLibDirectory, "clang*.lib")
                .Concat(Directory.EnumerateFiles(llvmLibDirectory, "LLVM*.lib"))
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray());
            Defines("_ITERATOR_DEBUG_LEVEL=0");
            Undefines("_DEBUG");
            MsvcRuntimeLibrary("MD");
            SystemLibraries(
                "advapi32.lib",
                "bcrypt.lib",
                "dbghelp.lib",
                "ntdll.lib",
                "ole32.lib",
                "shell32.lib",
                "user32.lib",
                "version.lib",
                "ws2_32.lib");
        }
    }
}
