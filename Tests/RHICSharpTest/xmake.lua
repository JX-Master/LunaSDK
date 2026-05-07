target("RHICSharpShaderAssets")
    add_luna_sdk_options()
    add_rules("luna.shader")
    set_group("Tests")
    set_kind("binary")
    set_default(false)
    add_files("ShaderAssetsDummy.cpp")
    add_luna_shader("TestComputeCS.cxx", {type = "compute", entry_point = "cs_main"})
target_end()

function luna_rhi_csharp_test_target(name, entrypoint)
    target(name)
        set_kind("binary")
        set_group("Tests")
        add_files("*.cs")
        add_files(entrypoint)
        add_deps("Luna.Runtime", "Luna.Window", "Luna.RHI", "Luna.RHIUtility", "Luna.Image", "RHITest2_Triangle", "RHITest3_Texture", "RHITest4_Box", "RHICSharpShaderAssets")
        set_luna_sdk_csharp_options()
        after_build(function(target)
            local target_dir = target:targetdir()
            os.cp(path.join("$(projectdir)", "Tests", "RHITests", "RHITest3_Texture", "uv_checker.png"), path.join(target_dir, "uv_checker.png"))
            os.cp(path.join("$(projectdir)", "Tests", "RHITests", "RHITest4_Box", "luna.png"), path.join(target_dir, "luna.png"))

            local extension = nil
            if is_config("rhi_api", "D3D12") then
                extension = "dxil"
            elseif is_config("rhi_api", "Vulkan") then
                extension = "spv"
            elseif is_config("rhi_api", "Metal") then
                extension = "metallib"
            else
                os.raise("Unsupported RHI backend for RHICSharpTest shader copy.")
            end

            local shaders =
            {
                { native_target = "RHITest2_Triangle", shader_name = "TestTriangleVS" },
                { native_target = "RHITest2_Triangle", shader_name = "TestTrianglePS" },
                { native_target = "RHITest3_Texture", shader_name = "TestTextureVS" },
                { native_target = "RHITest3_Texture", shader_name = "TestTexturePS" },
                { native_target = "RHITest4_Box", shader_name = "TestBoxVS" },
                { native_target = "RHITest4_Box", shader_name = "TestBoxPS" },
                { native_target = "RHICSharpShaderAssets", shader_name = "TestComputeCS" }
            }
            for _, shader in ipairs(shaders) do
                local source_path = path.join(
                    "$(projectdir)",
                    "build",
                    ".gens",
                    shader.native_target,
                    get_config("plat"),
                    get_config("arch"),
                    get_config("mode"),
                    "shaders",
                    ".cppsl",
                    shader.shader_name,
                    shader.shader_name .. "." .. extension)
                os.cp(source_path, path.join(target_dir, shader.shader_name .. "." .. extension))
            end
        end)
        after_clean(function(target)
            local target_dir = target:targetdir()
            local files =
            {
                "uv_checker.png",
                "luna.png",
                "TestTriangleVS.dxil",
                "TestTrianglePS.dxil",
                "TestTextureVS.dxil",
                "TestTexturePS.dxil",
                "TestBoxVS.dxil",
                "TestBoxPS.dxil",
                "TestTriangleVS.spv",
                "TestTrianglePS.spv",
                "TestTextureVS.spv",
                "TestTexturePS.spv",
                "TestBoxVS.spv",
                "TestBoxPS.spv",
                "TestTriangleVS.metallib",
                "TestTrianglePS.metallib",
                "TestTextureVS.metallib",
                "TestTexturePS.metallib",
                "TestBoxVS.metallib",
                "TestBoxPS.metallib",
                "TestComputeCS.dxil",
                "TestComputeCS.spv",
                "TestComputeCS.metallib"
            }
            for _, file in ipairs(files) do
                os.rm(path.join(target_dir, file))
            end
        end)
    target_end()
end

luna_rhi_csharp_test_target("RHICSharpTest", "EntryPoints/All.cs")
luna_rhi_csharp_test_target("RHICSharpTest0_Empty", "EntryPoints/Empty.cs")
luna_rhi_csharp_test_target("RHICSharpTest1_Clear", "EntryPoints/Clear.cs")
luna_rhi_csharp_test_target("RHICSharpTest2_Triangle", "EntryPoints/Triangle.cs")
luna_rhi_csharp_test_target("RHICSharpTest3_Texture", "EntryPoints/Texture.cs")
luna_rhi_csharp_test_target("RHICSharpTest4_Box", "EntryPoints/Box.cs")
