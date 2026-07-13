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
