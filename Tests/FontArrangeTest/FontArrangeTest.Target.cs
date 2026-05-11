namespace LunaBuild.Core.Targets;

public sealed class FontArrangeTestTargetRules : TargetRules
{
    public FontArrangeTestTargetRules()
        : base(
            name: "FontArrangeTest",
            targetDirectory: "Tests/FontArrangeTest",
            rulesPath: "Tests/FontArrangeTest/FontArrangeTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Window", "RHI", "Font", "VG");
    }
}
