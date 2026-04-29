using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class HlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByPath = new(StringComparer.Ordinal);

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        var builder = new StringBuilder();
        WriteHeader(builder, "HLSL", options);
        builder.AppendLine("#pragma pack_matrix(column_major)");
        builder.AppendLine();
        var entryPoint = FindEntryPoint(options, model);
        _resourceAccessByPath = BuildResourceAccessMap(
            model,
            static global => global.ResourceKind is not null && global.AccessPath is not null,
            static global => global.Name);
        WriteStructs(builder, options, model, shaderModel, entryPoint);
        WriteGroupSharedGlobals(builder, model);
        WriteResources(builder, model);
        WriteFunctions(builder, entryPoint, model, shaderModel);
        WriteEntryPoint(builder, entryPoint, model, shaderModel);
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

    private void WriteStructs(
        StringBuilder builder,
        CppslCompileOptions options,
        CppslSemanticModel model,
        CppslShaderModel shaderModel,
        CppslFunction? entryPoint)
    {
        var inputStructNames = entryPoint?.Parameters.Select(static parameter => parameter.Type).ToHashSet(StringComparer.Ordinal) ??
            new HashSet<string>(StringComparer.Ordinal);
        var outputStructName = entryPoint?.ReturnType;
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        foreach (var structure in model.Structs.Where(structure => !descriptorSetLayouts.Contains(structure.Name)))
        {
            builder.AppendLine($"struct {structure.Name}");
            builder.AppendLine("{");
            foreach (var field in structure.Fields)
            {
                builder.Append("    ");
                builder.Append(MapValueType(field.Type));
                builder.Append(' ');
                builder.Append(field.Name);
                var role = structure.Name == outputStructName
                    ? StructRole.StageOutput
                    : inputStructNames.Contains(structure.Name) ? StructRole.StageInput : StructRole.Plain;
                builder.Append(FieldSemantic(field, role, options.Stage));
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

    private static void WriteResources(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            var declaration = global.ResourceKind switch
            {
                "constant_buffer" => $"ConstantBuffer<{ResourceElementType(global)}> {global.Name}",
                "structured_buffer" => $"StructuredBuffer<{ResourceElementType(global)}> {global.Name}",
                "rw_structured_buffer" => $"RWStructuredBuffer<{ResourceElementType(global)}> {global.Name}",
                "texture" => $"{MapResourceType(global.Type)} {global.Name}",
                "rw_texture" => $"{global.Type} {global.Name}",
                "sampler" => $"SamplerState {global.Name}",
                _ => $"{global.Type} {global.Name}"
            };

            builder.AppendLine($"{declaration} : register({RegisterPrefix(global.ResourceKind)}{global.Binding}, space{global.DescriptorSet});");
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
            builder.AppendLine($"groupshared {FormatVariableDeclaration(MapValueType(global.Type), global.Name)};");
        }

        if (model.Globals.Any(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine();
        }
    }

    private void WriteEntryPoint(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        if (entryPoint is null)
        {
            return;
        }

        if (entryPoint.Attributes.FindAttribute("compute") is { Arguments.Count: >= 3 } compute)
        {
            builder.AppendLine($"[numthreads({compute.Arguments[0]}, {compute.Arguments[1]}, {compute.Arguments[2]})]");
        }
        builder.Append(MapValueType(entryPoint.ReturnType ?? "void"));
        builder.Append(' ');
        builder.Append(entryPoint.Name);
        builder.Append('(');
        builder.Append(string.Join(", ", entryPoint.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}{ParameterSemantic(parameter)}")));
        builder.AppendLine(")");
        WriteFunctionBody(builder, entryPoint, model, shaderModel);
    }

    protected override string LowerMul(string left, string right)
    {
        return $"mul({left}, {right})";
    }

    protected override string LowerExpression(CppslShaderModelNode node)
    {
        if (TryGetMappedResourceAccess(node, _resourceAccessByPath, out var resourceAccess))
        {
            return resourceAccess;
        }

        return base.LowerExpression(node);
    }

    protected override string LowerMemberCall(string receiver, string memberName, IReadOnlyList<string> arguments)
    {
        if (memberName == "Load" && arguments.Count == 1)
        {
            return $"{receiver}.Load(int3({arguments[0]}, 0))";
        }
        if (memberName == "Store" && arguments.Count == 2)
        {
            return $"{receiver}[{arguments[0]}] = {arguments[1]}";
        }

        return base.LowerMemberCall(receiver, memberName, arguments);
    }

    protected override string LowerCallExpression(CppslShaderModelNode node)
    {
        var name = node.DisplayName ?? node.Spelling;
        if (name == "discard_fragment")
        {
            return "clip(-1.0)";
        }

        return base.LowerCallExpression(node);
    }

    protected override string MapValueType(string type)
    {
        return type switch
        {
            "_Bool" => "bool",
            "bool_t" => "bool",
            _ when type.StartsWith("DepthTexture2D<", StringComparison.Ordinal) => $"Texture2D<{UnwrapTemplateArgument(type)}>",
            _ => type
        };
    }

    private static string MapResourceType(string type)
    {
        return type.StartsWith("DepthTexture2D<", StringComparison.Ordinal)
            ? $"Texture2D<{UnwrapTemplateArgument(type)}>"
            : type;
    }

    protected override string DefaultStructValue(CppslStruct structure, CppslSemanticModel model)
    {
        return $"({structure.Name})0";
    }

    protected override string DefaultAggregateValue(string mappedType)
    {
        return $"({mappedType})0";
    }

    private static string FieldSemantic(CppslField field, StructRole role, ShaderStage stage)
    {
        if (field.IsPosition)
        {
            return " : SV_Position";
        }

        if (field.Location is not { } location)
        {
            return string.Empty;
        }

        if (role == StructRole.StageOutput && (stage == ShaderStage.Fragment || stage == ShaderStage.Pixel))
        {
            return location == 0 ? " : SV_Target" : $" : SV_Target{location}";
        }

        if (role == StructRole.StageInput && stage == ShaderStage.Vertex)
        {
            return $" : TEXCOORD{location}";
        }

        // CPPSL stage varyings use location(1) for the first user value because
        // the raster position is represented by cppsl::position. HLSL stage
        // linkage, however, expects the user registers to be packed from
        // TEXCOORD0, especially when the pixel shader omits SV_Position.
        if (role is StructRole.StageInput or StructRole.StageOutput)
        {
            return $" : TEXCOORD{Math.Max(location - 1, 0)}";
        }

        return $" : TEXCOORD{location}";
    }

    private static string RegisterPrefix(string? resourceKind)
    {
        return resourceKind switch
        {
            "constant_buffer" => "b",
            "rw_structured_buffer" or "rw_texture" => "u",
            "sampler" => "s",
            _ => "t"
        };
    }

    private static string ParameterSemantic(CppslParameter parameter)
    {
        return parameter.Attributes.FindAttribute("builtin")?.Arguments.FirstOrDefault() switch
        {
            "dispatch_thread_id" => " : SV_DispatchThreadID",
            "group_index" => " : SV_GroupIndex",
            _ => string.Empty
        };
    }

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
