-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("ImGui")
    add_rules("luna.shader")
    add_headerfiles("*.hpp", {prefixdir = "Luna/ImGui"})
    add_headerfiles("*.h", {prefixdir = "Luna/ImGui"})
    add_headerfiles("Source/**.h", {install = false})
    add_files("Source/**.cpp")
    add_luna_shader("Source/ImGuiVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Source/ImGuiPS.cxx", {type = "pixel", entry_point = "ps_main"})
    add_includedirs(".")
    add_defines("LUNA_IMGUI_IMPL")
    add_deps("Window", "Runtime", "RHI", "RHIUtility", "HID", "Font")
target_end()
