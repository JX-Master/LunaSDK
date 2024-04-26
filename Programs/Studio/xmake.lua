target("Studio")
    set_luna_sdk_program()
    add_rules("luna.shader")
    add_options("rhi_api")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils", "HID", "Window", "RHI", "Image", "Font", "ImGui", "Asset", "ObjLoader", "RG", "JobSystem")
    
    add_files("Shaders/MipmapGenerationCS.hlsl", {type = "compute"})
    add_files("Shaders/SkyboxCS.hlsl", {type = "compute"})
    add_files("Shaders/ToneMappingCS.hlsl", {type = "compute"})
    add_files("Shaders/LumHistogramClear.hlsl", {type = "compute"})
    add_files("Shaders/LumHistogram.hlsl", {type = "compute"})
    add_files("Shaders/LumHistogramCollect.hlsl", {type = "compute"})
    add_files("Shaders/GeometryVert.hlsl", {type = "vertex"})
    add_files("Shaders/GeometryPixel.hlsl", {type = "pixel"})
    add_files("Shaders/DeferredLighting.hlsl", {type = "compute"})
    add_files("Shaders/BufferVisualization.hlsl", {type = "compute"})
    add_files("Shaders/PrecomputeIntegrateBRDF.hlsl", {type = "compute"})
    add_files("Shaders/PrecomputeEnvironmentMapMips.hlsl", {type = "compute"})
    add_files("Shaders/WireframeVert.hlsl", {type = "vertex"})
    add_files("Shaders/WireframePixel.hlsl", {type = "pixel"})
    add_files("Shaders/BloomSetupCS.hlsl", {type = "compute", debug = true})
    add_files("Shaders/BloomDownSampleCS.hlsl", {type = "compute", debug = true})
    add_files("Shaders/BloomUpSampleCS.hlsl", {type = "compute", debug = true})
    add_headerfiles("Shaders/Common.hlsl",
            "Shaders/BRDF.hlsl",
            "Shaders/CameraParams.hlsl",
            "Shaders/CommonVertex.hlsl",
            "Shaders/IBLCommon.hlsl",
            "Shaders/GeometryCommon.hlsl",
            "Shaders/MeshBuffer.hlsl", {install = false})
target_end()