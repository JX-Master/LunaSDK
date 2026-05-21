namespace LunaBuild.Core.Targets;

public sealed class ECSTestTargetRules : TargetRules
{
    public ECSTestTargetRules()
        : base(
            name: "ECSTest",
            targetDirectory: "Tests/ECSTest",
            rulesPath: "Tests/ECSTest/ECSTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("*.hpp");
        MetaHeaders("Position.hpp");
        Sources("*.cpp");
        DependsOn("Runtime", "JobSystem", "ECS");
    }
}
