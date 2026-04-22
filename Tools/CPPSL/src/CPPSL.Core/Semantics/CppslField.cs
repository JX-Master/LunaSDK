namespace CPPSL.Core.Semantics;

public sealed record CppslField(
    string Name,
    string Type,
    IReadOnlyList<CppslAttribute> Attributes,
    int? Location,
    bool IsPosition,
    string? File,
    int? Line,
    int? Column);
