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
            "FontAtlas.hpp",
            "ShapeBuffer.hpp",
            "ShapeDrawList.hpp",
            "ShapeRenderer.hpp",
            "Source/FontAtlasImpl.hpp",
            "Source/ShapeBufferImpl.hpp",
            "Source/ShapeDrawListImpl.hpp",
            "Source/ShapeRendererImpl.hpp");
        Sources("Source/**.cpp");
        Shader("Source/FillVS.cxx", "vertex", "vs_main");
        Shader("Source/FillPS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "RHIUtility", "Font");
    }
}
