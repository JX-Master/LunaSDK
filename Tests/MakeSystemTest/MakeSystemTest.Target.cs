namespace LunaBuild.Core.Targets;

public sealed class MakeSystemTestTargetRules : TargetRules
{
    public MakeSystemTestTargetRules()
        : base(
            name: "MakeSystemTest",
            targetDirectory: "Tests/MakeSystemTest",
            rulesPath: "Tests/MakeSystemTest/MakeSystemTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("Main.cpp");
        DependsOn("Runtime", "MakeSystem");
    }
}
