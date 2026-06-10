namespace LunaBuild.Core.Targets;

public sealed class ECSTargetRules : TargetRules
{
    public ECSTargetRules()
        : base(
            name: "ECS",
            targetDirectory: "Modules/Luna/ECS",
            rulesPath: "Modules/Luna/ECS/ECS.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("World.hpp", "Source/WorldImpl.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "JobSystem");
    }
}
