if is_os("windows") or is_os("macosx") then
    add_requires("glfw")
end

luna_sdk_module_target("Window")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Window"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_os("windows") or is_os("macosx") then
        add_headerfiles("(GLFW/*.hpp)", {prefixdir = "Luna/Window"})
        add_headerfiles("Source/GLFW/*.hpp", {install = false})
        add_files("Source/GLFW/*.cpp")
        add_defines("LUNA_WINDOW_GLFW")
        add_packages("glfw")
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
