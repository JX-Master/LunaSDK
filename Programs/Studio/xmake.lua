target("Studio")
    set_luna_sdk_program()
    add_rules("luna.shader")
    add_options("rhi_api")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils", "HID", "Window", "RHI", "Image", "Font", "ImGui", "Asset", "ObjLoader", "RG", "JobSystem")
    
    add_luna_shader("Shaders/MipmapGenerationCS.hlsl", {type = "compute"})
    add_luna_shader("Shaders/SkyboxCS.hlsl", {type = "compute"})
    add_luna_shader("Shaders/ToneMappingCS.hlsl", {type = "compute"})
    add_luna_shader("Shaders/LumHistogramClear.hlsl", {type = "compute"})
    add_luna_shader("Shaders/LumHistogram.hlsl", {type = "compute"})
    add_luna_shader("Shaders/LumHistogramCollect.hlsl", {type = "compute"})
    add_luna_shader("Shaders/GeometryVert.hlsl", {type = "vertex"})
    add_luna_shader("Shaders/GeometryPixel.hlsl", {type = "pixel"})
    add_luna_shader("Shaders/DeferredLighting.hlsl", {type = "compute"})
    add_luna_shader("Shaders/BufferVisualization.hlsl", {type = "compute"})
    add_luna_shader("Shaders/PrecomputeIntegrateBRDF.hlsl", {type = "compute"})
    add_luna_shader("Shaders/PrecomputeEnvironmentMapMips.hlsl", {type = "compute"})
    add_luna_shader("Shaders/WireframeVert.hlsl", {type = "vertex"})
    add_luna_shader("Shaders/WireframePixel.hlsl", {type = "pixel"})
    add_luna_shader("Shaders/BloomSetupCS.hlsl", {type = "compute", debug = true})
    add_luna_shader("Shaders/BloomDownSampleCS.hlsl", {type = "compute", debug = true})
    add_luna_shader("Shaders/BloomUpSampleCS.hlsl", {type = "compute", debug = true})
    add_headerfiles("Shaders/Common.hlsl",
            "Shaders/BRDF.hlsl",
            "Shaders/CameraParams.hlsl",
            "Shaders/CommonVertex.hlsl",
            "Shaders/IBLCommon.hlsl",
            "Shaders/GeometryCommon.hlsl",
            "Shaders/MeshBuffer.hlsl", {install = false})
target_end()