namespace LunaBuild.Core.Targets;

public sealed class DemoAppTargetRules : TargetRules
{
    public DemoAppTargetRules()
        : base(
            name: "DemoApp",
            targetDirectory: "LunaSDK-Docs/Res/DemoApp",
            rulesPath: "LunaSDK-Docs/Res/DemoApp/DemoApp.Target.cs")
    {
        Kind = BuildTargetKind.Application;
        Sources("**.cpp");
        Shader("DemoAppVS.cxx", "vertex", "vs_main");
        Shader("DemoAppPS.cxx", "pixel", "ps_main");
        RuntimeFiles("luna.png");
        DependsOn("Runtime", "Window", "RHI", "RHIUtility", "Image");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.MacOS)
        {
            AppleBundle("com.lunasdk.DemoApp", "DemoApp");
            AppleInfoPlist("MacOSInfo.plist");
        }
    }
}
