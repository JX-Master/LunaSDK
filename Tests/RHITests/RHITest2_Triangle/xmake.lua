-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("RHITest2_Triangle")
    add_luna_sdk_options()
    add_rules("luna.shader")
    set_group("Tests/RHITest")
    set_kind("binary")
    add_luna_shader("TestTriangleVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("TestTrianglePS.cxx", {type = "pixel", entry_point = "ps_main"})
    add_files("*.cpp")
    add_deps("Runtime", "RHI", "RHITestBed")
target_end()
