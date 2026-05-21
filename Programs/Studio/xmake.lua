-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("Studio")
    set_luna_sdk_program()
    add_rules("luna.shader")
    if is_plat("macosx") then
        add_rules("xcode.application")
        add_files("Info.plist")
        add_values("xcode.bundle_identifier", "com.lunasdk.studio")
    end
    add_options("rhi_api")
    add_headerfiles("**.hpp")
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils", "HID", "Window", "RHI", "Image", "Font", "ImGui", "Asset", "ObjLoader", "RG", "JobSystem", "ECS")
    
    add_luna_shader("Shaders/SkyboxCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/ToneMappingCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/LumHistogramClear.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/LumHistogram.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/LumHistogramCollect.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/GeometryVert.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Shaders/GeometryPixel.cxx", {type = "pixel", entry_point = "ps_main"})
    add_luna_shader("Shaders/DeferredLighting.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/BufferVisualization.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/PrecomputeIntegrateBRDF.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/PrecomputeEnvironmentMapMips.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/WireframeVert.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Shaders/WireframePixel.cxx", {type = "pixel", entry_point = "ps_main"})
    add_luna_shader("Shaders/BloomSetupCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/BloomDownSampleCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/BloomUpSampleCS.cxx", {type = "compute", entry_point = "cs_main"})
    add_luna_shader("Shaders/GridVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("Shaders/GridPS.cxx", {type = "pixel", entry_point = "ps_main"})
    add_headerfiles("Shaders/*.hxx", {install = false})
target_end()
