target("HID")
    set_luna_sdk_module()
    add_headerfiles("*.hpp")
    add_headerfiles("Source/*.hpp")
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_headerfiles("Source/Platform/Windows/*.hpp")
        add_files("Source/Platform/Windows/*.cpp")
    end
    add_deps("Runtime")
target_end()