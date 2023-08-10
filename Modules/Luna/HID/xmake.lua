option("hid_sdl")
    set_default(true)
    set_showmenu(true)
    set_description("Whether to SDL2 for HID inputs.")
    add_defines("LUNA_HID_SDL")
option_end()

if has_config("hid_sdl") then
    add_requires("libsdl", {configs = {shared = has_config("shared")}})
end

luna_sdk_module_target("HID")
    add_headerfiles("*.hpp")
    add_files("Source/*.cpp")
    if has_config("hid_sdl") then
        add_files("Source/Platform/SDL/*.cpp")
        add_packages("libsdl")
    elseif is_os("windows") then
        add_files("Source/Platform/Windows/*.cpp")
    end
    add_deps("Runtime")
target_end()