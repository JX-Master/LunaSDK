namespace CPPSL.Core.Semantics;

public sealed record CppslParameter(
    string Name,
    string Type,
    string? File,
    int? Line,
    int? Column);
