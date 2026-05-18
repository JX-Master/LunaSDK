namespace LunaBuild.Core;

public sealed record BuildEmbeddedHeaderDefinition(
    string SourceFile,
    string HeaderFile,
    string DataSymbol,
    string SizeSymbol);
