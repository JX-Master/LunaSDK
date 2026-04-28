using CPPSL.Core.Artifacts;
using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Compiler;

public sealed record CppslCompileResult(
    bool Succeeded,
    IReadOnlyList<CppslDiagnostic> Diagnostics,
    CppslArtifacts? Artifacts);
