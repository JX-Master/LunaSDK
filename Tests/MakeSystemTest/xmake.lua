-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("MakeSystemTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Main.cpp")
    add_deps("Runtime", "MakeSystem")
target_end()
