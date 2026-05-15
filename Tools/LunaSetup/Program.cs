using System.Diagnostics;
using System.IO.Compression;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;

return await LunaSetupApp.RunAsync(args);

internal static class LunaSetupApp
{
    private const string SdkVersion = "v6";
    private const string WindowsPlatform = "windows";
    private const string MacOSPlatform = "macosx";

    private static readonly IReadOnlyDictionary<string, string> SdkUrls = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
    {
        [WindowsPlatform] = "https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v6/SDKs-v6-windows.zip",
        [MacOSPlatform] = "https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v6/SDKs-v6-macosx.zip",
    };

    public static async Task<int> RunAsync(string[] args)
    {
        try
        {
            var options = SetupOptions.Parse(args);
            if(options.ShowHelp)
            {
                PrintUsage();
                return 0;
            }

            var root = DiscoverRoot(options.RootDirectory);
            var platform = options.Platform ?? DetectHostPlatform();
            if(!SdkUrls.TryGetValue(platform, out var url))
            {
                throw new NotSupportedException($"Unsupported SDK platform `{platform}`. Supported platforms: {string.Join(", ", SdkUrls.Keys)}.");
            }

            var sdksDirectory = Path.Combine(root, "SDKs");
            var markerPath = Path.Combine(sdksDirectory, ".luna-sdk-version");
            if(!options.Force && IsSdkReady(sdksDirectory, markerPath, platform))
            {
                WriteMarker(markerPath, platform, url);
                Console.WriteLine($"SDKs {SdkVersion}/{platform} already present: {sdksDirectory}");
                return 0;
            }

            var downloadDirectory = Path.Combine(root, "build", "LunaSetup");
            Directory.CreateDirectory(downloadDirectory);
            var archivePath = Path.Combine(downloadDirectory, $"SDKs-{SdkVersion}-{platform}.zip");

            await DownloadAsync(url, archivePath, options.Force);

            if(options.Force && Directory.Exists(sdksDirectory))
            {
                EnsureInsideRoot(root, sdksDirectory);
                Directory.Delete(sdksDirectory, recursive: true);
            }

            var extractDirectory = Path.Combine(downloadDirectory, $"extract-{platform}-{Guid.NewGuid():N}");
            Directory.CreateDirectory(extractDirectory);
            try
            {
                ExtractZipSafe(archivePath, extractDirectory);
                var contentRoot = ResolveArchiveContentRoot(extractDirectory);
                CopyDirectory(contentRoot, sdksDirectory);
            }
            finally
            {
                if(Directory.Exists(extractDirectory))
                {
                    Directory.Delete(extractDirectory, recursive: true);
                }
            }

            WriteMarker(markerPath, platform, url);
            Console.WriteLine($"SDKs {SdkVersion}/{platform} installed: {sdksDirectory}");
            return 0;
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine($"lunasetup: {ex.Message}");
            return 1;
        }
    }

    private static string DiscoverRoot(string? startDirectory)
    {
        var current = new DirectoryInfo(Path.GetFullPath(startDirectory ?? Environment.CurrentDirectory));
        while(current is not null)
        {
            if(Directory.Exists(Path.Combine(current.FullName, "Modules", "Luna")) &&
                Directory.Exists(Path.Combine(current.FullName, "Tools")))
            {
                return current.FullName;
            }
            current = current.Parent;
        }
        throw new DirectoryNotFoundException("Cannot find LunaSDK root. Pass --root or run under the repository.");
    }

