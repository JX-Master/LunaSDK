namespace LunaBuild.Core.Targets;

public sealed class LunaMetaToolSmokeTargetRules : TargetRules
{
    public LunaMetaToolSmokeTargetRules()
        : base(
            name: "LunaMetaToolSmoke",
            targetDirectory: "Tests/LunaMetaToolSmoke",
            rulesPath: "Tests/LunaMetaToolSmoke/LunaMetaToolSmoke.Target.cs")
    {
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Headers("Source/*.hpp");
        MetaHeaders("Source/MetaSmoke.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }
}
