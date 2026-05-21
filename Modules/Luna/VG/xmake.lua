-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("VG")
    add_rules("luna.shader")
    add_headerfiles("*.hpp", {prefixdir = "Luna/VG"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_luna_shader("Source/FillVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Source/FillPS.cxx", {type = "pixel", entry_point = "ps_main"})
    add_deps("Runtime", "RHI", "RHIUtility")
target_end()
