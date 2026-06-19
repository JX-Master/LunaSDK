namespace LunaBuild.Core.Targets;

public sealed class GUICoreTargetRules : TargetRules
{
    public GUICoreTargetRules()
        : base(
            name: "GUICore",
            targetDirectory: "Modules/Luna/GUICore",
            rulesPath: "Modules/Luna/GUICore/GUICore.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Context.hpp", "Source/GUICore.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "RHI", "VG", "Font");
    }
}
