add_requires("stb")

luna_sdk_module_target("Font")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Font"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_deps("Runtime")
    add_packages("stb")
target_end()
