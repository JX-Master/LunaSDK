target("Runtime")
    set_luna_sdk_module()
    add_headerfiles("*.hpp")
    add_headerfiles("Math/**.hpp")
    add_headerfiles("Source/Math/**.inl")
    add_files("Source/*.cpp")
    remove_files("Source/FuncInfo.cpp")
    if is_os("windows") then
        add_headerfiles("Platform/Windows/**.hpp")
        add_headerfiles("Source/Platform/Windows/*.hpp")
        add_files("Source/Platform/Windows/*.cpp")
    elseif is_os("linux", "macosx") then
        add_headerfiles("Source/Platform/POSIX/*.hpp")
        add_files("Source/Platform/POSIX/*.cpp")
        add_syslinks("pthread")
    end

    if has_config("check_memory_leak") then
        add_defines("LUNA_RUNTIME_CHECK_MEMORY_LEAK")
    end
target_end()