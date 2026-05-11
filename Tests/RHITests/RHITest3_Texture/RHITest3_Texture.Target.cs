namespace LunaBuild.Core.Targets;

public sealed class RHITest3TextureTargetRules : TargetRules
{
    public RHITest3TextureTargetRules()
        : base(
            name: "RHITest3_Texture",
            targetDirectory: "Tests/RHITests/RHITest3_Texture",
            rulesPath: "Tests/RHITests/RHITest3_Texture/RHITest3_Texture.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        Shader("TestTextureVS.cxx", "vertex", "vs_main");
        Shader("TestTexturePS.cxx", "pixel", "ps_main");
        RuntimeFiles("uv_checker.png");
        DependsOn("Runtime", "RHI", "RHIUtility", "RHITestBed", "Image");
    }
}
