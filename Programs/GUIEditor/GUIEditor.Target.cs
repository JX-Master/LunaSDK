namespace LunaBuild.Core.Targets;

public sealed class GUIEditorTargetRules : TargetRules
{
    public GUIEditorTargetRules()
        : base(
            name: "GUIEditor",
            targetDirectory: "Programs/GUIEditor",
            rulesPath: "Programs/GUIEditor/GUIEditor.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Kind = BuildTargetKind.Executable;
        Headers("Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn(
            "Runtime",
            "VariantUtils",
            "HID",
            "Window",
            "RHI",
            "Font",
            "VG",
            "GUICore",
            "GUILegacy",
            "GUIWindow",
            "Asset",
            "GUIAsset",
            "Frontend",
            "VFS");
    }
}
