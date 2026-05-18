namespace LunaBuild.Core.Targets;

public sealed class ExampleTargetRules : TargetRules
{
    public ExampleTargetRules()
        : base(
            name: "Example",
            targetDirectory: "Modules/Luna/Example",
            rulesPath: "Modules/Luna/Example/Example.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
