namespace LunaBuild.Core.Targets;

public abstract class ManagedTestTargetRules : TargetRules
{
    protected ManagedTestTargetRules(string name, string directory, params string[] dependencies)
        : base(name, directory, "Tests/CSharpTests.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.ManagedExecutable;
        DotNetSettings("net10.0");
        DependsOn(dependencies);
    }
}

public sealed class RuntimeCSharpTestTargetRules : ManagedTestTargetRules
{
    public RuntimeCSharpTestTargetRules() : base("RuntimeCSharpTest", "Tests/RuntimeCSharpTest", "Luna.Runtime")
    {
        Sources("Main.cs");
    }
}

public sealed class WindowCSharpTestTargetRules : ManagedTestTargetRules
{
    public WindowCSharpTestTargetRules() : base("WindowCSharpTest", "Tests/WindowCSharpTest", "Luna.Window")
    {
        Sources("Main.cs");
    }
}

public sealed class ImageCSharpTestTargetRules : ManagedTestTargetRules
{
    public ImageCSharpTestTargetRules() : base("ImageCSharpTest", "Tests/ImageCSharpTest", "Luna.Runtime", "Luna.Image")
    {
        Sources("Main.cs");
        RuntimeFiles("../RHITests/RHITest3_Texture/uv_checker.png");
    }
}

public sealed class VfsCSharpTestTargetRules : ManagedTestTargetRules
{
    public VfsCSharpTestTargetRules() : base("VFSCSharpTest", "Tests/VFSCSharpTest", "Luna.Runtime", "Luna.VFS")
    {
        Sources("Main.cs");
    }
}

public sealed class FontCSharpTestTargetRules : ManagedTestTargetRules
{
    public FontCSharpTestTargetRules() : base("FontCSharpTest", "Tests/FontCSharpTest", "Luna.Runtime", "Luna.Font")
    {
        Sources("Main.cs");
    }
}

public sealed class AssetCSharpTestTargetRules : ManagedTestTargetRules
{
    public AssetCSharpTestTargetRules() : base("AssetCSharpTest", "Tests/AssetCSharpTest", "Luna.Runtime", "Luna.VFS", "Luna.Asset", "Luna.Font")
    {
        Sources("Main.cs");
    }
}

public sealed class FontArrangeCSharpTestTargetRules : ManagedTestTargetRules
{
    public FontArrangeCSharpTestTargetRules() : base("FontArrangeCSharpTest", "Tests/FontArrangeCSharpTest", "Luna.Runtime", "Luna.Window", "Luna.RHI", "Luna.Font", "Luna.VG")
    {
        Sources("Main.cs");
    }
}

public sealed class VgCSharpTestTargetRules : ManagedTestTargetRules
{
    public VgCSharpTestTargetRules() : base("VGCSharpTest", "Tests/VGCSharpTest", "Luna.Runtime", "Luna.RHI", "Luna.Font", "Luna.VG")
    {
        Sources("Main.cs");
    }
}

public sealed class VgVisualCSharpTestTargetRules : ManagedTestTargetRules
{
    public VgVisualCSharpTestTargetRules() : base("VGVisualCSharpTest", "Tests/VGVisualCSharpTest", "Luna.Runtime", "Luna.Window", "Luna.RHI", "Luna.Font", "Luna.VG")
    {
        Sources("Main.cs");
    }
}

public sealed class ImGuiCSharpTestTargetRules : ManagedTestTargetRules
{
    public ImGuiCSharpTestTargetRules() : base("ImGuiCSharpTest", "Tests/ImGuiCSharpTest", "Luna.Runtime", "Luna.Window", "Luna.RHI", "Luna.RHIUtility", "Luna.ImGui")
    {
        Sources("Main.cs");
    }
}

public sealed class HidCSharpTestTargetRules : ManagedTestTargetRules
{
    public HidCSharpTestTargetRules() : base("HIDCSharpTest", "Tests/HIDCSharpTest", "Luna.Runtime", "Luna.Window", "Luna.HID")
    {
        Sources("Main.cs");
    }
}

public sealed class AhiCSharpTestTargetRules : ManagedTestTargetRules
{
    public AhiCSharpTestTargetRules() : base("AHICSharpTest", "Tests/AHICSharpTest", "Luna.Runtime", "Luna.AHI")
    {
        Sources("Main.cs");
    }
}

public sealed class VariantUtilsCSharpTestTargetRules : ManagedTestTargetRules
{
    public VariantUtilsCSharpTestTargetRules() : base("VariantUtilsCSharpTest", "Tests/VariantUtilsCSharpTest", "Luna.Runtime", "Luna.VariantUtils")
    {
        Sources("Main.cs");
    }
}

public sealed class RhiCSharpShaderAssetsTargetRules : TargetRules
{
    public RhiCSharpShaderAssetsTargetRules()
        : base("RHICSharpShaderAssets", "Tests/RHICSharpTest", "Tests/CSharpTests.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("ShaderAssetsDummy.cpp");
        Shader("TestComputeCS.cxx", "compute", "cs_main");
    }
}

public abstract class RhiCSharpTestTargetRules : ManagedTestTargetRules
{
    protected RhiCSharpTestTargetRules(string name, string entryPoint)
        : base(
            name,
            "Tests/RHICSharpTest",
            "Luna.Runtime",
            "Luna.Window",
            "Luna.RHI",
            "Luna.RHIUtility",
            "Luna.Image",
            "RHITest2_Triangle",
            "RHITest3_Texture",
            "RHITest4_Box",
            "RHICSharpShaderAssets")
    {
        Sources("*.cs", entryPoint);
        RuntimeFiles("../RHITests/RHITest3_Texture/uv_checker.png", "../RHITests/RHITest4_Box/luna.png");
        RuntimeShaderFile("RHITest2_Triangle", "TestTriangleVS");
        RuntimeShaderFile("RHITest2_Triangle", "TestTrianglePS");
        RuntimeShaderFile("RHITest3_Texture", "TestTextureVS");
        RuntimeShaderFile("RHITest3_Texture", "TestTexturePS");
        RuntimeShaderFile("RHITest4_Box", "TestBoxVS");
        RuntimeShaderFile("RHITest4_Box", "TestBoxPS");
        RuntimeShaderFile("RHICSharpShaderAssets", "TestComputeCS");
    }
}

public sealed class RhiCSharpTestAllTargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTestAllTargetRules() : base("RHICSharpTest", "EntryPoints/All.cs") { }
}

public sealed class RhiCSharpTest0TargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTest0TargetRules() : base("RHICSharpTest0_Empty", "EntryPoints/Empty.cs") { }
}

public sealed class RhiCSharpTest1TargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTest1TargetRules() : base("RHICSharpTest1_Clear", "EntryPoints/Clear.cs") { }
}

public sealed class RhiCSharpTest2TargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTest2TargetRules() : base("RHICSharpTest2_Triangle", "EntryPoints/Triangle.cs") { }
}

public sealed class RhiCSharpTest3TargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTest3TargetRules() : base("RHICSharpTest3_Texture", "EntryPoints/Texture.cs") { }
}

public sealed class RhiCSharpTest4TargetRules : RhiCSharpTestTargetRules
{
    public RhiCSharpTest4TargetRules() : base("RHICSharpTest4_Box", "EntryPoints/Box.cs") { }
}
