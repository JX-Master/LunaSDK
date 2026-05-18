namespace LunaBuild.Core.Targets;

public sealed class ExampleRuntimeFilesTargetRules : TargetRules
{
    public ExampleRuntimeFilesTargetRules()
        : base(
            name: "ExampleRuntimeFiles",
            targetDirectory: "Tests/ExampleRuntimeFiles",
            rulesPath: "Tests/ExampleRuntimeFiles/ExampleRuntimeFiles.Target.cs")
    {
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        RuntimeFiles("asset.png", "config.json");
        DependsOn("Runtime");
    }
}
