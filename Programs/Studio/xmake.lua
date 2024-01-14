target("Studio")
    set_luna_sdk_program()
    add_options("rhi_api")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils", "HID", "Window", "RHI", "Image", "Font", "ImGui", "Asset", "ObjLoader", "RG")

    local shader_files = {
            "Common.hlsl",
            "BRDF.hlsl",
            "CameraParams.hlsl",
            "CommonVertex.hlsl",
            "IBLCommon.hlsl",
            "MipmapGenerationCS.hlsl",
            "SkyboxCS.hlsl",
            "ToneMappingCS.hlsl",
            "LumHistogramClear.hlsl",
            "LumHistogram.hlsl",
            "LumHistogramCollect.hlsl",
            "GeometryCommon.hlsl",
            "GeometryVert.hlsl",
            "GeometryPixel.hlsl",
            "MeshBuffer.hlsl",
            "DeferredLighting.hlsl",
            "BufferVisualization.hlsl",
            "PrecomputeIntegrateBRDF.hlsl",
            "PrecomputeEnvironmentMapMips.hlsl",
            "WireframeVert.hlsl"
        }

    after_build(function (target)
        local target_dir = target:targetdir()
        local shader_source_dir = vformat("$(scriptdir)/Shaders")
        for _, i in pairs(shader_files) do
            os.cp(path.join(shader_source_dir, i), path.join(target_dir, "Shaders", i))
        end
    end)
    after_clean(function (target) 
        os.rm(path.join(target:targetdir(), "Shaders"))
    end)
    after_install(function (target)
        local shader_source_dir = vformat("$(scriptdir)/Shaders")
        for _, i in pairs(shader_files) do
            os.cp(path.join(shader_source_dir, i), path.join(target:installdir(), "bin", "Shaders", i))
        end
    end)
    after_uninstall(function (target) 
        os.rm(path.join(target:installdir(), "bin", "Shaders"))
    end)
target_end()