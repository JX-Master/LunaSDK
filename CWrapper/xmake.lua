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

luna_sdk_module_c_wrapper("VFSC")
    add_files("VFS/*.cpp")
    add_headerfiles("VFS/*.h", {prefixdir = "Luna/CWrapper/VFS"})
    add_deps("VFS")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("FontC")
    add_files("Font/*.cpp")
    add_headerfiles("Font/*.h", {prefixdir = "Luna/CWrapper/Font"})
    add_deps("Font")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("AssetC")
    add_files("Asset/*.cpp")
    add_headerfiles("Asset/*.h", {prefixdir = "Luna/CWrapper/Asset"})
    add_deps("Asset")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("VGC")
    add_files("VG/*.cpp")
    add_headerfiles("VG/*.h", {prefixdir = "Luna/CWrapper/VG"})
    add_deps("VG", "RHI", "Font")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("ImGuiC")
    add_files("ImGui/*.cpp")
    add_headerfiles("ImGui/*.h", {prefixdir = "Luna/CWrapper/ImGui"})
    add_deps("ImGui")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("HIDC")
    add_files("HID/*.cpp")
    add_headerfiles("HID/*.h", {prefixdir = "Luna/CWrapper/HID"})
    add_deps("HID")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()

luna_sdk_module_c_wrapper("AHIC")
    add_files("AHI/*.cpp")
    add_headerfiles("AHI/*.h", {prefixdir = "Luna/CWrapper/AHI"})
    add_deps("AHI")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
target_end()
