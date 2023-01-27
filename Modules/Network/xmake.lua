target("Network")
    set_luna_sdk_module()
    add_headerfiles("*.hpp")
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Platform/Windows/**.cpp")
    elseif is_os("linux", "macosx") then
        add_files("Source/Platform/POSIX/**.cpp")
    end
    add_deps("Runtime")
target_end()