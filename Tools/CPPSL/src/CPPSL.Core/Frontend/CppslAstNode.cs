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
    CppslTypeInfo? TypeInfo,
    CppslTypeInfo? ResultTypeInfo,
    IReadOnlyList<CppslFrontendAttribute> Attributes,
    IReadOnlyList<CppslAstNode> Children);

public sealed record CppslTypeInfo(
    string Spelling,
    string CanonicalName,
    string DesugaredName,
    IReadOnlyList<CppslTypeInfo> TemplateArguments);
