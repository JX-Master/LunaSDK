namespace LunaBuild.Core;

internal static class XmakePackageResolver
{
    private sealed record PackageProbes(string[] Headers, string[] Libraries);

    private static readonly IReadOnlyDictionary<string, PackageProbes> Probes = new Dictionary<string, PackageProbes>(StringComparer.OrdinalIgnoreCase)
    {
        ["stb"] = new(new[] { "stb_image.h", "stb_image_write.h", "stb_truetype.h" }, Array.Empty<string>()),
        ["miniaudio"] = new(new[] { "miniaudio.h" }, Array.Empty<string>()),
        ["d3d12-memory-allocator"] = new(new[] { "D3D12MemAlloc.h" }, new[] { "D3D12MA.lib" }),
        ["volk"] = new(new[] { "volk.h" }, Array.Empty<string>()),
        ["vulkan-memory-allocator"] = new(new[] { "vk_mem_alloc.h" }, Array.Empty<string>()),
    };

    public static IReadOnlyList<string> ResolveIncludeDirectories(IReadOnlyList<string> packageNames)
    {
        var includes = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach(var packageName in packageNames.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            foreach(var include in ResolveIncludeDirectories(packageName))
            {
                includes.Add(include);
            }
        }
        return includes.ToArray();
    }

    public static IReadOnlyList<string> ResolveLinkLibraries(IReadOnlyList<string> packageNames)
    {
        var libraries = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach(var packageName in packageNames.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            foreach(var library in ResolveLinkLibraries(packageName))
            {
                libraries.Add(library);
            }
        }
        return libraries.ToArray();
    }

    private static IEnumerable<string> ResolveIncludeDirectories(string packageName)
    {
        var probes = GetProbes(packageName);
        var packageRoot = FindPackageRoot(packageName);
        if(packageRoot is null)
        {
            throw new DirectoryNotFoundException($"Cannot find installed xmake package `{packageName}`. Run xmake package install or add a LunaBuild package resolver entry.");
        }

        foreach(var probe in probes.Headers)
        {
            var header = FindNewestFile(packageRoot, probe);
            if(header is not null)
            {
                yield return Path.GetDirectoryName(header)!;
            }
        }
    }

    private static IEnumerable<string> ResolveLinkLibraries(string packageName)
    {
        var probes = GetProbes(packageName);
        var packageRoot = FindPackageRoot(packageName);
        if(packageRoot is null)
        {
            throw new DirectoryNotFoundException($"Cannot find installed xmake package `{packageName}`. Run xmake package install or add a LunaBuild package resolver entry.");
        }

        foreach(var probe in probes.Libraries)
        {
            var library = FindNewestFile(packageRoot, probe);
            if(library is not null)
            {
                yield return library;
            }
        }
    }

    private static PackageProbes GetProbes(string packageName)
    {
        if(Probes.TryGetValue(packageName, out var probes))
        {
            return probes;
        }
        throw new InvalidOperationException($"Package `{packageName}` is not supported by LunaBuild package resolver yet.");
    }

    private static string? FindNewestFile(string root, string fileName)
    {
        return Directory.EnumerateFiles(root, fileName, SearchOption.AllDirectories)
            .OrderByDescending(path => path, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
    }

    private static string? FindPackageRoot(string packageName)
    {
        foreach(var root in CandidatePackageRoots())
        {
            var packageRoot = Path.Combine(root, packageName[0].ToString(), packageName);
            if(Directory.Exists(packageRoot))
            {
                return packageRoot;
            }
        }
        return null;
    }

    private static IEnumerable<string> CandidatePackageRoots()
    {
        var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        if(!string.IsNullOrWhiteSpace(localAppData))
        {
            yield return Path.Combine(localAppData, ".xmake", "packages");
        }

        var userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        if(!string.IsNullOrWhiteSpace(userProfile))
        {
            yield return Path.Combine(userProfile, ".xmake", "packages");
        }
    }
}
