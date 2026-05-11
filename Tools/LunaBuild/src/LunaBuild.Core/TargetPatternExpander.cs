namespace LunaBuild.Core;

internal static class TargetPatternExpander
{
    public static IReadOnlyList<string> ExpandPatterns(string baseDirectory, IReadOnlyList<string> patterns)
    {
        var files = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach(var pattern in patterns)
        {
            foreach(var file in ExpandPattern(baseDirectory, pattern))
            {
                files.Add(file);
            }
        }
        return files.ToArray();
    }

    private static IEnumerable<string> ExpandPattern(string baseDirectory, string pattern)
    {
        var normalized = pattern.Trim();
        if(normalized.StartsWith('(') && normalized.EndsWith(')'))
        {
            normalized = normalized[1..^1];
        }
        normalized = normalized.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);

        if(!normalized.Contains('*'))
        {
            var path = Path.GetFullPath(Path.Combine(baseDirectory, normalized));
            if(File.Exists(path))
            {
                yield return path;
            }
            yield break;
        }

        var recursive = normalized.Contains("**", StringComparison.Ordinal);
        var marker = $"{Path.DirectorySeparatorChar}**";
        var markerIndex = normalized.IndexOf(marker, StringComparison.Ordinal);
        string root;
        string searchPattern;
        if(markerIndex >= 0)
        {
            root = Path.Combine(baseDirectory, normalized[..markerIndex]);
            searchPattern = normalized[(markerIndex + marker.Length)..].TrimStart(Path.DirectorySeparatorChar);
        }
        else
        {
            var directory = Path.GetDirectoryName(normalized);
            root = string.IsNullOrEmpty(directory) ? baseDirectory : Path.Combine(baseDirectory, directory);
            searchPattern = Path.GetFileName(normalized);
        }

        searchPattern = searchPattern.Replace("**", "*");
        if(searchPattern.StartsWith(".", StringComparison.Ordinal))
        {
            searchPattern = "*" + searchPattern;
        }
        if(string.IsNullOrWhiteSpace(searchPattern))
        {
            searchPattern = "*";
        }
        if(!Directory.Exists(root))
        {
            yield break;
        }

        foreach(var file in Directory.EnumerateFiles(root, searchPattern, recursive ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly))
        {
            yield return Path.GetFullPath(file);
        }
    }
}
