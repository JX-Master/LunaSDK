namespace CPPSL.Core.Frontend;

public sealed record CppslDeclaration(
    CppslAstNodeKind Kind,
    string ProviderKind,
    string Spelling,
    string? DisplayName,
    string? DeclId,
    string? CanonicalDeclId,
    string? OwnerDeclId,
    bool IsImplicit,
    bool IsConstexpr,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslTemplateArgumentInfo> TemplateArguments,
    CppslSourceLocation? Location);
