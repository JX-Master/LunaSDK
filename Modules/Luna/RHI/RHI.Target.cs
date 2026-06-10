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

        MetaHeaders(
            "Adapter.hpp",
            "Buffer.hpp",
            "CommandBuffer.hpp",
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
            "SwapChain.hpp",
            "Texture.hpp");

        Sources("Source/*.cpp");
        DependsOn("Runtime", "Window");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(options.Properties.GetBoolean("rhi_debug"))
        {
            Defines("LUNA_RHI_DEBUG");
        }

        switch(RhiApi)
        {
            case RhiApi.D3D12:
                Defines("LUNA_RHI_D3D12");
                Headers("Source/DXGI/**.hpp", "Source/D3D12/**.hpp");
                MetaHeaders(
                    "Source/D3D12/D3D12Adapter.hpp",
                    "Source/D3D12/D3D12CommandBuffer.hpp",
                    "Source/D3D12/D3D12DescriptorSet.hpp",
                    "Source/D3D12/D3D12DescriptorSetLayout.hpp",
                    "Source/D3D12/D3D12Device.hpp",
                    "Source/D3D12/D3D12DeviceMemory.hpp",
                    "Source/D3D12/D3D12Fence.hpp",
                    "Source/D3D12/D3D12PipelineLayout.hpp",
                    "Source/D3D12/D3D12PipelineState.hpp",
                    "Source/D3D12/D3D12QueryHeap.hpp",
                    "Source/D3D12/D3D12Resource.hpp",
                    "Source/D3D12/D3D12SwapChain.hpp");
                Sources("Source/D3D12/**.cpp");
                DependsOn("d3d12-memory-allocator");
                break;
            case RhiApi.Vulkan:
                Defines("LUNA_RHI_VULKAN");
                Headers("Source/Vulkan/**.hpp");
                MetaHeaders(
                    "Source/Vulkan/VulkanAdapter.hpp",
                    "Source/Vulkan/VulkanCommandBuffer.hpp",
                    "Source/Vulkan/VulkanDescriptorSet.hpp",
                    "Source/Vulkan/VulkanDescriptorSetLayout.hpp",
                    "Source/Vulkan/VulkanDevice.hpp",
                    "Source/Vulkan/VulkanDeviceMemory.hpp",
                    "Source/Vulkan/VulkanFence.hpp",
                    "Source/Vulkan/VulkanImageView.hpp",
                    "Source/Vulkan/VulkanPipelineLayout.hpp",
                    "Source/Vulkan/VulkanPipelineState.hpp",
                    "Source/Vulkan/VulkanQueryHeap.hpp",
                    "Source/Vulkan/VulkanResource.hpp",
                    "Source/Vulkan/VulkanSampler.hpp",
                    "Source/Vulkan/VulkanSwapChain.hpp");
                Sources("Source/Vulkan/**.cpp", "Source/Vulkan/**.c");
                if(Platform == BuildPlatform.Android)
                {
                    SystemLibraries("android", "vulkan");
                }
                DependsOn("volk", "vulkan-memory-allocator");
                break;
            case RhiApi.Metal:
                Defines("LUNA_RHI_METAL");
                Headers("Source/Metal/**.h");
                MetaHeaders(
                    "Source/Metal/MetalAdapter.h",
                    "Source/Metal/MetalCommandBuffer.h",
                    "Source/Metal/MetalDescriptorSet.h",
                    "Source/Metal/MetalDescriptorSetLayout.h",
                    "Source/Metal/MetalDevice.h",
                    "Source/Metal/MetalDeviceMemory.h",
                    "Source/Metal/MetalFence.h",
                    "Source/Metal/MetalPipelineLayout.h",
                    "Source/Metal/MetalPipelineState.h",
                    "Source/Metal/MetalQueryHeap.h",
                    "Source/Metal/MetalResource.h",
                    "Source/Metal/MetalSwapChain.h",
                    "Source/Metal/MetalTextureView.h");
                Sources("Source/Metal/**.mm");
                Frameworks("Foundation", "QuartzCore", "Metal", "CoreGraphics");
                DependsOn("VariantUtils");
                break;
            default:
                throw new ArgumentOutOfRangeException(nameof(options.RhiApi), options.RhiApi, null);
        }
    }
}
