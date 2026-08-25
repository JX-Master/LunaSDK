namespace LunaBuild.Core.Targets;

public sealed class GameGUITestTargetRules : TargetRules
{
    public GameGUITestTargetRules()
        : base(
            name: "GameGUITest",
            targetDirectory: "Tests/GameGUITest",
            rulesPath: "Tests/GameGUITest/GameGUITest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GameGUI", "VariantUtils");
    }
}
