namespace LunaBuild.Core.Targets;

public sealed class GameGUITargetRules : TargetRules
{
    public GameGUITargetRules()
        : base(
            name: "GameGUI",
            targetDirectory: "Modules/Luna/GameGUI",
            rulesPath: "Modules/Luna/GameGUI/GameGUI.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Instance.hpp", "Source/InstanceInternal.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "GUI", "Asset", "VariantUtils", "VFS");
    }
}
