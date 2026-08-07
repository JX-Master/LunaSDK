namespace LunaBuild.Core.Targets;

public sealed class GUIWindowTargetRules : TargetRules
{
    public GUIWindowTargetRules()
        : base(
            name: "GUIWindow",
            targetDirectory: "Modules/Luna/GUIWindow",
            rulesPath: "Modules/Luna/GUIWindow/GUIWindow.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GUI", "Window", "HID");
    }
}
