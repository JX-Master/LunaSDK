namespace LunaBuild.Core.Targets;

public sealed class RHIUtilityTargetRules : TargetRules
{
    public RHIUtilityTargetRules()
        : base(
            name: "RHIUtility",
            targetDirectory: "Modules/Luna/RHIUtility",
            rulesPath: "Modules/Luna/RHIUtility/RHIUtility.Target.cs")
    {
        Headers("**.hpp");
        Sources("Source/**.cpp");
        Shader("Source/Shaders/MipmapGeneration1DCS.cxx", "compute", "cs_main");
        Shader("Source/Shaders/MipmapGeneration2DCS.cxx", "compute", "cs_main");
        Shader("Source/Shaders/MipmapGeneration3DCS.cxx", "compute", "cs_main");
        Shader("Source/Shaders/BlitVS.cxx", "vertex", "vs_main");
        Shader("Source/Shaders/BlitPS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI");
    }
}
