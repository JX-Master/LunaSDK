using LunaBuild.Core.MakeSystem;

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

        foreach(var target in targets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            foreach(var header in target.HeaderFiles.Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(header, Path.Combine(root, "include", InstallHeaderRelativePath(workspace, header)));
            }

            foreach(var generatedHeader in IdeProjectModel.EnumerateProjectFiles(workspace, graph, target)
                .Select(file => file.Path)
                .Where(IdeProjectModel.IsHeader)
                .Where(path => IsUnderDirectory(path, workspace.BuildDirectory))
                .Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(generatedHeader, Path.Combine(root, "include", InstallHeaderRelativePath(workspace, generatedHeader)));
            }

            foreach(var runtimeFile in target.RuntimeFiles.Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(runtimeFile, Path.Combine(root, "bin", Path.GetFileName(runtimeFile)));
            }

            foreach(var output in FindBuildOutputs(workspace, graph, target.Name).Order(StringComparer.OrdinalIgnoreCase))
            {
                filesCopied += CopyRequired(output, Path.Combine(root, OutputSubdirectory(output), Path.GetFileName(output)));
            }
        }

        return new InstallResult(filesCopied);
    }

    private static IEnumerable<string> FindBuildOutputs(BuildWorkspace workspace, BuildGraph graph, string targetName)
    {
        var nodesById = graph.Nodes.ToDictionary(node => node.Id, StringComparer.Ordinal);
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

    private static int CopyRequired(string source, string destination)
    {
        if(!File.Exists(source))
        {
            throw new FileNotFoundException($"Install input is missing. Build the target before installing: {source}", source);
        }

        Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
        File.Copy(source, destination, overwrite: true);
        return 1;
    }

    private static string InstallHeaderRelativePath(BuildWorkspace workspace, string path)
    {
        var relative = workspace.ToRepositoryRelativePath(path);
        const string modulesPrefix = "Modules/";
        if(relative.StartsWith(modulesPrefix, StringComparison.OrdinalIgnoreCase))
        {
            return relative[modulesPrefix.Length..];
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
