using System.ComponentModel;
using System.Diagnostics;
using System.Text.Json;
using System.Text.Json.Serialization;
using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed class NativeExtractorFrontend : ICppslFrontend
{
    public const int ModelVersion = 3;

    private readonly string _extractorPath;

    public NativeExtractorFrontend()
        : this(FindDefaultNativeExtractorPath())
    {
    }

    public NativeExtractorFrontend(string extractorPath)
    {
        _extractorPath = extractorPath;
    }

    public CppslFrontendResult Parse(CppslFrontendOptions options)
    {
        if (string.IsNullOrWhiteSpace(_extractorPath) || !File.Exists(_extractorPath))
        {
            return Failed($"CPPSL native extractor does not exist: `{_extractorPath}`.", options.SourcePath);
        }

        var startInfo = new ProcessStartInfo
        {
            FileName = _extractorPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false
        };
        startInfo.ArgumentList.Add("--source");
        startInfo.ArgumentList.Add(options.SourcePath);
        foreach (var includeRoot in options.IncludeRoots)
        {
            startInfo.ArgumentList.Add("--include");
            startInfo.ArgumentList.Add(includeRoot);
        }

        var sdkLibPath = FindLlvmSdkLibPath();
        if (sdkLibPath is not null)
        {
            startInfo.Environment["DYLD_LIBRARY_PATH"] = AppendPath(startInfo.Environment.TryGetValue("DYLD_LIBRARY_PATH", out var existing) ? existing : null, sdkLibPath);
        }
        var sdkBinPath = FindLlvmSdkBinPath();
        if (sdkBinPath is not null)
        {
            startInfo.Environment["PATH"] = AppendPath(startInfo.Environment.TryGetValue("PATH", out var existing) ? existing : null, sdkBinPath);
        }

        try
        {
            using var process = StartNativeExtractor(startInfo);
            if (process is null)
            {
                return Failed("CPPSL native extractor process could not be started.", options.SourcePath);
            }

            var stdout = process.StandardOutput.ReadToEnd();
            var stderr = process.StandardError.ReadToEnd();
            process.WaitForExit();

            var jsonOptions = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
            jsonOptions.Converters.Add(new JsonStringEnumConverter());
            var result = string.IsNullOrWhiteSpace(stdout)
                ? null
                : JsonSerializer.Deserialize<CppslFrontendResult>(stdout, jsonOptions);
            if (result is not null)
            {
                if (process.ExitCode == 0)
                {
                    return result;
                }

                var diagnostics = result.Diagnostics.ToList();
                if (!string.IsNullOrWhiteSpace(stderr))
                {
                    diagnostics.Add(CppslDiagnostic.Error(stderr.Trim(), options.SourcePath));
                }
                return result with { Succeeded = false, Diagnostics = diagnostics };
            }

            return Failed(
                $"CPPSL native extractor failed with exit code {process.ExitCode}: {stderr.Trim()}",
                options.SourcePath);
        }
        catch (Exception ex)
        {
            return Failed($"CPPSL native extractor failed: {ex.Message}", options.SourcePath);
        }
    }

    private static Process? StartNativeExtractor(ProcessStartInfo startInfo)
    {
        Exception? lastException = null;
        const int maxAttempts = 50;
        for (var attempt = 0; attempt < maxAttempts; ++attempt)
        {
            try
            {
                return Process.Start(startInfo);
            }
            catch (Exception ex) when (IsTransientStartFailure(ex) && attempt + 1 < maxAttempts)
            {
                lastException = ex;
                Thread.Sleep(100);
            }
        }

        if (lastException is not null)
        {
            throw lastException;
        }
        return Process.Start(startInfo);
    }

    private static bool IsTransientStartFailure(Exception exception)
    {
        if (!OperatingSystem.IsWindows())
        {
            return false;
        }

        return exception is Win32Exception { NativeErrorCode: 32 } ||
            exception is IOException { HResult: var hresult } && (hresult & 0xffff) == 32;
    }

    private static CppslFrontendResult Failed(string message, string sourcePath)
    {
        return new CppslFrontendResult(
            false,
            "Native",
            ModelVersion,
            new[] { CppslDiagnostic.Error(message, sourcePath) },
            Array.Empty<CppslDeclaration>(),
            Array.Empty<CppslAstNode>());
    }

    private static string? FindLlvmSdkLibPath()
    {
        if (!OperatingSystem.IsMacOS())
        {
            return null;
        }

        var current = new DirectoryInfo(AppContext.BaseDirectory);
        while (current is not null)
        {
            var candidate = Path.Combine(current.FullName, "SDKs", "llvm-21.1.1", "macosx", "arm64", "lib");
            if (Directory.Exists(candidate))
            {
                return candidate;
            }
            current = current.Parent;
        }
        return null;
    }

    private static string? FindLlvmSdkBinPath()
    {
        if (!OperatingSystem.IsWindows())
        {
            return null;
        }

        var current = new DirectoryInfo(AppContext.BaseDirectory);
        while (current is not null)
        {
            var candidate = Path.Combine(current.FullName, "SDKs", "llvm-21.1.1", "windows", "x64", "bin");
            if (Directory.Exists(candidate))
            {
                return candidate;
            }
            current = current.Parent;
        }
        return null;
    }

    private static string FindDefaultNativeExtractorPath()
    {
        var executableName = OperatingSystem.IsWindows()
            ? "cppsl-native-extractor.exe"
            : "cppsl-native-extractor";
        var current = new DirectoryInfo(AppContext.BaseDirectory);
        while (current is not null)
        {
            var candidate = Path.Combine(current.FullName, "Tools", "CPPSL", "native", "bin", executableName);
            if (File.Exists(candidate))
            {
                return candidate;
            }
            current = current.Parent;
        }
        return Path.Combine("Tools", "CPPSL", "native", "bin", executableName);
    }

    private static string AppendPath(string? existing, string path)
    {
        if (string.IsNullOrWhiteSpace(existing))
        {
            return path;
        }
        return path + Path.PathSeparator + existing;
    }
}
