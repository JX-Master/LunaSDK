using CPPSL.Core.Artifacts;
using CPPSL.Core.ShaderModel;

namespace CPPSL.Core.Compiler;

public sealed record CppslCompileOptions(
    string SourcePath,
    string OutputDirectory,
    IReadOnlyList<string> IncludeRoots,
    string EntryPoint,
    ShaderStage Stage,
    IReadOnlyList<CppslOutputTarget>? Targets = null);
