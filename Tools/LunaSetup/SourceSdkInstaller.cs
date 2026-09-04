using System.Formats.Tar;
using System.IO.Compression;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

internal static class SourceSdkInstaller
{
    private const string ReceiptName = ".luna-source-sdk.json";

    public static async Task InstallAsync(string root, string sdksDirectory, string downloadDirectory, bool force)
    {
        var setupDirectory = Path.Combine(root, "Tools", "LunaSetup");
        var packages = JsonSerializer.Deserialize<Dictionary<string, SourceSdkPackage>>(
            File.ReadAllText(Path.Combine(setupDirectory, "SourceSdks.json")),
            new JsonSerializerOptions { PropertyNameCaseInsensitive = true })
            ?? throw new InvalidDataException("SourceSdks.json contains no SDK recipes.");

        foreach(var (name, package) in packages)
        {
            var directoryName = name + "-" + package.Version;
            var installedDirectory = Path.Combine(sdksDirectory, directoryName);
            var configurationDirectory = Path.Combine(setupDirectory, "SourceSdks", name);
            if(!force && IsReady(installedDirectory, package.Sha256))
            {
                Prepare(name, installedDirectory, configurationDirectory);
                Console.WriteLine($"Source SDK {directoryName} already present: {installedDirectory}");
                continue;
            }

            var archivePath = Path.Combine(downloadDirectory, directoryName + ".tar.gz");
            await LunaSetupApp.DownloadAsync(package.Url, archivePath, force, package.Sha256);

            // Stage on the same filesystem as the installation so publication uses
            // directory renames and a failed extraction preserves the old SDK.
            Directory.CreateDirectory(sdksDirectory);
            var stagingDirectory = Path.Combine(sdksDirectory, ".setup-" + directoryName + "-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(stagingDirectory);
            try
            {
                Console.WriteLine($"Extracting {archivePath}");
                using(var archive = File.OpenRead(archivePath))
                using(var gzip = new GZipStream(archive, CompressionMode.Decompress))
                {
                    TarFile.ExtractToDirectory(gzip, stagingDirectory, overwriteFiles: false);
                }
                var contentDirectory = Path.Combine(stagingDirectory, directoryName);
                if(!Directory.Exists(contentDirectory))
                {
                    throw new InvalidDataException($"Source archive does not contain {directoryName}.");
                }
                Prepare(name, contentDirectory, configurationDirectory);
                var receipt = new SourceSdkReceipt(package.Sha256,
                    Directory.GetFiles(contentDirectory, "*", SearchOption.AllDirectories)
                        .Select(path => Path.GetRelativePath(contentDirectory, path))
                        .Order(StringComparer.Ordinal)
                        .ToArray());
                File.WriteAllText(Path.Combine(contentDirectory, ReceiptName), JsonSerializer.Serialize(receipt));
                Publish(contentDirectory, installedDirectory, stagingDirectory);
            }
            finally
            {
                Directory.Delete(stagingDirectory, recursive: true);
            }
            Console.WriteLine($"Source SDK {directoryName} installed: {installedDirectory}");
        }
    }

    private static bool IsReady(string directory, string sha256)
    {
        var receiptPath = Path.Combine(directory, ReceiptName);
        if(!File.Exists(receiptPath)) return false;
        try
        {
            var receipt = JsonSerializer.Deserialize<SourceSdkReceipt>(File.ReadAllText(receiptPath));
            return receipt is { Files.Length: > 0 } &&
                string.Equals(receipt.ArchiveSha256, sha256, StringComparison.OrdinalIgnoreCase) &&
                receipt.Files.All(file => File.Exists(Path.Combine(directory, file)));
        }
        catch(JsonException)
        {
            return false;
        }
    }

    private static void Prepare(string name, string directory, string configurationDirectory)
    {
        if(name == "libzip")
        {
            foreach(var header in new[] { "config.h", "zipconf.h" })
            {
                WriteIfChanged(Path.Combine(directory, header), File.ReadAllText(Path.Combine(configurationDirectory, header + ".in")));
            }
            WriteIfChanged(Path.Combine(directory, "lib", "zip_err_str.c"), GenerateLibZipErrors(directory));
        }

        var sourceDirectory = name == "libzip" ? Path.Combine(directory, "lib") : directory;
        foreach(var source in File.ReadAllLines(Path.Combine(configurationDirectory, "Sources.txt")))
        {
            if(!File.Exists(Path.Combine(sourceDirectory, source)))
            {
                throw new FileNotFoundException($"Source SDK is missing required file: {source}");
            }
        }
    }

    private static string GenerateLibZipErrors(string directory)
    {
        // Equivalent to libzip's cmake/GenerateZipErrorStrings.cmake. Generate the
        // upstream-derived table in SDKs instead of storing it in the repository.
        var result = new StringBuilder("""
            /* Generated by LunaSetup from libzip's zip.h and zipint.h. */
            #include "zipint.h"
            #define L ZIP_ET_LIBZIP
            #define N ZIP_ET_NONE
            #define S ZIP_ET_SYS
            #define Z ZIP_ET_ZLIB
            #define E ZIP_DETAIL_ET_ENTRY
            #define G ZIP_DETAIL_ET_GLOBAL

            """);
        AppendTable("zip.h", "ZIP_ER_", "LNSZ", "_zip_err_str");
        AppendTable("zipint.h", "ZIP_ER_DETAIL_", "EG", "_zip_err_details");
        return result.ToString();

        void AppendTable(string header, string prefix, string categories, string symbol)
        {
            var text = File.ReadAllText(Path.Combine(directory, "lib", header));
            var matches = Regex.Matches(text, @"(?m)^#define " + prefix + @"[A-Z0-9_]+\s+(\d+)\s+/\* ([" + categories + @"]) (.*?) \*/");
            if(matches.Count == 0) throw new InvalidDataException($"No libzip error definitions found in {header}.");
            result.AppendLine($"const struct _zip_err_info {symbol}[] = {{");
            for(var i = 0; i < matches.Count; ++i)
            {
                var match = matches[i];
                if(int.Parse(match.Groups[1].Value) != i)
                    throw new InvalidDataException($"Non-contiguous libzip error definitions in {header}.");
                var message = match.Groups[3].Value.Trim().Replace("\\", "\\\\").Replace("\"", "\\\"");
                result.AppendLine($"    {{ {match.Groups[2].Value}, \"{message}\" }},");
            }
            result.AppendLine("};");
            result.AppendLine($"const int {symbol}_count = sizeof({symbol}) / sizeof({symbol}[0]);");
        }
    }

    private static void WriteIfChanged(string path, string contents)
    {
        if(!File.Exists(path) || File.ReadAllText(path) != contents) File.WriteAllText(path, contents);
    }

    private static void Publish(string sourceDirectory, string destinationDirectory, string stagingDirectory)
    {
        var backupDirectory = Path.Combine(stagingDirectory, "previous");
        var hasPrevious = Directory.Exists(destinationDirectory);
        if(hasPrevious) Directory.Move(destinationDirectory, backupDirectory);
        try
        {
            Directory.Move(sourceDirectory, destinationDirectory);
        }
        catch
        {
            if(hasPrevious) Directory.Move(backupDirectory, destinationDirectory);
            throw;
        }
    }
}

internal sealed record SourceSdkPackage(string Version, string Url, string Sha256);
internal sealed record SourceSdkReceipt(string ArchiveSha256, string[] Files);
