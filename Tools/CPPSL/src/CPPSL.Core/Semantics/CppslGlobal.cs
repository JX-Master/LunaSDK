namespace CPPSL.Core.Semantics;

public sealed record CppslGlobal(
    string Name,
    string Type,
    string? ResourceKind,
    string? File,
    int? Line,
    int? Column);
