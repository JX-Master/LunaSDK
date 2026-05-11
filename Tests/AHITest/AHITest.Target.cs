namespace LunaBuild.Core.Targets;

public sealed class AHITestTargetRules : TargetRules
{
    public AHITestTargetRules()
        : base(
            name: "AHITest",
            targetDirectory: "Tests/AHITest",
            rulesPath: "Tests/AHITest/AHITest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        DependsOn("Runtime", "AHI", "RHI", "Window", "ImGui");
    }
}
