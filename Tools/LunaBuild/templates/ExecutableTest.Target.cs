namespace LunaBuild.Core.Targets;

public sealed class ExampleTestTargetRules : TargetRules
{
    public ExampleTestTargetRules()
        : base(
            name: "ExampleTest",
            targetDirectory: "Tests/ExampleTest",
            rulesPath: "Tests/ExampleTest/ExampleTest.Target.cs")
    {
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        Sources("Source/*.cpp");
        // Windows resource files can be added with:
        // Sources("Source/Windows/*.rc");
        DependsOn("Runtime");
    }
}
