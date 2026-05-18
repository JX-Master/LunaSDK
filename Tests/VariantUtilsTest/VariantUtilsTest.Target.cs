namespace LunaBuild.Core.Targets;

public sealed class VariantUtilsTestTargetRules : TargetRules
{
    public VariantUtilsTestTargetRules()
        : base(
            name: "VariantUtilsTest",
            targetDirectory: "Tests/VariantUtilsTest",
            rulesPath: "Tests/VariantUtilsTest/VariantUtilsTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "VariantUtils");
    }
}
