add_requires("miniaudio")

luna_sdk_module_target("Audio")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_packages("miniaudio")
target_end()