add_requires("miniaudio")

luna_sdk_module_target("AHI")
    add_headerfiles("*.hpp", {prefixdir = "Luna/AHI"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("**.cpp")
    add_packages("miniaudio")
    add_deps("Runtime")
target_end()