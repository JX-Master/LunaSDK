namespace LunaBuild.Core.Targets;

public sealed class VariantUtilsTargetRules : TargetRules
{
    public VariantUtilsTargetRules()
        : base(
            name: "VariantUtils",
            targetDirectory: "Modules/Luna/VariantUtils",
            rulesPath: "Modules/Luna/VariantUtils/VariantUtils.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
