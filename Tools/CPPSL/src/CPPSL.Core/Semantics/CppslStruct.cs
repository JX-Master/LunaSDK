using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed record CppslStruct(
    string? DeclId,
    string Name,
    string? DisplayName,
    string? File,
    int? Line,
    int? Column,
    IReadOnlyList<CppslAttribute> Attributes,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslTemplateArgumentInfo> TemplateArguments,
    IReadOnlyList<CppslField> Fields,
    IReadOnlyList<CppslMethod> Methods);
