namespace LunaBuild.Core.Targets;

public abstract class ManagedModuleTargetRules : TargetRules
{
    protected ManagedModuleTargetRules(string name, string directory, params string[] dependencies)
        : base(name, directory, "Managed/Managed.Target.cs")
    {
        Kind = BuildTargetKind.ManagedLibrary;
        DotNetSettings("net10.0");
        Sources("*.cs", "Internal/*.cs", "Internal/Generated/*.cs");
        ExcludeSources("bin/**.cs", "obj/**.cs");
        DependsOn(dependencies);
    }
}

public sealed class LunaRuntimeTargetRules : ManagedModuleTargetRules
{
    public LunaRuntimeTargetRules() : base("Luna.Runtime", "Managed/Runtime", "RuntimeC") { }
}

public sealed class LunaWindowTargetRules : ManagedModuleTargetRules
{
    public LunaWindowTargetRules() : base("Luna.Window", "Managed/Window", "Luna.Runtime", "WindowC") { }
}

public sealed class LunaRhiTargetRules : ManagedModuleTargetRules
{
    public LunaRhiTargetRules() : base("Luna.RHI", "Managed/RHI", "Luna.Runtime", "Luna.Window", "RHIC") { }
}

public sealed class LunaRhiUtilityTargetRules : ManagedModuleTargetRules
{
    public LunaRhiUtilityTargetRules() : base("Luna.RHIUtility", "Managed/RHIUtility", "Luna.Runtime", "Luna.RHI", "RHIUtilityC") { }
}

public sealed class LunaImageTargetRules : ManagedModuleTargetRules
{
    public LunaImageTargetRules() : base("Luna.Image", "Managed/Image", "Luna.Runtime", "ImageC") { }
}

public sealed class LunaVfsTargetRules : ManagedModuleTargetRules
{
    public LunaVfsTargetRules() : base("Luna.VFS", "Managed/VFS", "Luna.Runtime", "VFSC") { }
}

public sealed class LunaFontTargetRules : ManagedModuleTargetRules
{
    public LunaFontTargetRules() : base("Luna.Font", "Managed/Font", "Luna.Runtime", "FontC") { }
}

public sealed class LunaAssetTargetRules : ManagedModuleTargetRules
{
    public LunaAssetTargetRules() : base("Luna.Asset", "Managed/Asset", "Luna.Runtime", "AssetC") { }
}

public sealed class LunaVgTargetRules : ManagedModuleTargetRules
{
    public LunaVgTargetRules() : base("Luna.VG", "Managed/VG", "Luna.Runtime", "Luna.Font", "Luna.RHI", "VGC") { }
}

public sealed class LunaImGuiTargetRules : ManagedModuleTargetRules
{
    public LunaImGuiTargetRules() : base("Luna.ImGui", "Managed/ImGui", "Luna.Runtime", "Luna.Window", "Luna.RHI", "Luna.RHIUtility", "ImGuiC") { }
}

public sealed class LunaHidTargetRules : ManagedModuleTargetRules
{
    public LunaHidTargetRules() : base("Luna.HID", "Managed/HID", "Luna.Runtime", "HIDC") { }
}

public sealed class LunaAhiTargetRules : ManagedModuleTargetRules
{
    public LunaAhiTargetRules() : base("Luna.AHI", "Managed/AHI", "Luna.Runtime", "AHIC") { }
}

public sealed class LunaVariantUtilsTargetRules : ManagedModuleTargetRules
{
    public LunaVariantUtilsTargetRules() : base("Luna.VariantUtils", "Managed/VariantUtils", "Luna.Runtime", "VariantUtilsC") { }
}
