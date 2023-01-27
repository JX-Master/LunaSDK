target("WindowTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Windows/*.cpp")
        add_files("Source/Windows/*.rc")
    elseif is_os("linux") then
        add_files("Source/XLib/*.cpp")
    elseif is_os("macosx") then
        add_files("Source/Cocoa/*.cpp")
    end
    add_deps("Runtime", "Window")
target_end()