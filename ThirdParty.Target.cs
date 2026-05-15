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
        LinkLibraryFiles("SDKs/d3d12-memory-allocator/windows/x64/lib/D3D12MA.lib");
        RequiredFiles("SDKs/d3d12-memory-allocator/windows/x64/include/D3D12MemAlloc.h");
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
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux);
        PublicIncludeDirectories("SDKs/volk/include");
        RequiredFiles("SDKs/volk/include/volk.h");
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
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux);
        PublicIncludeDirectories("SDKs/vulkan-memory-allocator/include");
        RequiredFiles("SDKs/vulkan-memory-allocator/include/vk_mem_alloc.h");
    }
}
