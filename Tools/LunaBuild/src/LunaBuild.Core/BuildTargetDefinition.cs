namespace LunaBuild.Core;

public enum BuildTargetKind
{
    Runtime,
    StaticLibrary,
    SharedLibrary,
    Executable,
    HeaderOnly,
    DotNetProject,
}

public sealed record BuildTargetDefinition(
    string Name,
    string Directory,
    string ScriptPath,
    IReadOnlyList<string> Dependencies,
    IReadOnlyList<string> SourceFiles,
    IReadOnlyList<string> HeaderFiles,
    IReadOnlyList<string> IncludeDirectories,
    IReadOnlyList<string> Defines,
    IReadOnlyList<string> Undefines,
    IReadOnlyList<string> PackageNames,
    IReadOnlyList<string> LinkLibraryFiles,
    IReadOnlyList<string> SystemLibraries,
    IReadOnlyList<string> Frameworks,
    IReadOnlyList<string> RuntimeFiles,
    IReadOnlyList<BuildEmbeddedHeaderDefinition> EmbeddedHeaders,
    IReadOnlyList<BuildShaderDefinition> Shaders,
    BuildTargetKind Kind,
    bool IsTest,
    string? MsvcRuntimeLibrary,
    string? DotNetProjectFile,
    string? DotNetOutputFile);
