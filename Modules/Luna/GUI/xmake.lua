luna_sdk_module_target("GUI")
    add_headerfiles("*.hpp", {prefixdir = "Luna/GUI"})
    --add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/*.cpp")
    add_deps("Runtime", "VG")
target_end()