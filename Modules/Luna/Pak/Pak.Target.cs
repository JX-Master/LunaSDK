namespace LunaBuild.Core.Targets;

public sealed class PakTargetRules : TargetRules
{
    public PakTargetRules() : base("Pak", "Modules/Luna/Pak", "Modules/Luna/Pak/Pak.Target.cs")
    {
        Headers("*.hpp", "Source/*.hpp");
        MetaHeaders("Pak.hpp", "Source/Package.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime", "Zip");
    }
}
