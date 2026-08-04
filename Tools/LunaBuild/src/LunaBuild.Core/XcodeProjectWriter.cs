using System.Text;
using System.Xml.Linq;

namespace LunaBuild.Core;

public static class XcodeProjectWriter
{
    public static void Write(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        string outputPath)
    {
        var paths = ResolvePaths(workspace, outputPath);
        Directory.CreateDirectory(paths.OutputDirectory);
        Directory.CreateDirectory(paths.ProjectDirectory);
        CleanGeneratedProject(paths.ProjectDirectory);

        var orderedTargets = targets.OrderBy(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase).ToArray();
        var targetMap = orderedTargets.ToDictionary(target => target.QualifiedName, StringComparer.OrdinalIgnoreCase);
        var scriptPath = Path.Combine(paths.OutputDirectory, "lunabuild-xcode.sh");
        WriteHelperScript(scriptPath, workspace, options);

        var model = CreateModel(workspace, options, graph, orderedTargets, targetMap, paths, scriptPath);
        File.WriteAllText(Path.Combine(paths.ProjectDirectory, "project.pbxproj"), model.ProjectText, new UTF8Encoding(false));
        WriteSchemes(paths.ProjectDirectory, paths.ProjectFileName, model.Schemes);
    }

    private static XcodeModel CreateModel(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        IReadOnlyList<BuildTargetDefinition> targets,
        IReadOnlyDictionary<string, BuildTargetDefinition> targetMap,
        XcodePaths paths,
        string scriptPath)
    {
        var objects = new List<XcodeObject>();
        var schemes = new List<XcodeScheme>();
        var projectId = Id("Project");
        var mainGroupId = Id("Group:Main");
        var targetsGroupId = Id("Group:Targets");
        var productsGroupId = Id("Group:Products");
        var supportGroupId = Id("Group:LunaBuild");
        var scriptFileId = Id("File:LunaBuildScript");
        var projectConfigListId = Id("ConfigList:Project");
        var projectConfigId = Id("Config:Project:" + options.Mode);

        var targetInfos = targets
            .Select(target => CreateTargetInfo(workspace, options, graph, paths, scriptPath, target))
            .ToArray();
        var utilityTargets = CreateUtilityTargets(paths, scriptPath, options);
        var allInfos = targetInfos.Concat(utilityTargets).ToArray();
        var targetIdByName = targetInfos.ToDictionary(info => info.Name, info => info.TargetId, StringComparer.OrdinalIgnoreCase);

        foreach(var info in targetInfos)
        {
            AddTargetFileObjects(workspace, graph, objects, info.Target!, info.SourceGroupId);
            objects.Add(FileReference(info.ProductFileId, info.ProductName, info.ProductPath, ProductFileType(info.Target!.Kind, info.ProductPath)));
            objects.Add(TargetBuildConfiguration(workspace, info.ConfigId, options, info.Target));
            objects.Add(ConfigurationList(info.ConfigListId, info.ConfigId, info.Name, options.Mode.ToString()));
        }

        foreach(var info in utilityTargets)
        {
            objects.Add(FileReference(info.ProductFileId, info.ProductName, info.ProductPath, "text"));
            objects.Add(UtilityBuildConfiguration(info.ConfigId, options, info.Name));
            objects.Add(ConfigurationList(info.ConfigListId, info.ConfigId, info.Name, options.Mode.ToString()));
        }

        foreach(var info in allInfos)
        {
            objects.Add(LegacyTarget(info, projectId, targetMap, targetIdByName));
            schemes.Add(new XcodeScheme(info.Name, info.TargetId, info.ProductName, paths.ProjectFileName));
        }

        objects.Add(FileReference(scriptFileId, Path.GetFileName(scriptPath), scriptPath, "text.script.sh"));
        objects.Add(new XcodeObject(
            supportGroupId,
            "LunaBuild",
            string.Join('\n',
                "\t\t\tisa = PBXGroup;",
                $"\t\t\tchildren = ({Ref(scriptFileId, Path.GetFileName(scriptPath))},);",
                "\t\t\tname = LunaBuild;",
                "\t\t\tsourceTree = \"<group>\";")));
        objects.Add(new XcodeObject(
            targetsGroupId,
            "Targets",
            string.Join('\n',
                "\t\t\tisa = PBXGroup;",
                "\t\t\tchildren = (",
                string.Concat(targetInfos.Select(info => $"\t\t\t\t{Ref(info.SourceGroupId, info.Name)},\n")),
                "\t\t\t);",
                "\t\t\tname = Targets;",
                "\t\t\tsourceTree = \"<group>\";")));
        objects.Add(new XcodeObject(
            productsGroupId,
            "Products",
            string.Join('\n',
                "\t\t\tisa = PBXGroup;",
                "\t\t\tchildren = (",
                string.Concat(allInfos.Select(info => $"\t\t\t\t{Ref(info.ProductFileId, info.ProductName)},\n")),
                "\t\t\t);",
                "\t\t\tname = Products;",
                "\t\t\tsourceTree = \"<group>\";")));
        objects.Add(new XcodeObject(
            mainGroupId,
            Path.GetFileName(workspace.RootDirectory),
            string.Join('\n',
                "\t\t\tisa = PBXGroup;",
                "\t\t\tchildren = (",
                $"\t\t\t\t{Ref(targetsGroupId, "Targets")},",
                $"\t\t\t\t{Ref(productsGroupId, "Products")},",
                $"\t\t\t\t{Ref(supportGroupId, "LunaBuild")},",
                "\t\t\t);",
                $"\t\t\tname = {PbxString(Path.GetFileName(workspace.RootDirectory))};",
                "\t\t\tsourceTree = \"<group>\";")));
        objects.Add(ProjectBuildConfiguration(projectConfigId, options));
        objects.Add(ConfigurationList(projectConfigListId, projectConfigId, "Project", options.Mode.ToString()));
        objects.Add(ProjectObject(projectId, mainGroupId, productsGroupId, projectConfigListId, allInfos, options));

        return new XcodeModel(BuildProjectText(objects, projectId), schemes);
    }

