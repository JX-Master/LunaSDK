add_requires("stb")

luna_sdk_module_target("Image")
    add_headerfiles("**.hpp")
    add_files("Source/**.cpp")
    add_luna_modules("Runtime")
    add_packages("stb")
target_end()