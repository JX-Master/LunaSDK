namespace LunaBuild.Core.Targets;

public sealed class NetworkTargetRules : TargetRules
{
    public NetworkTargetRules()
        : base(
            name: "Network",
            targetDirectory: "Modules/Luna/Network",
            rulesPath: "Modules/Luna/Network/Network.Target.cs")
    {
        Headers("*.hpp");
        MetaHeaders("Network.hpp", "SocketPoller.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Headers("Source/Platform/Windows/**.hpp");
            MetaHeaders("Source/Platform/Windows/Socket.hpp");
            Sources("Source/Platform/Windows/**.cpp");
        }
        else if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.IOS)
        {
            Headers("Source/Platform/POSIX/**.hpp");
            MetaHeaders("Source/Platform/POSIX/Socket.hpp");
            Sources("Source/Platform/POSIX/**.cpp");
        }
    }
}
