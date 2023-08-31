add_requires("dxc", "spirv-cross")

luna_sdk_module_target("ShaderCompiler")
    add_headerfiles("*.hpp", "Source/*.hpp")
    add_files("Source/*.cpp")
    add_luna_modules("Runtime", "VariantUtils")
    add_packages("dxc", "spirv-cross")
    if is_os("macosx") then
        add_cxflags("-fms-extensions")
    end
target_end()