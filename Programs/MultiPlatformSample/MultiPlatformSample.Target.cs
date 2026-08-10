namespace LunaBuild.Core.Targets;

public sealed class MultiPlatformSampleTargetRules : TargetRules
{
    public MultiPlatformSampleTargetRules()
        : base(
            name: "MultiPlatformSample",
            targetDirectory: "Programs/MultiPlatformSample",
            rulesPath: "Programs/MultiPlatformSample/MultiPlatformSample.Target.cs")
    {
        Kind = BuildTargetKind.Application;
        Sources("Source/**.cpp");
        Shader("Source/BoxVert.cxx", "vertex", "vs_main");
        Shader("Source/BoxPixel.cxx", "pixel", "ps_main");
        EmbeddedHeader("Res/luna.png", "LunaTex.hpp", "LUNA_PNG_DATA", "LUNA_PNG_SIZE");
        AppleBundle("com.lunasdk.MultiPlatformSample", "MultiPlatformSample");
        AppleInfoPlist("Source/Info.plist");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Image");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Android)
        {
            SystemLibraries("android", "log");
        }
        if(Platform == BuildPlatform.MacOS)
        {
            AppleInfoPlist("Source/MacOSInfo.plist");
        }
    }
}
