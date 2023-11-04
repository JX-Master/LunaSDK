luna_sdk_module_target("Runtime")
    add_headerfiles("*.hpp")
    add_headerfiles("Math/**.hpp")
    add_headerfiles("Source/*.hpp")
    add_headerfiles("Source/Math/**.inl")
    add_files("Source/*.cpp")
    if is_plat("windows") then
        add_headerfiles("Platform/Windows/**.hpp")
        add_headerfiles("Source/Platform/Windows/*.hpp")
        add_files("Source/Platform/Windows/*.cpp")
    elseif is_plat("linux", "macosx", "android") then
        add_headerfiles("Source/Platform/POSIX/*.hpp")
        add_files("Source/Platform/POSIX/*.cpp")
        if is_plat("linux", "macosx") then
            add_syslinks("pthread")
        end
    end
target_end()