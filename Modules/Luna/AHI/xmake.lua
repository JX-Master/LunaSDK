-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

add_requires("miniaudio", {configs = {header_only = true}})

luna_sdk_module_target("AHI")
    add_headerfiles("*.hpp", {prefixdir = "Luna/AHI"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("**.cpp")
    if is_plat("macosx", "iphoneos") then
        remove_files("Source/MiniAudio/miniaudio.cpp") 
        add_files("Source/MiniAudio/miniaudio.mm")
    end
    add_packages("miniaudio")
    add_deps("Runtime")
target_end()