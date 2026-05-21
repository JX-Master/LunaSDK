namespace LunaBuild.Core.Targets;

public sealed class RGTargetRules : TargetRules
{
    public RGTargetRules()
        : base(
            name: "RG",
            targetDirectory: "Modules/Luna/RG",
            rulesPath: "Modules/Luna/RG/RG.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Source/RenderGraph.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "RHI");
    }
}
