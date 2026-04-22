namespace CPPSL.Core.Semantics;

public sealed record CppslFunction(
    string Name,
    string? DisplayName,
    string? ReturnType,
    IReadOnlyList<CppslParameter> Parameters,
    bool IsEntryPoint,
    string? Stage,
    string? File,
    int? Line,
    int? Column);
