namespace LunaBuild.Core.Targets;

public sealed class GameGUIEditorServiceTestTargetRules : TargetRules
{
    public GameGUIEditorServiceTestTargetRules()
        : base(
            name: "GameGUIEditorServiceTest",
            targetDirectory: "Tests/GameGUIEditorServiceTest",
            rulesPath: "Tests/GameGUIEditorServiceTest/GameGUIEditorServiceTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GameGUI", "GameGUIEditorService", "Frontend", "VariantUtils");
    }
}
