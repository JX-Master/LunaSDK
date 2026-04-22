namespace CPPSL.Core.Frontend;

public sealed record CppslAstNode(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? Type,
    string? ResultType,
    string? File,
    int? Line,
    int? Column,
    IReadOnlyList<CppslFrontendAttribute> Attributes,
    IReadOnlyList<CppslAstNode> Children);
