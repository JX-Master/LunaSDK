namespace CPPSL.Core.Semantics;

public sealed record CppslSemanticModel(
    IReadOnlyList<CppslStruct> Structs,
    IReadOnlyList<CppslGlobal> Globals,
    IReadOnlyList<CppslFunction> Functions);
