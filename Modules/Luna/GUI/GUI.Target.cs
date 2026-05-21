namespace LunaBuild.Core.Targets;

public sealed class GUITargetRules : TargetRules
{
    public GUITargetRules()
        : base(
            name: "GUI",
            targetDirectory: "Modules/Luna/GUI",
            rulesPath: "Modules/Luna/GUI/GUI.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "RHI", "VG", "Font");
    }
}
