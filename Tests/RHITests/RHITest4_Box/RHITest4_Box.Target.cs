namespace LunaBuild.Core.Targets;

public sealed class RHITest4BoxTargetRules : TargetRules
{
    public RHITest4BoxTargetRules()
        : base(
            name: "RHITest4_Box",
            targetDirectory: "Tests/RHITests/RHITest4_Box",
            rulesPath: "Tests/RHITests/RHITest4_Box/RHITest4_Box.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        Shader("TestBoxVS.cxx", "vertex", "vs_main");
        Shader("TestBoxPS.cxx", "pixel", "ps_main");
        RuntimeFiles("luna.png");
        DependsOn("Runtime", "RHI", "RHIUtility", "RHITestBed", "Image");
    }
}
