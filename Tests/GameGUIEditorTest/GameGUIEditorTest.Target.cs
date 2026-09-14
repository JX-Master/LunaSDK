namespace LunaBuild.Core.Targets;

public sealed class GameGUIEditorTestTargetRules : TargetRules
{
    public GameGUIEditorTestTargetRules()
        : base(
            name: "GameGUIEditorTest",
            targetDirectory: "Tests/GameGUIEditorTest",
            rulesPath: "Tests/GameGUIEditorTest/GameGUIEditorTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp", "../../Programs/GameGUIEditor/Source/**.cpp");
        ExcludeSources("../../Programs/GameGUIEditor/Source/main.cpp");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Font", "VG", "GUI",
            "EditorGUI", "GUIWindow", "GameGUI", "GameGUIEditorService", "Frontend",
            "Asset", "VariantUtils", "VFS");
    }
}
