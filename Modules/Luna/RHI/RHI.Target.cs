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
                MetaHeaders(
                    "Source/D3D12/Adapter.hpp",
                    "Source/D3D12/CommandBuffer.hpp",
                    "Source/D3D12/DescriptorSet.hpp",
                    "Source/D3D12/DescriptorSetLayout.hpp",
                    "Source/D3D12/Device.hpp",
                    "Source/D3D12/DeviceMemory.hpp",
                    "Source/D3D12/Fence.hpp",
                    "Source/D3D12/PipelineLayout.hpp",
                    "Source/D3D12/PipelineState.hpp",
                    "Source/D3D12/QueryHeap.hpp",
                    "Source/D3D12/Resource.hpp",
                    "Source/D3D12/SwapChain.hpp");
                Sources("Source/D3D12/**.cpp");
                DependsOn("d3d12-memory-allocator");
                break;
            case RhiApi.Vulkan:
                Defines("LUNA_RHI_VULKAN");
                Headers("Source/Vulkan/**.hpp");
                MetaHeaders(
                    "Source/Vulkan/Adapter.hpp",
                    "Source/Vulkan/CommandBuffer.hpp",
                    "Source/Vulkan/DescriptorSet.hpp",
                    "Source/Vulkan/DescriptorSetLayout.hpp",
                    "Source/Vulkan/Device.hpp",
                    "Source/Vulkan/DeviceMemory.hpp",
                    "Source/Vulkan/Fence.hpp",
                    "Source/Vulkan/ImageView.hpp",
                    "Source/Vulkan/PipelineLayout.hpp",
                    "Source/Vulkan/PipelineState.hpp",
                    "Source/Vulkan/QueryHeap.hpp",
                    "Source/Vulkan/Resource.hpp",
                    "Source/Vulkan/Sampler.hpp",
                    "Source/Vulkan/SwapChain.hpp");
                Sources("Source/Vulkan/**.cpp", "Source/Vulkan/**.c");
                DependsOn("volk", "vulkan-memory-allocator");
                break;
            case RhiApi.Metal:
                Defines("LUNA_RHI_METAL");
                Headers("Source/Metal/**.h");
                MetaHeaders(
                    "Source/Metal/Adapter.h",
                    "Source/Metal/CommandBuffer.h",
                    "Source/Metal/DescriptorSet.h",
                    "Source/Metal/DescriptorSetLayout.h",
                    "Source/Metal/Device.h",
                    "Source/Metal/DeviceMemory.h",
                    "Source/Metal/Fence.h",
                    "Source/Metal/PipelineLayout.h",
                    "Source/Metal/PipelineState.h",
                    "Source/Metal/QueryHeap.h",
                    "Source/Metal/Resource.h",
                    "Source/Metal/SwapChain.h",
                    "Source/Metal/TextureView.h");
                Sources("Source/Metal/**.mm");
                Frameworks("Foundation", "QuartzCore", "Metal", "CoreGraphics");
                DependsOn("VariantUtils");
                break;
            default:
                throw new ArgumentOutOfRangeException(nameof(options.RhiApi), options.RhiApi, null);
        }
    }
}
