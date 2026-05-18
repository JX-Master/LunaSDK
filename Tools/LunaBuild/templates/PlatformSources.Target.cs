namespace LunaBuild.Core.Targets;

public sealed class ExamplePlatformTargetRules : TargetRules
{
    public ExamplePlatformTargetRules()
        : base(
            name: "ExamplePlatform",
            targetDirectory: "Modules/Luna/ExamplePlatform",
            rulesPath: "Modules/Luna/ExamplePlatform/ExamplePlatform.Target.cs")
    {
        Headers("*.hpp", "Source/*.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Sources("Source/Platform/Windows/*.cpp");
        }
        else if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.Android or BuildPlatform.IOS)
        {
            Sources("Source/Platform/POSIX/*.cpp");
        }
    }
}
