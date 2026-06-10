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
        MetaHeaders(
            "Context.hpp",
            "Debug.hpp",
            "Description.hpp",
            "DrawList.hpp",
            "State.hpp",
            "Source/GUI.hpp",
            "Source/GUIDrawList.hpp",
            "Source/Nodes/BasicNodes.hpp",
            "Source/Nodes/ButtonGroupNodes.hpp",
            "Source/Nodes/ColorNodes.hpp",
            "Source/Nodes/DrawingNodes.hpp",
            "Source/Nodes/InputNodes.hpp",
            "Source/Nodes/LayoutNodes.hpp",
            "Source/Nodes/MenuNodes.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "RHI", "VG", "Font");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(options.Mode == BuildMode.Debug || ProjectBoolOption("gui_debug"))
        {
            Defines("LUNA_GUI_ENABLE_DEBUG");
            PublicDefines("LUNA_GUI_ENABLE_DEBUG");
        }
    }
}
