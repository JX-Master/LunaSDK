option("window_glfw")
    set_default(true)
    set_showmenu(true)
    set_description("Whether to use GLFW framework for window.")
    add_defines("LUNA_WINDOW_GLFW")
option_end()

if has_config("window_glfw") then
    add_requires("glfw", {configs = {shared = has_config("build_shared_lib")}})
end

target("Window")
    set_luna_sdk_module()
    add_options("window_glfw")
    add_headerfiles("*.hpp", "Source/*.hpp")
    add_files("Source/*.cpp")
    if has_config("window_glfw") then
        add_headerfiles("GLFW/*.hpp", "Source/GLFW/*.hpp")
        add_files("Source/GLFW/*.cpp")
    end
    if is_os("windows") then
        add_headerfiles("Windows/*.hpp")
        add_files("Source/Windows/*.cpp")
    end
    add_deps("Runtime")
    if has_config("window_glfw") then
        add_packages("glfw")
    end
target_end()

