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
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        DependsOn("Runtime", "JobSystem", "ECS");
    }
}
