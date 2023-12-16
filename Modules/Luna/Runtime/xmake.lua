luna_sdk_module_target("Runtime")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Impl/**.hpp)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Impl/**.inl)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Math/**.hpp)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Math/**.inl)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_plat("windows") then
        add_headerfiles("(Platform/Windows/**.hpp)", {prefixdir = "Luna/Runtime"})
        add_headerfiles("Source/Platform/Windows/*.hpp", {install = false})
        add_files("Source/Platform/Windows/*.cpp")
    elseif is_plat("linux", "macosx", "android") then
        add_headerfiles("Source/Platform/POSIX/*.hpp", {install = false})
        add_files("Source/Platform/POSIX/*.cpp")
        if is_plat("linux", "macosx") then
            add_syslinks("pthread")
        end
    end
target_end()