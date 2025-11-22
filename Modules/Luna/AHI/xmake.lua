add_requires("miniaudio", {configs = {header_only = true}})

luna_sdk_module_target("AHI")
    add_headerfiles("*.hpp", {prefixdir = "Luna/AHI"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("**.cpp")
    remove_files("Source/MiniAudio/miniaudio.cpp")
    if is_plat("macosx", "iphoneos") then 
        add_files("Source/MiniAudio/miniaudio.mm")
    else 
        add_files("Source/MiniAudio/miniaudio.cpp")
    end
    add_packages("miniaudio")
    add_deps("Runtime")
target_end()