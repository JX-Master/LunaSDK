namespace LunaBuild.Core.Targets;

public sealed class StbTargetRules : TargetRules
{
    public StbTargetRules()
        : base(
            name: "stb",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        PublicIncludeDirectories(
            "SDKs/stb/include",
            "SDKs/stb/include/stb");
        RequiredFiles(
            "SDKs/stb/include/stb/stb_image.h",
            "SDKs/stb/include/stb/stb_image_write.h",
            "SDKs/stb/include/stb/stb_rect_pack.h",
            "SDKs/stb/include/stb/stb_truetype.h");
    }
}

public sealed class MiniAudioTargetRules : TargetRules
{
    public MiniAudioTargetRules()
        : base(
            name: "miniaudio",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        PublicIncludeDirectories("SDKs/miniaudio/include");
        RequiredFiles("SDKs/miniaudio/include/miniaudio.h");
    }
}

public sealed class D3D12MemoryAllocatorTargetRules : TargetRules
{
    public D3D12MemoryAllocatorTargetRules()
        : base(
            name: "d3d12-memory-allocator",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows);
        PublicIncludeDirectories("SDKs/d3d12-memory-allocator/windows/x64/include");
        RequiredFiles("SDKs/d3d12-memory-allocator/windows/x64/include/D3D12MemAlloc.h");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var configurationDirectory = options.Mode == BuildMode.Debug ? "Debug" : "Release";
        LinkLibraryFiles($"SDKs/d3d12-memory-allocator/windows/x64/lib/{configurationDirectory}/D3D12MA.lib");
    }
}

public sealed class VolkTargetRules : TargetRules
{
    public VolkTargetRules()
        : base(
            name: "volk",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
        DependsOn("vulkan-headers");
        PublicIncludeDirectories("SDKs/volk/include");
        RequiredFiles(
            "SDKs/volk/include/volk.h",
            "SDKs/volk/include/volk.c");
    }
}

public sealed class VulkanHeadersTargetRules : TargetRules
{
    public VulkanHeadersTargetRules()
        : base(
            name: "vulkan-headers",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var projectIncludeDirectory = workspace.ResolveRepositoryPath("SDKs/vulkan-headers/include");
        var includeDirectory = IsVulkanHeadersIncludeDirectory(projectIncludeDirectory)
            ? projectIncludeDirectory
            : FindVulkanSdkIncludeDirectory() ?? projectIncludeDirectory;

        PublicIncludeDirectories(includeDirectory);
        RequiredFiles(
            Path.Combine(includeDirectory, "vulkan", "vk_platform.h"),
            Path.Combine(includeDirectory, "vulkan", "vulkan.h"));
    }

    private static string? FindVulkanSdkIncludeDirectory()
    {
        var vulkanSdk = Environment.GetEnvironmentVariable("VULKAN_SDK");
        if(string.IsNullOrWhiteSpace(vulkanSdk))
        {
            return null;
        }

        var includeDirectory = Path.Combine(vulkanSdk, OperatingSystem.IsWindows() ? "Include" : "include");
        return IsVulkanHeadersIncludeDirectory(includeDirectory) ? includeDirectory : null;
    }

    private static bool IsVulkanHeadersIncludeDirectory(string includeDirectory)
    {
        return File.Exists(Path.Combine(includeDirectory, "vulkan", "vk_platform.h")) &&
            File.Exists(Path.Combine(includeDirectory, "vulkan", "vulkan.h"));
    }
}

public sealed class VulkanMemoryAllocatorTargetRules : TargetRules
{
    public VulkanMemoryAllocatorTargetRules()
        : base(
            name: "vulkan-memory-allocator",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
        DependsOn("vulkan-headers");
        PublicIncludeDirectories("SDKs/vulkan-memory-allocator/include");
        RequiredFiles("SDKs/vulkan-memory-allocator/include/vk_mem_alloc.h");
    }
}
