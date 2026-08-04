using LunaBuild.Core.MakeSystem;
using System.Security.Cryptography;

namespace LunaBuild.Core;

public static class InstallWriter
{
    public static InstallResult Install(
        BuildWorkspace workspace,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputDirectory)
    {
        var root = Path.GetFullPath(outputDirectory);
        Directory.CreateDirectory(root);
        var filesCopied = 0;
        var installedFiles = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        foreach(var target in targets.OrderBy(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase))
        {
            foreach(var header in target.HeaderFiles.Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(header, Path.Combine(root, "include", InstallHeaderRelativePath(target, header)), installedFiles);
            }

            foreach(var generatedHeader in IdeProjectModel.EnumerateProjectFiles(workspace, graph, target)
                .Select(file => file.Path)
                .Where(IdeProjectModel.IsHeader)
                .Where(path => IsUnderDirectory(path, target.ProjectBuildDirectory))
                .Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(generatedHeader, Path.Combine(root, "include", InstallHeaderRelativePath(target, generatedHeader)), installedFiles);
            }

            foreach(var runtimeFile in target.RuntimeFiles.Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(runtimeFile, Path.Combine(root, "bin", Path.GetFileName(runtimeFile)), installedFiles);
            }

            foreach(var output in FindBuildOutputs(workspace, graph, target.QualifiedName).Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(output, Path.Combine(root, OutputSubdirectory(output), Path.GetFileName(output)), installedFiles);
            }
        }

        return new InstallResult(filesCopied);
    }

    private static IEnumerable<string> FindBuildOutputs(BuildWorkspace workspace, BuildGraph graph, string targetName)
    {
        var nodesById = graph.Nodes.ToDictionary(node => node.Id, StringComparer.Ordinal);
        if(nodesById.TryGetValue(BuildGraphIds.Target(targetName), out var targetNode))
        {
            foreach(var dependencyId in targetNode.Dependencies)
            {
                if(nodesById.TryGetValue(dependencyId, out var dependency) &&
                    dependency.Kind == BuildGraphNodeKind.File && dependency.Path is not null &&
                    dependency.Command is not null && BuildActionKind.Extract(dependency.Command) == "file.copy")
                {
                    yield return workspace.ResolveRepositoryPath(dependency.Path);
                }
            }
        }
        foreach(var node in graph.Nodes)
        {
            if(string.IsNullOrWhiteSpace(node.Command) || !IsBuildOutputAction(node.Command) || !ActionBelongsToTarget(node.Command, targetName))
            {
                continue;
            }

            foreach(var candidate in new[] { node.Id }.Concat(node.Outputs))
            {
                if(nodesById.TryGetValue(candidate, out var outputNode) && outputNode.Kind == BuildGraphNodeKind.File && outputNode.Path is not null)
                {
                    yield return workspace.ResolveRepositoryPath(outputNode.Path);
                }
            }
        }
    }

    private static bool IsBuildOutputAction(string command)
    {
        return BuildActionKind.Extract(command) is "cpp.link.executable" or "cpp.link.shared" or "cpp.link.static" or "dotnet.build";
    }

    private static bool ActionBelongsToTarget(string command, string targetName)
    {
        var payload = ActionPayload.Parse(command);
        var name = payload.Contains("target") ? payload.Required("target") : payload.Contains("name") ? payload.Required("name") : null;
        return string.Equals(name, targetName, StringComparison.OrdinalIgnoreCase);
    }

    private static int CopyRequired(
        string source,
        string destination,
        IDictionary<string, string> installedFiles)
    {
        if(!File.Exists(source))
        {
            throw new FileNotFoundException($"Install input is missing. Build the target before installing: {source}", source);
        }

        var fullDestination = Path.GetFullPath(destination);
        if(installedFiles.TryGetValue(fullDestination, out var existingSource))
        {
            if(Path.GetFullPath(existingSource).Equals(Path.GetFullPath(source), StringComparison.OrdinalIgnoreCase) ||
                FilesEqual(existingSource, source))
            {
                return 0;
            }
            throw new InvalidOperationException(
                $"Install destination collision: `{fullDestination}` is produced by both `{existingSource}` and `{source}`.");
        }
        installedFiles.Add(fullDestination, source);
        Directory.CreateDirectory(Path.GetDirectoryName(fullDestination)!);
        File.Copy(source, fullDestination, overwrite: true);
        return 1;
    }

    private static bool FilesEqual(string left, string right)
    {
        var leftInfo = new FileInfo(left);
        var rightInfo = new FileInfo(right);
        if(!leftInfo.Exists || !rightInfo.Exists || leftInfo.Length != rightInfo.Length)
        {
            return false;
        }
        using var leftStream = File.OpenRead(left);
        using var rightStream = File.OpenRead(right);
        return SHA256.HashData(leftStream).SequenceEqual(SHA256.HashData(rightStream));
    }

    private static string InstallHeaderRelativePath(BuildTargetDefinition target, string path)
    {
        var root = IsUnderDirectory(path, target.ProjectBuildDirectory)
            ? target.ProjectBuildDirectory
            : target.ProjectRootDirectory;
        var relative = Path.GetRelativePath(root, Path.GetFullPath(path)).Replace('\\', '/');
        if(relative.StartsWith("..", StringComparison.Ordinal) || Path.IsPathRooted(relative))
        {
            throw new InvalidOperationException(
                $"Install header `{path}` escapes project `{target.ProjectName}` source/build roots.");
        }
        var publicRoot = target.Options.GlobalIncludeDirectories
            .Where(directory => IsUnderDirectory(path, directory))
            .OrderByDescending(directory => directory.Length)
            .FirstOrDefault();
        if(publicRoot is not null)
        {
            return Path.GetRelativePath(publicRoot, path);
        }

        const string generatedSegment = "/generated/";
        var generatedIndex = relative.IndexOf(generatedSegment, StringComparison.OrdinalIgnoreCase);
        if(generatedIndex >= 0)
        {
            return Path.Combine("Generated", relative[(generatedIndex + generatedSegment.Length)..].Replace('/', Path.DirectorySeparatorChar));
        }

        return relative.Replace('/', Path.DirectorySeparatorChar);
    }

    private static string OutputSubdirectory(string path)
    {
        var extension = Path.GetExtension(path);
        if(extension.Equals(".lib", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".a", StringComparison.OrdinalIgnoreCase))
        {
            return "lib";
        }
        return "bin";
    }

    private static bool IsUnderDirectory(string path, string directory)
    {
        var relative = Path.GetRelativePath(Path.GetFullPath(directory), Path.GetFullPath(path));
        return !relative.StartsWith("..", StringComparison.Ordinal) && !Path.IsPathRooted(relative);
    }
}

public sealed record InstallResult(int FilesCopied);
