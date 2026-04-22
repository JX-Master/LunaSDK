namespace CPPSL.Core.Semantics;

public sealed record CppslField(
    string Name,
    string Type,
    string? File,
    int? Line,
    int? Column);
