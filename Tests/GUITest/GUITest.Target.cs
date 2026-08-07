namespace LunaBuild.Core.Targets;

public sealed class GUITestTargetRules : TargetRules
{
    public GUITestTargetRules()
        : base(
            name: "GUITest",
            targetDirectory: "Tests/GUITest",
            rulesPath: "Tests/GUITest/GUITest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/**.cpp");
        RuntimeFiles("Assets/*.png");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Font", "Image", "VG", "GUICore", "GUI", "GUIWindow");
    }
}
