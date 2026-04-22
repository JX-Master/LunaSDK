using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.IR;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class MslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslIrModule irModule)
    {
        var builder = new StringBuilder();
        builder.AppendLine("#include <metal_stdlib>");
        builder.AppendLine("using namespace metal;");
        builder.AppendLine();
        WriteHeader(builder, "MSL", options);
        var entryPoint = FindEntryPoint(options, model);
        WriteStructs(builder, model, entryPoint);
        WriteEntryPoint(builder, options, entryPoint, model, irModule);
        return builder.ToString();
    }

    private void WriteStructs(StringBuilder builder, CppslSemanticModel model, CppslFunction? entryPoint)
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
                var role = structure.Name == outputStructName
                    ? StructRole.StageOutput
                    : inputStructNames.Contains(structure.Name) ? StructRole.StageInput : StructRole.Plain;
                builder.Append("    ");
                builder.Append(MapValueType(field.Type));
                builder.Append(' ');
                builder.Append(field.Name);
                builder.Append(FieldAttribute(field, role));
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
        WriteFunctionBody(builder, entryPoint, model, irModule);
    }

    private IEnumerable<string> BuildEntryParameters(CppslFunction entryPoint, CppslSemanticModel model)
    {
        foreach (var parameter in entryPoint.Parameters)
        {
            yield return $"{MapValueType(parameter.Type)} {parameter.Name} [[stage_in]]";
        }

        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            yield return BuildResourceParameter(global);
        }
    }

    private string BuildResourceParameter(CppslGlobal global)
    {
        return global.ResourceKind switch
        {
            "constant_buffer" => $"constant {MapValueType(UnwrapTemplateArgument(global.Type))}& {global.Name} [[buffer({global.Binding})]]",
            "structured_buffer" => $"device const {MapValueType(UnwrapTemplateArgument(global.Type))}* {global.Name} [[buffer({global.Binding})]]",
            "rw_structured_buffer" => $"device {MapValueType(UnwrapTemplateArgument(global.Type))}* {global.Name} [[buffer({global.Binding})]]",
            "texture" => $"texture2d<float> {global.Name} [[texture({global.Binding})]]",
            "rw_texture" => $"texture2d<float, access::write> {global.Name} [[texture({global.Binding})]]",
            "sampler" => $"sampler {global.Name} [[sampler({global.Binding})]]",
            _ => $"{global.Type} {global.Name}"
        };
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
        return $"{structure.Name}{{}}";
    }

    protected override string DefaultAggregateValue(string mappedType)
    {
        return $"{mappedType}(0)";
    }

    private static string FieldAttribute(CppslField field, StructRole role)
    {
        if (field.IsPosition)
        {
            return " [[position]]";
        }

        if (field.Location is not { } location)
        {
            return string.Empty;
        }

        return role == StructRole.StageInput ? $" [[attribute({location})]]" : $" [[user(locn{location})]]";
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

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
