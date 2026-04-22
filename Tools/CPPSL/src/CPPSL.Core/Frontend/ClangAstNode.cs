namespace CPPSL.Core.Frontend;

public sealed record ClangAstNode(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? Type,
    string? ResultType,
    string? File,
    int? Line,
    int? Column,
    IReadOnlyList<ClangAstNode> Children);
