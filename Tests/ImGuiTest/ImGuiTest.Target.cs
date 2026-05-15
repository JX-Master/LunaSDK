namespace LunaBuild.Core.Targets;

public sealed class ImGuiTestTargetRules : TargetRules
{
    public ImGuiTestTargetRules()
        : base(
            name: "ImGuiTest",
            targetDirectory: "Tests/ImGuiTest",
            rulesPath: "Tests/ImGuiTest/ImGuiTest.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        DependsOn("Runtime", "Window", "RHI", "ImGui");
    }
}
