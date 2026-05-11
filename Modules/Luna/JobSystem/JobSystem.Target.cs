namespace LunaBuild.Core.Targets;

public sealed class JobSystemTargetRules : TargetRules
{
    public JobSystemTargetRules()
        : base(
            name: "JobSystem",
            targetDirectory: "Modules/Luna/JobSystem",
            rulesPath: "Modules/Luna/JobSystem/JobSystem.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
