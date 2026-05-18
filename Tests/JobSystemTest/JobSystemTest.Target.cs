namespace LunaBuild.Core.Targets;

public sealed class JobSystemTestTargetRules : TargetRules
{
    public JobSystemTestTargetRules()
        : base(
            name: "JobSystemTest",
            targetDirectory: "Tests/JobSystemTest",
            rulesPath: "Tests/JobSystemTest/JobSystemTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("**.cpp");
        DependsOn("Runtime", "JobSystem");
    }
}
