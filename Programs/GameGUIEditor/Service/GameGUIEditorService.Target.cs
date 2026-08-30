namespace LunaBuild.Core.Targets;

public sealed class GameGUIEditorServiceTargetRules : TargetRules
{
    public GameGUIEditorServiceTargetRules()
        : base(
            name: "GameGUIEditorService",
            targetDirectory: "Programs/GameGUIEditor/Service",
            rulesPath: "Programs/GameGUIEditor/Service/GameGUIEditorService.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Authoring.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GameGUI", "Asset", "Frontend", "VariantUtils", "VFS");
    }
}
