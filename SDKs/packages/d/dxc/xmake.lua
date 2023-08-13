package("dxc")
    set_homepage("https://github.com/microsoft/DirectXShaderCompiler/")
    set_description("DirectX Shader Compiler")
    set_license("LLVM")

    on_load("windows|x64", "windows|arm64", function (package)
        local install_path = path.join(os.scriptdir(), package:plat(), package:arch())
        package:set("installdir", install_path)
        package:addenv("PATH", "bin")
    end)

    on_load("macosx|x86_64", "macosx|arm64", function(package)
        local install_path = path.join(os.scriptdir(), package:plat())
        package:set("installdir", install_path)
        package:addenv("PATH", "bin")
    end)

    on_test(function (package)
        os.vrun("dxc -help")
        assert(package:has_cxxfuncs("DxcCreateInstance", {includes = {"windows.h", "dxcapi.h"}}))
    end)

    on_fetch("windows|x64", "windows|arm64", function (package)
        local result = {}
        result.links = "dxcompiler"
        result.linkdirs = package:installdir("lib")
        result.includedirs = package:installdir("../include")
        result.libfiles = {
            path.join(package:installdir("bin"), "dxcompiler.dll"),
            path.join(package:installdir("bin"), "dxil.dll") }
        return result
    end)

    on_fetch("macosx|x86_64", "macosx|arm64", function(package)
        local result = {}
        result.links = "dxcompiler"
        result.linkdirs = package:installdir("lib")
        result.includedirs = package:installdir("include")
        return result
    end)