namespace LunaBuild.Core.Targets;

public sealed class ZipTargetRules : TargetRules
{
    public ZipTargetRules() : base("Zip", "Modules/Luna/Zip", "Modules/Luna/Zip/Zip.Target.cs")
    {
        Headers("*.hpp", "Source/*.hpp");
        MetaHeaders("Zip.hpp", "Source/Archive.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "libzip");
    }
}
