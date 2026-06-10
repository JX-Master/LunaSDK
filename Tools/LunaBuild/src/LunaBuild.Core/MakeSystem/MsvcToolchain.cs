namespace LunaBuild.Core.MakeSystem;

internal sealed record MsvcToolchain(string VcVarsBat, string ClExe, string LinkExe, string LibExe);

internal static class MsvcToolchainLocator
{
    public static MsvcToolchain Locate()
    {
        foreach(var vsRoot in CandidateVisualStudioRoots().Distinct(StringComparer.OrdinalIgnoreCase))
        {
            var toolchain = TryCreateToolchain(vsRoot);
            if(toolchain is not null)
            {
                return toolchain;
            }
        }

        throw new MakeSystemException(
            "Cannot locate MSVC x64 toolchain. Install Visual Studio 2022 or later with the C++ x64/x86 build tools workload.");
    }

    private static MsvcToolchain? TryCreateToolchain(string vsRoot)
    {
        var vcvars = Path.Combine(vsRoot, "VC", "Auxiliary", "Build", "vcvars64.bat");
        var toolsRoot = Path.Combine(vsRoot, "VC", "Tools", "MSVC");
        if(!File.Exists(vcvars) || !Directory.Exists(toolsRoot))
        {
            return null;
        }

        var version = Directory.GetDirectories(toolsRoot)
            .OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
        if(version is null)
        {
            return null;
        }

        var bin = Path.Combine(version, "bin", "Hostx64", "x64");
        var cl = Path.Combine(bin, "cl.exe");
        var link = Path.Combine(bin, "link.exe");
        var lib = Path.Combine(bin, "lib.exe");
        return File.Exists(cl) && File.Exists(link) && File.Exists(lib)
            ? new MsvcToolchain(vcvars, cl, link, lib)
            : null;
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
