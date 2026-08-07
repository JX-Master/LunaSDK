namespace LunaBuild.Core;

public enum BuildMode
{
    Debug,
    Profile,
    Release,
}

public enum BuildPlatform
{
    Windows,
    MacOS,
    Linux,
    Android,
    IOS,
}

public enum RhiApi
{
    D3D12,
    Vulkan,
    Metal,
}

public sealed record BuildOptions(
    BuildMode Mode,
    BuildPlatform Platform,
    string Architecture,
    bool Shared,
    RhiApi RhiApi,
    BuildProperties Properties,
    IReadOnlyList<string> GlobalDefines,
    IReadOnlyList<string> GlobalUndefines,
    IReadOnlyList<string> GlobalIncludeDirectories,
    IReadOnlyList<BuildActionConfigurationDefinition> ActionConfigurations,
    string LibraryPrefix)
{
    public BuildActionConfigurationDefinition? FindActionConfiguration(string name)
    {
        return ActionConfigurations.FirstOrDefault(configuration =>
            configuration.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
    }

    public static BuildOptions HostDefault()
    {
        var platform = OperatingSystem.IsWindows()
            ? BuildPlatform.Windows
            : OperatingSystem.IsMacOS()
                ? BuildPlatform.MacOS
                : BuildPlatform.Linux;

        var rhiApi = platform switch
        {
            BuildPlatform.Windows => RhiApi.D3D12,
            BuildPlatform.MacOS or BuildPlatform.IOS => RhiApi.Metal,
            _ => RhiApi.Vulkan,
        };

        return new BuildOptions(
            Mode: BuildMode.Debug,
            Platform: platform,
            Architecture: HostArchitecture(),
            Shared: true,
            RhiApi: rhiApi,
            Properties: BuildProperties.Empty,
            GlobalDefines: Array.Empty<string>(),
            GlobalUndefines: Array.Empty<string>(),
            GlobalIncludeDirectories: Array.Empty<string>(),
            ActionConfigurations: Array.Empty<BuildActionConfigurationDefinition>(),
            LibraryPrefix: string.Empty);
    }

    private static string HostArchitecture()
    {
        return System.Runtime.InteropServices.RuntimeInformation.ProcessArchitecture switch
        {
            System.Runtime.InteropServices.Architecture.Arm64 => "arm64",
            System.Runtime.InteropServices.Architecture.X64 => "x64",
            System.Runtime.InteropServices.Architecture.X86 => "x86",
            System.Runtime.InteropServices.Architecture.Arm => "arm",
            _ => "x64",
        };
    }
}

public sealed record BuildActionConfigurationDefinition(
    string Name,
    string ProviderProjectName,
    string ProviderProjectRootDirectory,
    string ProviderProjectBuildDirectory,
    BuildProperties ProviderProperties,
    IReadOnlyDictionary<string, string> Targets,
    IReadOnlyDictionary<string, string> Files,
    IReadOnlyDictionary<string, string> Directories,
    IReadOnlyDictionary<string, string> Values);
