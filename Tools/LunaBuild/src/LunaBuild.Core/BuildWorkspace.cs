namespace LunaBuild.Core;

public sealed class BuildWorkspace
{
    public BuildWorkspace(string rootDirectory, string? buildDirectory = null, string? runnerProjectPath = null)
    {
        RootDirectory = CanonicalizeDirectory(rootDirectory);
        BuildDirectory = CanonicalizePath(buildDirectory ?? Path.Combine(RootDirectory, "build", "LunaBuild"));
        RunnerProjectPath = runnerProjectPath is null ? LocateRunnerProject() : Path.GetFullPath(runnerProjectPath);
    }

    public string RootDirectory { get; }

    public string BuildDirectory { get; }

    public string? RunnerProjectPath { get; }

    public static BuildWorkspace Discover(string? startDirectory = null)
    {
        var current = new DirectoryInfo(Path.GetFullPath(startDirectory ?? Environment.CurrentDirectory));
        while(current is not null)
        {
            if(Directory.EnumerateFiles(current.FullName, "*.Project.cs", SearchOption.TopDirectoryOnly).Any())
            {
                return new BuildWorkspace(current.FullName);
            }
            current = current.Parent;
        }
        throw new DirectoryNotFoundException("Cannot find a LunaBuild project root containing a root-level *.Project.cs file. Pass --root or run under the project.");
    }

    public string ToRepositoryRelativePath(string path)
    {
        return Path.GetRelativePath(RootDirectory, Path.GetFullPath(path)).Replace('\\', '/');
    }

    public string ResolveRepositoryPath(string path)
    {
        return Path.IsPathFullyQualified(path)
            ? Path.GetFullPath(path)
            : Path.GetFullPath(Path.Combine(RootDirectory, path.Replace('/', Path.DirectorySeparatorChar)));
    }

    internal static string CanonicalizeDirectory(string path)
    {
        var fullPath = Path.GetFullPath(path);
        if(!Directory.Exists(fullPath))
        {
            throw new DirectoryNotFoundException($"Project root does not exist: {fullPath}");
        }

        return ResolveExistingDirectory(fullPath);
    }

    private static string CanonicalizePath(string path)
    {
        var fullPath = Path.GetFullPath(path);
        var missingComponents = new Stack<string>();
        var existingPath = fullPath;
        while(!Directory.Exists(existingPath))
        {
            var name = Path.GetFileName(existingPath);
            var parent = Path.GetDirectoryName(existingPath);
            if(string.IsNullOrEmpty(name) || string.IsNullOrEmpty(parent))
            {
                return fullPath;
            }
            missingComponents.Push(name);
            existingPath = parent;
        }

        var canonicalPath = ResolveExistingDirectory(existingPath);
        while(missingComponents.TryPop(out var component))
        {
            canonicalPath = Path.Combine(canonicalPath, component);
        }
        return Path.GetFullPath(canonicalPath);
    }

    private static string ResolveExistingDirectory(string fullPath)
    {
        var root = Path.GetPathRoot(fullPath)
            ?? throw new InvalidOperationException($"Project root has no filesystem root: {fullPath}");
        var current = root;
        foreach(var component in Path.GetRelativePath(root, fullPath).Split(
            new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
            StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = new DirectoryInfo(Path.Combine(current, component));
            current = candidate.ResolveLinkTarget(returnFinalTarget: true)?.FullName ?? candidate.FullName;
        }
        return Path.GetFullPath(current);
    }

    private static string? LocateRunnerProject()
    {
        foreach(var start in new[] { Environment.CurrentDirectory, AppContext.BaseDirectory })
        {
            var current = new DirectoryInfo(Path.GetFullPath(start));
            while(current is not null)
            {
                var candidate = Path.Combine(current.FullName, "LunaBuild.csproj");
                if(File.Exists(candidate))
                {
                    return candidate;
                }
                current = current.Parent;
            }
        }
        return null;
    }
}
