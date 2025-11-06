option("sdl3_window")
    set_default(false)
    set_showmenu(true)
    set_description("Whether to use SDL3 for window backend.")
option_end()

if (has_config("sdl3_window") and (is_os("windows") or is_os("macosx"))) or is_os("linux") or is_os("ios") or is_os("android") then
    add_requires("libsdl3")
end

luna_sdk_module_target("Window")
    add_options("rhi_api")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Window"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if (has_config("sdl3_window") and (is_os("windows") or is_os("macosx"))) or is_os("linux") or is_os("ios") or is_os("android") then
        add_headerfiles("Source/Backend/SDL/*.hpp", {install = false})
        add_headerfiles("(SDL/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/Backend/SDL/*.cpp")
        add_packages("libsdl3", {public=true})
    elseif is_os("windows") then
        add_headerfiles("Source/Backend/Windows/*.hpp", {install = false})
        add_files("Source/Backend/Windows/*.cpp")
    elseif is_os("macosx") then 
        add_headerfiles("Source/Backend/Cocoa/*.h", {install = false})
        add_files("Source/Backend/Cocoa/*.mm")
    end
    if is_os("windows") then
        add_headerfiles("(Windows/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/Windows/*.cpp")
    end
    if is_os("macosx") then
        add_headerfiles("(Cocoa/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/Cocoa/*.mm")
        add_frameworks("AppKit", "UniformTypeIdentifiers")
    end
    add_deps("Runtime")
target_end()
