namespace CPPSL.Core.Frontend;

public sealed record CppslDeclaration(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? File,
    int? Line,
    int? Column);
