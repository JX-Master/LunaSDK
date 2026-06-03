-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

add_requires("stb")

luna_sdk_module_target("Image")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Image"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_deps("Runtime")
    add_packages("stb")
target_end()