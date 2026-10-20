namespace LunaBuild.Core.Targets;

public sealed class TouchInputTestTargetRules : TargetRules
{
    public TouchInputTestTargetRules()
        : base("TouchInputTest", "Tests/TouchInputTest", "Tests/TouchInputTest/TouchInputTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Window", "GUI", "EditorGUI", "GUIWindow");
    }
}
