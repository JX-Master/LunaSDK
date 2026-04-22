namespace CPPSL.Core.Artifacts;

public sealed record CppslArtifacts(
    string IrPath,
    string ReflectionPath,
    string HlslPath,
    string GlslPath,
    string MslPath);
