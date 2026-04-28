using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class GlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByPath = new(StringComparer.Ordinal);

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#version 450");
        builder.AppendLine("#extension GL_EXT_samplerless_texture_functions : require");
        WriteHeader(builder, "GLSL", options);
        var entryPoint = FindEntryPoint(options, model);
        _resourceAccessByPath = BuildResourceAccessMap(model);
        WriteStructs(builder, model);
        WriteComputeLayout(builder, entryPoint);
        WriteGroupSharedGlobals(builder, model);
        WriteResources(builder, model);
        WriteStageIo(builder, entryPoint, model, options.Stage);
        WriteFunctions(builder, entryPoint, model, shaderModel);
        WriteEntryPointHelper(builder, entryPoint, model, shaderModel);
        WriteMain(builder, entryPoint, model);
        var source = RewriteResidualResourceAccessPaths(builder.ToString(), model, static global => global.Name);
        _resourceAccessByPath = new Dictionary<string, string>(StringComparer.Ordinal);
        return source;
    }

    private void WriteFunctions(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        foreach (var function in model.Functions.Where(function => function.Name != entryPoint?.Name))
        {
            var shaderModelFunction = shaderModel.Functions.FirstOrDefault(candidate => candidate.Name == function.Name);
            if (shaderModelFunction?.Body is null)
            {
                continue;
            }

            builder.Append(MapValueType(function.ReturnType ?? "void"));
            builder.Append(' ');
            builder.Append(function.Name);
            builder.Append('(');
            builder.Append(string.Join(", ", function.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
            builder.AppendLine(")");
            WriteFunctionBody(builder, function, model, shaderModelFunction.Body);
            builder.AppendLine();
        }
    }

    private void WriteStructs(StringBuilder builder, CppslSemanticModel model)
    {
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        foreach (var structure in model.Structs.Where(structure => !descriptorSetLayouts.Contains(structure.Name)))
        {
            builder.AppendLine($"struct {structure.Name}");
            builder.AppendLine("{");
            foreach (var field in structure.Fields)
            {
                builder.AppendLine($"    {MapValueType(field.Type)} {field.Name};");
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private void WriteResources(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            switch (global.ResourceKind)
            {
                case "constant_buffer":
                    WriteConstantBuffer(builder, GlslBufferLayout(global, "std140"), global, model);
                    break;
                case "texture":
                    builder.AppendLine($"{GlslResourceLayout(global)} uniform {GlslTextureType(global.Type)} {global.Name};");
                    break;
                case "rw_texture":
                    builder.AppendLine($"layout(set = {global.DescriptorSet}, binding = {global.Binding}, {GlslImageFormat(global)}) uniform {GlslImageType(global)} {global.Name};");
                    break;
                case "sampler":
                    builder.AppendLine($"{GlslResourceLayout(global)} uniform sampler {global.Name};");
                    break;
                default:
                    builder.AppendLine($"{GlslBufferLayout(global, "std430")} buffer {global.Name}_Block");
                    builder.AppendLine("{");
                    builder.AppendLine($"    {MapValueType(ResourceElementType(global))} {global.Name}[];");
                    builder.AppendLine("};");
                    break;
            }
        }

        if (model.Globals.Any(static global => global.ResourceKind is not null))
        {
            builder.AppendLine();
        }
    }

    private static string GlslResourceLayout(CppslGlobal global)
    {
        return $"layout(set = {global.DescriptorSet}, binding = {global.Binding})";
    }

    private static string GlslBufferLayout(CppslGlobal global, string packing)
    {
        return $"layout(set = {global.DescriptorSet}, binding = {global.Binding}, {packing}, column_major)";
    }

    private static void WriteComputeLayout(StringBuilder builder, CppslFunction? entryPoint)
    {
        if (entryPoint?.Attributes.FindAttribute("compute") is not { Arguments.Count: >= 3 } compute)
        {
            return;
        }

        builder.AppendLine($"layout(local_size_x = {compute.Arguments[0]}, local_size_y = {compute.Arguments[1]}, local_size_z = {compute.Arguments[2]}) in;");
        builder.AppendLine();
    }

    private void WriteGroupSharedGlobals(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var global in model.Globals.Where(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine($"shared {FormatVariableDeclaration(MapValueType(global.Type), global.Name)};");
        }

        if (model.Globals.Any(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine();
        }
    }

    private void WriteConstantBuffer(
        StringBuilder builder,
        string layout,
        CppslGlobal global,
        CppslSemanticModel model)
    {
        var elementType = ResourceElementType(global);
        var structure = model.Structs.FirstOrDefault(candidate => candidate.Name == elementType);
        builder.AppendLine($"{layout} uniform {global.Name}_Block");
        builder.AppendLine("{");
        if (structure is null)
        {
            builder.AppendLine($"    {MapValueType(elementType)} value;");
        }
        else
        {
            foreach (var field in structure.Fields)
            {
                builder.AppendLine($"    {MapValueType(field.Type)} {field.Name};");
            }
        }
        builder.AppendLine($"}} {global.Name};");
    }

    private void WriteStageIo(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        ShaderStage stage)
    {
        if (entryPoint is null)
        {
            return;
        }

        foreach (var parameter in entryPoint.Parameters)
        {
            var inputStruct = model.Structs.FirstOrDefault(structure => structure.Name == parameter.Type);
            if (inputStruct is null)
            {
                continue;
            }

            foreach (var field in inputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"layout(location = {field.Location}) {StageInputQualifier(stage, field.Type)}in {MapValueType(field.Type)} cppsl_in_{field.Name};");
            }
        }

        var outputStruct = model.Structs.FirstOrDefault(structure => structure.Name == entryPoint.ReturnType);
        if (outputStruct is not null)
        {
            foreach (var field in outputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"layout(location = {field.Location}) {StageOutputQualifier(stage, field.Type)}out {MapValueType(field.Type)} cppsl_out_{field.Name};");
            }
        }

        builder.AppendLine();
    }

    private static string StageInputQualifier(ShaderStage stage, string type)
    {
        return (stage == ShaderStage.Fragment || stage == ShaderStage.Pixel) && IsIntegerLikeStageIoType(type)
            ? "flat "
            : string.Empty;
    }

    private static string StageOutputQualifier(ShaderStage stage, string type)
    {
        return stage == ShaderStage.Vertex && IsIntegerLikeStageIoType(type)
            ? "flat "
            : string.Empty;
    }

    private static bool IsIntegerLikeStageIoType(string type)
    {
        return NormalizeShaderTypeName(type) switch
        {
            "bool" or "bool_t" or "bool2" or "bool3" or "bool4" => true,
            "int" or "int2" or "int3" or "int4" => true,
            "uint" or "uint2" or "uint3" or "uint4" => true,
            _ => false
        };
    }

    private static bool IsBoolVectorType(string? type)
    {
        return NormalizeShaderTypeName(type ?? string.Empty) is "bool2" or "bool3" or "bool4";
    }

    private void WriteEntryPointHelper(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        if (entryPoint is null)
        {
            return;
        }

        builder.Append(MapValueType(entryPoint.ReturnType ?? "void"));
        builder.Append(' ');
        builder.Append(entryPoint.Name);
        builder.Append('(');
        builder.Append(string.Join(", ", entryPoint.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
        builder.AppendLine(")");
        WriteFunctionBody(builder, entryPoint, model, shaderModel);
    }

    private void WriteMain(StringBuilder builder, CppslFunction? entryPoint, CppslSemanticModel model)
    {
        if (entryPoint is null)
        {
            return;
        }

        builder.AppendLine();
        builder.AppendLine("void main()");
        builder.AppendLine("{");
        var entryArguments = new List<string>();
        foreach (var parameter in entryPoint.Parameters)
        {
            var parameterName = parameter.Name;
            var inputStruct = model.Structs.FirstOrDefault(structure => structure.Name == parameter.Type);
            if (inputStruct is not null)
            {
                builder.AppendLine($"    {parameter.Type} {parameterName};");
                foreach (var field in inputStruct.Fields.Where(static field => field.Location is not null))
                {
                    builder.AppendLine($"    {parameterName}.{field.Name} = cppsl_in_{field.Name};");
                }
                entryArguments.Add(parameterName);
                continue;
            }

            if (TryGetBuiltinExpression(parameter, out var builtinExpression))
            {
                builder.AppendLine($"    {MapValueType(parameter.Type)} {parameterName} = {builtinExpression};");
                entryArguments.Add(parameterName);
                continue;
            }

            entryArguments.Add(parameterName);
        }

        if (entryPoint.ReturnType is not null && entryPoint.ReturnType != "void")
        {
            builder.AppendLine($"    {entryPoint.ReturnType} cppsl_output = {entryPoint.Name}({string.Join(", ", entryArguments)});");
            var outputStruct = model.Structs.FirstOrDefault(structure => structure.Name == entryPoint.ReturnType);
            if (outputStruct is not null)
            {
                foreach (var field in outputStruct.Fields)
                {
                    if (field.IsPosition)
                    {
                        builder.AppendLine($"    gl_Position = cppsl_output.{field.Name};");
                    }
                    else if (field.Location is not null)
                    {
                        builder.AppendLine($"    cppsl_out_{field.Name} = cppsl_output.{field.Name};");
                    }
                }
            }
        }
        else
        {
            builder.AppendLine($"    {entryPoint.Name}({string.Join(", ", entryArguments)});");
        }

        builder.AppendLine("}");
    }

    protected override string FormatFloatingLiteral(string spelling)
    {
        return spelling.EndsWith('f') || spelling.EndsWith('F')
            ? spelling[..^1]
            : spelling;
    }

    protected override string LowerExpression(CppslShaderModelNode node)
    {
        if (node.Kind == "CStyleCastExpression" &&
            NormalizeShaderTypeName(node.Type ?? node.TypeInfo?.Spelling ?? string.Empty) == "void" &&
            node.Children.Count == 1)
        {
            return LowerExpression(node.Children[0]);
        }

        if (node.Kind == "BinaryOperator" &&
            node.DisplayName is "==" or "!=" &&
            node.Children.Count >= 2 &&
            IsBoolVectorType(node.Type ?? node.TypeInfo?.Spelling))
        {
            var functionName = node.DisplayName == "==" ? "equal" : "notEqual";
            return $"{functionName}({LowerExpression(node.Children[0])}, {LowerExpression(node.Children[1])})";
        }

        if (node.Kind == "OperatorCallExpression" &&
            node.DisplayName is "operator==" or "operator!=" &&
            node.Children.Count >= 3 &&
            IsBoolVectorType(node.Type ?? node.TypeInfo?.Spelling))
        {
            var functionName = node.DisplayName == "operator==" ? "equal" : "notEqual";
            return $"{functionName}({LowerExpression(node.Children[1])}, {LowerExpression(node.Children[2])})";
        }

        if (TryGetAccessPath(node, out var accessPath) &&
            _resourceAccessByPath.TryGetValue(accessPath, out var resourceAccess))
        {
            return resourceAccess;
        }

        return base.LowerExpression(node);
    }

    protected override string LowerMemberCall(string receiver, string memberName, IReadOnlyList<string> arguments)
    {
        if (memberName == "Sample" && arguments.Count == 2)
        {
            return $"texture(sampler2D({receiver}, {arguments[0]}), {arguments[1]})";
        }
        if (memberName == "SampleLevel" && arguments.Count == 3)
        {
            return $"textureLod(sampler2D({receiver}, {arguments[0]}), {arguments[1]}, {arguments[2]})";
        }
        if (memberName == "Load" && arguments.Count == 1)
        {
            return $"texelFetch({receiver}, {GlslTexelCoordinate(2, arguments[0])}, 0)";
        }
        if (memberName == "Store" && arguments.Count == 2)
        {
            return $"imageStore({receiver}, {GlslTexelCoordinate(2, arguments[0])}, {arguments[1]})";
        }

        return base.LowerMemberCall(receiver, memberName, arguments);
    }

    protected override string LowerCallExpression(CppslShaderModelNode node)
    {
        if (TryGetGlslMemberCall(node, out var receiverNode, out var receiver, out var memberName, out var memberArguments))
        {
            if (memberName == "Sample" && memberArguments.Count == 2)
            {
                var dimension = TextureDimension(receiverNode);
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texture({GlslSamplerConstructor(dimension, receiver, memberArguments[0])}, {memberArguments[1]})");
            }
            if (memberName == "SampleLevel" && memberArguments.Count == 3)
            {
                var dimension = TextureDimension(receiverNode);
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"textureLod({GlslSamplerConstructor(dimension, receiver, memberArguments[0])}, {memberArguments[1]}, {memberArguments[2]})");
            }
            if (memberName == "Load" && memberArguments.Count == 1)
            {
                var dimension = TextureDimension(receiverNode);
                var coordinate = GlslTexelCoordinate(dimension, memberArguments[0]);
                if (IsStorageTexture(receiverNode))
                {
                    return AdaptTextureReadExpression(
                        receiverNode,
                        $"imageLoad({receiver}, {coordinate})");
                }

                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texelFetch({receiver}, {coordinate}, 0)");
            }
            if (memberName == "Store" && memberArguments.Count == 2)
            {
                var dimension = TextureDimension(receiverNode);
                return $"imageStore({receiver}, {GlslTexelCoordinate(dimension, memberArguments[0])}, {AdaptTextureStoreValue(receiverNode, memberArguments[1])})";
            }
        }

        var name = node.DisplayName ?? node.Spelling;
        var arguments = node.Children
            .Skip(IsCalleeReference(node.Children.FirstOrDefault(), name) ? 1 : 0)
            .Select(LowerExpression)
            .ToArray();

        if (name == "GroupMemoryBarrierWithGroupSync" && arguments.Length == 0)
        {
            return "barrier()";
        }
        if (name == "discard_fragment" && arguments.Length == 0)
        {
            return "discard";
        }
        if (name == "InterlockedAdd" && arguments.Length == 2)
        {
            return $"atomicAdd({arguments[0]}, {arguments[1]})";
        }
        if (name == "lerp" && arguments.Length == 3)
        {
            return $"mix({arguments[0]}, {arguments[1]}, {arguments[2]})";
        }
        if (name == "atan2" && arguments.Length == 2)
        {
            return $"atan({arguments[0]}, {arguments[1]})";
        }
        if (name == "saturate" && arguments.Length == 1)
        {
            return $"clamp({arguments[0]}, 0.0, 1.0)";
        }

        return base.LowerCallExpression(node);
    }

    private static string AdaptTextureReadExpression(CppslShaderModelNode receiverNode, string expression)
    {
        if (!TryGetTextureValueType(receiverNode, out var valueType))
        {
            return expression;
        }

        return NormalizeShaderTypeName(valueType) switch
        {
            "float" or "int" or "uint" => $"{expression}.x",
            "float2" or "int2" or "uint2" => $"{expression}.xy",
            "float3" or "int3" or "uint3" => $"{expression}.xyz",
            _ => expression
        };
    }

    private string AdaptTextureStoreValue(CppslShaderModelNode receiverNode, string value)
    {
        if (!TryGetTextureValueType(receiverNode, out var valueType))
        {
            return value;
        }

        return NormalizeShaderTypeName(valueType) switch
        {
            "float" => $"vec4({value})",
            "float2" => $"vec4({value}, 0.0, 0.0)",
            "float3" => $"vec4({value}, 0.0)",
            "int" => $"ivec4({value})",
            "int2" => $"ivec4({value}, 0, 0)",
            "int3" => $"ivec4({value}, 0)",
            "uint" => $"uvec4({value})",
            "uint2" => $"uvec4({value}, 0u, 0u)",
            "uint3" => $"uvec4({value}, 0u)",
            _ => value
        };
    }

    private bool TryGetGlslMemberCall(
        CppslShaderModelNode node,
        out CppslShaderModelNode receiverNode,
        out string receiver,
        out string memberName,
        out IReadOnlyList<string> arguments)
    {
        receiverNode = node;
        receiver = string.Empty;
        memberName = string.Empty;
        arguments = Array.Empty<string>();

        if (node.Children.FirstOrDefault() is not { Kind: "MemberExpression" } member ||
            member.DisplayName is null ||
            member.Children.Count != 1)
        {
            return false;
        }

        receiverNode = member.Children[0];
        receiver = LowerExpression(receiverNode);
        memberName = member.DisplayName;
        arguments = node.Children.Skip(1).Select(LowerExpression).ToArray();
        return true;
    }

    protected override string MapValueType(string type)
    {
        return type switch
        {
            "float2" => "vec2",
            "float3" => "vec3",
            "float4" => "vec4",
            "float4x4" => "mat4",
            "int2" => "ivec2",
            "int3" => "ivec3",
            "int4" => "ivec4",
            "bool2" => "bvec2",
            "bool3" => "bvec3",
            "bool4" => "bvec4",
            "uint2" => "uvec2",
            "uint3" => "uvec3",
            "uint4" => "uvec4",
            "uint" => "uint",
            "_Bool" => "bool",
            "bool_t" => "bool",
            _ => type
        };
    }

    protected override string DefaultStructValue(CppslStruct structure, CppslSemanticModel model)
    {
        return $"{structure.Name}({string.Join(", ", structure.Fields.Select(field => DefaultValue(field.Type, model)))})";
    }

    protected override string DefaultAggregateValue(string mappedType)
    {
        return $"{mappedType}(0.0)";
    }

    private static Dictionary<string, string> BuildResourceAccessMap(CppslSemanticModel model)
    {
        var map = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null && global.AccessPath is not null))
        {
            map[global.AccessPath!] = global.Name;
        }
        return map;
    }

    private static bool TryGetBuiltinExpression(CppslParameter parameter, out string expression)
    {
        expression = parameter.Attributes.FindAttribute("builtin")?.Arguments.FirstOrDefault() switch
        {
            "dispatch_thread_id" => "uvec3(gl_GlobalInvocationID)",
            "group_index" => "uint(gl_LocalInvocationIndex)",
            _ => string.Empty
        };
        return expression.Length != 0;
    }

    private static string GlslTextureType(string type)
    {
        return TextureDimension(type) switch
        {
            1 => "texture1D",
            3 => "texture3D",
            _ => "texture2D"
        };
    }

    private static string GlslImageType(CppslGlobal global)
    {
        var prefix = TextureElementType(global) switch
        {
            "int" or "int2" or "int3" or "int4" => "i",
            "uint" or "uint2" or "uint3" or "uint4" => "u",
            _ => string.Empty
        };
        return TextureDimension(global.Type) switch
        {
            1 => $"{prefix}image1D",
            3 => $"{prefix}image3D",
            _ => $"{prefix}image2D"
        };
    }

    private static string GlslImageFormat(CppslGlobal global)
    {
        return TextureElementType(global) switch
        {
            "float" => "r32f",
            "float2" => "rg32f",
            "float3" or "float4" => "rgba32f",
            "int" => "r32i",
            "int2" => "rg32i",
            "int3" or "int4" => "rgba32i",
            "uint" => "r32ui",
            "uint2" => "rg32ui",
            "uint3" or "uint4" => "rgba32ui",
            _ => "rgba32f"
        };
    }

    private static string TextureElementType(CppslGlobal global)
    {
        return global.Type.Contains('<', StringComparison.Ordinal)
            ? NormalizeShaderTypeName(UnwrapTemplateArgument(global.Type))
            : NormalizeShaderTypeName(global.Type);
    }

    private static int TextureDimension(CppslShaderModelNode node)
    {
        return TryGetTextureDimension(node, out var dimension) ? dimension : 2;
    }

    private static int TextureDimension(string type)
    {
        if (TryGetTextureDimension(type, out var dimension))
        {
            return dimension;
        }
        return 2;
    }

    private static bool TryGetTextureDimension(CppslShaderModelNode node, out int dimension)
    {
        foreach (var candidate in NodeTypeCandidates(node))
        {
            if (TryGetTextureDimension(candidate, out dimension))
            {
                return true;
            }
        }

        foreach (var child in node.Children)
        {
            if (TryGetTextureDimension(child, out dimension))
            {
                return true;
            }
        }

        dimension = 0;
        return false;
    }

    private static bool TryGetTextureDimension(string type, out int dimension)
    {
        var normalized = NormalizeShaderTypeName(type);
        if (normalized.Contains("Texture1D<", StringComparison.Ordinal) ||
            normalized.Contains("RWTexture1D<", StringComparison.Ordinal))
        {
            dimension = 1;
            return true;
        }
        if (normalized.Contains("Texture3D<", StringComparison.Ordinal) ||
            normalized.Contains("RWTexture3D<", StringComparison.Ordinal))
        {
            dimension = 3;
            return true;
        }
        if (normalized.Contains("Texture2D<", StringComparison.Ordinal) ||
            normalized.Contains("DepthTexture2D<", StringComparison.Ordinal) ||
            normalized.Contains("RWTexture2D<", StringComparison.Ordinal))
        {
            dimension = 2;
            return true;
        }

        dimension = 0;
        return false;
    }

    private static bool IsStorageTexture(CppslShaderModelNode node)
    {
        foreach (var candidate in NodeTypeCandidates(node))
        {
            if (NormalizeShaderTypeName(candidate).Contains("RWTexture", StringComparison.Ordinal))
            {
                return true;
            }
        }

        return node.Children.Any(IsStorageTexture);
    }

    private static string GlslSamplerConstructor(int dimension, string texture, string sampler)
    {
        var samplerType = dimension switch
        {
            1 => "sampler1D",
            3 => "sampler3D",
            _ => "sampler2D"
        };
        return $"{samplerType}({texture}, {sampler})";
    }

    private static string GlslTexelCoordinate(int dimension, string coordinate)
    {
        return dimension switch
        {
            1 => $"int({coordinate})",
            3 => $"ivec3({coordinate})",
            _ => $"ivec2({coordinate})"
        };
    }

    private static IEnumerable<string> NodeTypeCandidates(CppslShaderModelNode node)
    {
        if (!string.IsNullOrWhiteSpace(node.Type))
        {
            yield return node.Type!;
        }
        if (!string.IsNullOrWhiteSpace(node.TypeInfo?.Spelling))
        {
            yield return node.TypeInfo!.Spelling;
        }
        if (!string.IsNullOrWhiteSpace(node.TypeInfo?.CanonicalName))
        {
            yield return node.TypeInfo!.CanonicalName;
        }
        if (!string.IsNullOrWhiteSpace(node.TypeInfo?.DesugaredName))
        {
            yield return node.TypeInfo!.DesugaredName;
        }
    }
}
