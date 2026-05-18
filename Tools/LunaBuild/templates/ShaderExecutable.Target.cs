namespace LunaBuild.Core.Targets;

public sealed class ExampleShaderTestTargetRules : TargetRules
{
    public ExampleShaderTestTargetRules()
        : base(
            name: "ExampleShaderTest",
            targetDirectory: "Tests/ExampleShaderTest",
            rulesPath: "Tests/ExampleShaderTest/ExampleShaderTest.Target.cs")
    {
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        Shader("ExampleVS.cxx", "vertex", "vs_main");
        Shader("ExamplePS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "RHITestBed");
    }
}
