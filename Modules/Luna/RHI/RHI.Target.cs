namespace LunaBuild.Core.Targets;

public sealed class RHITargetRules : TargetRules
{
    public RHITargetRules()
        : base(
            name: "RHI",
            targetDirectory: "Modules/Luna/RHI",
            rulesPath: "Modules/Luna/RHI/RHI.Target.cs")
    {
        Headers(
            "Adapter.hpp",
            "Buffer.hpp",
            "CommandBuffer.hpp",
            "CppslShaderHelper.hpp",
            "DescriptorSet.hpp",
            "DescriptorSetLayout.hpp",
            "Device.hpp",
            "DeviceChild.hpp",
            "DeviceMemory.hpp",
            "Fence.hpp",
            "PipelineLayout.hpp",
            "PipelineState.hpp",
            "QueryHeap.hpp",
            "Resource.hpp",
            "RHI.hpp",
            "SwapChain.hpp",
            "Texture.hpp",
            "Source/*.hpp");

        Sources("Source/*.cpp");
        DependsOn("Runtime", "Window");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        switch(RhiApi)
        {
            case RhiApi.D3D12:
                Defines("LUNA_RHI_D3D12");
                Headers("Source/DXGI/**.hpp", "Source/D3D12/**.hpp");
                Sources("Source/D3D12/**.cpp");
                Packages("d3d12-memory-allocator");
                break;
            case RhiApi.Vulkan:
                Defines("LUNA_RHI_VULKAN");
                Headers("Source/Vulkan/**.hpp");
                Sources("Source/Vulkan/**.cpp", "Source/Vulkan/**.c");
                Packages("volk", "vulkan-memory-allocator");
                break;
            case RhiApi.Metal:
                Defines("LUNA_RHI_METAL");
                Headers("Source/Metal/**.h");
                Sources("Source/Metal/**.mm");
                DependsOn("VariantUtils");
                break;
            default:
                throw new ArgumentOutOfRangeException(nameof(options.RhiApi), options.RhiApi, null);
        }
    }
}
