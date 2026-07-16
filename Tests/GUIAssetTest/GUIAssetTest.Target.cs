namespace LunaBuild.Core.Targets;

public sealed class GUIAssetTestTargetRules : TargetRules
{
    public GUIAssetTestTargetRules()
        : base(
            name: "GUIAssetTest",
            targetDirectory: "Tests/GUIAssetTest",
            rulesPath: "Tests/GUIAssetTest/GUIAssetTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Window", "RHI", "Font", "VG", "GUI", "GUICore", "GUIAsset", "Asset", "VariantUtils", "VFS");
    }
}
