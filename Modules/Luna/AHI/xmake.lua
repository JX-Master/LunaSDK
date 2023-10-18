add_requires("miniaudio")

luna_sdk_module_target("AHI")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_packages("miniaudio")
    add_luna_modules("Runtime")
target_end()