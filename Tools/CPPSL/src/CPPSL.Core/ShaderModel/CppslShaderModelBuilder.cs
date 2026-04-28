using CPPSL.Core.Compiler;
using CPPSL.Core.Frontend;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.ShaderModel;

public sealed class CppslShaderModelBuilder
{
    public const string SchemaName = "cppsl.shader_model";
    public const int SchemaVersion = 1;
    private readonly CppslShaderModelBodyLowerer _bodyLowerer = new();

    public CppslShaderModel Build(
        CppslCompileOptions options,
        string sourcePath,
        CppslSemanticModel semanticModel,
        IReadOnlyList<CppslAstNode> astNodes)
    {
        sourcePath = Path.GetFullPath(sourcePath);
        return new CppslShaderModel(
            SchemaName,
            SchemaVersion,
            sourcePath,
            semanticModel.Structs.Select(static structure => new CppslShaderModelStruct(
                structure.Name,
                structure.Fields.Select(static field => new CppslShaderModelField(
                    field.Name,
                    field.Type,
                    field.Location,
                    field.IsPosition)).ToArray())).ToArray(),
            semanticModel.Globals
                .Where(static global => global.ResourceKind is not null)
                .Select(static global => new CppslShaderModelResource(
                    global.Name,
                    global.Type,
                    global.ResourceKind!,
                    global.DescriptorSet!.Value,
                    global.Binding!.Value)).ToArray(),
            semanticModel.Functions
                .Select(function => new CppslShaderModelFunction(
                    function.Name,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslShaderModelParameter(parameter.Name, parameter.Type)).ToArray(),
                    _bodyLowerer.LowerFunctionBody(astNodes, function.Name))).ToArray(),
            semanticModel.Functions
                .Where(static function => function.IsEntryPoint)
                .Select(function => new CppslShaderModelEntryPoint(
                    function.Name,
                    options.Stage.ToString(),
                    function.DeclaredStage,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslShaderModelParameter(parameter.Name, parameter.Type)).ToArray(),
                    _bodyLowerer.LowerFunctionBody(astNodes, function.Name))).ToArray());
    }
}

public sealed record CppslShaderModel(
    string Schema,
    int Version,
    string Source,
    IReadOnlyList<CppslShaderModelStruct> Structs,
    IReadOnlyList<CppslShaderModelResource> Resources,
    IReadOnlyList<CppslShaderModelFunction> Functions,
    IReadOnlyList<CppslShaderModelEntryPoint> EntryPoints);

public sealed record CppslShaderModelStruct(
    string Name,
    IReadOnlyList<CppslShaderModelField> Fields);

public sealed record CppslShaderModelField(
    string Name,
    string Type,
    int? Location,
    bool IsPosition);

public sealed record CppslShaderModelResource(
    string Name,
    string Type,
    string ResourceKind,
    int Set,
    int Binding);

public sealed record CppslShaderModelEntryPoint(
    string Name,
    string Stage,
    string? DeclaredStage,
    string? ReturnType,
    IReadOnlyList<CppslShaderModelParameter> Parameters,
    CppslShaderModelNode? Body);

public sealed record CppslShaderModelFunction(
    string Name,
    string? ReturnType,
    IReadOnlyList<CppslShaderModelParameter> Parameters,
    CppslShaderModelNode? Body);

public sealed record CppslShaderModelParameter(
    string Name,
    string Type);

public sealed record CppslShaderModelNode(
    CppslShaderModelNodeKind Kind,
    string Spelling,
    string? DisplayName,
    string? Type,
    CppslShaderModelType? TypeInfo,
    IReadOnlyList<CppslShaderModelNode> Children);

public sealed record CppslShaderModelType(
    string Spelling,
    string CanonicalName,
    string DesugaredName,
    IReadOnlyList<CppslShaderModelType> TemplateArguments);
