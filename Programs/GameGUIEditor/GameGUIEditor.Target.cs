namespace LunaBuild.Core.Targets;

public sealed class GameGUIEditorTargetRules : TargetRules
{
    public GameGUIEditorTargetRules()
        : base(
            name: "GameGUIEditor",
            targetDirectory: "Programs/GameGUIEditor",
            rulesPath: "Programs/GameGUIEditor/GameGUIEditor.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Kind = BuildTargetKind.Application;
        Sources("Source/**.cpp");
        DependsOn(
            "Runtime",
            "Window",
            "RHI",
            "RHIUtility",
            "Font",
            "VG",
            "GUI",
            "EditorGUI",
            "GUIWindow",
            "GameGUI",
            "GameGUIEditorService",
            "Frontend",
            "Asset",
            "VariantUtils",
            "VFS");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.MacOS)
        {
            AppleBundle("com.lunasdk.GameGUIEditor", "GameGUIEditor");
            AppleInfoPlist("MacOSInfo.plist");
        }
    }
}
