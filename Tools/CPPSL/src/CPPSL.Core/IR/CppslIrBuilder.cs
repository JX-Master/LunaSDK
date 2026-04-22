using CPPSL.Core.Compiler;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.IR;

public sealed class CppslIrBuilder
{
    public const string SchemaName = "cppsl.ir";
    public const int SchemaVersion = 0;

    public CppslIrModule Build(CppslCompileOptions options, string sourcePath, CppslSemanticModel semanticModel)
    {
        return new CppslIrModule(
            SchemaName,
            SchemaVersion,
            sourcePath,
            semanticModel.Structs.Select(static structure => new CppslIrStruct(
                structure.Name,
                structure.Fields.Select(static field => new CppslIrField(
                    field.Name,
                    field.Type,
                    field.Location,
                    field.IsPosition)).ToArray())).ToArray(),
            semanticModel.Globals
                .Where(static global => global.ResourceKind is not null)
                .Select(static global => new CppslIrResource(
                    global.Name,
                    global.Type,
                    global.ResourceKind!,
                    global.DescriptorSet!.Value,
                    global.Binding!.Value)).ToArray(),
            semanticModel.Functions
                .Where(static function => function.IsEntryPoint)
                .Select(function => new CppslIrEntryPoint(
                    function.Name,
                    options.Stage.ToString(),
                    function.DeclaredStage,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslIrParameter(parameter.Name, parameter.Type)).ToArray())).ToArray());
    }
}

public sealed record CppslIrModule(
    string Schema,
    int Version,
    string Source,
    IReadOnlyList<CppslIrStruct> Structs,
    IReadOnlyList<CppslIrResource> Resources,
    IReadOnlyList<CppslIrEntryPoint> EntryPoints);

public sealed record CppslIrStruct(
    string Name,
    IReadOnlyList<CppslIrField> Fields);

public sealed record CppslIrField(
    string Name,
    string Type,
    int? Location,
    bool IsPosition);

public sealed record CppslIrResource(
    string Name,
    string Type,
    string ResourceKind,
    int Set,
    int Binding);

public sealed record CppslIrEntryPoint(
    string Name,
    string Stage,
    string? DeclaredStage,
    string? ReturnType,
    IReadOnlyList<CppslIrParameter> Parameters);

public sealed record CppslIrParameter(
    string Name,
    string Type);
