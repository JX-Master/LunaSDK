namespace LunaBuild.Core.Targets;

public sealed class ExampleToolTargetRules : TargetRules
{
    public ExampleToolTargetRules()
        : base(
            name: "ExampleTool",
            targetDirectory: "Tools/ExampleTool",
            rulesPath: "Tools/ExampleTool/ExampleTool.Target.cs")
    {
        Kind = BuildTargetKind.DotNetProject;
        Sources("src/ExampleTool.Cli/*.cs", "src/ExampleTool.Core/**.cs");
        ExcludeSources("src/ExampleTool.Core/bin/**.cs", "src/ExampleTool.Core/obj/**.cs");
        DotNetProject("src/ExampleTool.Cli/ExampleTool.Cli.csproj", "src/ExampleTool.Cli/bin/Debug/net9.0/exampletool.exe");
    }
}
