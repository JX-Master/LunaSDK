target("RHITestBed")
    add_luna_sdk_options()
    set_group("Tests/RHITest")
    if has_config("shared") then
        set_kind("shared")
    else
        set_kind("static")
    end
    add_headerfiles("*.hpp", {install = false})
    add_files("*.cpp")
    add_deps("Runtime", "RHI", "RHIUtility", "Window")
target_end()
