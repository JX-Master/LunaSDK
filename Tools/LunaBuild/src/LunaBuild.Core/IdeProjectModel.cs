using System.Security.Cryptography;
using System.Text;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Core;

internal static class IdeProjectModel
{
    public static IReadOnlyList<IdeProjectFile> EnumerateProjectFiles(
        BuildWorkspace workspace,
        BuildGraph graph,
        BuildTargetDefinition target)
    {
        var files = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
        Add(files, target.ScriptPath);
        Add(files, target.SourceFiles);
        Add(files, target.HeaderFiles);
        Add(files, target.RuntimeFiles);
        Add(files, target.EmbeddedHeaders.Select(header => header.SourceFile));
        Add(files, target.Shaders.Select(shader => shader.SourceFile));
        if(target.DotNetProjectFile is not null)
        {
            Add(files, target.DotNetProjectFile);
        }
        foreach(var generatedHeader in FindGeneratedHeaders(workspace, graph, target.Name))
        {
            Add(files, generatedHeader);
        }

        return files
            .Select(file => new IdeProjectFile(file, VisualStudioItemType(file)))
            .ToArray();
    }

    public static string? FindPrimaryOutput(BuildWorkspace workspace, BuildGraph graph, string targetName)
    {
        foreach(var node in graph.Nodes)
        {
            if(node.Command is null)
            {
                continue;
            }
            var kind = BuildActionKind.Extract(node.Command);
            if(kind is not ("cpp.link.executable" or "cpp.link.shared" or "cpp.link.static" or "dotnet.build"))
            {
                continue;
            }
            if(!ActionBelongsToTarget(node.Command, targetName))
            {
                continue;
            }
            if(node.Path is not null)
            {
                return workspace.ResolveRepositoryPath(node.Path);
            }
        }
        return null;
    }

    public static IEnumerable<(string TargetName, string OutputPath)> FindExecutableOutputs(
        BuildWorkspace workspace,
        BuildGraph graph)
    {
        foreach(var node in graph.Nodes)
        {
            if(node.Command is null || BuildActionKind.Extract(node.Command) != "cpp.link.executable" || node.Path is null)
            {
                continue;
            }
            var payload = ActionPayload.Parse(node.Command);
            yield return (payload.Required("target"), workspace.ResolveRepositoryPath(node.Path));
        }
    }

    public static string LunaBuildCommand(
        BuildWorkspace workspace,
        string command,
        string? targetName,
        BuildOptions options,
        bool all,
        bool force)
    {
        return "dotnet " + string.Join(' ', LunaBuildArguments(
            workspace,
            options,
            command,
            targetName,
            all,
            force,
            Path.Combine(workspace.RootDirectory, "LunaBuild.csproj"),
            workspace.RootDirectory).Select(Quote));
    }

    public static IReadOnlyList<string> LunaBuildArguments(
        BuildWorkspace workspace,
        BuildOptions options,
        string command,
        string? targetName,
        bool all,
        bool force,
        string projectPath,
        string rootPath)
    {
        var args = new List<string>
        {
            "run",
            "--no-restore",
            "--project",
            projectPath,
            "--",
            command,
            "--root",
            rootPath,
        };
        if(all)
        {
            args.Add("--all");
        }
        else if(targetName is not null)
        {
            args.Add("--target");
            args.Add(targetName);
        }
        args.AddRange(CommonBuildOptionArguments(options));
        if(force)
        {
            args.Add("--force");
        }
        return args;
    }

    public static IReadOnlyList<string> CommonBuildOptionArguments(BuildOptions options)
    {
        var args = new List<string>
        {
            "--mode",
            options.Mode.ToString(),
            "--platform",
            options.Platform.ToString(),
            "--arch",
            options.Architecture,
            "--rhi",
            options.RhiApi.ToString(),
            options.Shared ? "--shared" : "--static",
        };
        foreach(var property in options.Properties.NonDefaultValues())
        {
            args.Add("--property");
            args.Add($"{property.Name}={property.Value}");
        }
        return args;
    }

    public static string StableGuid(string value)
    {
        var hash = MD5.HashData(Encoding.UTF8.GetBytes(value));
        return new Guid(hash).ToString("B").ToUpperInvariant();
    }

    public static string StableXcodeId(string value)
    {
        var hash = SHA1.HashData(Encoding.UTF8.GetBytes(value));
        return Convert.ToHexString(hash, 0, 12);
    }

    public static string SanitizeFileName(string value)
    {
        var invalid = Path.GetInvalidFileNameChars().ToHashSet();
        return string.Concat(value.Select(ch => invalid.Contains(ch) ? '_' : ch));
    }

    public static bool IsHeader(string path)
    {
        var extension = Path.GetExtension(path);
        return extension.Equals(".h", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hh", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hpp", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hxx", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".inl", StringComparison.OrdinalIgnoreCase);
    }

    public static string Quote(string value)
    {
        return value.Contains(' ') || value.Contains('\t')
            ? $"\"{value}\""
            : value;
    }

    private static IEnumerable<string> FindGeneratedHeaders(BuildWorkspace workspace, BuildGraph graph, string targetName)
    {
        var nodesById = graph.Nodes.ToDictionary(node => node.Id, StringComparer.Ordinal);
        foreach(var node in graph.Nodes)
        {
            if(string.IsNullOrWhiteSpace(node.Command) || !ActionBelongsToTarget(node.Command, targetName))
            {
                continue;
            }

            foreach(var candidate in new[] { node.Id }.Concat(node.Outputs))
            {
                if(!nodesById.TryGetValue(candidate, out var fileNode) || fileNode.Kind != BuildGraphNodeKind.File || fileNode.Path is null)
                {
                    continue;
                }
                var path = workspace.ResolveRepositoryPath(fileNode.Path);
                if(IsHeader(path))
                {
                    yield return path;
                }
            }
        }
    }

    private static bool ActionBelongsToTarget(string command, string targetName)
    {
        var payload = ActionPayload.Parse(command);
        var name = payload.Contains("target") ? payload.Required("target") : payload.Contains("name") ? payload.Required("name") : null;
        return string.Equals(name, targetName, StringComparison.OrdinalIgnoreCase);
    }

    private static void Add(SortedSet<string> files, IEnumerable<string> values)
    {
        foreach(var value in values)
        {
            Add(files, value);
        }
    }

    private static void Add(SortedSet<string> files, string? value)
    {
        if(!string.IsNullOrWhiteSpace(value))
        {
            files.Add(Path.GetFullPath(value));
        }
    }

    private static string VisualStudioItemType(string path)
    {
        var extension = Path.GetExtension(path);
        if(IsHeader(path))
        {
            return "ClInclude";
        }
        if(extension.Equals(".c", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cpp", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cc", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".cxx", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".m", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".mm", StringComparison.OrdinalIgnoreCase))
        {
            return "ClCompile";
        }
        if(extension.Equals(".rc", StringComparison.OrdinalIgnoreCase))
        {
            return "ResourceCompile";
        }
        return "None";
    }
}

internal sealed record IdeProjectFile(string Path, string ItemType);
