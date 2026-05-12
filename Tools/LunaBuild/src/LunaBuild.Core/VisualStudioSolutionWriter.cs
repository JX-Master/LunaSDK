using System.Security.Cryptography;
using System.Text;
using System.Xml.Linq;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Core;

public static class VisualStudioSolutionWriter
{
    private const string VcProjectTypeGuid = "{BC8A1FFA-BEE3-4634-8014-F334798102B3}";
    private static readonly XNamespace Msbuild = "http://schemas.microsoft.com/developer/msbuild/2003";

    public static void Write(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputDirectory)
    {
        Directory.CreateDirectory(outputDirectory);
        CleanGeneratedFiles(outputDirectory);
        var targetMap = targets.ToDictionary(target => target.Name, StringComparer.OrdinalIgnoreCase);
        var orderedTargets = targets.OrderBy(target => target.Name, StringComparer.OrdinalIgnoreCase).ToArray();
        var projectInfos = orderedTargets
            .Select(target => new ProjectInfo(
                target,
                StableGuid("LunaBuild.VS2022.Project:" + target.Name),
                SanitizeFileName(target.Name) + ".vcxproj"))
            .ToArray();
        var projectGuidByName = projectInfos.ToDictionary(info => info.Target.Name, info => info.Guid, StringComparer.OrdinalIgnoreCase);

        foreach(var project in projectInfos)
        {
            var projectPath = Path.Combine(outputDirectory, project.FileName);
            WriteProject(workspace, options, graph, project.Target, project.Guid, projectGuidByName, targetMap, projectPath);
            WriteFilters(workspace, graph, project.Target, projectPath + ".filters");
        }

        WriteSolution(workspace, options, outputDirectory, projectInfos, targetMap, projectGuidByName);
    }

    private static void CleanGeneratedFiles(string outputDirectory)
    {
        foreach(var pattern in new[] { "*.sln", "*.vcxproj", "*.vcxproj.filters" })
        {
            foreach(var file in Directory.EnumerateFiles(outputDirectory, pattern, SearchOption.TopDirectoryOnly))
            {
                File.Delete(file);
            }
        }
    }

