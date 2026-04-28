using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed record CppslFrontendResult(
    bool Succeeded,
    string Provider,
    int ModelVersion,
    IReadOnlyList<CppslDiagnostic> Diagnostics,
    IReadOnlyList<CppslDeclaration> Declarations,
    IReadOnlyList<CppslAstNode> AstNodes);
