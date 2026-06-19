namespace LunaBuild.Core.Targets;

public sealed class GUIAssetTargetRules : TargetRules
{
    public GUIAssetTargetRules()
        : base(
            name: "GUIAsset",
            targetDirectory: "Modules/Luna/GUIAsset",
            rulesPath: "Modules/Luna/GUIAsset/GUIAsset.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("GUIAsset.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Asset", "GUI", "GUICore", "VariantUtils", "VFS");
    }
}
