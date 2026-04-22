namespace CPPSL.Core.Semantics;

public sealed record CppslStruct(
    string Name,
    string? File,
    int? Line,
    int? Column,
    IReadOnlyList<CppslField> Fields);
