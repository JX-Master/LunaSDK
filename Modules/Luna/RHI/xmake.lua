-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

option("rhi_debug")
    set_default(false)
    set_showmenu(true)
    set_description("Whether to enable debug layer for the RHI.")
    add_defines("LUNA_RHI_DEBUG")
option_end()

luna_sdk_module_target("RHI")
    add_options("rhi_api", "rhi_debug")
    add_headerfiles(
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
        {prefixdir = "Luna/RHI"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_config("rhi_api", "D3D12") then
        add_defines("LUNA_RHI_D3D12")
        add_headerfiles("Source/DXGI/**.hpp", "Source/D3D12/**.hpp", {install = false})
        add_files("Source/D3D12/**.cpp")
        add_packages("d3d12-memory-allocator")
    elseif is_config("rhi_api", "Vulkan") then
        add_defines("LUNA_RHI_VULKAN")
        add_headerfiles("Source/Vulkan/**.hpp", {install = false})
        add_files("Source/Vulkan/**.cpp", "Source/Vulkan/**.c")
        add_packages("volk", "vulkan-memory-allocator")
    elseif is_config("rhi_api", "Metal") then
        add_defines("LUNA_RHI_METAL")
        add_headerfiles("Source/Metal/**.h", {install = false})
        add_files("Source/Metal/**.mm")
        add_frameworks("Foundation", "QuartzCore", "Metal", "CoreGraphics")
        add_deps("VariantUtils")
    end
    add_deps("Runtime", "Window")
target_end()
