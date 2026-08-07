namespace LunaBuild.Core.Targets;

public sealed class GUITargetRules : TargetRules
{
    public GUITargetRules()
        : base(
            name: "GUI",
            targetDirectory: "Modules/Luna/GUI",
            rulesPath: "Modules/Luna/GUI/GUI.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Context.hpp", "Renderer.hpp", "Source/GUI.hpp", "Source/RendererImpl.hpp");
        Sources("Source/**.cpp");
        Shader("Source/SDFVS.cxx", "vertex", "vs_main");
        Shader("Source/SDFPS.cxx", "pixel", "ps_main");
        Shader("Source/BackdropBlurCS.cxx", "compute", "cs_main");
        DependsOn("Runtime", "RHI", "VG", "Font");
    }
}
