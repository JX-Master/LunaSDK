using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class GlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#version 450");
        WriteHeader(builder, "GLSL", options);
        var entryPoint = FindEntryPoint(options, model);
        WriteStructs(builder, model);
        WriteResources(builder, model);
        WriteStageIo(builder, entryPoint, model);
        WriteEntryPointHelper(builder, entryPoint, model, irModule);
        WriteMain(builder, entryPoint, model);
        return builder.ToString();
    }

    private void WriteStructs(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var structure in model.Structs)
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
                    builder.AppendLine($"    {MapValueType(UnwrapTemplateArgument(global.Type))} {global.Name}[];");
                    builder.AppendLine("};");
                    break;
            }
        }

        if (model.Globals.Any(static global => global.ResourceKind is not null))
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
        var elementType = UnwrapTemplateArgument(global.Type);
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

    protected override string MapValueType(string type)
    {
        return type switch
        {
            "float2" => "vec2",
            "float3" => "vec3",
            "float4" => "vec4",
            "float4x4" => "mat4",
            "uint" => "uint",
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
}
