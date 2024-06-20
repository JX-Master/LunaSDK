if is_os("windows") or is_os("macosx") or is_os("linux") or is_os("ios") or is_os("android") then
    add_requires("libsdl 2.30.2")
end

luna_sdk_module_target("Window")
    add_options("rhi_api")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Window"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_os("windows") or is_os("macosx") or is_os("linux") or is_os("ios") or is_os("android") then
        add_headerfiles("Source/SDL/*.hpp", {install = false})
        add_files("Source/SDL/*.cpp")
        add_packages("libsdl")
    end
    if is_os("windows") then
        add_headerfiles("(Windows/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/Windows/*.cpp")
    end
    if is_os("macosx") then
        add_files("Source/Cocoa/*.mm")
        add_frameworks("AppKit", "UniformTypeIdentifiers")
    end
    add_deps("Runtime")
target_end()
