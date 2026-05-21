-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("RHITest1_FillBackBuffer")
    add_luna_sdk_options()
    set_group("Tests/RHITest")
    set_kind("binary")
    add_files("*.cpp")
    add_deps("Runtime", "RHI", "RHITestBed")
target_end()