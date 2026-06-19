namespace LunaBuild.Core.Targets;

public sealed class GUICoreTestTargetRules : TargetRules
{
    public GUICoreTestTargetRules()
        : base(
            name: "GUICoreTest",
            targetDirectory: "Tests/GUICoreTest",
            rulesPath: "Tests/GUICoreTest/GUICoreTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GUICore");
    }
}
