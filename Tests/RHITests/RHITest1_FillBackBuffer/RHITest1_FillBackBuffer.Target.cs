namespace LunaBuild.Core.Targets;

public sealed class RHITest1FillBackBufferTargetRules : TargetRules
{
    public RHITest1FillBackBufferTargetRules()
        : base(
            name: "RHITest1_FillBackBuffer",
            targetDirectory: "Tests/RHITests/RHITest1_FillBackBuffer",
            rulesPath: "Tests/RHITests/RHITest1_FillBackBuffer/RHITest1_FillBackBuffer.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        DependsOn("Runtime", "RHI", "RHITestBed");
    }
}
