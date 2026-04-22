using CPPSL.Core.IR;

namespace CPPSL.Core.Compiler;

public sealed record CppslCompileOptions(
    string SourcePath,
    string OutputDirectory,
    IReadOnlyList<string> IncludeRoots,
    string EntryPoint,
    ShaderStage Stage);
