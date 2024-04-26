luna_sdk_module_target("VG")
    add_rules("luna.shader")
    add_headerfiles("*.hpp", {prefixdir = "Luna/VG"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_luna_shader("Source/FillVS.hlsl", {type = "vertex"})
    add_luna_shader("Source/FillPS.hlsl", {type = "pixel"})
    add_deps("Runtime", "RHI", "ShaderCompiler")
target_end()