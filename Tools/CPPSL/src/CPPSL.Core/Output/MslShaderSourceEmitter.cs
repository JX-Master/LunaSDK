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
        WriteEntryPoint(builder, options, entryPoint, model, irModule);
        _resourceAccessByName = new Dictionary<string, string>(StringComparer.Ordinal);
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
        WriteFunctionBody(builder, entryPoint, model, irModule);
    }

    private IEnumerable<string> BuildEntryParameters(CppslFunction entryPoint, CppslSemanticModel model)
    {
        foreach (var parameter in entryPoint.Parameters)
        {
            yield return $"{MapValueType(parameter.Type)} {parameter.Name} [[stage_in]]";
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
            foreach (var global in descriptorSetGroup.OrderBy(static global => global.Binding ?? 0))
            {
                builder.Append("    ");
                builder.Append(ArgumentBufferField(global));
                builder.AppendLine(";");
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private string ArgumentBufferField(CppslGlobal global)
    {
        return global.ResourceKind switch
        {
            "constant_buffer" => $"constant {MapValueType(ResourceElementType(global))}* {global.Name} [[id({global.Binding})]]",
            "structured_buffer" => $"device const {MapValueType(ResourceElementType(global))}* {global.Name} [[id({global.Binding})]]",
            "rw_structured_buffer" => $"device {MapValueType(ResourceElementType(global))}* {global.Name} [[id({global.Binding})]]",
            "texture" => $"texture2d<float> {global.Name} [[id({global.Binding})]]",
            "rw_texture" => $"texture2d<float, access::write> {global.Name} [[id({global.Binding})]]",
            "sampler" => $"sampler {global.Name} [[id({global.Binding})]]",
            _ => $"{global.Type} {global.Name}"
        };
    }

    private static Dictionary<string, string> BuildResourceAccessMap(CppslSemanticModel model)
    {
        var map = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null))
        {
            var descriptorSet = DescriptorSetParameterName(global.DescriptorSet!.Value);
            var access = $"{descriptorSet}.{global.Name}";
            map[global.Name] = global.ResourceKind == "constant_buffer"
                ? $"(*{access})"
                : access;
        }
        return map;
    }

    protected override string LowerExpression(CppslIrNode node)
    {
        if (node.Kind == "DeclRefExpression")
        {
            var name = node.DisplayName ?? node.Spelling;
            if (_resourceAccessByName.TryGetValue(name, out var access))
            {
                return access;
            }
        }

        return base.LowerExpression(node);
    }

    protected override string MapValueType(string type)
    {
        return type switch
        {
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

        return base.LowerMemberCall(receiver, memberName, arguments);
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

    private enum StructRole
    {
        Plain,
        StageInput,
        StageOutput
    }
}
