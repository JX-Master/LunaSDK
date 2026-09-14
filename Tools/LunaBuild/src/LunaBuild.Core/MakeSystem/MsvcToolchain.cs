namespace LunaBuild.Core.MakeSystem;

using System.Collections.ObjectModel;

internal sealed record MsvcToolchain(
    string VcVarsBat,
    string ClExe,
    string LinkExe,
    string LibExe,
    string RcExe,
    IReadOnlyDictionary<string, string> EnvironmentVariables);

internal static class MsvcToolchainLocator
{
    private const string EnvironmentMarker = "__LUNABUILD_MSVC_ENVIRONMENT__";

    public static async Task<MsvcToolchain> LocateAsync(
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken)
    {
        var vcvars = LocateVcVarsBat();
        var command = $"call {QuoteForCmd(vcvars)} >nul && echo {EnvironmentMarker} && set";
        var result = await ProcessRunner.RunAsync(
            "cmd.exe",
            $"/d /s /c \"{command}\"",
            workingDirectory,
            timeout,
            cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException(
                $"Failed to initialize the MSVC environment with `{vcvars}` (exit code {result.ExitCode})." +
                Environment.NewLine + result.Output.TrimEnd());
        }

        var environmentVariables = ParseEnvironmentOutput(result.Output);
        var vcToolsInstallDir = RequireEnvironmentVariable(environmentVariables, "VCToolsInstallDir", vcvars);
        var windowsSdkBin = RequireEnvironmentVariable(environmentVariables, "WindowsSdkVerBinPath", vcvars);
        var vcBin = Path.Combine(vcToolsInstallDir, "bin", "Hostx64", "x64");
        return new MsvcToolchain(
            vcvars,
            RequireTool(Path.Combine(vcBin, "cl.exe"), "C++ compiler"),
            RequireTool(Path.Combine(vcBin, "link.exe"), "linker"),
            RequireTool(Path.Combine(vcBin, "lib.exe"), "library manager"),
            RequireTool(Path.Combine(windowsSdkBin, "x64", "rc.exe"), "resource compiler"),
            environmentVariables);
    }

    internal static IReadOnlyDictionary<string, string> ParseEnvironmentOutput(string output)
    {
        var environmentVariables = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        var foundMarker = false;
        foreach(var rawLine in output.Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries))
        {
            var line = rawLine.TrimEnd();
            if(!foundMarker)
            {
                foundMarker = line.Equals(EnvironmentMarker, StringComparison.Ordinal);
                continue;
            }

            var separator = line.IndexOf('=');
            if(separator <= 0)
            {
                continue;
            }
            var name = line[..separator];
            var value = line[(separator + 1)..];
            if(!environmentVariables.TryAdd(name, value) && name.Equals("PATH", StringComparison.Ordinal))
            {
                // Windows environments may contain both PATH and Path. vcvars updates
                // the canonical uppercase entry, so it must win over a stale alias.
                environmentVariables[name] = value;
            }
        }

        if(!foundMarker || environmentVariables.Count == 0)
        {
            throw new MakeSystemException("vcvars64.bat completed without returning an MSVC environment.");
        }
        return new ReadOnlyDictionary<string, string>(environmentVariables);
    }

    private static string LocateVcVarsBat()
    {
        foreach(var vsRoot in CandidateVisualStudioRoots().Distinct(StringComparer.OrdinalIgnoreCase))
        {
            var vcvars = Path.Combine(vsRoot, "VC", "Auxiliary", "Build", "vcvars64.bat");
            if(File.Exists(vcvars))
            {
                return vcvars;
            }
        }

        throw new MakeSystemException(
            "Cannot locate MSVC x64 toolchain. Install Visual Studio 2022 or later with the C++ x64/x86 build tools workload.");
    }

    private static string RequireEnvironmentVariable(
        IReadOnlyDictionary<string, string> environmentVariables,
        string name,
        string vcvars)
    {
        if(environmentVariables.TryGetValue(name, out var value) && !string.IsNullOrWhiteSpace(value))
        {
            return value;
        }
        throw new MakeSystemException($"`{vcvars}` did not define the required environment variable `{name}`.");
    }

    private static string RequireTool(string path, string description)
    {
        if(File.Exists(path))
        {
            return path;
        }
        throw new MakeSystemException($"MSVC {description} was not found: {path}");
    }

    private static string QuoteForCmd(string value)
    {
        return value.Contains(' ') || value.Contains('\t')
            ? $"\"{value}\""
            : value;
    }

    private static IEnumerable<string> CandidateVisualStudioRoots()
    {
        var currentVsInstallDir = Environment.GetEnvironmentVariable("VSINSTALLDIR");
        if(!string.IsNullOrWhiteSpace(currentVsInstallDir))
        {
            yield return currentVsInstallDir;
        }

        foreach(var vsRoot in VisualStudioRootsFromVsWhere())
        {
            yield return vsRoot;
        }

        foreach(var versionRoot in CommonVisualStudioVersionRoots())
        {
            foreach(var edition in new[] { "Enterprise", "Professional", "Community", "BuildTools", "Preview" })
            {
                yield return Path.Combine(versionRoot, edition);
            }
        }
    }

    private static IEnumerable<string> CommonVisualStudioVersionRoots()
    {
        foreach(var root in new[]
        {
            Environment.GetEnvironmentVariable("ProgramFiles"),
            Environment.GetEnvironmentVariable("ProgramFiles(x86)"),
        }.Where(root => !string.IsNullOrWhiteSpace(root)))
        {
            yield return Path.Combine(root!, "Microsoft Visual Studio", "2022");
        }
    }

    private static IEnumerable<string> VisualStudioRootsFromVsWhere()
    {
        var vswhere = LocateVsWhere();
        if(vswhere is null)
        {
            yield break;
        }

        foreach(var line in RunVsWhere(vswhere).Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries))
        {
            var root = line.Trim();
            if(Directory.Exists(root))
            {
                yield return root;
            }
        }
    }

    private static string? LocateVsWhere()
    {
        var programFilesX86 = Environment.GetEnvironmentVariable("ProgramFiles(x86)");
        if(string.IsNullOrWhiteSpace(programFilesX86))
        {
            return null;
        }

        var vswhere = Path.Combine(programFilesX86, "Microsoft Visual Studio", "Installer", "vswhere.exe");
        return File.Exists(vswhere) ? vswhere : null;
    }

    private static string RunVsWhere(string vswhere)
    {
        try
        {
            var startInfo = new System.Diagnostics.ProcessStartInfo(vswhere)
            {
                RedirectStandardOutput = true,
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            startInfo.ArgumentList.Add("-latest");
            startInfo.ArgumentList.Add("-products");
            startInfo.ArgumentList.Add("*");
            startInfo.ArgumentList.Add("-requires");
            startInfo.ArgumentList.Add("Microsoft.VisualStudio.Component.VC.Tools.x86.x64");
            startInfo.ArgumentList.Add("-property");
            startInfo.ArgumentList.Add("installationPath");

            using var process = System.Diagnostics.Process.Start(startInfo);
            if(process is null)
            {
                return string.Empty;
            }

            if(!process.WaitForExit(10000))
            {
                process.Kill(entireProcessTree: true);
                return string.Empty;
            }

            var output = process.StandardOutput.ReadToEnd();
            return process.ExitCode == 0 ? output : string.Empty;
        }
        catch
        {
            return string.Empty;
        }
    }
}
