namespace LunaBuild.Core.Targets;

public sealed class ZipTestTargetRules : TargetRules
{
    public ZipTestTargetRules() : base("ZipTest", "Tests/ZipTest", "Tests/ZipTest/ZipTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/TestStream.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Zip");
    }
}
