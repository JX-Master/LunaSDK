target("WindowTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Windows/*.rc")
    end
    add_deps("Runtime", "Window")
target_end()