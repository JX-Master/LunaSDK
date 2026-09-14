using System.Text.Json;

namespace LunaBuild.Core.Targets;

internal static class SourceSdkPaths
{
    public static string DirectoryFor(BuildWorkspace workspace, string name)
    {
        using var recipes = JsonDocument.Parse(File.ReadAllText(workspace.ResolveRepositoryPath("Tools/LunaSetup/SourceSdks.json")));
        var version = recipes.RootElement.GetProperty(name).GetProperty("version").GetString();
        return "SDKs/" + name + "-" + version;
    }

    public static string[] SourcesFor(BuildWorkspace workspace, string name)
    {
        return File.ReadAllLines(workspace.ResolveRepositoryPath("Tools/LunaSetup/SourceSdks/" + name + "/Sources.txt"));
    }
}

public sealed class LibZipTargetRules : TargetRules
{
    public LibZipTargetRules() : base("libzip", ".", "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.StaticLibrary;
        DependsOn("zlib");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var directory = SourceSdkPaths.DirectoryFor(workspace, "libzip");
        Headers(directory + "/*.h", directory + "/lib/*.h");
        IncludeDirectories(directory, directory + "/lib");
        PublicIncludeDirectories(directory, directory + "/lib");
        foreach(var source in SourceSdkPaths.SourcesFor(workspace, "libzip"))
        {
            Sources(directory + "/lib/" + source);
        }
        if(Platform == BuildPlatform.Windows)
        {
            Defines("WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS");
            foreach(var source in new[] { "zip_source_file_win32.c", "zip_source_file_win32_named.c",
                "zip_source_file_win32_utf16.c", "zip_source_file_win32_utf8.c",
                "zip_source_file_win32_ansi.c", "zip_random_win32.c" })
            {
                Sources(directory + "/lib/" + source);
            }
            SystemLibraries("advapi32");
        }
        else
        {
            Defines("_FILE_OFFSET_BITS=64");
            Sources(directory + "/lib/zip_source_file_stdio_named.c", directory + "/lib/zip_random_unix.c");
        }
    }
}

public sealed class ZlibTargetRules : TargetRules
{
    public ZlibTargetRules() : base("zlib", ".", "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.StaticLibrary;
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var directory = SourceSdkPaths.DirectoryFor(workspace, "zlib");
        Headers(directory + "/*.h");
        PublicIncludeDirectories(directory);
        foreach(var source in SourceSdkPaths.SourcesFor(workspace, "zlib"))
        {
            Sources(directory + "/" + source);
        }
    }
}

public sealed class StbTargetRules : TargetRules
{
    public StbTargetRules()
        : base(
            name: "stb",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        PublicIncludeDirectories(
            "SDKs/stb/include",
            "SDKs/stb/include/stb");
        RequiredFiles(
            "SDKs/stb/include/stb/stb_image.h",
            "SDKs/stb/include/stb/stb_image_write.h",
            "SDKs/stb/include/stb/stb_rect_pack.h",
            "SDKs/stb/include/stb/stb_truetype.h");
    }
}

public sealed class MiniAudioTargetRules : TargetRules
{
    public MiniAudioTargetRules()
        : base(
            name: "miniaudio",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        PublicIncludeDirectories("SDKs/miniaudio/include");
        RequiredFiles("SDKs/miniaudio/include/miniaudio.h");
    }
}

public sealed class D3D12MemoryAllocatorTargetRules : TargetRules
{
    public D3D12MemoryAllocatorTargetRules()
        : base(
            name: "d3d12-memory-allocator",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows);
        PublicIncludeDirectories("SDKs/d3d12-memory-allocator/windows/x64/include");
        RequiredFiles("SDKs/d3d12-memory-allocator/windows/x64/include/D3D12MemAlloc.h");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var configurationDirectory = options.Mode == BuildMode.Debug ? "Debug" : "Release";
        LinkLibraryFiles($"SDKs/d3d12-memory-allocator/windows/x64/lib/{configurationDirectory}/D3D12MA.lib");
    }
}

public sealed class VolkTargetRules : TargetRules
{
    public VolkTargetRules()
        : base(
            name: "volk",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
        DependsOn("vulkan-headers");
        PublicIncludeDirectories("SDKs/volk/include");
        RequiredFiles(
            "SDKs/volk/include/volk.h",
            "SDKs/volk/include/volk.c");
    }
}

public sealed class VulkanHeadersTargetRules : TargetRules
{
    public VulkanHeadersTargetRules()
        : base(
            name: "vulkan-headers",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var projectIncludeDirectory = workspace.ResolveRepositoryPath("SDKs/vulkan-headers/include");
        var includeDirectory = IsVulkanHeadersIncludeDirectory(projectIncludeDirectory)
            ? projectIncludeDirectory
            : FindVulkanSdkIncludeDirectory() ?? projectIncludeDirectory;

        PublicIncludeDirectories(includeDirectory);
        RequiredFiles(
            Path.Combine(includeDirectory, "vulkan", "vk_platform.h"),
            Path.Combine(includeDirectory, "vulkan", "vulkan.h"));
    }

    private static string? FindVulkanSdkIncludeDirectory()
    {
        var vulkanSdk = Environment.GetEnvironmentVariable("VULKAN_SDK");
        if(string.IsNullOrWhiteSpace(vulkanSdk))
        {
            return null;
        }

        var includeDirectory = Path.Combine(vulkanSdk, OperatingSystem.IsWindows() ? "Include" : "include");
        return IsVulkanHeadersIncludeDirectory(includeDirectory) ? includeDirectory : null;
    }

    private static bool IsVulkanHeadersIncludeDirectory(string includeDirectory)
    {
        return File.Exists(Path.Combine(includeDirectory, "vulkan", "vk_platform.h")) &&
            File.Exists(Path.Combine(includeDirectory, "vulkan", "vulkan.h"));
    }
}

public sealed class VulkanMemoryAllocatorTargetRules : TargetRules
{
    public VulkanMemoryAllocatorTargetRules()
        : base(
            name: "vulkan-memory-allocator",
            targetDirectory: ".",
            rulesPath: "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.Linux, BuildPlatform.Android);
        DependsOn("vulkan-headers");
        PublicIncludeDirectories("SDKs/vulkan-memory-allocator/include");
        RequiredFiles("SDKs/vulkan-memory-allocator/include/vk_mem_alloc.h");
    }
}
