luna_sdk_module_target("Window")
    add_options("rhi_api")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Window"})
    add_headerfiles("Source/*.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_headerfiles("Source/Backend/Windows/*.hpp", {install = false})
        add_files("Source/Backend/Windows/*.cpp")
    elseif is_os("macosx") then 
        add_headerfiles("Source/Backend/Cocoa/*.h", {install = false})
        add_files("Source/Backend/Cocoa/*.mm")
    elseif is_os("ios") then
        add_headerfiles("Source/Backend/UIKit/*.h", {install = false})
        add_files("Source/Backend/UIKit/*.mm")
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
    if is_os("ios") then
        add_headerfiles("(UIKit/*.hpp)", {prefixdir = "Luna/Window"})
        add_files("Source/UIKit/*.mm")
        add_frameworks("UIKit", "CoreGraphics", "QuartzCore")
    end
    add_deps("Runtime")
target_end()
