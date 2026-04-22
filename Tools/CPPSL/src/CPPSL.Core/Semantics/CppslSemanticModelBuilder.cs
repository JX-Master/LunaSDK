using CPPSL.Core.Compiler;
using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed class CppslSemanticModelBuilder
{
    private readonly CppslAttributeParser _attributeParser = new();

    public CppslSemanticModel Build(CppslCompileOptions options, string sourcePath, IReadOnlyList<CppslAstNode> astNodes)
    {
        sourcePath = Path.GetFullPath(sourcePath);
        var sourceTopLevelNodes = astNodes
            .Where(node => node.File is not null && Path.GetFullPath(node.File) == sourcePath)
            .ToArray();

        var structs = sourceTopLevelNodes
            .Where(static node => node.Kind == "CXCursor_StructDecl")
            .Select(ToStruct)
            .ToArray();

        var globals = sourceTopLevelNodes
            .Where(static node => node.Kind == "CXCursor_VarDecl")
            .Select(ToGlobal)
            .ToArray();

        var functions = sourceTopLevelNodes
            .Where(static node => node.Kind == "CXCursor_FunctionDecl")
            .Select(node => ToFunction(node, options.EntryPoint, options.Stage.ToString()))
            .ToArray();

        return new CppslSemanticModel(structs, globals, functions);
    }

    private CppslStruct ToStruct(CppslAstNode node)
    {
        var fields = node.Children
            .Where(static child => child.Kind == "CXCursor_FieldDecl")
            .Select(ToField)
            .ToArray();

        return new CppslStruct(
            node.Spelling,
            node.File,
            node.Line,
            node.Column,
            _attributeParser.GetAttributes(node),
            fields);
    }

    private CppslField ToField(CppslAstNode node)
    {
        var attributes = _attributeParser.GetAttributes(node);
        return new CppslField(
            node.Spelling,
            node.Type ?? string.Empty,
            attributes,
            attributes.FindAttribute("location").FirstIntArgument(),
            attributes.FindAttribute("position") is not null,
            node.File,
            node.Line,
            node.Column);
    }

    private CppslGlobal ToGlobal(CppslAstNode node)
    {
        var type = node.Type ?? string.Empty;
        var attributes = _attributeParser.GetAttributes(node);
        return new CppslGlobal(
            node.Spelling,
            type,
            ClassifyResourceKind(type),
            attributes,
            attributes.FindAttribute("set").FirstIntArgument(),
            attributes.FindAttribute("binding").FirstIntArgument(),
            node.File,
            node.Line,
            node.Column);
    }

    private CppslFunction ToFunction(CppslAstNode node, string entryPoint, string stage)
    {
        var attributes = _attributeParser.GetAttributes(node);
        var parameters = node.Children
            .Where(static child => child.Kind == "CXCursor_ParmDecl")
            .Select(ToParameter)
            .ToArray();

        var isEntryPoint = node.Spelling == entryPoint;
        return new CppslFunction(
            node.Spelling,
            node.DisplayName,
            node.ResultType,
            parameters,
            attributes,
            isEntryPoint,
            isEntryPoint ? stage : null,
            FindDeclaredStage(attributes),
            node.File,
            node.Line,
            node.Column);
    }

    private CppslParameter ToParameter(CppslAstNode node)
    {
        return new CppslParameter(
            node.Spelling,
            node.Type ?? string.Empty,
            _attributeParser.GetAttributes(node, includeLeadingLines: false),
            node.File,
            node.Line,
            node.Column);
    }

    private static string? ClassifyResourceKind(string type)
    {
        if (type.StartsWith("ConstantBuffer<", StringComparison.Ordinal)) return "constant_buffer";
        if (type.StartsWith("StructuredBuffer<", StringComparison.Ordinal)) return "structured_buffer";
        if (type.StartsWith("RWStructuredBuffer<", StringComparison.Ordinal)) return "rw_structured_buffer";
        if (type.StartsWith("Texture", StringComparison.Ordinal)) return "texture";
        if (type.StartsWith("RWTexture", StringComparison.Ordinal)) return "rw_texture";
        if (type == "SamplerState") return "sampler";
        if (type == "AccelerationStructure") return "acceleration_structure";
        return null;
    }

    private static string? FindDeclaredStage(IReadOnlyList<CppslAttribute> attributes)
    {
        foreach (var stage in new[] { "vertex", "fragment", "pixel", "compute", "raygen", "miss", "closest_hit", "any_hit", "intersection", "callable" })
        {
            if (attributes.FindAttribute(stage) is not null)
            {
                return stage;
            }
        }
        return null;
    }
}
