namespace LunaBuild.Core.Targets;

public sealed class AssetDatabaseTestTargetRules : TargetRules
{
    public AssetDatabaseTestTargetRules()
        : base(name: "AssetDatabaseTest", targetDirectory: "Tests/AssetDatabaseTest",
            rulesPath: "Tests/AssetDatabaseTest/AssetDatabaseTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/TestTypes.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "VariantUtils", "VFS", "Asset", "Pak");
    }
}
