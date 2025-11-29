target("MultiPlatformSample")
    set_luna_sdk_program()
    add_rules("luna.shader")
    add_files("Source/**.cpp")
    add_deps("Runtime", "Window", "RHI", "RHIUtility", "ShaderCompiler", "Image")
    add_luna_shader("Source/BoxVert.hlsl", {type = "vertex"})
    add_luna_shader("Source/BoxPixel.hlsl", {type = "pixel"})
    if is_plat("android") then 
        add_syslinks("android")
    end
    before_build( function (target)
        import("bin_to_cpp")
        local file_dir = path.join(target:scriptdir(), "Res", "Luna.png")
        local header_file = path.join(target:autogendir(), "LunaTex.hpp")
        if not os.exists(header_file) then
            bin_to_cpp.bin_to_cpp_header_only(file_dir, header_file)
        end
        target:add("includedirs", target:autogendir())
    end)
target_end()