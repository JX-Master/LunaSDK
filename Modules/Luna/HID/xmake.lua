luna_sdk_module_target("HID")
    add_headerfiles("*.hpp", {prefixdir = "Luna/HID"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Platform/Windows/*.cpp")
    elseif is_os("macosx") then 
        add_files("Source/Platform/MacOS/*.mm")
        add_frameworks("ApplicationServices", "AppKit")
    end
    add_deps("Runtime")
target_end()