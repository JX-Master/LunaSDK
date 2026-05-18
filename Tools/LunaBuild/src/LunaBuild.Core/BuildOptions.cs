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
            Architecture: HostArchitecture(),
            Shared: true,
            RhiApi: rhiApi);
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
