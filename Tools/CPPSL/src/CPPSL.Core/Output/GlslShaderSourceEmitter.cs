using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class GlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByPath = new(StringComparer.Ordinal);

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#version 450");
        WriteHeader(builder, "GLSL", options);
        var entryPoint = FindEntryPoint(options, model);
        _resourceAccessByPath = BuildResourceAccessMap(model);
        WriteStructs(builder, model);
        WriteGroupSharedGlobals(builder, model);
        WriteResources(builder, model);
        WriteStageIo(builder, entryPoint, model);
        WriteFunctions(builder, entryPoint, model, irModule);
        WriteEntryPointHelper(builder, entryPoint, model, irModule);
        WriteMain(builder, entryPoint, model);
        var source = RewriteResidualResourceAccessPaths(builder.ToString(), model, static global => global.Name);
        _resourceAccessByPath = new Dictionary<string, string>(StringComparer.Ordinal);
        return source;
    }

    private void WriteFunctions(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslIrModule irModule)
    {
        foreach (var function in model.Functions.Where(function => function.Name != entryPoint?.Name))
        {
            var irFunction = irModule.Functions.FirstOrDefault(candidate => candidate.Name == function.Name);
            if (irFunction?.Body is null)
            {
                continue;
            }

            builder.Append(MapValueType(function.ReturnType ?? "void"));
            builder.Append(' ');
            builder.Append(function.Name);
            builder.Append('(');
            builder.Append(string.Join(", ", function.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
            builder.AppendLine(")");
            WriteFunctionBody(builder, function, model, irFunction.Body);
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
            var layout = $"layout(set = {global.DescriptorSet}, binding = {global.Binding})";
            switch (global.ResourceKind)
            {
                case "constant_buffer":
                    WriteConstantBuffer(builder, layout, global, model);
                    break;
                case "texture":
                    builder.AppendLine($"{layout} uniform texture2D {global.Name};");
                    break;
                case "rw_texture":
                    builder.AppendLine($"{layout} uniform image2D {global.Name};");
                    break;
                case "sampler":
                    builder.AppendLine($"{layout} uniform sampler {global.Name};");
                    break;
                default:
                    builder.AppendLine($"{layout} buffer {global.Name}_Block");
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

    private void WriteStageIo(StringBuilder builder, CppslFunction? entryPoint, CppslSemanticModel model)
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
                builder.AppendLine($"layout(location = {field.Location}) in {MapValueType(field.Type)} cppsl_in_{field.Name};");
            }
        }

        var outputStruct = model.Structs.FirstOrDefault(structure => structure.Name == entryPoint.ReturnType);
        if (outputStruct is not null)
        {
            foreach (var field in outputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"layout(location = {field.Location}) out {MapValueType(field.Type)} cppsl_out_{field.Name};");
            }
        }

        builder.AppendLine();
    }

    private void WriteEntryPointHelper(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslIrModule irModule)
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
        WriteFunctionBody(builder, entryPoint, model, irModule);
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
        foreach (var parameter in entryPoint.Parameters)
        {
            var inputStruct = model.Structs.FirstOrDefault(structure => structure.Name == parameter.Type);
            if (inputStruct is null)
            {
                continue;
            }

            builder.AppendLine($"    {parameter.Type} {parameter.Name};");
            foreach (var field in inputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"    {parameter.Name}.{field.Name} = cppsl_in_{field.Name};");
            }
        }

        if (entryPoint.ReturnType is not null && entryPoint.ReturnType != "void")
        {
            builder.AppendLine($"    {entryPoint.ReturnType} cppsl_output = {entryPoint.Name}({string.Join(", ", entryPoint.Parameters.Select(static parameter => parameter.Name))});");
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
            builder.AppendLine($"    {entryPoint.Name}({string.Join(", ", entryPoint.Parameters.Select(static parameter => parameter.Name))});");
        }

        builder.AppendLine("}");
    }

    protected override string FormatFloatingLiteral(string spelling)
    {
        return spelling.EndsWith('f') || spelling.EndsWith('F')
            ? spelling[..^1]
            : spelling;
    }

    protected override string LowerExpression(CppslIrNode node)
    {
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
            return $"texelFetch(sampler2D({receiver}, sampler()), ivec2({arguments[0]}), 0)";
        }
        if (memberName == "Store" && arguments.Count == 2)
        {
            return $"imageStore({receiver}, ivec2({arguments[0]}), {arguments[1]})";
        }

        return base.LowerMemberCall(receiver, memberName, arguments);
    }

    protected override string LowerCallExpression(CppslIrNode node)
    {
        if (TryGetGlslMemberCall(node, out var receiverNode, out var receiver, out var memberName, out var memberArguments))
        {
            if (memberName == "Sample" && memberArguments.Count == 2)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texture(sampler2D({receiver}, {memberArguments[0]}), {memberArguments[1]})");
            }
            if (memberName == "SampleLevel" && memberArguments.Count == 3)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"textureLod(sampler2D({receiver}, {memberArguments[0]}), {memberArguments[1]}, {memberArguments[2]})");
            }
            if (memberName == "Load" && memberArguments.Count == 1)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texelFetch(sampler2D({receiver}, sampler()), ivec2({memberArguments[0]}), 0)");
            }
            if (memberName == "Store" && memberArguments.Count == 2)
            {
                return $"imageStore({receiver}, ivec2({memberArguments[0]}), {AdaptTextureStoreValue(receiverNode, memberArguments[1])})";
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

        return base.LowerCallExpression(node);
    }

    private static string AdaptTextureReadExpression(CppslIrNode receiverNode, string expression)
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

    private string AdaptTextureStoreValue(CppslIrNode receiverNode, string value)
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
        CppslIrNode node,
        out CppslIrNode receiverNode,
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
}
