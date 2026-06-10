namespace LunaBuild.Core.Targets;

public sealed class NetworkTestTargetRules : TargetRules
{
    public NetworkTestTargetRules()
        : base(
            name: "NetworkTest",
            targetDirectory: "Tests/NetworkTest",
            rulesPath: "Tests/NetworkTest/NetworkTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Network");
    }
}
