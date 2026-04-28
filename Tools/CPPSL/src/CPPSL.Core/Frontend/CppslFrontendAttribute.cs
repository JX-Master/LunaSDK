namespace CPPSL.Core.Frontend;

public sealed record CppslFrontendAttribute(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? File,
    int? Line,
    int? Column);
