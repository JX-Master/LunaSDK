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
    if not is_plat("android") then
        before_build (function (target)
            os.cp("$(scriptdir)/Res/luna.png", target:targetdir() .. "/luna.png")
        end)
        after_install (function (target)
            os.cp(target:targetdir() .. "/luna.png", target:installdir() .. "/bin/luna.png")
        end)
    end
target_end()