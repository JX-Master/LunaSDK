namespace LunaBuild.Core.Targets;

public sealed class FrontendTestTargetRules : TargetRules
{
    public FrontendTestTargetRules()
        : base(
            name: "FrontendTest",
            targetDirectory: "Tests/FrontendTest",
            rulesPath: "Tests/FrontendTest/FrontendTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Frontend");
    }
}
