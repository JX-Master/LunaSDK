namespace LunaBuild.Core.Targets;

public sealed class VGTestTargetRules : TargetRules
{
    public VGTestTargetRules()
        : base(
            name: "VGTest",
            targetDirectory: "Tests/VGTest",
            rulesPath: "Tests/VGTest/VGTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Window", "RHI", "Font", "VG", "HID");
    }
}
