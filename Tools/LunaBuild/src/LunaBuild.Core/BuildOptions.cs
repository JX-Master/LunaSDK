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
    bool BuildTests,
    RhiApi RhiApi)
{
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
            Architecture: "x64",
            Shared: true,
            BuildTests: true,
            RhiApi: rhiApi);
    }
}
