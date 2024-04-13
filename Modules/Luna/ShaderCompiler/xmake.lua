add_requires("spirv-cross")

luna_sdk_module_target("ShaderCompiler")
    add_headerfiles("*.hpp", {prefixdir = "Luna/ShaderCompiler"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/*.cpp")
    add_deps("Runtime", "VariantUtils")
    add_packages("spirv-cross")
    if is_os("windows") then 
        add_includedirs("$(projectdir)/SDKs/dxc/windows/include")
        if is_arch("x64") then 
            add_linkdirs("$(projectdir)/SDKs/dxc/windows/x64/lib")
        elseif is_arch("arm64") then 
            add_linkdirs("$(projectdir)/SDKs/dxc/windows/arm64/lib")
        end
    elseif is_os("macosx") then
        add_includedirs("$(projectdir)/SDKs/dxc/macosx/include")
        add_cxflags("-fms-extensions")
        add_linkdirs("$(projectdir)/SDKs/dxc/macosx/lib")
        add_rpathdirs("@executable_path/.")
    end
    add_links("dxcompiler")
    after_build(function(target)
        if target:is_plat("windows") then 
            if target:is_arch("x64") then 
                os.cp("$(projectdir)/SDKs/dxc/windows/x64/bin/dxcompiler.dll", path.join(target:targetdir(), "dxcompiler.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/x64/bin/dxil.dll", path.join(target:targetdir(), "dxil.dll"))
            elseif target:is_arch("arm64") then 
                os.cp("$(projectdir)/SDKs/dxc/windows/arm64/bin/dxcompiler.dll", path.join(target:targetdir(), "dxcompiler.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/arm64/bin/dxil.dll", path.join(target:targetdir(), "dxil.dll"))
            end
        end
    end)
    after_clean(function(target)
        if target:is_plat("windows") then 
            os.rm(path.join(target:targetdir(), "dxcompiler.dll"))
            os.rm(path.join(target:targetdir(), "dxil.dll"))
        end
    end)
    after_install(function(target) 
        if target:is_plat("windows") then 
            if target:is_arch("x64") then 
                os.cp("$(projectdir)/SDKs/dxc/windows/x64/bin/dxcompiler.dll", path.join(target:installdir(), "bin", "dxcompiler.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/x64/bin/dxil.dll", path.join(target:installdir(), "bin", "dxil.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/x64/lib/dxcompiler.lib", path.join(target:installdir(), "lib", "dxcompiler.lib"))
            elseif target:is_arch("arm64") then 
                os.cp("$(projectdir)/SDKs/dxc/windows/arm64/bin/dxcompiler.dll", path.join(target:installdir(), "bin", "dxcompiler.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/arm64/bin/dxil.dll", path.join(target:installdir(), "bin", "dxil.dll"))
                os.cp("$(projectdir)/SDKs/dxc/windows/arm64/lib/dxcompiler.lib", path.join(target:installdir(), "lib", "dxcompiler.lib"))
            end
        elseif target:is_plat("macosx") then
            os.cp("$(projectdir)/SDKs/dxc/macosx/lib/libdxcompiler.dylib", path.join(target:installdir(), "bin", "libdxcompiler.dylib"))
        end
    end)
    after_uninstall(function(target) 
        if target:is_plat("windows") then 
            os.rm(path.join(target:installdir(), "bin", "dxcompiler.dll"))
            os.rm(path.join(target:installdir(), "bin", "dxil.dll"))
            os.rm(path.join(target:installdir(), "lib", "dxcompiler.lib"))
        elseif target:is_plat("macosx") then
            os.rm(path.join(target:installdir(), "bin", "libdxcompiler.dylib"))
        end
    end)
target_end()
