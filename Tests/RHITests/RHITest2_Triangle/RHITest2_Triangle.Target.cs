namespace LunaBuild.Core.Targets;

public sealed class RHITest2TriangleTargetRules : TargetRules
{
    public RHITest2TriangleTargetRules()
        : base(
            name: "RHITest2_Triangle",
            targetDirectory: "Tests/RHITests/RHITest2_Triangle",
            rulesPath: "Tests/RHITests/RHITest2_Triangle/RHITest2_Triangle.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        Shader("TestTriangleVS.cxx", "vertex", "vs_main");
        Shader("TestTrianglePS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "RHITestBed");
    }
}
