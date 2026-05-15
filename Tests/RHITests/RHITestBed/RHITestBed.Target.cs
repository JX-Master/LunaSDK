namespace LunaBuild.Core.Targets;

public sealed class RHITestBedTargetRules : TargetRules
{
    public RHITestBedTargetRules()
        : base(
            name: "RHITestBed",
            targetDirectory: "Tests/RHITests/RHITestBed",
            rulesPath: "Tests/RHITests/RHITestBed/RHITestBed.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Headers("*.hpp");
        Sources("*.cpp");
        DependsOn("Runtime", "RHI", "RHIUtility", "Window");
    }
}
