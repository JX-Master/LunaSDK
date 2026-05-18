namespace LunaBuild.Core.Targets;

public sealed class ImGuiTargetRules : TargetRules
{
    public ImGuiTargetRules()
        : base(
            name: "ImGui",
            targetDirectory: "Modules/Luna/ImGui",
            rulesPath: "Modules/Luna/ImGui/ImGui.Target.cs")
    {
        Headers("*.hpp", "*.h", "Source/**.h");
        Sources("Source/**.cpp");
        IncludeDirectories(".");
        Defines("LUNA_IMGUI_IMPL");
        Shader("Source/ImGuiVS.cxx", "vertex", "vs_main");
        Shader("Source/ImGuiPS.cxx", "pixel", "ps_main");
        DependsOn("Window", "Runtime", "RHI", "RHIUtility", "HID", "Font");
    }
}
