namespace LunaBuild.Core.Targets;

public sealed class GUILegacyTargetRules : TargetRules
{
    public GUILegacyTargetRules()
        : base(
            name: "GUILegacy",
            targetDirectory: "Modules/Luna/GUI/Legacy",
            rulesPath: "Modules/Luna/GUI/Legacy/GUILegacy.Target.cs")
    {
        Headers(
            "Base.hpp",
            "GUI.hpp",
            "Editor.hpp",
            "EditorState.hpp",
            "EditorWidgets.hpp",
            "EditorViews.hpp");
        MetaHeaders("EditorState.hpp");
        Sources(
            "Source/GUI.cpp",
            "Source/Editor*.cpp");
        DependsOn("Runtime", "RHI", "VG", "Font", "GUICore");
    }
}
