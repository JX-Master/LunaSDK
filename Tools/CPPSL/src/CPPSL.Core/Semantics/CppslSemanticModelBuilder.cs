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
            .Where(node => IsUserSourceNode(sourcePath, node))
            .ToArray();

        var structs = sourceTopLevelNodes
            .Where(static node => node.Kind == CppslAstNodeKind.Struct)
            .Select(ToStruct)
            .ToArray();

        var rawGlobals = sourceTopLevelNodes
            .Where(static node => node.Kind == CppslAstNodeKind.GlobalVariable)
            .Select(ToGlobal)
            .ToArray();
        var globals = ExpandDescriptorSetGlobals(rawGlobals, structs).ToArray();

        var functions = sourceTopLevelNodes
            .Where(static node => node.Kind == CppslAstNodeKind.Function)
            .Select(node => ToFunction(node, options.EntryPoint, options.Stage.ToString()))
            .ToArray();

        return new CppslSemanticModel(structs, globals, functions);
    }

    private static bool IsUserSourceNode(string sourcePath, CppslAstNode node)
    {
        if (node.Location?.File is null)
        {
            return false;
        }

        var file = Path.GetFullPath(node.Location.File);
        if (file == sourcePath)
        {
            return true;
        }

        var normalized = file.Replace('\\', '/');
        if (normalized.Contains("/Tools/CPPSL/std/", StringComparison.Ordinal))
        {
            return false;
        }

        return Path.GetExtension(file).Equals(".hxx", StringComparison.OrdinalIgnoreCase);
    }

    private CppslStruct ToStruct(CppslAstNode node)
    {
        var fields = node.Children
            .Where(static child => child.Kind == CppslAstNodeKind.Field)
            .Select(ToField)
            .ToArray();

        return new CppslStruct(
            node.Spelling,
            node.Location?.File,
            node.Location?.Line,
            node.Location?.Column,
            _attributeParser.GetAttributes(node),
            fields);
    }

    private CppslField ToField(CppslAstNode node)
    {
        var attributes = _attributeParser.GetAttributes(node);
        return new CppslField(
            node.Spelling,
            node.TypeName ?? string.Empty,
            attributes,
            attributes.FindAttribute("location").FirstIntArgument(),
            attributes.FindAttribute("position") is not null,
            node.Location?.File,
            node.Location?.Line,
            node.Location?.Column);
    }

    private CppslGlobal ToGlobal(CppslAstNode node)
    {
        var type = node.TypeName ?? string.Empty;
        var attributes = _attributeParser.GetAttributes(node);
        var legacyResourceKind = CppslResourceClassifier.ClassifyGlobal(type, attributes);
        var descriptorSet = attributes.FindAttribute("desc_set").FirstIntArgument();
        var isDescriptorSet = descriptorSet is not null && legacyResourceKind is null;
        return new CppslGlobal(
            node.Spelling,
            type,
            null,
            attributes,
            descriptorSet,
            attributes.FindAttribute("binding").FirstIntArgument(),
            node.Location?.File,
            node.Location?.Line,
            node.Location?.Column,
            node.Spelling,
            isDescriptorSet ? type : null,
            isDescriptorSet);
    }

    private static IEnumerable<CppslGlobal> ExpandDescriptorSetGlobals(
        IReadOnlyList<CppslGlobal> rawGlobals,
        IReadOnlyList<CppslStruct> structs)
    {
        foreach (var global in rawGlobals)
        {
            yield return global;
            if (!global.IsDescriptorSet || global.DescriptorSet is null)
            {
                continue;
            }

            var layout = structs.FirstOrDefault(candidate => candidate.Name == global.Type);
            if (layout is null)
            {
                continue;
            }

            foreach (var field in layout.Fields)
            {
                var resourceKind = CppslResourceClassifier.ClassifyDescriptorSetField(field.Type, field.Attributes);
                if (resourceKind is null)
                {
                    continue;
                }

                yield return new CppslGlobal(
                    $"{global.Name}_{field.Name}",
                    field.Type,
                    resourceKind,
                    field.Attributes,
                    global.DescriptorSet,
                    field.Attributes.FindAttribute("binding").FirstIntArgument(),
                    field.File,
                    field.Line,
                    field.Column,
                    $"{global.Name}.{field.Name}",
                    global.Type,
                    false);
            }
        }
    }

    private CppslFunction ToFunction(CppslAstNode node, string entryPoint, string stage)
    {
        var attributes = _attributeParser.GetAttributes(node);
        var parameters = node.Children
            .Where(static child => child.Kind == CppslAstNodeKind.Parameter)
            .Select(ToParameter)
            .ToArray();

        var isEntryPoint = node.Spelling == entryPoint;
        return new CppslFunction(
            node.Spelling,
            node.DisplayName,
            node.ResultTypeName,
            parameters,
            attributes,
            isEntryPoint,
            isEntryPoint ? stage : null,
            FindDeclaredStage(attributes),
            node.Location?.File,
            node.Location?.Line,
            node.Location?.Column);
    }

    private CppslParameter ToParameter(CppslAstNode node)
    {
        return new CppslParameter(
            node.Spelling,
            node.TypeName ?? string.Empty,
            _attributeParser.GetAttributes(node, includeLeadingLines: false),
            node.Location?.File,
            node.Location?.Line,
            node.Location?.Column);
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
