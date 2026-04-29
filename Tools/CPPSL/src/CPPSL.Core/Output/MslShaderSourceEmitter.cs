using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class MslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByName = new(StringComparer.Ordinal);

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#include <metal_stdlib>");
        builder.AppendLine("using namespace metal;");
        builder.AppendLine();
        WriteHeader(builder, "MSL", options);
        var entryPoint = FindEntryPoint(options, model);
        WriteStructs(builder, options, model, shaderModel, entryPoint);
        WriteArgumentBufferStructs(builder, model);
        WriteGlobalArgumentBuffers(builder, model);
        _resourceAccessByName = BuildResourceAccessMap(
            model,
            static global => global.ResourceKind is not null && global.DescriptorSet is not null,
            static global => MslResourceAccess(global));
        WriteFunctions(builder, entryPoint, model, shaderModel);
        WriteEntryPoint(builder, options, entryPoint, model, shaderModel);
        _resourceAccessByName = new Dictionary<string, string>(StringComparer.Ordinal);
        return builder.ToString();
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

    private void WriteStructs(
        StringBuilder builder,
        CppslCompileOptions options,
        CppslSemanticModel model,
        CppslShaderModel shaderModel,
        CppslFunction? entryPoint)
    {
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        var inputStructNames = entryPoint?.Parameters.Select(static parameter => parameter.Type).ToHashSet(StringComparer.Ordinal) ??
            new HashSet<string>(StringComparer.Ordinal);
        var outputStructName = entryPoint?.ReturnType;
        var packedStructs = StructuredBufferElementStructNames(model);
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
                builder.Append(packedStructs.Contains(structure.Name)
                    ? MapStructuredBufferFieldType(field.Type)
                    : MapValueType(field.Type));
                builder.Append(' ');
                builder.Append(field.Name);
                builder.Append(FieldAttribute(field, role, options.Stage));
                builder.AppendLine(";");
            }
            foreach (var method in structure.Methods)
            {
                var shaderModelMethod = shaderModel.Structs
                    .FirstOrDefault(candidate => candidate.Name == structure.Name)?
                    .Methods
                    .FirstOrDefault(candidate => candidate.Name == method.Name);
                if (shaderModelMethod?.Body is null)
                {
                    continue;
                }

                builder.Append("    ");
                builder.Append(MapValueType(method.ReturnType ?? "void"));
                builder.Append(' ');
                builder.Append(method.Name);
                builder.Append('(');
                builder.Append(string.Join(", ", method.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
                builder.AppendLine(")");
                WriteIndentedMethodBody(builder, method, model, shaderModelMethod.Body, 1);
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
        CppslShaderModel shaderModel)
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
        WriteEntryFunctionBody(builder, entryPoint, model, shaderModel);
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

    }

    private void WriteArgumentBufferStructs(
        StringBuilder builder,
        CppslSemanticModel model)
    {
        foreach (var descriptorSetGroup in model.Globals
            .Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null)
            .GroupBy(static global => global.DescriptorSet!.Value)
            .OrderBy(static group => group.Key))
        {
            var orderedGlobals = descriptorSetGroup
                .OrderBy(static global => global.Binding ?? 0)
                .ThenBy(static global => global.Name, StringComparer.Ordinal)
                .ToArray();
            builder.AppendLine($"struct {DescriptorSetStructName(descriptorSetGroup.Key)}");
            builder.AppendLine("{");
            for (var metalArgumentIndex = 0; metalArgumentIndex < orderedGlobals.Length; ++metalArgumentIndex)
            {
                var global = orderedGlobals[metalArgumentIndex];
                builder.Append("    ");
                builder.Append(ArgumentBufferField(global, metalArgumentIndex));
                builder.AppendLine(";");
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private void WriteGlobalArgumentBuffers(
        StringBuilder builder,
        CppslSemanticModel model)
    {
        foreach (var descriptorSet in model.Globals
            .Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null)
            .Select(static global => global.DescriptorSet!.Value)
            .Distinct()
            .Order())
        {
            builder.AppendLine($"constant {DescriptorSetStructName(descriptorSet)}* constant {DescriptorSetParameterName(descriptorSet)} [[buffer({descriptorSet})]];");
        }

        if (model.Globals.Any(static global => global.ResourceKind is not null && global.DescriptorSet is not null))
        {
            builder.AppendLine();
        }
    }

    private void WriteEntryFunctionBody(
        StringBuilder builder,
        CppslFunction entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        builder.AppendLine("{");
        foreach (var global in model.Globals.Where(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine($"    threadgroup {FormatVariableDeclaration(MapValueType(global.Type), global.Name)};");
        }

        var shaderModelEntryPoint = shaderModel.EntryPoints.FirstOrDefault(entry => entry.Name == entryPoint.Name);
        if (shaderModelEntryPoint?.Body is null)
        {
            WriteDefaultReturn(builder, entryPoint.ReturnType ?? "void", model, 1);
        }
        else
        {
            WriteStatementChildren(builder, shaderModelEntryPoint.Body, 1);
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

    private static HashSet<string> StructuredBufferElementStructNames(CppslSemanticModel model)
    {
        return model.Globals
            .Where(static global => global.ResourceKind is "structured_buffer" or "rw_structured_buffer")
            .Select(static global => NormalizeShaderTypeName(ResourceElementType(global)))
            .ToHashSet(StringComparer.Ordinal);
    }

    private string MapStructuredBufferFieldType(string type)
    {
        return NormalizeShaderTypeName(type) switch
        {
            "float3" => "packed_float3",
            _ => MapValueType(type)
        };
    }

    protected override string LowerExpression(CppslShaderModelNode node)
    {
        if (TryGetMappedResourceAccess(node, _resourceAccessByName, out var resourceAccess))
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

    protected override string LowerCallExpression(CppslShaderModelNode node)
    {
        if (TryGetSwizzleConversionOperand(node, out var swizzleOperand))
        {
            return LowerExpression(swizzleOperand);
        }

        if (TryGetMemberCall(node, out var receiverNode, out var receiver, out var memberName, out var memberArguments))
        {
            if (memberName == "Sample" && memberArguments.Count == 2)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]})",
                    preserveDepthTextureScalar: true);
            }
            if (memberName == "SampleLevel" && memberArguments.Count == 3)
            {
                var expression = IsTexture1DReceiver(receiverNode)
                    ? $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]})"
                    : $"{receiver}.sample({memberArguments[0]}, {memberArguments[1]}, level({memberArguments[2]}))";
                return AdaptTextureReadExpression(receiverNode, expression, preserveDepthTextureScalar: true);
            }
            if (memberName == "Load" && memberArguments.Count == 1)
            {
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"{receiver}.read({memberArguments[0]})",
                    preserveDepthTextureScalar: true);
            }
            if (memberName == "Store" && memberArguments.Count == 2)
            {
                return $"{receiver}.write({AdaptTextureStoreValue(receiverNode, memberArguments[1], "float4", "int4", "uint4", "0.0f")}, {memberArguments[0]})";
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
            var addressSpace = AtomicTargetAddressSpace(arguments[0]);
            return $"atomic_fetch_add_explicit(({addressSpace} atomic_uint*)&{arguments[0]}, {arguments[1]}, memory_order_relaxed)";
        }
        if (name == "lerp" && arguments.Length == 3)
        {
            return $"mix({arguments[0]}, {arguments[1]}, {arguments[2]})";
        }
        return base.LowerCallExpression(node);
    }

    private static string AtomicTargetAddressSpace(string target)
    {
        return target.Contains("spvDescriptorSet", StringComparison.Ordinal)
            ? "device"
            : "threadgroup";
    }

    private static string MslResourceAccess(CppslGlobal global)
    {
        var access = $"(*{DescriptorSetParameterName(global.DescriptorSet!.Value)}).{global.Name}";
        return global.ResourceKind == "constant_buffer"
            ? $"(*{access})"
            : access;
    }

    private static bool IsTexture1DReceiver(CppslShaderModelNode node)
    {
        return TryGetTextureInfo(node, out var textureInfo) && textureInfo.Dimension == 1;
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
            _ when type.StartsWith("DepthTexture2D<", StringComparison.Ordinal) => "depth2d",
            _ when type.StartsWith("Texture3D<", StringComparison.Ordinal) ||
                   type.StartsWith("RWTexture3D<", StringComparison.Ordinal) => "texture3d",
            _ => "texture2d"
        };

        return writable
            ? $"{shape}<float, access::read_write>"
            : $"{shape}<float>";
    }

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
