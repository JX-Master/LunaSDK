namespace CPPSL.Core.Frontend;

public sealed record CppslDeclaration(
    CppslAstNodeKind Kind,
    string ProviderKind,
    string Spelling,
    string? DisplayName,
    CppslSourceLocation? Location);
