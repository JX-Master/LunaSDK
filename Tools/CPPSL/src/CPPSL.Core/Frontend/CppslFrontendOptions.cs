namespace CPPSL.Core.Frontend;

public sealed record CppslFrontendOptions(
    string SourcePath,
    IReadOnlyList<string> IncludeRoots);