    private static void WriteProject(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        BuildTargetDefinition target,
        string projectGuid,
        IReadOnlyDictionary<string, string> projectGuidByName,
        IReadOnlyDictionary<string, BuildTargetDefinition> targetMap,
        string projectPath)
    {
        var configuration = options.Mode.ToString();
        var platform = VisualStudioPlatform(options.Architecture);
        var buildCommand = LunaBuildCommand(workspace, "build", target.Name, options, force: false);
        var rebuildCommand = LunaBuildCommand(workspace, "build", target.Name, options, force: true);
        var cleanCommand = LunaBuildCommand(workspace, "clean", target.Name, options, force: false);
        var output = FindPrimaryOutput(workspace, graph, target.Name) ?? Path.Combine(workspace.BuildDirectory, "VS2022", target.Name + ".stamp");
        var includeSearchPath = string.Join(';', target.IncludeDirectories
            .Concat(new[] { workspace.ResolveRepositoryPath("Modules") })
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase));
        var defines = string.Join(';', target.Defines
            .Concat(new[] { "LUNA_MANUAL_CONFIG_DEBUG_LEVEL" })
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal));

        var document = new XDocument(
            new XElement(Msbuild + "Project",
                new XAttribute("DefaultTargets", "Build"),
                new XAttribute("ToolsVersion", "17.0"),
                new XElement(Msbuild + "ItemGroup",
                    new XAttribute("Label", "ProjectConfigurations"),
                    new XElement(Msbuild + "ProjectConfiguration",
                        new XAttribute("Include", $"{configuration}|{platform}"),
                        new XElement(Msbuild + "Configuration", configuration),
                        new XElement(Msbuild + "Platform", platform))),
                new XElement(Msbuild + "PropertyGroup",
                    new XAttribute("Label", "Globals"),
                    new XElement(Msbuild + "VCProjectVersion", "17.0"),
                    new XElement(Msbuild + "Keyword", "MakeFileProj"),
                    new XElement(Msbuild + "ProjectGuid", projectGuid),
                    new XElement(Msbuild + "RootNamespace", target.Name)),
                new XElement(Msbuild + "Import", new XAttribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props")),
                new XElement(Msbuild + "PropertyGroup",
                    new XAttribute("Condition", $"'$(Configuration)|$(Platform)'=='{configuration}|{platform}'"),
                    new XAttribute("Label", "Configuration"),
                    new XElement(Msbuild + "ConfigurationType", "Makefile"),
                    new XElement(Msbuild + "UseDebugLibraries", options.Mode == BuildMode.Debug ? "true" : "false"),
                    new XElement(Msbuild + "PlatformToolset", "v143")),
                new XElement(Msbuild + "Import", new XAttribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props")),
                new XElement(Msbuild + "PropertyGroup",
                    new XAttribute("Condition", $"'$(Configuration)|$(Platform)'=='{configuration}|{platform}'"),
                    new XElement(Msbuild + "NMakeBuildCommandLine", buildCommand),
                    new XElement(Msbuild + "NMakeReBuildCommandLine", rebuildCommand),
                    new XElement(Msbuild + "NMakeCleanCommandLine", cleanCommand),
                    new XElement(Msbuild + "NMakeOutput", output),
                    new XElement(Msbuild + "NMakePreprocessorDefinitions", defines),
                    new XElement(Msbuild + "NMakeIncludeSearchPath", includeSearchPath)),
                CreateFileItemGroups(workspace, graph, target),
                new XElement(Msbuild + "Import", new XAttribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets"))));

        document.Save(projectPath);
    }

    private static void WriteFilters(BuildWorkspace workspace, BuildGraph graph, BuildTargetDefinition target, string filtersPath)
    {
        var files = EnumerateProjectFiles(workspace, graph, target).ToArray();
        var filters = files
            .Select(file => Path.GetDirectoryName(workspace.ToRepositoryRelativePath(file.Path))?.Replace('/', '\\'))
            .Where(filter => !string.IsNullOrWhiteSpace(filter))
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var document = new XDocument(
            new XElement(Msbuild + "Project",
                new XAttribute("ToolsVersion", "4.0"),
                new XElement(Msbuild + "ItemGroup",
                    filters.Select(filter => new XElement(Msbuild + "Filter",
                        new XAttribute("Include", filter!),
                        new XElement(Msbuild + "UniqueIdentifier", StableGuid("LunaBuild.VS2022.Filter:" + filter))))),
                CreateFilterFileItemGroups(workspace, files)));

        document.Save(filtersPath);
    }

    private static void WriteSolution(
        BuildWorkspace workspace,
        BuildOptions options,
        string outputDirectory,
        IReadOnlyList<ProjectInfo> projects,
        IReadOnlyDictionary<string, BuildTargetDefinition> targetMap,
        IReadOnlyDictionary<string, string> projectGuidByName)
    {
        var configurationPlatform = $"{options.Mode}|{VisualStudioPlatform(options.Architecture)}";
        var solutionPath = Path.Combine(outputDirectory, Path.GetFileName(workspace.RootDirectory) + ".sln");
        var builder = new StringBuilder();
        builder.AppendLine("Microsoft Visual Studio Solution File, Format Version 12.00");
        builder.AppendLine("# Visual Studio Version 17");
        builder.AppendLine("VisualStudioVersion = 17.0.31903.59");
        builder.AppendLine("MinimumVisualStudioVersion = 10.0.40219.1");
        foreach(var project in projects)
        {
            builder.AppendLine($"Project(\"{VcProjectTypeGuid}\") = \"{project.Target.Name}\", \"{project.FileName}\", \"{project.Guid}\"");
            var dependencies = project.Target.Dependencies
                .Where(targetMap.ContainsKey)
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray();
            if(dependencies.Length > 0)
            {
                builder.AppendLine("\tProjectSection(ProjectDependencies) = postProject");
                foreach(var dependency in dependencies)
                {
                    var guid = projectGuidByName[dependency];
                    builder.AppendLine($"\t\t{guid} = {guid}");
                }
                builder.AppendLine("\tEndProjectSection");
            }
            builder.AppendLine("EndProject");
        }
        builder.AppendLine("Global");
        builder.AppendLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
        builder.AppendLine($"\t\t{configurationPlatform} = {configurationPlatform}");
        builder.AppendLine("\tEndGlobalSection");
        builder.AppendLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
        foreach(var project in projects)
        {
            builder.AppendLine($"\t\t{project.Guid}.{configurationPlatform}.ActiveCfg = {configurationPlatform}");
            builder.AppendLine($"\t\t{project.Guid}.{configurationPlatform}.Build.0 = {configurationPlatform}");
        }
        builder.AppendLine("\tEndGlobalSection");
        builder.AppendLine("EndGlobal");
        File.WriteAllText(solutionPath, builder.ToString(), new UTF8Encoding(false));
    }

    private static object[] CreateFileItemGroups(BuildWorkspace workspace, BuildGraph graph, BuildTargetDefinition target)
    {
        return EnumerateProjectFiles(workspace, graph, target)
            .GroupBy(file => file.ItemType)
            .OrderBy(group => group.Key, StringComparer.Ordinal)
            .Select(group => new XElement(Msbuild + "ItemGroup",
                group.OrderBy(file => file.Path, StringComparer.OrdinalIgnoreCase)
                    .Select(file => new XElement(Msbuild + file.ItemType, new XAttribute("Include", file.Path)))))
            .Cast<object>()
            .ToArray();
    }

    private static object[] CreateFilterFileItemGroups(BuildWorkspace workspace, IReadOnlyList<ProjectFile> files)
    {
        return files
            .GroupBy(file => file.ItemType)
            .OrderBy(group => group.Key, StringComparer.Ordinal)
            .Select(group => new XElement(Msbuild + "ItemGroup",
                group.OrderBy(file => file.Path, StringComparer.OrdinalIgnoreCase).Select(file =>
                {
                    var filter = Path.GetDirectoryName(workspace.ToRepositoryRelativePath(file.Path))?.Replace('/', '\\');
                    var element = new XElement(Msbuild + file.ItemType, new XAttribute("Include", file.Path));
                    if(!string.IsNullOrWhiteSpace(filter))
                    {
                        element.Add(new XElement(Msbuild + "Filter", filter));
                    }
                    return element;
                })))
            .Cast<object>()
            .ToArray();
    }

    private static IEnumerable<ProjectFile> EnumerateProjectFiles(BuildWorkspace workspace, BuildGraph graph, BuildTargetDefinition target)
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

        foreach(var file in files)
        {
            yield return new ProjectFile(file, ItemType(file));
        }
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

    private static string? FindPrimaryOutput(BuildWorkspace workspace, BuildGraph graph, string targetName)
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

    private static bool ActionBelongsToTarget(string command, string targetName)
    {
        var payload = ActionPayload.Parse(command);
        var name = payload.Contains("target") ? payload.Required("target") : payload.Contains("name") ? payload.Required("name") : null;
        return string.Equals(name, targetName, StringComparison.OrdinalIgnoreCase);
    }

    private static string LunaBuildCommand(BuildWorkspace workspace, string command, string targetName, BuildOptions options, bool force)
    {
        var args = new List<string>
        {
            "run",
            "--no-restore",
            "--project",
            Quote(Path.Combine(workspace.RootDirectory, "Tools", "LunaBuild", "src", "LunaBuild.Cli", "LunaBuild.Cli.csproj")),
            "--",
            command,
            "--root",
            Quote(workspace.RootDirectory),
            "--target",
            targetName,
            "--mode",
            options.Mode.ToString(),
            "--platform",
            options.Platform.ToString(),
            "--arch",
            options.Architecture,
            "--rhi",
            options.RhiApi.ToString(),
        };
        args.Add(options.Shared ? "--shared" : "--static");
        if(!options.BuildTests)
        {
            args.Add("--no-tests");
        }
        if(force)
        {
            args.Add("--force");
        }
        return "dotnet " + string.Join(' ', args);
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

    private static string ItemType(string path)
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

    private static bool IsHeader(string path)
    {
        var extension = Path.GetExtension(path);
        return extension.Equals(".h", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hh", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hpp", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".hxx", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".inl", StringComparison.OrdinalIgnoreCase);
    }

    private static string VisualStudioPlatform(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "x64" or "x86_64" => "x64",
            "arm64" or "aarch64" => "ARM64",
            "x86" => "Win32",
            _ => architecture,
        };
    }

    private static string StableGuid(string value)
    {
        var hash = MD5.HashData(Encoding.UTF8.GetBytes(value));
        return new Guid(hash).ToString("B").ToUpperInvariant();
    }

    private static string SanitizeFileName(string value)
    {
        var invalid = Path.GetInvalidFileNameChars().ToHashSet();
        return string.Concat(value.Select(ch => invalid.Contains(ch) ? '_' : ch));
    }

    private static string Quote(string value)
    {
        return value.Contains(' ') || value.Contains('\t')
            ? $"\"{value}\""
            : value;
    }

    private sealed record ProjectInfo(BuildTargetDefinition Target, string Guid, string FileName);

    private sealed record ProjectFile(string Path, string ItemType);
}
