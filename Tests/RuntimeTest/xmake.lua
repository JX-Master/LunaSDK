-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("RuntimeTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_headerfiles("Source/*.hpp")
    add_files("Source/*.cpp")
    add_deps("Runtime")
target_end()