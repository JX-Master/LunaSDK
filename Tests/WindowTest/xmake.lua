-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("WindowTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Source/*.cpp")
    if is_os("windows") then
        add_files("Source/Windows/*.rc")
    end
    add_deps("Runtime", "Window")
target_end()