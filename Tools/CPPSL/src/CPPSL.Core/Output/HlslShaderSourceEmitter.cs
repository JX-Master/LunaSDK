using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class HlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        var builder = new StringBuilder();
        WriteHeader(builder, "HLSL", options);
        var entryPoint = FindEntryPoint(options, model);
        WriteStructs(builder, options, model, entryPoint);
        WriteResources(builder, model);
        WriteEntryPoint(builder, entryPoint, model, irModule);
        return builder.ToString();
    }

    private void WriteStructs(
        StringBuilder builder,
        CppslCompileOptions options,
        CppslSemanticModel model,
        CppslFunction? entryPoint)
    {
        var inputStructNames = entryPoint?.Parameters.Select(static parameter => parameter.Type).ToHashSet(StringComparer.Ordinal) ??
            new HashSet<string>(StringComparer.Ordinal);
        var outputStructName = entryPoint?.ReturnType;
        foreach (var structure in model.Structs)
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
                "texture" => $"{global.Type} {global.Name}",
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

    private void WriteEntryPoint(
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

    protected override string LowerMul(string left, string right)
    {
        return $"mul({left}, {right})";
    }

    protected override string MapValueType(string type)
    {
        return type switch
        {
            "bool_t" => "bool",
            _ => type
        };
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

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
