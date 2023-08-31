target("RHITestBed")
    add_luna_sdk_options()
    set_group("Tests/RHITest")
    if has_config("shared") then
        set_kind("shared")
    else
        set_kind("static")
    end
    add_headerfiles("*.hpp")
    add_files("*.cpp")
    add_luna_modules("Runtime", "RHI", "Window")
target_end()
