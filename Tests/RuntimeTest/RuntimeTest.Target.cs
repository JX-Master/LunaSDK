namespace LunaBuild.Core.Targets;

public sealed class RuntimeTestTargetRules : TargetRules
{
    public RuntimeTestTargetRules()
        : base(
            name: "RuntimeTest",
            targetDirectory: "Tests/RuntimeTest",
            rulesPath: "Tests/RuntimeTest/RuntimeTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }
}
