target("VGTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("Source/**.cpp")
    add_deps("Runtime", "Window", "RHI", "Font", "VG", "HID")
target_end()