package("imgui")
    set_homepage("https://github.com/ocornut/imgui")
    set_description("Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies")
    set_license("MIT")

    on_load("windows|x64", "windows|x86", function (package)
        local install_path = path.join(os.scriptdir(), package:plat(), package:arch())
        if package:config("shared") then
            install_path = path.join(install_path, "Shared")
        else
            install_path = path.join(install_path, "Static")
        end
        package:set("installdir", install_path)
        if package:config("shared") then
            package:addenv("PATH", "bin")
        end
    end)

    on_load("macosx|x86_64", "macosx|arm64", function(package)
        local install_path = path.join(os.scriptdir(), package:plat())
        package:set("installdir", install_path)
        if package:config("shared") then
            package:addenv("PATH", "lib")
        end
    end)

    on_test(function (package)
        local user_config = package:config("user_config")
        assert(user_config ~= nil or package:check_cxxsnippets({test = [[
            void test() {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                ImGui::NewFrame();
                ImGui::Text("Hello, world!");
                ImGui::ShowDemoWindow(NULL);
                ImGui::Render();
                ImGui::DestroyContext();
            }
        ]]}, {configs = {languages = "c++11"}, includes = {"imgui.h"}}))
    end)

    on_fetch("windows|x64", "windows|arm64", function (package)
        local result = {}
        result.links = "ImGuiLib"
        result.linkdirs = package:installdir("lib")
        if package:config("shared") then
            result.libfiles = path.join(package:installdir("bin"), "ImGuiLib.dll")
        end
        result.includedirs = package:installdir("../../include")
        return result
    end)

    on_fetch("macosx|x86_64", "macosx|arm64", function(package)
        local result = {}
        result.links = "ImGuiLib"
        result.linkdirs = package:installdir("lib")
        result.includedirs = package:installdir("include")
        return result
    end)