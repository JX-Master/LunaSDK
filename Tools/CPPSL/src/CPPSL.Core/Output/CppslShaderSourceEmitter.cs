using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

public sealed class CppslShaderSourceEmitter
{
    public string Emit(CppslOutputTarget target, CppslCompileOptions options, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        return target switch
        {
            CppslOutputTarget.Hlsl => new HlslShaderSourceEmitter().Emit(options, model, shaderModel),
            CppslOutputTarget.Glsl => new GlslShaderSourceEmitter().Emit(options, model, shaderModel),
            CppslOutputTarget.Msl => new MslShaderSourceEmitter().Emit(options, model, shaderModel),
            _ => throw new ArgumentOutOfRangeException(nameof(target), target, null)
        };
    }
}
