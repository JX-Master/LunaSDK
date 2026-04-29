namespace CPPSL.Core.Frontend;

public sealed record CppslAstNode(
    CppslAstNodeKind Kind,
    string ProviderKind,
    string Spelling,
    string? DisplayName,
    string? TypeName,
    string? ResultTypeName,
    string? DeclId,
    string? CanonicalDeclId,
    string? ReferencedDeclId,
    string? DirectCalleeDeclId,
    string? OwnerDeclId,
    string? TemplatePatternDeclId,
    bool IsImplicit,
    bool IsConstexpr,
    bool IsTemplateInstantiation,
    bool UsesDefaultArgument,
    string? ConstantValue,
    CppslSourceLocation? Location,
    CppslSourceRange? Range,
    CppslTypeInfo? TypeInfo,
    CppslTypeInfo? ResultTypeInfo,
    IReadOnlyList<CppslTemplateArgumentInfo> TemplateArguments,
    IReadOnlyList<CppslFrontendAttribute> Attributes,
    IReadOnlyList<CppslAstNode> Children);

public sealed record CppslTypeInfo(
    string Spelling,
    string CanonicalName,
    string DesugaredName,
    IReadOnlyList<CppslTypeInfo> TemplateArguments);

public sealed record CppslTemplateArgumentInfo(
    string Kind,
    string? Spelling,
    string? Value,
    CppslTypeInfo? TypeInfo);
