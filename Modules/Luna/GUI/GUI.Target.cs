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
            "Icons.hpp",
            "IconNames.inl",
            "Style.hpp",
            "Widgets.hpp",
            "Layouts.hpp",
            "Overlay.hpp",
            "Workspace.hpp",
            "GUI.hpp",
            "Source/**.hpp");
        MetaHeaders("Source/State.hpp");
        Sources("Source/**.cpp");
        EmbeddedHeader("Res/PhosphorCore.bin", "PhosphorCoreData.hpp", "PHOSPHOR_CORE_DATA", "PHOSPHOR_CORE_SIZE");
        DependsOn("Runtime", "RHI", "VG", "Font", "GUICore");
    }
}
