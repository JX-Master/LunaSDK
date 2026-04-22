namespace CPPSL.Core.Semantics;

public sealed record CppslGlobal(
    string Name,
    string Type,
    string? ResourceKind,
    IReadOnlyList<CppslAttribute> Attributes,
    int? DescriptorSet,
    int? Binding,
    string? File,
    int? Line,
    int? Column);
