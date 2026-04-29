namespace CPPSL.Core.Semantics;

public sealed record CppslMethod(
    string? DeclId,
    string OwnerType,
    string Name,
    string? DisplayName,
    string? ReturnType,
    IReadOnlyList<CppslParameter> Parameters,
    IReadOnlyList<CppslAttribute> Attributes,
    bool IsConst,
    string? File,
    int? Line,
    int? Column);
