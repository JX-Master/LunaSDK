namespace CPPSL.Core.Frontend;

public sealed record CppslAstNode(
    CppslAstNodeKind Kind,
    string ProviderKind,
    string Spelling,
    string? DisplayName,
    string? TypeName,
    string? ResultTypeName,
    CppslSourceLocation? Location,
    CppslSourceRange? Range,
    IReadOnlyList<CppslFrontendAttribute> Attributes,
    IReadOnlyList<CppslAstNode> Children);
