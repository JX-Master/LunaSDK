namespace LunaBuild.Core;

public sealed class BuildWorkspace
{
    public BuildWorkspace(string rootDirectory)
    {
        RootDirectory = Path.GetFullPath(rootDirectory);
        ModulesDirectory = Path.Combine(RootDirectory, "Modules", "Luna");
        BuildDirectory = Path.Combine(RootDirectory, "build", "LunaBuild");
    }

    public string RootDirectory { get; }

    public string ModulesDirectory { get; }

    public string BuildDirectory { get; }

    public static BuildWorkspace Discover(string? startDirectory = null)
    {
        var current = new DirectoryInfo(Path.GetFullPath(startDirectory ?? Environment.CurrentDirectory));
        while(current is not null)
        {
            if(Directory.Exists(Path.Combine(current.FullName, "Modules", "Luna")) &&
                Directory.Exists(Path.Combine(current.FullName, "Tools", "LunaBuild")))
            {
                return new BuildWorkspace(current.FullName);
            }
            current = current.Parent;
        }
        throw new DirectoryNotFoundException("Cannot find LunaSDK root. Pass --root or run under the repository.");
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
}
