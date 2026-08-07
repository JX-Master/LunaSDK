namespace LunaBuild.Core.Targets;

public sealed class EditorGUITargetRules : TargetRules
{
    public EditorGUITargetRules()
        : base(
            name: "EditorGUI",
            targetDirectory: "Modules/Luna/EditorGUI",
            rulesPath: "Modules/Luna/EditorGUI/EditorGUI.Target.cs")
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
            "EditorGUI.hpp",
            "Source/**.hpp");
        MetaHeaders("Source/State.hpp");
        Sources("Source/**.cpp");
        EmbeddedHeader("Res/PhosphorCore.bin", "PhosphorCoreData.hpp", "PHOSPHOR_CORE_DATA", "PHOSPHOR_CORE_SIZE");
        DependsOn("Runtime", "RHI", "VG", "Font", "GUI");
    }
}
