namespace LunaBuild.Core.Targets;

public sealed class VFSPakTestTargetRules : TargetRules
{
    public VFSPakTestTargetRules() : base("VFSPakTest", "Tests/VFSPakTest", "Tests/VFSPakTest/VFSPakTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/TestStorage.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Pak", "VFS");
    }
}
