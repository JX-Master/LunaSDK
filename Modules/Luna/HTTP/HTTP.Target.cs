namespace LunaBuild.Core.Targets;

public sealed class HTTPTargetRules : TargetRules
{
    public HTTPTargetRules()
        : base(
            name: "HTTP",
            targetDirectory: "Modules/Luna/HTTP",
            rulesPath: "Modules/Luna/HTTP/HTTP.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("HTTP.hpp", "Source/HTTPImpl.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Network");
    }
}
