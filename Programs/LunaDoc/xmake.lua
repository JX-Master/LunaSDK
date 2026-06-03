-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("LunaDoc")
    set_luna_sdk_program()
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils")
target_end()