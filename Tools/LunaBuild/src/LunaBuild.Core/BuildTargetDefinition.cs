namespace LunaBuild.Core;

public enum BuildTargetKind
{
    StaticLibrary,
    SharedLibrary,
    Executable,
    Application,
    HeaderOnly,
    DotNetProject,
    External,
}

public static class BuildTargetKindExtensions
{
    public static bool ProducesNativeExecutable(this BuildTargetKind kind)
    {
        return kind is BuildTargetKind.Executable or BuildTargetKind.Application;
    }
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
    string? AppleBundleIdentifier,
    string? AppleBundleDisplayName,
    string? AppleInfoPlistFile,
    string? AppleEntitlementsFile,
    IReadOnlyList<string> AppleBundleResources,
    IReadOnlyList<BuildEmbeddedHeaderDefinition> EmbeddedHeaders,
    IReadOnlyList<BuildShaderDefinition> Shaders,
    BuildTargetKind Kind,
    BuildTargetCategory Category,
    string? MsvcRuntimeLibrary,
    bool EnableRtti,
    string? DotNetProjectFile,
    string? DotNetOutputFile,
    string ProjectName,
    string QualifiedName,
    string ProjectRootDirectory,
    string ProjectBuildDirectory,
    string ConfigurationId,
    BuildOptions Options,
    bool IsHostProject);
