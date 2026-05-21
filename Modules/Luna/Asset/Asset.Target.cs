namespace LunaBuild.Core.Targets;

public sealed class AssetTargetRules : TargetRules
{
    public AssetTargetRules()
        : base(
            name: "Asset",
            targetDirectory: "Modules/Luna/Asset",
            rulesPath: "Modules/Luna/Asset/Asset.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders(
            "Asset.hpp",
            "Source/AssetMetaFile.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "VariantUtils", "VFS");
    }
}
