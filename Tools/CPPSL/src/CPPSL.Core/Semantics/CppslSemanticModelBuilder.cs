using CPPSL.Core.Compiler;
using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed class CppslSemanticModelBuilder
{
    public CppslSemanticModel Build(CppslCompileOptions options, string sourcePath, IReadOnlyList<ClangAstNode> astNodes)
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

    private static CppslStruct ToStruct(ClangAstNode node)
    {
        var fields = node.Children
            .Where(static child => child.Kind == "CXCursor_FieldDecl")
            .Select(static child => new CppslField(
                child.Spelling,
                child.Type ?? string.Empty,
                child.File,
                child.Line,
                child.Column))
            .ToArray();

        return new CppslStruct(
            node.Spelling,
            node.File,
            node.Line,
            node.Column,
            fields);
    }

    private static CppslGlobal ToGlobal(ClangAstNode node)
    {
        var type = node.Type ?? string.Empty;
        return new CppslGlobal(
            node.Spelling,
            type,
            ClassifyResourceKind(type),
            node.File,
            node.Line,
            node.Column);
    }

    private static CppslFunction ToFunction(ClangAstNode node, string entryPoint, string stage)
    {
        var parameters = node.Children
            .Where(static child => child.Kind == "CXCursor_ParmDecl")
            .Select(static child => new CppslParameter(
                child.Spelling,
                child.Type ?? string.Empty,
                child.File,
                child.Line,
                child.Column))
            .ToArray();

        var isEntryPoint = node.Spelling == entryPoint;
        return new CppslFunction(
            node.Spelling,
            node.DisplayName,
            node.ResultType,
            parameters,
            isEntryPoint,
            isEntryPoint ? stage : null,
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
}
