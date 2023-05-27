add_requires("dxc")

luna_sdk_module_target("ShaderCompiler")
    add_headerfiles("*.hpp", "Source/*.hpp")
    add_files("Source/*.cpp")
    add_deps("Runtime")
    add_packages("dxc")
target_end()