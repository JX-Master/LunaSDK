luna_sdk_module_target("ImGui")
    add_headerfiles("*.hpp", {prefixdir = "Luna/ImGui"})
    add_files("Source/**.cpp")
    add_luna_modules("Runtime", "RHI", "HID", "Font", "ShaderCompiler")
    add_packages("imgui", {})

    after_install(function(target)
        local imgui_package = target:pkg("imgui")
        local imgui_includedir = imgui_package:installdir()
        if os.host() == "windows" then 
            imgui_includedir = path.join(imgui_includedir, "..", "..", "include")
        elseif os.host() == "macosx" then 
            imgui_includedir = path.join(imgui_includedir, "include")
        end
        print(imgui_includedir)
        os.cp(path.join(imgui_includedir, "*.h"), path.join(target:installdir(), "include", "Luna", "ImGui"))
    end)
target_end()
