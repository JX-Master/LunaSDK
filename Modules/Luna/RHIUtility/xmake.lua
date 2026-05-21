-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("RHIUtility")
    add_rules("luna.shader")
    add_headerfiles("**.hpp")
    add_files("Source/**.cpp")
    add_deps("Runtime", "RHI")
    add_luna_shader("Source/Shaders/MipmapGeneration1DCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Source/Shaders/MipmapGeneration2DCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Source/Shaders/MipmapGeneration3DCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Source/Shaders/BlitVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Source/Shaders/BlitPS.cxx", {type = "pixel", entry_point = "ps_main"})
target_end()
