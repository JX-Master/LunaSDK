option("window_glfw")
    set_default(true)
    set_showmenu(true)
    set_description("Whether to use GLFW framework for window.")
    add_defines("LUNA_WINDOW_GLFW")
option_end()

if has_config("window_glfw") then
    add_requires("glfw", {configs = {shared = has_config("shared")}})
end

luna_sdk_module_target("Window")
    add_options("window_glfw")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Window"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if has_config("window_glfw") then
        add_headerfiles("(GLFW/*.hpp)", {prefixdir = "Luna/Window"})
        add_headerfiles("Source/GLFW/*.hpp", {install = false})
        add_files("Source/GLFW/*.cpp")
    end
    if is_os("windows") then
        add_headerfiles("(Windows/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/Windows/*.cpp")
    end
    if is_os("macosx") then
        add_files("Source/Cocoa/*.mm")
        add_frameworks("AppKit", "UniformTypeIdentifiers")
    end
    add_luna_modules("Runtime")
    if has_config("window_glfw") then
        add_packages("glfw")
    end
target_end()

