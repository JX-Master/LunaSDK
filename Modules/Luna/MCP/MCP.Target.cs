namespace LunaBuild.Core.Targets;

public sealed class MCPTargetRules : TargetRules
{
    public MCPTargetRules()
        : base(
            name: "MCP",
            targetDirectory: "Modules/Luna/MCP",
            rulesPath: "Modules/Luna/MCP/MCP.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("MCP.hpp", "Source/MCPImpl.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "VariantUtils", "Frontend", "HTTP");
    }
}
