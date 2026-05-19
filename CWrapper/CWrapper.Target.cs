namespace LunaBuild.Core.Targets;

public abstract class CWrapperTargetRules : TargetRules
{
    protected CWrapperTargetRules(string name, string directory, params string[] dependencies)
        : base(name, directory, "CWrapper/CWrapper.Target.cs")
    {
        Headers("*.h");
        Sources("*.cpp");
        DependsOn(dependencies);
    }
}

public sealed class RuntimeCTargetRules : CWrapperTargetRules
{
    public RuntimeCTargetRules() : base("RuntimeC", "CWrapper/Runtime", "Runtime") { }
}

public sealed class WindowCTargetRules : CWrapperTargetRules
{
    public WindowCTargetRules() : base("WindowC", "CWrapper/Window", "Window") { }
}

public sealed class RhiCTargetRules : CWrapperTargetRules
{
    public RhiCTargetRules() : base("RHIC", "CWrapper/RHI", "RHI") { }
}

public sealed class RhiUtilityCTargetRules : CWrapperTargetRules
{
    public RhiUtilityCTargetRules() : base("RHIUtilityC", "CWrapper/RHIUtility", "RHIUtility", "RHI") { }
}

public sealed class ImageCTargetRules : CWrapperTargetRules
{
    public ImageCTargetRules() : base("ImageC", "CWrapper/Image", "Image") { }
}

public sealed class VfsCTargetRules : CWrapperTargetRules
{
    public VfsCTargetRules() : base("VFSC", "CWrapper/VFS", "VFS") { }
}

public sealed class FontCTargetRules : CWrapperTargetRules
{
    public FontCTargetRules() : base("FontC", "CWrapper/Font", "Font") { }
}

public sealed class AssetCTargetRules : CWrapperTargetRules
{
    public AssetCTargetRules() : base("AssetC", "CWrapper/Asset", "Asset") { }
}

public sealed class VgCTargetRules : CWrapperTargetRules
{
    public VgCTargetRules() : base("VGC", "CWrapper/VG", "VG", "RHI", "Font") { }
}

public sealed class ImGuiCTargetRules : CWrapperTargetRules
{
    public ImGuiCTargetRules() : base("ImGuiC", "CWrapper/ImGui", "ImGui") { }
}

public sealed class HidCTargetRules : CWrapperTargetRules
{
    public HidCTargetRules() : base("HIDC", "CWrapper/HID", "HID") { }
}

public sealed class AhiCTargetRules : CWrapperTargetRules
{
    public AhiCTargetRules() : base("AHIC", "CWrapper/AHI", "AHI") { }
}

public sealed class VariantUtilsCTargetRules : CWrapperTargetRules
{
    public VariantUtilsCTargetRules() : base("VariantUtilsC", "CWrapper/VariantUtils", "VariantUtils") { }
}
