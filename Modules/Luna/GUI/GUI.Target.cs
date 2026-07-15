namespace LunaBuild.Core.Targets;

public sealed class GUITargetRules : TargetRules
{
    public GUITargetRules()
        : base(
            name: "GUI",
            targetDirectory: "Modules/Luna/GUI",
            rulesPath: "Modules/Luna/GUI/GUI.Target.cs")
    {
        Headers(
            "Base.hpp",
            "Style.hpp",
            "Widgets.hpp",
            "Layouts.hpp",
            "Overlay.hpp",
            "GUI.hpp",
            "Source/**.hpp");
        MetaHeaders("Source/State.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "RHI", "VG", "Font", "GUICore");
    }
}
