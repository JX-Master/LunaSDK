namespace LunaBuild.Core.Targets;

public sealed class AHITargetRules : TargetRules
{
    public AHITargetRules()
        : base(
            name: "AHI",
            targetDirectory: "Modules/Luna/AHI",
            rulesPath: "Modules/Luna/AHI/AHI.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("**.cpp");
        Packages("miniaudio");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform is BuildPlatform.MacOS or BuildPlatform.IOS)
        {
            ExcludeSources("Source/MiniAudio/miniaudio.cpp");
            Sources("Source/MiniAudio/miniaudio.mm");
        }
    }
}
