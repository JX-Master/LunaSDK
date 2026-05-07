using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed record IncludeScanResult(
    IReadOnlyList<string> Files,
    IReadOnlyList<CppslDiagnostic> Diagnostics);