    private static XcodeTargetInfo CreateTargetInfo(
        BuildWorkspace workspace,
        BuildOptions options,
        BuildGraph graph,
        XcodePaths paths,
        string scriptPath,
        BuildTargetDefinition target)
    {
        var outputPath = IdeProjectModel.FindPrimaryOutput(workspace, graph, target.QualifiedName)
            ?? Path.Combine(paths.OutputDirectory, IdeProjectModel.SanitizeFileName(target.QualifiedName) + ".stamp");
        return new XcodeTargetInfo(
            Name: target.QualifiedName,
            Target: target,
            TargetId: Id("Target:" + target.QualifiedName),
            ProductFileId: Id("Product:" + target.QualifiedName),
            ProductName: Path.GetFileName(outputPath),
            ProductPath: outputPath,
            SourceGroupId: Id("Group:Target:" + target.QualifiedName),
            ConfigListId: Id("ConfigList:Target:" + target.QualifiedName),
            ConfigId: Id("Config:Target:" + target.QualifiedName + ":" + target.Options.Mode),
            BuildArguments: $"{ShellQuote(scriptPath)} target {ShellQuote(target.QualifiedName)}");
    }

    private static IReadOnlyList<XcodeTargetInfo> CreateUtilityTargets(XcodePaths paths, string scriptPath, BuildOptions options)
    {
        var utilityNames = new[]
        {
            ("LunaBuild-All", "all-build"),
            ("LunaBuild-Rebuild-All", "all-rebuild"),
            ("LunaBuild-Clean-All", "all-clean"),
            ("LunaBuild-Clean-Full", "full-clean"),
        };
        return utilityNames.Select(pair =>
        {
            var productPath = Path.Combine(paths.OutputDirectory, pair.Item1 + ".stamp");
            return new XcodeTargetInfo(
                Name: pair.Item1,
                Target: null,
                TargetId: Id("Target:" + pair.Item1),
                ProductFileId: Id("Product:" + pair.Item1),
                ProductName: Path.GetFileName(productPath),
                ProductPath: productPath,
                SourceGroupId: Id("Group:Target:" + pair.Item1),
                ConfigListId: Id("ConfigList:Target:" + pair.Item1),
                ConfigId: Id("Config:Target:" + pair.Item1 + ":" + options.Mode),
                BuildArguments: $"{ShellQuote(scriptPath)} {pair.Item2}");
        }).ToArray();
    }

