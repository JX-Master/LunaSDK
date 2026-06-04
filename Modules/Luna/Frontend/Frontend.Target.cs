namespace LunaBuild.Core.Targets;

public sealed class FrontendTargetRules : TargetRules
{
    public FrontendTargetRules()
        : base(
            name: "Frontend",
            targetDirectory: "Modules/Luna/Frontend",
            rulesPath: "Modules/Luna/Frontend/Frontend.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Frontend.hpp", "Source/FrontendImpl.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
