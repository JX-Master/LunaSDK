-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("Lua")
    add_rules("c++")
    add_headerfiles("*.hpp")
    add_includedirs("Source/lua-5.5.0/src")
    add_files("Source/*cpp")
    add_files("Source/lua-5.5.0/src/*.c")
    remove_files("Source/lua-5.5.0/src/luac.c") -- remove lua compiler CLI.
    remove_files("Source/lua-5.5.0/src/lua.c") -- remove lua CLI.
    add_deps("Runtime")
target_end()