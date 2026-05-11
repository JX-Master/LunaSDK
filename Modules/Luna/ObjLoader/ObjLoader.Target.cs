namespace LunaBuild.Core.Targets;

public sealed class ObjLoaderTargetRules : TargetRules
{
    public ObjLoaderTargetRules()
        : base(
            name: "ObjLoader",
            targetDirectory: "Modules/Luna/ObjLoader",
            rulesPath: "Modules/Luna/ObjLoader/ObjLoader.Target.cs")
    {
        Headers("*.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
