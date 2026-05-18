namespace LunaBuild.Core.Targets;

public sealed class MultiPlatformSampleTargetRules : TargetRules
{
    public MultiPlatformSampleTargetRules()
        : base(
            name: "MultiPlatformSample",
            targetDirectory: "Programs/MultiPlatformSample",
            rulesPath: "Programs/MultiPlatformSample/MultiPlatformSample.Target.cs")
    {
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        Shader("Source/BoxVert.cxx", "vertex", "vs_main");
        Shader("Source/BoxPixel.cxx", "pixel", "ps_main");
        EmbeddedHeader("Res/luna.png", "LunaTex.hpp", "LUNA_PNG_DATA", "LUNA_PNG_SIZE");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Image");
    }
}
