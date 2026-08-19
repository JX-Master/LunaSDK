namespace LunaBuild.Core.Targets;

public sealed class HTTPTestTargetRules : TargetRules
{
    public HTTPTestTargetRules()
        : base(
            name: "HTTPTest",
            targetDirectory: "Tests/HTTPTest",
            rulesPath: "Tests/HTTPTest/HTTPTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Network", "HTTP");
    }
}
