namespace LunaBuild.Core.Targets;

public sealed class ImageTargetRules : TargetRules
{
    public ImageTargetRules()
        : base(
            name: "Image",
            targetDirectory: "Modules/Luna/Image",
            rulesPath: "Modules/Luna/Image/Image.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "stb");
    }
}
