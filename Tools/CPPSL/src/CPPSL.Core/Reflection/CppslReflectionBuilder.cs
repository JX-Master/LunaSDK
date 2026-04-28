using CPPSL.Core.Compiler;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Reflection;

public sealed class CppslReflectionBuilder
{
    public const string SchemaName = "cppsl.reflection";
    public const int SchemaVersion = 0;

    public CppslReflectionModel Build(CppslCompileOptions options, string sourcePath, CppslSemanticModel semanticModel)
    {
        var entryPoint = semanticModel.Functions.FirstOrDefault(function => function.IsEntryPoint);
        var descriptors = semanticModel.Globals
            .Where(static global => global.ResourceKind is not null)
            .Select(static global => new CppslDescriptorReflection(
                global.Name,
                global.Type,
                global.ResourceKind!,
                global.DescriptorSet!.Value,
                global.Binding!.Value))
            .OrderBy(static descriptor => descriptor.Set)
            .ThenBy(static descriptor => descriptor.Binding)
            .ThenBy(static descriptor => descriptor.Name, StringComparer.Ordinal)
            .ToArray();

        var stageInputs = entryPoint?.Parameters
            .Select(parameter => FindStruct(semanticModel, parameter.Type))
            .Where(static structure => structure is not null)
            .Cast<CppslStruct>()
            .SelectMany(static structure => structure.Fields.Select(field => ToStageIo(structure.Name, field)))
            .ToArray() ?? Array.Empty<CppslStageIoReflection>();

        var stageOutputs = entryPoint is null
            ? Array.Empty<CppslStageIoReflection>()
            : FindStruct(semanticModel, entryPoint.ReturnType)?.Fields
                .Select(field => ToStageIo(entryPoint.ReturnType!, field))
                .ToArray() ?? Array.Empty<CppslStageIoReflection>();

        return new CppslReflectionModel(
            SchemaName,
            SchemaVersion,
            sourcePath,
            options.EntryPoint,
            options.Stage.ToString(),
            entryPoint is null ? null : new CppslEntryPointReflection(
                entryPoint.Name,
                entryPoint.DisplayName,
                entryPoint.ReturnType,
                entryPoint.DeclaredStage,
                entryPoint.Parameters.Select(static parameter => new CppslParameterReflection(parameter.Name, parameter.Type)).ToArray(),
                WorkgroupSize(entryPoint)),
            descriptors,
            stageInputs,
            stageOutputs,
            Array.Empty<string>());
    }

    private static CppslStruct? FindStruct(CppslSemanticModel semanticModel, string? type)
    {
        if (string.IsNullOrWhiteSpace(type))
        {
            return null;
        }

        return semanticModel.Structs.FirstOrDefault(structure => structure.Name == type);
    }

    private static CppslStageIoReflection ToStageIo(string structureName, CppslField field)
    {
        return new CppslStageIoReflection(
            structureName,
            field.Name,
            field.Type,
            field.Location,
            field.IsPosition);
    }

    private static IReadOnlyList<int>? WorkgroupSize(CppslFunction entryPoint)
    {
        var compute = entryPoint.Attributes.FindAttribute("compute");
        if (compute is null || compute.Arguments.Count < 3)
        {
            return null;
        }

        var values = compute.Arguments.Take(3)
            .Select(static argument => int.TryParse(argument, out var value) ? value : 0)
            .ToArray();
        return values.All(static value => value > 0) ? values : null;
    }
}

public sealed record CppslReflectionModel(
    string Schema,
    int Version,
    string Source,
    string EntryPoint,
    string Stage,
    CppslEntryPointReflection? EntryPointInfo,
    IReadOnlyList<CppslDescriptorReflection> Descriptors,
    IReadOnlyList<CppslStageIoReflection> StageInputs,
    IReadOnlyList<CppslStageIoReflection> StageOutputs,
    IReadOnlyList<string> Features);

public sealed record CppslEntryPointReflection(
    string Name,
    string? DisplayName,
    string? ReturnType,
    string? DeclaredStage,
    IReadOnlyList<CppslParameterReflection> Parameters,
    IReadOnlyList<int>? WorkgroupSize);

public sealed record CppslParameterReflection(
    string Name,
    string Type);

public sealed record CppslDescriptorReflection(
    string Name,
    string Type,
    string ResourceKind,
    int Set,
    int Binding);

public sealed record CppslStageIoReflection(
    string Struct,
    string Name,
    string Type,
    int? Location,
    bool IsPosition);
