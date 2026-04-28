namespace CPPSL.Core.Semantics;

public sealed record CppslAttribute(
    string Name,
    IReadOnlyList<string> Arguments,
    string? File,
    int? Line,
    int? Column);
