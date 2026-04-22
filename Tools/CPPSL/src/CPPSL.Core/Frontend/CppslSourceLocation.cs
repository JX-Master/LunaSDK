namespace CPPSL.Core.Frontend;

public sealed record CppslSourceLocation(
    string File,
    int Line,
    int Column);

public sealed record CppslSourceRange(
    CppslSourceLocation? Start,
    CppslSourceLocation? End);
