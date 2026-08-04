using LunaBuild.Core;

public sealed class LunaSDKProjectRules : ProjectRules
{
    public LunaSDKProjectRules()
        : base("LunaSDK")
    {
    }

    protected override void ConfigureProperties(BuildWorkspace workspace)
    {
        BooleanProperty(
            "api_validation",
            defaultValue: false,
            description: "Enable Luna public API validation checks. Debug mode enables this automatically.",
            "api-validation",
            "contract-assertion");
        BooleanProperty(
            "thread_safe_assertion",
            defaultValue: false,
            description: "Enable Luna thread-safety assertion checks.",
            "thread-safe-assertion");
        BooleanProperty(
            "memory_profiler",
            defaultValue: false,
            description: "Enable Luna runtime memory profiler instrumentation.",
            "memory-profiler");
        BooleanProperty(
            "rhi_debug",
            defaultValue: false,
            description: "Enable RHI backend debug layers and validation helpers.",
            "rhi-debug");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        GlobalIncludeDirectories("Modules");
        LibraryPrefix("Luna");
        ActionConfiguration(
            "luna.meta",
            targets: new Dictionary<string, string>
            {
                ["tool"] = "LunaMetaTool",
            },
            directories: MetaResourceDirectories(workspace, options));
        ActionConfiguration(
            "cppsl.shader",
            targets: new Dictionary<string, string>
            {
                ["compiler"] = "CPPSL",
                ["native_extractor"] = "cppsl-native-extractor",
            },
            files: ShaderBackendFiles(workspace),
            directories: new Dictionary<string, string>
            {
                ["standard_library"] = "Tools/CPPSL/std",
            },
            values: new Dictionary<string, string>
            {
                ["helper_header"] = "Luna/RHI/CppslShaderHelper.hpp",
                ["namespace"] = "Luna",
            });
        if(options.Mode == BuildMode.Debug || GetBoolean("api_validation"))
        {
            GlobalDefines("LUNA_ENABLE_API_VALIDATION");
        }
        if(GetBoolean("thread_safe_assertion"))
        {
            GlobalDefines("LUNA_ENABLE_THREAD_SAFE_ASSERTION");
        }
        if(GetBoolean("memory_profiler"))
        {
            GlobalDefines("LUNA_ENABLE_MEMORY_PROFILER");
        }
    }

    private static IReadOnlyDictionary<string, string> ShaderBackendFiles(BuildWorkspace workspace)
    {
        var platform = OperatingSystem.IsWindows() ? "windows" : "macosx";
        var architecture = OperatingSystem.IsWindows() ? "x64" : "arm64";
        var executableExtension = OperatingSystem.IsWindows() ? ".exe" : string.Empty;
        return new Dictionary<string, string>
        {
            ["dxc"] = Path.Combine(workspace.RootDirectory, "SDKs", "vulkan-tools", platform, architecture, "bin", "dxc" + executableExtension),
            ["glslang"] = Path.Combine(workspace.RootDirectory, "SDKs", "vulkan-tools", platform, architecture, "bin", "glslang" + executableExtension),
        };
    }

    private static IReadOnlyDictionary<string, string> MetaResourceDirectories(BuildWorkspace workspace, BuildOptions options)
    {
        var platform = options.Platform == BuildPlatform.Windows ? "windows" : "macosx";
        var architecture = options.Platform == BuildPlatform.Windows ? "x64" : "arm64";
        var clangRoot = Path.Combine(workspace.RootDirectory, "SDKs", "llvm-21.1.1", platform, architecture, "lib", "clang");
        if(!Directory.Exists(clangRoot))
        {
            return new Dictionary<string, string>();
        }
        var resourceDirectory = Directory.EnumerateDirectories(clangRoot)
            .Where(directory => Directory.Exists(Path.Combine(directory, "include")))
            .OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
        return resourceDirectory is null
            ? new Dictionary<string, string>()
            : new Dictionary<string, string> { ["clang_resource"] = resourceDirectory };
    }
}
