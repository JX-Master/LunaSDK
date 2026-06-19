namespace LunaBuild.Core.Targets;

public sealed class GUIDirectCoreTestTargetRules : TargetRules
{
    public GUIDirectCoreTestTargetRules()
        : base(
            name: "GUIDirectCoreTest",
            targetDirectory: "Tests/GUIDirectCoreTest",
            rulesPath: "Tests/GUIDirectCoreTest/GUIDirectCoreTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GUICore", "GUI", "GUIWindow", "VG", "Font");
    }
}
