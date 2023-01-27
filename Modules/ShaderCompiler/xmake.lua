add_requires("dxc")

target("ShaderCompiler")
    set_luna_sdk_module()
    add_headerfiles("*.hpp", "Source/*.hpp")
    add_files("Source/*.cpp")
    add_deps("Runtime", "VFS")
    add_packages("dxc")
target_end()