    private static void AddTargetFileObjects(
        BuildWorkspace workspace,
        BuildGraph graph,
        List<XcodeObject> objects,
        BuildTargetDefinition target,
        string groupId)
    {
        var files = IdeProjectModel.EnumerateProjectFiles(workspace, graph, target)
            .OrderBy(file => workspace.ToRepositoryRelativePath(file.Path), StringComparer.OrdinalIgnoreCase)
            .ToArray();
        var childRefs = new StringBuilder();
        foreach(var file in files)
        {
            var fileId = Id("File:" + target.QualifiedName + ":" + file.Path);
            var name = Path.GetFileName(file.Path);
            objects.Add(FileReference(fileId, name, file.Path, XcodeFileType(file.Path)));
            childRefs.Append("\t\t\t\t");
            childRefs.Append(Ref(fileId, name));
            childRefs.AppendLine(",");
        }

        objects.Add(new XcodeObject(
            groupId,
            target.Name,
            string.Join('\n',
                "\t\t\tisa = PBXGroup;",
                "\t\t\tchildren = (",
                childRefs.ToString().TrimEnd('\n', '\r'),
                "\t\t\t);",
                $"\t\t\tname = {PbxString(target.Name)};",
                "\t\t\tsourceTree = \"<group>\";")));
    }

