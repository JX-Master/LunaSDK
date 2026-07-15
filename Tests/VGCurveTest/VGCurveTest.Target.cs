namespace LunaBuild.Core.Targets;

public sealed class VGCurveTestTargetRules : TargetRules
{
    public VGCurveTestTargetRules()
        : base(
            name: "VGCurveTest",
            targetDirectory: "Tests/VGCurveTest",
            rulesPath: "Tests/VGCurveTest/VGCurveTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Window", "RHI", "Font", "VG", "GUILegacy", "GUIWindow");
    }
}
