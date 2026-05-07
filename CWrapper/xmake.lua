luna_sdk_module_c_wrapper("RuntimeC")
    add_files("Runtime/*.cpp")
    add_headerfiles("Runtime/*.h", {prefixdir = "Luna/CWrapper/Runtime"})
    add_deps("Runtime")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("WindowC")
    add_files("Window/*.cpp")
    add_headerfiles("Window/*.h", {prefixdir = "Luna/CWrapper/Window"})
    add_deps("Window")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("RHIC")
    add_files("RHI/*.cpp")
    add_headerfiles("RHI/*.h", {prefixdir = "Luna/CWrapper/RHI"})
    add_deps("RHI")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("RHIUtilityC")
    add_files("RHIUtility/*.cpp")
    add_headerfiles("RHIUtility/*.h", {prefixdir = "Luna/CWrapper/RHIUtility"})
    add_deps("RHIUtility", "RHI")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("ImageC")
    add_files("Image/*.cpp")
    add_headerfiles("Image/*.h", {prefixdir = "Luna/CWrapper/Image"})
    add_deps("Image")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()
