namespace LunaBuild.Core.Targets;

public sealed class PakTestTargetRules : TargetRules
{
    public PakTestTargetRules() : base("PakTest", "Tests/PakTest", "Tests/PakTest/PakTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/TestStream.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Zip", "Pak");
    }
}
