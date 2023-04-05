target("Studio")
    set_luna_sdk_program()
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_deps("Runtime", "HID", "Window", "RHI", "Image", "Font", "ImGui", "Asset", "ObjLoader", "RG")
    add_packages("imgui")

    after_build(function (target)
        import("compile_shader")
        local runenvs = target:toolchain("msvc"):runenvs()
        local target_dir = target:targetdir()
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/LightingPassPixel.hlsl"), 
            {type = "ps", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/MipmapGenerationCS.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/SkyboxCS.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/ToneMappingCS.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/LumHistogram.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/LumHistogramCollect.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/GeometryPixel.hlsl"), 
            {type = "ps", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/DepthVert.hlsl"), 
            {type = "vs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/DepthPixel.hlsl"), 
            {type = "ps", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/DeferredLighting.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
        compile_shader.compile_shader(vformat("$(scriptdir)/Shaders/BufferVisualization.hlsl"), 
            {type = "cs", shading_model = "5_1", output_path = target_dir, envs = runenvs})
    end)

    after_install(function (target)
        local shader_files = {
            "Studio.exe",
            "LightingPassPixel.cso",
            "MipmapGenerationCS.cso",
            "SkyboxCS.cso",
            "ToneMappingCS.cso",
            "LumHistogram.cso",
            "LumHistogramCollect.cso",
            "GeometryPixel.cso",
            "DepthVert.cso",
            "DepthPixel.cso",
            "DeferredLighting.cso",
            "BufferVisualization.cso"
        }

        for _, i in pairs(shader_files) do
            os.cp(path.join(target:targetdir(), i), path.join(target:installdir(), "bin", i))
        end
    end)
    
target_end()