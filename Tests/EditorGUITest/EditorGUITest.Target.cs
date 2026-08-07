namespace LunaBuild.Core.Targets;

public sealed class EditorGUITestTargetRules : TargetRules
{
    public EditorGUITestTargetRules()
        : base(
            name: "EditorGUITest",
            targetDirectory: "Tests/EditorGUITest",
            rulesPath: "Tests/EditorGUITest/EditorGUITest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/**.cpp");
        RuntimeFiles("Assets/*.png");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Font", "Image", "VG", "GUI", "EditorGUI", "GUIWindow");
    }
}
