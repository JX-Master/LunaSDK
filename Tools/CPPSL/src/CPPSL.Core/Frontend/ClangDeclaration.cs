namespace CPPSL.Core.Frontend;

public sealed record ClangDeclaration(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? File,
    int? Line,
    int? Column);
