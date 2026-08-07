using System.Text;
using System.Xml.Linq;

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
        var targetMap = targets.ToDictionary(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase);
        var orderedTargets = targets.OrderBy(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase).ToArray();
        var projectInfos = orderedTargets
            .Select(target => new ProjectInfo(
                target,
                IdeProjectModel.StableGuid("LunaBuild.VS2022.Project:" + target.QualifiedName),
                IdeProjectModel.SanitizeFileName(target.QualifiedName) + ".vcxproj"))
            .ToArray();
        var projectGuidByName = projectInfos.ToDictionary(info => info.Target.QualifiedName, info => info.Guid, StringComparer.OrdinalIgnoreCase);

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
        var buildCommand = IdeProjectModel.LunaBuildCommand(workspace, "build", target.QualifiedName, options, all: false, force: false);
        var rebuildCommand = IdeProjectModel.LunaBuildCommand(workspace, "build", target.QualifiedName, options, all: false, force: true);
        var cleanCommand = IdeProjectModel.LunaBuildCommand(workspace, "clean", target.QualifiedName, options, all: false, force: false);
        var output = IdeProjectModel.FindPrimaryOutput(workspace, graph, target.QualifiedName) ?? Path.Combine(workspace.BuildDirectory, "VS2022", target.ProjectName + "." + target.Name + ".stamp");
        var includeSearchPath = string.Join(';', target.IncludeDirectories
            .Concat(target.Options.GlobalIncludeDirectories)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase));
        var defines = string.Join(';', target.Defines
            .Concat(target.Options.GlobalDefines)
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
        var files = IdeProjectModel.EnumerateProjectFiles(workspace, graph, target).ToArray();
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
                        new XElement(Msbuild + "UniqueIdentifier", IdeProjectModel.StableGuid("LunaBuild.VS2022.Filter:" + filter))))),
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
            builder.AppendLine($"Project(\"{VcProjectTypeGuid}\") = \"{project.Target.QualifiedName}\", \"{project.FileName}\", \"{project.Guid}\"");
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
        return IdeProjectModel.EnumerateProjectFiles(workspace, graph, target)
            .GroupBy(file => file.ItemType)
            .OrderBy(group => group.Key, StringComparer.Ordinal)
            .Select(group => new XElement(Msbuild + "ItemGroup",
                group.OrderBy(file => file.Path, StringComparer.OrdinalIgnoreCase)
                    .Select(file => new XElement(Msbuild + file.ItemType, new XAttribute("Include", file.Path)))))
            .Cast<object>()
            .ToArray();
    }

    private static object[] CreateFilterFileItemGroups(BuildWorkspace workspace, IReadOnlyList<IdeProjectFile> files)
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

    private sealed record ProjectInfo(BuildTargetDefinition Target, string Guid, string FileName);
}
