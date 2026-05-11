namespace LunaBuild.Core.Targets;

public sealed class WindowTestTargetRules : TargetRules
{
    public WindowTestTargetRules()
        : base(
            name: "WindowTest",
            targetDirectory: "Tests/WindowTest",
            rulesPath: "Tests/WindowTest/WindowTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        IsTest = true;
        Kind = BuildTargetKind.Executable;
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Window");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Sources("Source/Windows/*.rc");
        }
    }
}
