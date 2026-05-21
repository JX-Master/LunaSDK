namespace LunaBuild.Core.Targets;

public sealed class FontTargetRules : TargetRules
{
    public FontTargetRules()
        : base(
            name: "Font",
            targetDirectory: "Modules/Luna/Font",
            rulesPath: "Modules/Luna/Font/Font.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Source/FontFileTTF.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "stb");
    }
}
