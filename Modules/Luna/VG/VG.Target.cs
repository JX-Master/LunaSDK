namespace LunaBuild.Core.Targets;

public sealed class VGTargetRules : TargetRules
{
    public VGTargetRules()
        : base(
            name: "VG",
            targetDirectory: "Modules/Luna/VG",
            rulesPath: "Modules/Luna/VG/VG.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders(
            "Source/FontAtlas.hpp",
            "Source/ShapeBuffer.hpp",
            "Source/ShapeDrawList.hpp",
            "Source/ShapeRenderer.hpp");
        Sources("Source/**.cpp");
        Shader("Source/FillVS.cxx", "vertex", "vs_main");
        Shader("Source/FillPS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "RHIUtility");
    }
}
