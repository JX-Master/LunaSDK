namespace LunaBuild.Core.MakeSystem;

internal sealed record MsvcToolchain(string VcVarsBat, string ClExe, string LinkExe, string LibExe);

internal static class MsvcToolchainLocator
{
    public static MsvcToolchain Locate()
    {
        var roots = new[]
        {
            Environment.GetEnvironmentVariable("ProgramFiles"),
            Environment.GetEnvironmentVariable("ProgramFiles(x86)"),
        }
        .Where(root => !string.IsNullOrWhiteSpace(root))
        .Select(root => Path.Combine(root!, "Microsoft Visual Studio", "2022"))
        .Where(Directory.Exists)
        .ToArray();

        foreach(var root in roots)
        {
            foreach(var edition in new[] { "Community", "Professional", "Enterprise", "BuildTools" })
            {
                var vsRoot = Path.Combine(root, edition);
                var vcvars = Path.Combine(vsRoot, "VC", "Auxiliary", "Build", "vcvars64.bat");
                var toolsRoot = Path.Combine(vsRoot, "VC", "Tools", "MSVC");
                if(!File.Exists(vcvars) || !Directory.Exists(toolsRoot))
                {
                    continue;
                }

                var version = Directory.GetDirectories(toolsRoot)
                    .OrderByDescending(Path.GetFileName, StringComparer.OrdinalIgnoreCase)
                    .FirstOrDefault();
                if(version is null)
                {
                    continue;
                }

                var bin = Path.Combine(version, "bin", "Hostx64", "x64");
                var cl = Path.Combine(bin, "cl.exe");
                var link = Path.Combine(bin, "link.exe");
                var lib = Path.Combine(bin, "lib.exe");
                if(File.Exists(cl) && File.Exists(link) && File.Exists(lib))
                {
                    return new MsvcToolchain(vcvars, cl, link, lib);
                }
            }
        }

        throw new MakeSystemException("Cannot locate Visual Studio 2022 MSVC x64 toolchain.");
    }
}
