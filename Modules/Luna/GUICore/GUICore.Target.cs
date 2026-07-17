namespace LunaBuild.Core.Targets;

public sealed class GUICoreTargetRules : TargetRules
{
    public GUICoreTargetRules()
        : base(
            name: "GUICore",
            targetDirectory: "Modules/Luna/GUICore",
            rulesPath: "Modules/Luna/GUICore/GUICore.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders("Context.hpp", "Renderer.hpp", "Source/GUICore.hpp", "Source/RendererImpl.hpp");
        Sources("Source/**.cpp");
        Shader("Source/SDFVS.cxx", "vertex", "vs_main");
        Shader("Source/SDFPS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "VG", "Font");
    }
}