    private static string DetectHostPlatform()
    {
        if(RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            return WindowsPlatform;
        }
        if(RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
        {
            return MacOSPlatform;
        }
        throw new NotSupportedException("Automatic SDK setup is currently available for Windows and macOS hosts.");
    }

    private static bool IsSdkReady(string sdksDirectory, string markerPath, string platform)
    {
        if(File.Exists(markerPath))
        {
            var marker = File.ReadAllText(markerPath);
            if(marker.Contains($"version={SdkVersion}", StringComparison.OrdinalIgnoreCase) &&
                marker.Contains($"platform={platform}", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }

        if(!Directory.Exists(sdksDirectory))
        {
            return false;
        }

        var commonRequired = new[]
        {
            Path.Combine(sdksDirectory, "CPPSL"),
            Path.Combine(sdksDirectory, "llvm-21.1.1"),
            Path.Combine(sdksDirectory, "miniaudio", "include", "miniaudio.h"),
            Path.Combine(sdksDirectory, "stb", "include", "stb", "stb_image.h"),
        };
        if(commonRequired.Any(path => !Path.Exists(path)))
        {
            return false;
        }

        if(platform.Equals(WindowsPlatform, StringComparison.OrdinalIgnoreCase))
        {
            return File.Exists(Path.Combine(sdksDirectory, "d3d12-memory-allocator", "windows", "x64", "include", "D3D12MemAlloc.h")) &&
                File.Exists(Path.Combine(sdksDirectory, "d3d12-memory-allocator", "windows", "x64", "lib", "D3D12MA.lib")) &&
                File.Exists(Path.Combine(sdksDirectory, "volk", "include", "volk.h")) &&
                File.Exists(Path.Combine(sdksDirectory, "vulkan-memory-allocator", "include", "vk_mem_alloc.h"));
        }

        return Directory.Exists(Path.Combine(sdksDirectory, "CPPSL", "macosx")) &&
            Directory.Exists(Path.Combine(sdksDirectory, "llvm-21.1.1", "macosx"));
    }

    private static async Task DownloadAsync(string url, string archivePath, bool force)
    {
        if(File.Exists(archivePath) && !force)
        {
            Console.WriteLine($"Using cached archive: {archivePath}");
            return;
        }

        Console.WriteLine($"Downloading {url}");
        using var http = new HttpClient(new HttpClientHandler { AllowAutoRedirect = true });
        http.DefaultRequestHeaders.UserAgent.Add(new ProductInfoHeaderValue("LunaSetup", SdkVersion));
        using var response = await http.GetAsync(url, HttpCompletionOption.ResponseHeadersRead);
        response.EnsureSuccessStatusCode();

        var totalBytes = response.Content.Headers.ContentLength;
        await using var source = await response.Content.ReadAsStreamAsync();
        await using var destination = File.Create(archivePath);

        var buffer = new byte[1024 * 1024];
        long downloaded = 0;
        var stopwatch = Stopwatch.StartNew();
        while(true)
        {
            var read = await source.ReadAsync(buffer);
            if(read == 0)
            {
                break;
            }
            await destination.WriteAsync(buffer.AsMemory(0, read));
            downloaded += read;
            if(stopwatch.ElapsedMilliseconds >= 1000)
            {
                PrintProgress(downloaded, totalBytes);
                stopwatch.Restart();
            }
        }
        PrintProgress(downloaded, totalBytes);
        Console.WriteLine();
    }

    private static void PrintProgress(long downloaded, long? totalBytes)
    {
        if(totalBytes is > 0)
        {
            var percent = downloaded * 100.0 / totalBytes.Value;
            Console.Write($"\rDownloaded {downloaded / 1024 / 1024} MiB / {totalBytes.Value / 1024 / 1024} MiB ({percent:0.0}%)");
        }
        else
        {
            Console.Write($"\rDownloaded {downloaded / 1024 / 1024} MiB");
        }
    }

    private static void ExtractZipSafe(string archivePath, string destinationDirectory)
    {
        Console.WriteLine($"Extracting {archivePath}");
        var destinationRoot = Path.GetFullPath(destinationDirectory);
        using var archive = ZipFile.OpenRead(archivePath);
        foreach(var entry in archive.Entries)
        {
            var targetPath = Path.GetFullPath(Path.Combine(destinationRoot, entry.FullName));
            if(!targetPath.StartsWith(destinationRoot + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase) &&
                !string.Equals(targetPath, destinationRoot, StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidDataException($"Zip entry escapes extraction directory: {entry.FullName}");
            }

            if(string.IsNullOrEmpty(entry.Name))
            {
                Directory.CreateDirectory(targetPath);
                continue;
            }

            Directory.CreateDirectory(Path.GetDirectoryName(targetPath)!);
            entry.ExtractToFile(targetPath, overwrite: true);
        }
    }

    private static string ResolveArchiveContentRoot(string extractDirectory)
    {
        var directSdks = Path.Combine(extractDirectory, "SDKs");
        if(Directory.Exists(directSdks))
        {
            return directSdks;
        }
        if(LooksLikeSdkRoot(extractDirectory))
        {
            return extractDirectory;
        }

        var childDirectories = Directory.GetDirectories(extractDirectory);
        if(childDirectories.Length == 1)
        {
            var child = childDirectories[0];
            var childSdks = Path.Combine(child, "SDKs");
            if(Directory.Exists(childSdks))
            {
                return childSdks;
            }
            if(LooksLikeSdkRoot(child))
            {
                return child;
            }
        }

        return extractDirectory;
    }

    private static bool LooksLikeSdkRoot(string directory)
    {
        return Directory.Exists(Path.Combine(directory, "CPPSL")) ||
            Directory.Exists(Path.Combine(directory, "llvm-21.1.1")) ||
            Directory.Exists(Path.Combine(directory, "stb"));
    }

    private static void CopyDirectory(string sourceDirectory, string destinationDirectory)
    {
        Directory.CreateDirectory(destinationDirectory);
        foreach(var directory in Directory.EnumerateDirectories(sourceDirectory, "*", SearchOption.AllDirectories))
        {
            var relative = Path.GetRelativePath(sourceDirectory, directory);
            Directory.CreateDirectory(Path.Combine(destinationDirectory, relative));
        }

        foreach(var file in Directory.EnumerateFiles(sourceDirectory, "*", SearchOption.AllDirectories))
        {
            var relative = Path.GetRelativePath(sourceDirectory, file);
            var destination = Path.Combine(destinationDirectory, relative);
            Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
            File.Copy(file, destination, overwrite: true);
        }
    }

    private static void WriteMarker(string markerPath, string platform, string url)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(markerPath)!);
        File.WriteAllText(markerPath, string.Join(Environment.NewLine,
            $"version={SdkVersion}",
            $"platform={platform}",
            $"url={url}",
            $"updated_utc={DateTimeOffset.UtcNow:O}",
            string.Empty));
    }

    private static void EnsureInsideRoot(string root, string path)
    {
        var relative = Path.GetRelativePath(Path.GetFullPath(root), Path.GetFullPath(path));
        if(relative.StartsWith("..", StringComparison.Ordinal) || Path.IsPathRooted(relative))
        {
            throw new InvalidOperationException($"Refusing to delete path outside repository root: {path}");
        }
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: lunasetup [options]");
        Console.WriteLine();
        Console.WriteLine("Options:");
        Console.WriteLine("  --root <path>       LunaSDK repository root. Defaults to auto-discovery.");
        Console.WriteLine("  --platform <name>   windows or macosx. Defaults to host platform.");
        Console.WriteLine("  --force             Redownload and replace SDKs.");
        Console.WriteLine("  --help              Show this help text.");
    }
}

internal sealed class SetupOptions
{
    public string? RootDirectory { get; private init; }

    public string? Platform { get; private init; }

    public bool Force { get; private init; }

    public bool ShowHelp { get; private init; }

    public static SetupOptions Parse(string[] args)
    {
        var root = default(string);
        var platform = default(string);
        var force = false;
        var showHelp = false;

        for(var i = 0; i < args.Length; ++i)
        {
            switch(args[i])
            {
                case "--root":
                    root = RequireValue(args, ref i, "--root");
                    break;
                case "--platform":
                    platform = RequireValue(args, ref i, "--platform").ToLowerInvariant();
                    break;
                case "--force":
                    force = true;
                    break;
                case "-h":
                case "--help":
                case "help":
                    showHelp = true;
                    break;
                default:
                    throw new ArgumentException($"Unknown option: {args[i]}");
            }
        }

        return new SetupOptions
        {
            RootDirectory = root,
            Platform = platform,
            Force = force,
            ShowHelp = showHelp,
        };
    }

    private static string RequireValue(string[] args, ref int index, string optionName)
    {
        if(index + 1 >= args.Length)
        {
            throw new ArgumentException($"{optionName} requires a value.");
        }
        ++index;
        return args[index];
    }
}
