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
    add_requires("vulkansdk", {configs = {shared = has_config("shared")}})
end

target("RHI")
    set_luna_sdk_module()
    add_options("rhi_api", "rhi_debug")
    add_headerfiles("*.hpp", "Source/*.hpp")
    add_files("Source/*.cpp")
    if is_config("rhi_api", "D3D12") then
        add_defines("LUNA_RHI_D3D12")
        add_headerfiles("Source/DXGI/**.hpp", "Source/D3D12/**.hpp")
        add_files("Source/D3D12/**.cpp")
    elseif is_config("rhi_api", "Vulkan") then
        add_defines("LUNA_RHI_VULKAN")
        add_headerfiles("Source/Vulkan/**.hpp")
        add_files("Source/Vulkan/**.cpp")
        add_packages("vulkansdk", "glfw")
    elseif is_config("rhi_api", "Metal") then
        add_defines("LUNA_RHI_METAL")
    end
    add_deps("Runtime", "Window")
    -- Generate shader file.
    if is_os("windows") and is_config("rhi_api", "D3D12") then
        before_build(function (target)
            os.mkdir("$(buildir)/Shaders")
            local output_path = vformat("$(buildir)/Shaders")
            local vs_header_path = vformat("$(scriptdir)/Source/D3D12/SwapChainVS.hpp")
            local vs_source_path = vformat("$(scriptdir)/Source/D3D12/SwapChainVS.cpp")
            local ps_header_path = vformat("$(scriptdir)/Source/D3D12/SwapChainPS.hpp")
            local ps_source_path = vformat("$(scriptdir)/Source/D3D12/SwapChainPS.cpp")
            local vs_source = vformat("$(scriptdir)/Source/D3D12/SwapChainVS.hlsl")
            local ps_source = vformat("$(scriptdir)/Source/D3D12/SwapChainPS.hlsl")
            import("compile_shader")
            local runenvs = target:toolchain("msvc"):runenvs()
            compile_shader.compile_shader(vs_source, 
                {type = "vs", shading_model = "5_0", output_path = output_path, envs = runenvs})
            compile_shader.compile_shader(ps_source, 
                {type = "ps", shading_model = "5_0", output_path = output_path, envs = runenvs})
            local vs_path = output_path .. "/SwapChainVS.cso"
            local ps_path = output_path .. "/SwapChainPS.cso"
            import("bin_to_cpp")
            bin_to_cpp.bin_to_cpp(vs_path, vs_header_path, vs_source_path)
            bin_to_cpp.bin_to_cpp(ps_path, ps_header_path, ps_source_path)
        end)
    end
target_end()

