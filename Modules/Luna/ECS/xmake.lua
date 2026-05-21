-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("ECS")
    add_headerfiles("*.hpp", {prefixdir = "Luna/ECS"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_deps("Runtime", "JobSystem")
target_end()