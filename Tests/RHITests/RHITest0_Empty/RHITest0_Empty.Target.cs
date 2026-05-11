namespace LunaBuild.Core.Targets;

public sealed class RHITest0EmptyTargetRules : TargetRules
{
    public RHITest0EmptyTargetRules()
        : base(
            name: "RHITest0_Empty",
            targetDirectory: "Tests/RHITests/RHITest0_Empty",
            rulesPath: "Tests/RHITests/RHITest0_Empty/RHITest0_Empty.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("*.cpp");
        DependsOn("Runtime", "RHI", "RHITestBed");
    }
}
