namespace LunaBuild.Core.Targets;

public sealed class StudioTargetRules : TargetRules
{
    public StudioTargetRules()
        : base(
            name: "Studio",
            targetDirectory: "Programs/Studio",
            rulesPath: "Programs/Studio/Studio.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Kind = BuildTargetKind.Executable;
        Headers("**.hpp", "Shaders/*.hxx");
        Sources("**.cpp");
        Shader("Shaders/SkyboxCS.cxx", "compute", "cs_main");
        Shader("Shaders/ToneMappingCS.cxx", "compute", "cs_main");
        Shader("Shaders/LumHistogramClear.cxx", "compute", "cs_main");
        Shader("Shaders/LumHistogram.cxx", "compute", "cs_main");
        Shader("Shaders/LumHistogramCollect.cxx", "compute", "cs_main");
        Shader("Shaders/GeometryVert.cxx", "vertex", "vs_main");
        Shader("Shaders/GeometryPixel.cxx", "pixel", "ps_main");
        Shader("Shaders/DeferredLighting.cxx", "compute", "cs_main");
        Shader("Shaders/BufferVisualization.cxx", "compute", "cs_main");
        Shader("Shaders/PrecomputeIntegrateBRDF.cxx", "compute", "cs_main");
        Shader("Shaders/PrecomputeEnvironmentMapMips.cxx", "compute", "cs_main");
        Shader("Shaders/WireframeVert.cxx", "vertex", "vs_main");
        Shader("Shaders/WireframePixel.cxx", "pixel", "ps_main");
        Shader("Shaders/BloomSetupCS.cxx", "compute", "cs_main");
        Shader("Shaders/BloomDownSampleCS.cxx", "compute", "cs_main");
        Shader("Shaders/BloomUpSampleCS.cxx", "compute", "cs_main");
        Shader("Shaders/GridVS.cxx", "vertex", "vs_main");
        Shader("Shaders/GridPS.cxx", "pixel", "ps_main");
        DependsOn(
            "Runtime",
            "VariantUtils",
            "HID",
            "Window",
            "RHI",
            "Image",
            "Font",
            "ImGui",
            "Asset",
            "ObjLoader",
            "RG",
            "JobSystem",
            "ECS");
    }
}
