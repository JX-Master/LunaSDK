namespace LunaBuild.Core;

public sealed record BuildShaderDefinition(
    string SourceFile,
    string Stage,
    string EntryPoint);
