add_requires("stb")

target("Font")
    set_luna_sdk_module()
    add_headerfiles("**.hpp")
    add_files("Source/**.cpp")
    add_deps("Runtime")
    add_packages("stb")
target_end()
