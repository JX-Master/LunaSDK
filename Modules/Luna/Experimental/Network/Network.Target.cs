namespace LunaBuild.Core.Targets;

public sealed class NetworkTargetRules : TargetRules
{
    public NetworkTargetRules()
        : base(
            name: "Network",
            targetDirectory: "Modules/Luna/Experimental/Network",
            rulesPath: "Modules/Luna/Experimental/Network/Network.Target.cs")
    {
        Headers("*.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Sources("Source/Platform/Windows/**.cpp");
        }
        else if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.IOS)
        {
            Sources("Source/Platform/POSIX/**.cpp");
        }
    }
}
