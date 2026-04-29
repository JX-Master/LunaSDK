using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed record CppslMethod(
    string? DeclId,
    string OwnerType,
    string Name,
    string? DisplayName,
    string? ReturnType,
    IReadOnlyList<CppslParameter> Parameters,
    IReadOnlyList<CppslAttribute> Attributes,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslTemplateArgumentInfo> TemplateArguments,
    bool IsConst,
    string? File,
    int? Line,
    int? Column);
