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
        Sources("Source/**.cpp");
        DependsOn("Runtime", "JobSystem");
    }
}
