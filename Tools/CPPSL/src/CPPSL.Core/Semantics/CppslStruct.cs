namespace CPPSL.Core.Semantics;

public sealed record CppslStruct(
    string Name,
    string? File,
    int? Line,
    int? Column,
    IReadOnlyList<CppslAttribute> Attributes,
    IReadOnlyList<CppslField> Fields);
