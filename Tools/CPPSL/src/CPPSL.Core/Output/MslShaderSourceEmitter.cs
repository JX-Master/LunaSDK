using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class MslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByName = new(StringComparer.Ordinal);

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#include <metal_stdlib>");
        builder.AppendLine("using namespace metal;");
        builder.AppendLine();
        WriteHeader(builder, "MSL", options);
        var entryPoint = FindEntryPoint(options, model);
        WriteStructs(builder, options, model, entryPoint);
        WriteArgumentBufferStructs(builder, model);
        _resourceAccessByName = BuildResourceAccessMap(model);
        WriteFunctions(builder, entryPoint, model, irModule);
        WriteEntryPoint(builder, options, entryPoint, model, irModule);
        _resourceAccessByName = new Dictionary<string, string>(StringComparer.Ordinal);
        return builder.ToString();
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

    private void WriteStructs(
        StringBuilder builder,
        CppslCompileOptions options,
        CppslSemanticModel model,
        CppslFunction? entryPoint)
    {
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        var inputStructNames = entryPoint?.Parameters.Select(static parameter => parameter.Type).ToHashSet(StringComparer.Ordinal) ??
            new HashSet<string>(StringComparer.Ordinal);
        var outputStructName = entryPoint?.ReturnType;
        foreach (var structure in model.Structs.Where(structure => !descriptorSetLayouts.Contains(structure.Name)))
        {
            builder.AppendLine($"struct {structure.Name}");
            builder.AppendLine("{");
            foreach (var field in structure.Fields)
            {
                var role = structure.Name == outputStructName
                    ? StructRole.StageOutput
                    : inputStructNames.Contains(structure.Name) ? StructRole.StageInput : StructRole.Plain;
                builder.Append("    ");
                builder.Append(MapValueType(field.Type));
                builder.Append(' ');
                builder.Append(field.Name);
                builder.Append(FieldAttribute(field, role, options.Stage));
                builder.AppendLine(";");
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private void WriteEntryPoint(
        StringBuilder builder,
        CppslCompileOptions options,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslIrModule irModule)
    {
        if (entryPoint is null)
        {
            return;
        }

        builder.Append(StagePrefix(options));
        builder.Append(MapValueType(entryPoint.ReturnType ?? "void"));
        builder.Append(' ');
        builder.Append(entryPoint.Name);
        builder.Append('(');
        builder.Append(string.Join(", ", BuildEntryParameters(entryPoint, model)));
        builder.AppendLine(")");
        WriteEntryFunctionBody(builder, entryPoint, model, irModule);
    }

    private IEnumerable<string> BuildEntryParameters(CppslFunction entryPoint, CppslSemanticModel model)
    {
        foreach (var parameter in entryPoint.Parameters)
        {
            if (parameter.Attributes.FindAttribute("builtin")?.Arguments.FirstOrDefault() == "dispatch_thread_id")
            {
                yield return $"{MapValueType(parameter.Type)} {parameter.Name} [[thread_position_in_grid]]";
            }
            else if (parameter.Attributes.FindAttribute("builtin")?.Arguments.FirstOrDefault() == "group_index")
            {
                yield return $"{MapValueType(parameter.Type)} {parameter.Name} [[thread_index_in_threadgroup]]";
            }
            else
            {
                yield return $"{MapValueType(parameter.Type)} {parameter.Name} [[stage_in]]";
            }
        }

        foreach (var descriptorSet in model.Globals
            .Where(static global => global.ResourceKind is not null)
            .Select(static global => global.DescriptorSet)
            .Where(static set => set is not null)
            .Cast<int>()
            .Distinct()
            .Order())
        {
            yield return $"constant {DescriptorSetStructName(descriptorSet)}& {DescriptorSetParameterName(descriptorSet)} [[buffer({descriptorSet})]]";
        }
    }

    private void WriteArgumentBufferStructs(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var descriptorSetGroup in model.Globals
            .Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null)
            .GroupBy(static global => global.DescriptorSet!.Value)
            .OrderBy(static group => group.Key))
        {
            builder.AppendLine($"struct {DescriptorSetStructName(descriptorSetGroup.Key)}");
            builder.AppendLine("{");
            var metalArgumentIndex = 0;
            foreach (var global in descriptorSetGroup.OrderBy(static global => global.Binding ?? 0))
            {
                builder.Append("    ");
                builder.Append(ArgumentBufferField(global, metalArgumentIndex));
                builder.AppendLine(";");
                ++metalArgumentIndex;
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private void WriteEntryFunctionBody(
        StringBuilder builder,
        CppslFunction entryPoint,
        CppslSemanticModel model,
        CppslIrModule irModule)
    {
        builder.AppendLine("{");
        foreach (var global in model.Globals.Where(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine($"    threadgroup {FormatVariableDeclaration(MapValueType(global.Type), global.Name)};");
        }

        var irEntryPoint = irModule.EntryPoints.FirstOrDefault(entry => entry.Name == entryPoint.Name);
        if (irEntryPoint?.Body is null)
        {
            WriteDefaultReturn(builder, entryPoint.ReturnType ?? "void", model, 1);
        }
        else
        {
            WriteStatementChildren(builder, irEntryPoint.Body, 1);
        }
        builder.AppendLine("}");
    }

    private string ArgumentBufferField(CppslGlobal global, int metalArgumentIndex)
    {
        return global.ResourceKind switch
        {
            "constant_buffer" => $"constant {MapValueType(ResourceElementType(global))}* {global.Name} [[id({metalArgumentIndex})]]",
            "structured_buffer" => $"device const {MapValueType(ResourceElementType(global))}* {global.Name} [[id({metalArgumentIndex})]]",
            "rw_structured_buffer" => $"device {MapValueType(ResourceElementType(global))}* {global.Name} [[id({metalArgumentIndex})]]",
            "texture" => $"{MetalTextureType(global.Type, false)} {global.Name} [[id({metalArgumentIndex})]]",
            "rw_texture" => $"{MetalTextureType(global.Type, true)} {global.Name} [[id({metalArgumentIndex})]]",
            "sampler" => $"sampler {global.Name} [[id({metalArgumentIndex})]]",
            _ => $"{global.Type} {global.Name}"
        };
    }

    private static Dictionary<string, string> BuildResourceAccessMap(CppslSemanticModel model)
    {
        var map = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null))
        {
            var descriptorSet = DescriptorSetParameterName(global.DescriptorSet!.Value);
            var accessPath = global.AccessPath ?? global.Name;
            var access = $"{descriptorSet}.{global.Name}";
            map[accessPath] = global.ResourceKind == "constant_buffer"
                ? $"(*{access})"
                : access;
        }
        return map;
    }

    protected override string LowerExpression(CppslIrNode node)
    {
        if (TryGetAccessPath(node, out var accessPath) &&
            _resourceAccessByName.TryGetValue(accessPath, out var resourceAccess))
        {
            return resourceAccess;
        }

        return base.LowerExpression(node);
    }

    protected override string MapValueType(string type)
    {
        return type switch
        {
            "_Bool" => "bool",
            "bool_t" => "bool",
            _ => type
        };
    }

    protected override string LowerMemberCall(string receiver, string memberName, IReadOnlyList<string> arguments)
    {
        if (memberName == "Sample" && arguments.Count == 2)
        {
            return $"{receiver}.sample({arguments[0]}, {arguments[1]})";
        }
        if (memberName == "SampleLevel" && arguments.Count == 3)
        {
            return $"{receiver}.sample({arguments[0]}, {arguments[1]}, level({arguments[2]}))";
        }
        if (memberName == "Load" && arguments.Count == 1)
        {
            return $"{receiver}.read({arguments[0]})";
        }
        if (memberName == "Store" && arguments.Count == 2)
        {
            return $"{receiver}.write({arguments[1]}, {arguments[0]})";
        }

        return base.LowerMemberCall(receiver, memberName, arguments);
    }

    protected override string LowerCallExpression(CppslIrNode node)
    {
        if (TryGetMslMemberCall(node, out var receiverNode, out var receiver, out var memberName, out var memberArguments))
        {
            if (memberName == "Sample" && memberArguments.Count == 2)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]})");
            }
            if (memberName == "SampleLevel" && memberArguments.Count == 3)
            {
                var expression = IsTexture1DReceiver(receiverNode)
                    ? $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]})"
                    : $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]}, level({memberArguments[2]}))";
                return AdaptTextureReadExpression(receiverNode, expression);
            }
            if (memberName == "Load" && memberArguments.Count == 1)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"{receiver}.read({memberArguments[0]})");
            }
            if (memberName == "Store" && memberArguments.Count == 2)
            {
                return $"{receiver}.write({AdaptTextureStoreValue(receiverNode, memberArguments[1])}, {memberArguments[0]})";
            }

            return LowerMemberCall(receiver, memberName, memberArguments);
        }

        var name = node.DisplayName ?? node.Spelling;
        var arguments = node.Children
            .Skip(IsCalleeReference(node.Children.FirstOrDefault(), name) ? 1 : 0)
            .Select(LowerExpression)
            .ToArray();

        if (name == "GroupMemoryBarrierWithGroupSync" && arguments.Length == 0)
        {
            return "threadgroup_barrier(mem_flags::mem_threadgroup)";
        }
        if (name == "discard_fragment" && arguments.Length == 0)
        {
            return "discard_fragment()";
        }

        if (name == "InterlockedAdd" && arguments.Length == 2)
        {
            return $"atomic_fetch_add_explicit((threadgroup atomic_uint*)&{arguments[0]}, {arguments[1]}, memory_order_relaxed)";
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

    private static string AdaptTextureStoreValue(CppslIrNode receiverNode, string value)
    {
        if (!TryGetTextureValueType(receiverNode, out var valueType))
        {
            return value;
        }

        return NormalizeShaderTypeName(valueType) switch
        {
            "float" => $"float4({value})",
            "float2" => $"float4({value}, 0.0f, 0.0f)",
            "float3" => $"float4({value}, 0.0f)",
            "int" => $"int4({value})",
            "int2" => $"int4({value}, 0, 0)",
            "int3" => $"int4({value}, 0)",
            "uint" => $"uint4({value})",
            "uint2" => $"uint4({value}, 0u, 0u)",
            "uint3" => $"uint4({value}, 0u)",
            _ => value
        };
    }

    private bool TryGetMslMemberCall(
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

    private static bool IsTexture1DReceiver(CppslIrNode node)
    {
        if (ContainsTexture1DType(node.Type) ||
            ContainsTexture1DType(node.TypeInfo?.Spelling) ||
            ContainsTexture1DType(node.TypeInfo?.CanonicalName) ||
            ContainsTexture1DType(node.TypeInfo?.DesugaredName))
        {
            return true;
        }

        return node.Children.Any(IsTexture1DReceiver);
    }

    private static bool ContainsTexture1DType(string? type)
    {
        return type is not null &&
            type.Contains("Texture1D", StringComparison.Ordinal);
    }

    protected override string DefaultStructValue(CppslStruct structure, CppslSemanticModel model)
    {
        return $"{structure.Name}{{}}";
    }

    protected override string DefaultAggregateValue(string mappedType)
    {
        return $"{mappedType}(0)";
    }

    private static string FieldAttribute(CppslField field, StructRole role, ShaderStage stage)
    {
        if (field.IsPosition)
        {
            return " [[position]]";
        }

        if (field.Location is not { } location)
        {
            return string.Empty;
        }

        if (role == StructRole.StageOutput && (stage == ShaderStage.Fragment || stage == ShaderStage.Pixel))
        {
            return $" [[color({location})]]";
        }

        return role == StructRole.StageInput && stage == ShaderStage.Vertex
            ? $" [[attribute({location})]]"
            : $" [[user(locn{location})]]";
    }

    private static string StagePrefix(CppslCompileOptions options)
    {
        return options.Stage switch
        {
            ShaderStage.Vertex => "vertex ",
            ShaderStage.Fragment or ShaderStage.Pixel => "fragment ",
            ShaderStage.Compute => "kernel ",
            _ => string.Empty
        };
    }

    private static string DescriptorSetStructName(int descriptorSet)
    {
        return $"spvDescriptorSetBuffer{descriptorSet}";
    }

    private static string DescriptorSetParameterName(int descriptorSet)
    {
        return $"spvDescriptorSet{descriptorSet}";
    }

    private static string MetalTextureType(string type, bool writable)
    {
        var shape = type switch
        {
            _ when type.StartsWith("Texture1D<", StringComparison.Ordinal) ||
                   type.StartsWith("RWTexture1D<", StringComparison.Ordinal) => "texture1d",
            _ when type.StartsWith("Texture3D<", StringComparison.Ordinal) ||
                   type.StartsWith("RWTexture3D<", StringComparison.Ordinal) => "texture3d",
            _ => "texture2d"
        };

        return writable
            ? $"{shape}<float, access::write>"
            : $"{shape}<float>";
    }

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
