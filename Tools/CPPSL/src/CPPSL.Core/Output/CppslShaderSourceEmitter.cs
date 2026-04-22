using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

public sealed class CppslShaderSourceEmitter
{
    public string Emit(CppslOutputTarget target, CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        return target switch
        {
            CppslOutputTarget.Hlsl => new HlslShaderSourceEmitter().Emit(options, model, irModule),
            CppslOutputTarget.Glsl => new GlslShaderSourceEmitter().Emit(options, model, irModule),
            CppslOutputTarget.Msl => new MslShaderSourceEmitter().Emit(options, model, irModule),
            _ => throw new ArgumentOutOfRangeException(nameof(target), target, null)
        };
    }
}
