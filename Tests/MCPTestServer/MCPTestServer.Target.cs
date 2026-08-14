namespace LunaBuild.Core.Targets;

public sealed class MCPTestServerTargetRules : TargetRules
{
    public MCPTestServerTargetRules()
        : base(
            name: "MCPTestServer",
            targetDirectory: "Tests/MCPTestServer",
            rulesPath: "Tests/MCPTestServer/MCPTestServer.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Frontend", "MCP");
    }
}
