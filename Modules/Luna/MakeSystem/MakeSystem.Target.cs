namespace LunaBuild.Core.Targets;

public sealed class MakeSystemTargetRules : TargetRules
{
    public MakeSystemTargetRules()
        : base(
            name: "MakeSystem",
            targetDirectory: "Modules/Luna/MakeSystem",
            rulesPath: "Modules/Luna/MakeSystem/MakeSystem.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS);
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders(
            "Source/BuildCache.hpp",
            "Source/MakeSystem.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime", "VariantUtils", "JobSystem");
    }
}
