namespace LunaBuild.Core.Targets;

public sealed class HIDTargetRules : TargetRules
{
    public HIDTargetRules()
        : base(
            name: "HID",
            targetDirectory: "Modules/Luna/HID",
            rulesPath: "Modules/Luna/HID/HID.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Sources("Source/Platform/Windows/*.cpp");
        }
        else if(Platform == BuildPlatform.MacOS)
        {
            Sources("Source/Platform/MacOS/*.mm");
            Frameworks("ApplicationServices", "AppKit");
        }
        else if(Platform == BuildPlatform.IOS)
        {
            Sources("Source/Platform/iOS/*.mm");
            Frameworks("GameController");
        }
    }
}