    private static XcodeObject LegacyTarget(
        XcodeTargetInfo info,
        string projectId,
        IReadOnlyDictionary<string, BuildTargetDefinition> targetMap,
        IReadOnlyDictionary<string, string> targetIdByName)
    {
        var dependencyRefs = new StringBuilder();
        var dependencyObjects = new List<XcodeObject>();
        if(info.Target is not null)
        {
            foreach(var dependency in info.Target.Dependencies.Where(targetMap.ContainsKey).Order(StringComparer.OrdinalIgnoreCase))
            {
                var dependencyTargetId = targetIdByName[dependency];
                var proxyId = Id("Proxy:" + info.Name + ":" + dependency);
                var dependencyId = Id("Dependency:" + info.Name + ":" + dependency);
                dependencyObjects.Add(new XcodeObject(
                    proxyId,
                    dependency,
                    string.Join('\n',
                        "\t\t\tisa = PBXContainerItemProxy;",
                        $"\t\t\tcontainerPortal = {Ref(projectId, "Project object")};",
                        "\t\t\tproxyType = 1;",
                        $"\t\t\tremoteGlobalIDString = {dependencyTargetId};",
                        $"\t\t\tremoteInfo = {PbxString(dependency)};")));
                dependencyObjects.Add(new XcodeObject(
                    dependencyId,
                    dependency,
                    string.Join('\n',
                        "\t\t\tisa = PBXTargetDependency;",
                        $"\t\t\ttarget = {Ref(dependencyTargetId, dependency)};",
                        $"\t\t\ttargetProxy = {Ref(proxyId, dependency)};")));
                dependencyRefs.Append("\t\t\t\t");
                dependencyRefs.Append(Ref(dependencyId, dependency));
                dependencyRefs.AppendLine(",");
            }
        }

        dependencyObjects.Add(new XcodeObject(
            info.TargetId,
            info.Name,
            string.Join('\n',
                "\t\t\tisa = PBXLegacyTarget;",
                $"\t\t\tbuildArgumentsString = {PbxString(info.BuildArguments)};",
                $"\t\t\tbuildConfigurationList = {Ref(info.ConfigListId, $"Build configuration list for PBXLegacyTarget \"{info.Name}\"")};",
                "\t\t\tbuildPhases = ();",
                "\t\t\tbuildToolPath = /bin/sh;",
                $"\t\t\tbuildWorkingDirectory = {PbxString(info.Target?.Directory ?? Path.GetDirectoryName(info.ProductPath) ?? string.Empty)};",
                "\t\t\tdependencies = (",
                dependencyRefs.ToString().TrimEnd('\n', '\r'),
                "\t\t\t);",
                $"\t\t\tname = {PbxString(info.Name)};",
                "\t\t\tpassBuildSettingsInEnvironment = 1;",
                $"\t\t\tproductName = {PbxString(info.Name)};",
                $"\t\t\tproductReference = {Ref(info.ProductFileId, info.ProductName)};")));
        return new XcodeObject(
            info.TargetId + ":bundle",
            info.Name,
            string.Join('\n', dependencyObjects.Select(obj => obj.Render())));
    }

    private static XcodeObject ProjectObject(
        string projectId,
        string mainGroupId,
        string productsGroupId,
        string projectConfigListId,
        IReadOnlyList<XcodeTargetInfo> targets,
        BuildOptions options)
    {
        var targetAttributes = string.Concat(targets.Select(info =>
            $"\t\t\t\t{info.TargetId} = {{CreatedOnToolsVersion = 16.0; }};\n"));
        var suppressBuildableAutocreation = string.Concat(targets.Select(info =>
            $"\t\t\t\t{info.TargetId} = {{primary = 1; }};\n"));
        var targetRefs = string.Concat(targets.Select(info => $"\t\t\t\t{Ref(info.TargetId, info.Name)},\n"));
        return new XcodeObject(
            projectId,
            "Project object",
            string.Join('\n',
                "\t\t\tisa = PBXProject;",
                "\t\t\tattributes = {",
                "\t\t\t\tBuildIndependentTargetsInParallel = 1;",
                "\t\t\t\tLastUpgradeCheck = 1600;",
                "\t\t\t\tTargetAttributes = {",
                targetAttributes.TrimEnd('\n', '\r'),
                "\t\t\t\t};",
                "\t\t\t\tSuppressBuildableAutocreation = {",
                suppressBuildableAutocreation.TrimEnd('\n', '\r'),
                "\t\t\t\t};",
                "\t\t\t};",
                $"\t\t\tbuildConfigurationList = {Ref(projectConfigListId, $"Build configuration list for PBXProject \"{options.Mode}\"")};",
                "\t\t\tcompatibilityVersion = \"Xcode 14.0\";",
                "\t\t\tdevelopmentRegion = en;",
                "\t\t\thasScannedForEncodings = 0;",
                "\t\t\tknownRegions = (en, Base);",
                $"\t\t\tmainGroup = {Ref(mainGroupId, "Main Group")};",
                $"\t\t\tproductRefGroup = {Ref(productsGroupId, "Products")};",
                "\t\t\tprojectDirPath = \"\";",
                "\t\t\tprojectRoot = \"\";",
                "\t\t\ttargets = (",
                targetRefs.TrimEnd('\n', '\r'),
                "\t\t\t);"));
    }

    private static XcodeObject ProjectBuildConfiguration(string id, BuildOptions options)
    {
        return new XcodeObject(
            id,
            options.Mode.ToString(),
            string.Join('\n',
                "\t\t\tisa = XCBuildConfiguration;",
                "\t\t\tbuildSettings = {",
                $"\t\t\t\tARCHS = {PbxString(XcodeArchitecture(options.Architecture))};",
                "\t\t\t\tONLY_ACTIVE_ARCH = YES;",
                "\t\t\t\tSDKROOT = macosx;",
                "\t\t\t};",
                $"\t\t\tname = {PbxString(options.Mode.ToString())};"));
    }

    private static XcodeObject TargetBuildConfiguration(BuildWorkspace workspace, string id, BuildOptions options, BuildTargetDefinition target)
    {
        var includePaths = target.IncludeDirectories
            .Concat(target.Options.GlobalIncludeDirectories)
            .Concat(new[] { target.Directory })
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Order(StringComparer.OrdinalIgnoreCase)
            .Select(PbxString)
            .ToArray();
        var defines = target.Defines
            .Concat(target.Options.GlobalDefines)
            .Distinct(StringComparer.Ordinal)
            .Order(StringComparer.Ordinal)
            .Select(PbxString)
            .ToArray();
        return new XcodeObject(
            id,
            target.Name,
            string.Join('\n',
                "\t\t\tisa = XCBuildConfiguration;",
                "\t\t\tbuildSettings = {",
                $"\t\t\t\tARCHS = {PbxString(XcodeArchitecture(options.Architecture))};",
                "\t\t\t\tCLANG_CXX_LANGUAGE_STANDARD = \"c++20\";",
                $"\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = ({string.Join(", ", defines)});",
                $"\t\t\t\tHEADER_SEARCH_PATHS = ({string.Join(", ", includePaths)});",
                "\t\t\t\tONLY_ACTIVE_ARCH = YES;",
                $"\t\t\t\tPRODUCT_NAME = {PbxString(target.Name)};",
                "\t\t\t\tSDKROOT = macosx;",
                "\t\t\t};",
                $"\t\t\tname = {PbxString(options.Mode.ToString())};"));
    }

    private static XcodeObject UtilityBuildConfiguration(string id, BuildOptions options, string name)
    {
        return new XcodeObject(
            id,
            name,
            string.Join('\n',
                "\t\t\tisa = XCBuildConfiguration;",
                "\t\t\tbuildSettings = {",
                $"\t\t\t\tARCHS = {PbxString(XcodeArchitecture(options.Architecture))};",
                "\t\t\t\tONLY_ACTIVE_ARCH = YES;",
                $"\t\t\t\tPRODUCT_NAME = {PbxString(name)};",
                "\t\t\t\tSDKROOT = macosx;",
                "\t\t\t};",
                $"\t\t\tname = {PbxString(options.Mode.ToString())};"));
    }

    private static XcodeObject ConfigurationList(string id, string configId, string name, string defaultConfigurationName)
    {
        return new XcodeObject(
            id,
            name,
            string.Join('\n',
                "\t\t\tisa = XCConfigurationList;",
                $"\t\t\tbuildConfigurations = ({Ref(configId, name)},);",
                "\t\t\tdefaultConfigurationIsVisible = 0;",
                $"\t\t\tdefaultConfigurationName = {PbxString(defaultConfigurationName)};"));
    }

    private static XcodeObject FileReference(string id, string name, string path, string fileType)
    {
        return new XcodeObject(
            id,
            name,
            string.Join('\n',
                "\t\t\tisa = PBXFileReference;",
                $"\t\t\tlastKnownFileType = {PbxString(fileType)};",
                $"\t\t\tname = {PbxString(name)};",
                $"\t\t\tpath = {PbxString(path)};",
                "\t\t\tsourceTree = \"<absolute>\";"));
    }

    private static string BuildProjectText(IReadOnlyList<XcodeObject> objects, string rootObjectId)
    {
        var builder = new StringBuilder();
        builder.AppendLine("// !$*UTF8*$!");
        builder.AppendLine("{");
        builder.AppendLine("\tarchiveVersion = 1;");
        builder.AppendLine("\tclasses = {");
        builder.AppendLine("\t};");
        builder.AppendLine("\tobjectVersion = 56;");
        builder.AppendLine("\tobjects = {");
        foreach(var obj in objects.OrderBy(obj => obj.Id, StringComparer.Ordinal))
        {
            builder.Append(obj.Render());
        }
        builder.AppendLine("\t};");
        builder.AppendLine($"\trootObject = {Ref(rootObjectId, "Project object")};");
        builder.AppendLine("}");
        return builder.ToString();
    }

    private static void WriteHelperScript(string scriptPath, BuildWorkspace workspace, BuildOptions options)
    {
        var projectPath = IdeProjectModel.ResolveRunnerProject(workspace);
        var commonArgs = string.Join(" ", IdeProjectModel.CommonBuildOptionArguments(options));
        var text = string.Join('\n',
            "#!/bin/sh",
            "set -e",
            $"ROOT={ShellQuote(workspace.RootDirectory)}",
            $"LUNABUILD_PROJECT={ShellQuote(projectPath)}",
            "DOTNET=${DOTNET:-dotnet}",
            $"COMMON_ARGS=\"{commonArgs}\"",
            "",
            "run_dotnet() {",
            "    env -u ACTION -u ARCHS -u CONFIGURATION -u PLATFORM_NAME -u PRODUCT_NAME -u PROJECT -u PROJECT_NAME -u SDKROOT -u TARGETNAME -u TARGET_NAME \"$DOTNET\" \"$@\"",
            "}",
            "",
            "run_lunabuild() {",
            "    run_dotnet run --no-restore --project \"$LUNABUILD_PROJECT\" -- \"$@\" $COMMON_ARGS",
            "}",
            "",
            "case \"$1\" in",
            "    target)",
            "        target_name=\"$2\"",
            "        if [ \"${ACTION:-build}\" = \"clean\" ]; then",
            "            run_lunabuild clean --root \"$ROOT\" --target \"$target_name\"",
            "        else",
            "            run_lunabuild build --root \"$ROOT\" --target \"$target_name\"",
            "        fi",
            "        ;;",
            "    all-build)",
            "        run_lunabuild build --root \"$ROOT\" --all",
            "        ;;",
            "    all-rebuild)",
            "        run_lunabuild build --root \"$ROOT\" --all --force",
            "        ;;",
            "    all-clean)",
            "        run_lunabuild clean --root \"$ROOT\" --all",
            "        ;;",
            "    full-clean)",
            "        run_dotnet run --no-restore --project \"$LUNABUILD_PROJECT\" -- clean --root \"$ROOT\" --full",
            "        ;;",
            "    *)",
            "        echo \"Unknown LunaBuild Xcode action: $1\" >&2",
            "        exit 2",
            "        ;;",
            "esac",
            "");
        File.WriteAllText(scriptPath, text, new UTF8Encoding(false));
        if(!OperatingSystem.IsWindows())
        {
            File.SetUnixFileMode(scriptPath, UnixFileMode.UserRead | UnixFileMode.UserWrite | UnixFileMode.UserExecute |
                UnixFileMode.GroupRead | UnixFileMode.GroupExecute |
                UnixFileMode.OtherRead | UnixFileMode.OtherExecute);
        }
    }

    private static void WriteSchemes(string projectDirectory, string projectFileName, IReadOnlyList<XcodeScheme> schemes)
    {
        var schemeDirectory = Path.Combine(projectDirectory, "xcshareddata", "xcschemes");
        Directory.CreateDirectory(schemeDirectory);
        foreach(var file in Directory.EnumerateFiles(schemeDirectory, "*.xcscheme", SearchOption.TopDirectoryOnly))
        {
            File.Delete(file);
        }

        foreach(var scheme in schemes.OrderBy(scheme => scheme.Name, StringComparer.OrdinalIgnoreCase))
        {
            var document = new XDocument(
                new XDeclaration("1.0", "UTF-8", null),
                new XElement("Scheme",
                    new XAttribute("LastUpgradeVersion", "1600"),
                    new XAttribute("version", "1.7"),
                    new XElement("BuildAction",
                        new XAttribute("parallelizeBuildables", "YES"),
                        new XAttribute("buildImplicitDependencies", "YES"),
                        new XElement("BuildActionEntries",
                            new XElement("BuildActionEntry",
                                new XAttribute("buildForTesting", "YES"),
                                new XAttribute("buildForRunning", "YES"),
                                new XAttribute("buildForProfiling", "YES"),
                                new XAttribute("buildForArchiving", "YES"),
                                new XAttribute("buildForAnalyzing", "YES"),
                                new XElement("BuildableReference",
                                    new XAttribute("BuildableIdentifier", "primary"),
                                    new XAttribute("BlueprintIdentifier", scheme.TargetId),
                                    new XAttribute("BuildableName", scheme.BuildableName),
                                    new XAttribute("BlueprintName", scheme.Name),
                                    new XAttribute("ReferencedContainer", "container:" + projectFileName)))))));
            document.Save(Path.Combine(schemeDirectory, IdeProjectModel.SanitizeFileName(scheme.Name) + ".xcscheme"));
        }
    }

    private static void CleanGeneratedProject(string projectDirectory)
    {
        var projectFile = Path.Combine(projectDirectory, "project.pbxproj");
        if(File.Exists(projectFile))
        {
            File.Delete(projectFile);
        }
    }

    private static XcodePaths ResolvePaths(BuildWorkspace workspace, string outputPath)
    {
        var fullOutput = Path.GetFullPath(outputPath);
        if(fullOutput.EndsWith(".xcodeproj", StringComparison.OrdinalIgnoreCase))
        {
            return new XcodePaths(
                OutputDirectory: Path.GetDirectoryName(fullOutput) ?? workspace.RootDirectory,
                ProjectDirectory: fullOutput,
                ProjectFileName: Path.GetFileName(fullOutput),
                ProjectName: Path.GetFileNameWithoutExtension(fullOutput));
        }

        var projectName = Path.GetFileName(workspace.RootDirectory);
        var projectFileName = projectName + ".xcodeproj";
        return new XcodePaths(
            OutputDirectory: fullOutput,
            ProjectDirectory: Path.Combine(fullOutput, projectFileName),
            ProjectFileName: projectFileName,
            ProjectName: projectName);
    }

    private static string ProductFileType(BuildTargetKind kind, string path)
    {
        if(kind == BuildTargetKind.Executable)
        {
            return "compiled.mach-o.executable";
        }
        return Path.GetExtension(path).Equals(".a", StringComparison.OrdinalIgnoreCase)
            ? "archive.ar"
            : "compiled.mach-o.dylib";
    }

    private static string XcodeFileType(string path)
    {
        var extension = Path.GetExtension(path).ToLowerInvariant();
        return extension switch
        {
            ".c" => "sourcecode.c.c",
            ".cc" or ".cpp" or ".cxx" => "sourcecode.cpp.cpp",
            ".h" => "sourcecode.c.h",
            ".hh" or ".hpp" or ".hxx" or ".inl" => "sourcecode.cpp.h",
            ".m" => "sourcecode.c.objc",
            ".mm" => "sourcecode.cpp.objcpp",
            ".s" or ".S" => "sourcecode.asm",
            ".cs" => "sourcecode.csharp",
            ".json" => "text.json",
            ".plist" => "text.plist.xml",
            ".png" => "image.png",
            ".csl" => "sourcecode.cpp.cpp",
            _ => "text",
        };
    }

    private static string XcodeArchitecture(string architecture)
    {
        return architecture.ToLowerInvariant() switch
        {
            "arm64" or "aarch64" => "arm64",
            _ => throw new ArgumentException($"Unsupported Xcode architecture: {architecture}"),
        };
    }

    private static string Id(string value) => IdeProjectModel.StableXcodeId("LunaBuild.Xcode:" + value);

    private static string Ref(string id, string comment) => $"{id} /* {comment.Replace("*/", "* /")} */";

    private static string PbxString(string value)
    {
        return "\"" + value
            .Replace("\\", "\\\\")
            .Replace("\"", "\\\"")
            .Replace("\n", "\\n")
            .Replace("\r", string.Empty) + "\"";
    }

    private static string ShellQuote(string value) => "'" + value.Replace("'", "'\\''") + "'";

    private sealed record XcodePaths(string OutputDirectory, string ProjectDirectory, string ProjectFileName, string ProjectName);

    private sealed record XcodeModel(string ProjectText, IReadOnlyList<XcodeScheme> Schemes);

    private sealed record XcodeScheme(string Name, string TargetId, string BuildableName, string ProjectFileName);

    private sealed record XcodeTargetInfo(
        string Name,
        BuildTargetDefinition? Target,
        string TargetId,
        string ProductFileId,
        string ProductName,
        string ProductPath,
        string SourceGroupId,
        string ConfigListId,
        string ConfigId,
        string BuildArguments);

    private sealed record XcodeObject(string Id, string Comment, string Body)
    {
        public string Render()
        {
            if(Id.EndsWith(":bundle", StringComparison.Ordinal))
            {
                return Body;
            }
            return $"\t\t{Ref(Id, Comment)} = {{\n{Body}\n\t\t}};\n";
        }
    }
}
