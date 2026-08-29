namespace LunaBuild.Core.Targets;

public sealed class AssetTestTargetRules : TargetRules
{
    public AssetTestTargetRules()
        : base(
            name: "AssetTest",
            targetDirectory: "Tests/AssetTest",
            rulesPath: "Tests/AssetTest/AssetTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/TestTypes.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "VariantUtils", "VFS", "Asset");
    }
}
