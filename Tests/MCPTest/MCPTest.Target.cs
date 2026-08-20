namespace LunaBuild.Core.Targets;

public sealed class MCPTestTargetRules : TargetRules
{
    public MCPTestTargetRules()
        : base(
            name: "MCPTest",
            targetDirectory: "Tests/MCPTest",
            rulesPath: "Tests/MCPTest/MCPTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "VariantUtils", "Frontend", "Network", "HTTP", "MCP");
    }
}
