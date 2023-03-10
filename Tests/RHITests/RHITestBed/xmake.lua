target("RHITestBed")
    add_luna_sdk_options()
    set_group("Tests/RHITest")
    if has_config("build_shared_lib") then
        set_kind("shared")
    else
        set_kind("static")
    end
    add_headerfiles("*.hpp")
    add_files("*.cpp")
    add_deps("Runtime", "RHI", "Window")
target_end()
