using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed record CppslFunction(
    string? DeclId,
    string Name,
    string? DisplayName,
    string? ReturnType,
    IReadOnlyList<CppslParameter> Parameters,
    IReadOnlyList<CppslAttribute> Attributes,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslTemplateArgumentInfo> TemplateArguments,
    bool IsEntryPoint,
    string? Stage,
    string? DeclaredStage,
    string? File,
    int? Line,
    int? Column);
