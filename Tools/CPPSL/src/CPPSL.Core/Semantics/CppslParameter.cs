namespace CPPSL.Core.Semantics;

public sealed record CppslParameter(
    string Name,
    string Type,
    IReadOnlyList<CppslAttribute> Attributes,
    string? File,
    int? Line,
    int? Column);
