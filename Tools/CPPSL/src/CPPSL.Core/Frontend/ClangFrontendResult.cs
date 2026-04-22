using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed record ClangFrontendResult(
    bool Succeeded,
    IReadOnlyList<CppslDiagnostic> Diagnostics,
    IReadOnlyList<ClangDeclaration> Declarations,
    IReadOnlyList<ClangAstNode> AstNodes);
