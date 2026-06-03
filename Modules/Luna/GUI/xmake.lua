option("gui_debug")
    set_default(false)
    set_showmenu(true)
    set_description("Whether to enable GUI debug features. This is always enabled in debug mode.")
option_end()

luna_sdk_module_target("GUI")
    add_options("gui_debug")
    if is_mode("debug") or has_config("gui_debug") then
        add_defines("LUNA_GUI_ENABLE_DEBUG", {public = true})
    end
    add_headerfiles("*.hpp", {prefixdir = "Luna/GUI"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_deps("Runtime", "RHI", "VG", "Font")
target_end()
