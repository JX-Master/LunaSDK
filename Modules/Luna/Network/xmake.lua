luna_sdk_module_target("Network")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Network"})
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Platform/Windows/**.cpp")
    elseif is_os("linux", "macosx") then
        add_files("Source/Platform/POSIX/**.cpp")
    end
    add_deps("Runtime")
target_end()