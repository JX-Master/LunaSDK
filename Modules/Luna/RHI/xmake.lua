local default_rhi_api = false
if is_os("windows") then
    default_rhi_api = "D3D12"
elseif is_os("macosx", "ios") then
    default_rhi_api = "Metal"
elseif is_os("linux", "android") then
    default_rhi_api = "Vulkan"
end
if default_rhi_api == false then
    raise("No Graphics API is present for the current platform!")
end

option("rhi_api")
    set_default(default_rhi_api)
    set_showmenu(true)
    if is_os("windows") then
        set_values("D3D12", "Vulkan")
    elseif is_os("macosx", "ios") then
        set_values("Metal")
    elseif is_os("linux", "android") then
        set_values("Vulkan")
    end
    set_description("The Graphics API to use for RHI")
option_end()

option("rhi_debug")
    set_default(false)
    set_showmenu(true)
    set_description("Whether to enable debug layer for the RHI.")
    add_defines("LUNA_RHI_DEBUG")
option_end()

if is_config("rhi_api", "Vulkan") then 
    add_requires("volk", {configs = {header_only = true}})
    add_requires("vulkan-memory-allocator")
elseif is_config("rhi_api", "D3D12") then 
    add_requires("d3d12-memory-allocator")
end

luna_sdk_module_target("RHI")
    add_options("rhi_api", "rhi_debug")
    add_headerfiles("*.hpp", {prefixdir = "Luna/RHI"})
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
        add_packages("volk", "vulkan-memory-allocator", "glfw")
    elseif is_config("rhi_api", "Metal") then
        add_defines("LUNA_RHI_METAL")
        add_headerfiles("Source/Metal/**.hpp", {install = false})
        add_files("Source/Metal/**.cpp", "Source/Metal/**.mm")
        add_frameworks("Foundation", "QuartzCore", "Metal")
        add_deps("VariantUtils")
    end
    add_deps("Runtime", "Window")
target_end()

