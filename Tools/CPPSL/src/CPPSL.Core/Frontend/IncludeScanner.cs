using System.Text.RegularExpressions;
using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed partial class IncludeScanner
{
    private readonly string[] _includeRoots;

    public IncludeScanner(IReadOnlyList<string> includeRoots)
    {
        _includeRoots = includeRoots.Select(NormalizeDirectory).ToArray();
    }

    public IncludeScanResult ScanTransitive(string entryFile)
    {
        var files = new List<string>();
        var diagnostics = new List<CppslDiagnostic>();
        var visited = new HashSet<string>(StringComparer.Ordinal);
        ScanFile(Path.GetFullPath(entryFile), files, diagnostics, visited);
        return new IncludeScanResult(files, diagnostics);
    }

    private void ScanFile(
        string path,
        List<string> files,
        List<CppslDiagnostic> diagnostics,
        HashSet<string> visited)
    {
        path = Path.GetFullPath(path);
        if (!visited.Add(path))
        {
            return;
        }
        files.Add(path);

        var extension = Path.GetExtension(path);
        if (extension is not ".cxx" and not ".hxx")
        {
            diagnostics.Add(CppslDiagnostic.Error("CPPSL files must use `.cxx` or `.hxx`.", path));
            return;
        }

        var lines = File.ReadAllLines(path);
        for (var i = 0; i < lines.Length; ++i)
        {
            var match = IncludeRegex().Match(lines[i]);
            if (!match.Success)
            {
                continue;
            }

            var includeName = match.Groups["path"].Value;
            if (Path.GetExtension(includeName) != ".hxx")
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL can only include `.hxx` headers, but found `{includeName}`.",
                    path,
                    i + 1));
                continue;
            }

            var resolved = ResolveInclude(path, includeName);
            if (resolved is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"Cannot resolve CPPSL include `{includeName}` from configured include roots.",
                    path,
                    i + 1));
                continue;
            }

            if (!IsInsideIncludeRoot(resolved))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"Include `{includeName}` resolves outside CPPSL include roots.",
                    path,
                    i + 1));
                continue;
            }

            ScanFile(resolved, files, diagnostics, visited);
        }
    }

    private string? ResolveInclude(string currentFile, string includeName)
    {
        if (includeName.StartsWith('.'))
        {
            var local = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(currentFile)!, includeName));
            return File.Exists(local) ? local : null;
        }

        foreach (var includeRoot in _includeRoots)
        {
            var candidate = Path.GetFullPath(Path.Combine(includeRoot, includeName));
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }
        return null;
    }

    private bool IsInsideIncludeRoot(string path)
    {
        path = Path.GetFullPath(path);
        return _includeRoots.Any(root => path.StartsWith(root, StringComparison.Ordinal));
    }

    private static string NormalizeDirectory(string path)
    {
        var full = Path.GetFullPath(path);
        return full.EndsWith(Path.DirectorySeparatorChar)
            ? full
            : full + Path.DirectorySeparatorChar;
    }

    [GeneratedRegex("^\\s*#\\s*include\\s*[<\"](?<path>[^>\"]+)[>\"]")]
    private static partial Regex IncludeRegex();
}
