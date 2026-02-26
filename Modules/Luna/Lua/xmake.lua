add_requires("lua")

luna_sdk_module_target("Lua")
    add_rules("c++")
    add_headerfiles("*.hpp")
    add_files("Source/*cpp")
    add_packages("lua")
    add_deps("Runtime")
target_end()