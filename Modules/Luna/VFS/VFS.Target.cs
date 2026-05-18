namespace LunaBuild.Core.Targets;

public sealed class VFSTargetRules : TargetRules
{
    public VFSTargetRules()
        : base(
            name: "VFS",
            targetDirectory: "Modules/Luna/VFS",
            rulesPath: "Modules/Luna/VFS/VFS.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
