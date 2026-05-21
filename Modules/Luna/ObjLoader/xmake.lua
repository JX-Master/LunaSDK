-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("ObjLoader")
    add_headerfiles("*.hpp", {prefixdir = "Luna/ObjLoader"})
    add_files("Source/**.cpp")
    add_deps("Runtime")
target_end()