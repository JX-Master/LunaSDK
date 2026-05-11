namespace LunaBuild.Core.Targets;

public sealed class LunaDocTargetRules : TargetRules
{
    public LunaDocTargetRules()
        : base(
            name: "LunaDoc",
            targetDirectory: "Programs/LunaDoc",
            rulesPath: "Programs/LunaDoc/LunaDoc.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Kind = BuildTargetKind.Executable;
        Sources("**.cpp");
        DependsOn("Runtime", "VariantUtils");
    }
}
