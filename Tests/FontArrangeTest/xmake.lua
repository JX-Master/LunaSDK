target("FontArrangeTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Source/**.cpp")
    add_deps("Runtime", "Window", "RHI", "Font", "VG")
target_end()