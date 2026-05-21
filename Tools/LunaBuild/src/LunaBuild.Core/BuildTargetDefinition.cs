namespace LunaBuild.Core;

public enum BuildTargetKind
{
    Runtime,
    StaticLibrary,
    SharedLibrary,
    Executable,
    HeaderOnly,
    DotNetProject,
    External,
}

public enum BuildTargetCategory
{
    Engine,
    Tests,
    Tools,
}

public static class BuildTargetCategoryPolicy
{
    public static bool IsDefaultEnabled(BuildTargetCategory category)
    {
        return category switch
        {
            BuildTargetCategory.Engine => true,
            BuildTargetCategory.Tests => false,
            BuildTargetCategory.Tools => false,
            _ => false,
        };
    }
}

public sealed record BuildTargetDefinition(
    string Name,
    string Directory,
    string ScriptPath,
    IReadOnlyList<string> Dependencies,
    IReadOnlyList<string> SourceFiles,
    IReadOnlyList<string> HeaderFiles,
    IReadOnlyList<string> MetaHeaderFiles,
    IReadOnlyList<string> IncludeDirectories,
    IReadOnlyList<string> PublicIncludeDirectories,
    IReadOnlyList<string> Defines,
    IReadOnlyList<string> PublicDefines,
    IReadOnlyList<string> Undefines,
    IReadOnlyList<string> PublicUndefines,
    IReadOnlyList<string> LinkLibraryFiles,
    IReadOnlyList<string> SystemLibraries,
    IReadOnlyList<string> Frameworks,
    IReadOnlyList<string> RuntimeFiles,
    IReadOnlyList<string> RequiredFiles,
    IReadOnlyList<BuildEmbeddedHeaderDefinition> EmbeddedHeaders,
    IReadOnlyList<BuildShaderDefinition> Shaders,
    BuildTargetKind Kind,
    BuildTargetCategory Category,
    string? MsvcRuntimeLibrary,
    string? DotNetProjectFile,
    string? DotNetOutputFile);
