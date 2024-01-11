luna_sdk_module_target("VG")
    add_headerfiles("*.hpp", {prefixdir = "Luna/VG"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/**.cpp")
    add_deps("Runtime", "RHI", "ShaderCompiler")
target_end()