using System.Text;

namespace LunaProjectGenerator;

public static class LunaProjectGeneratorApp
{
    public static int Run(string[] args)
    {
        try
        {
            var options = ProjectGeneratorCommandLine.Parse(args);
            if(options.ShowHelp)
            {
                PrintUsage();
                return 0;
            }

            var result = ProjectGenerator.Create(new ProjectGeneratorOptions(
                options.SdkRoot ?? DiscoverSdkRoot(),
                options.ProjectPath!,
                options.ProjectName));
            Console.WriteLine($"Created LunaSDK project `{result.ProjectName}`: {result.ProjectRoot}");
            Console.WriteLine($"Build: dotnet run --project \"{Path.Combine(result.ProjectRoot, "LunaBuild.csproj")}\" -- build --target {result.ProjectName}");
            Console.WriteLine($"Run:   dotnet run --project \"{Path.Combine(result.ProjectRoot, "LunaBuild.csproj")}\" -- run --target {result.ProjectName}");
            return 0;
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine($"lunaproject: {ex.Message}");
            return 1;
        }
    }

    private static string DiscoverSdkRoot()
    {
        var current = new DirectoryInfo(Environment.CurrentDirectory);
        while(current is not null)
        {
            if(File.Exists(Path.Combine(current.FullName, "LunaSDK.Project.cs")) &&
                File.Exists(Path.Combine(current.FullName, "LunaBuild.csproj")))
            {
                return current.FullName;
            }
            current = current.Parent;
        }
        throw new DirectoryNotFoundException("Cannot find the LunaSDK repository. Pass --sdk-root <path> or run under the repository.");
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: lunaproject <project-path> [--name <project-name>] [--sdk-root <path>]");
        Console.WriteLine();
        Console.WriteLine("Creates a new standalone LunaBuild project that imports the selected LunaSDK checkout.");
        Console.WriteLine("The destination must be absent or empty and must not overlap the LunaSDK directory tree.");
    }
}

public sealed record ProjectGeneratorOptions(
    string SdkRoot,
    string ProjectPath,
    string? ProjectName = null);

public sealed record GeneratedProject(
    string ProjectRoot,
    string ProjectName,
    IReadOnlyList<string> Files);

public static class ProjectGenerator
{
    private static readonly UTF8Encoding Utf8WithoutBom = new(false);

    public static GeneratedProject Create(ProjectGeneratorOptions options)
    {
        ArgumentNullException.ThrowIfNull(options);

        var sdkRoot = CanonicalizePath(options.SdkRoot);
        ValidateSdkRoot(sdkRoot);

        var projectRoot = CanonicalizePath(options.ProjectPath);
        ValidateDisjointTrees(sdkRoot, projectRoot);
        ValidateDestination(projectRoot);

        var inferredName = Path.GetFileName(projectRoot.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
        var projectName = string.IsNullOrWhiteSpace(options.ProjectName) ? inferredName : options.ProjectName.Trim();
        ValidateProjectName(projectName);

        var typeName = ToIdentifier(projectName);
        var sdkReference = Path.GetRelativePath(projectRoot, sdkRoot).Replace('\\', '/');
        var files = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["global.json"] = DotNetSdkSelection(),
            ["LunaBuild.csproj"] = RunnerProject(sdkReference),
            [$"{projectName}.Project.cs"] = ProjectRules(projectName, typeName, sdkReference),
            [$"{projectName}.Target.cs"] = TargetRules(projectName, typeName),
            ["Source/Main.cpp"] = MainSource(projectName),
            ["README.md"] = ProjectReadme(projectName, sdkReference),
            [".gitignore"] = "/build/\n/bin/\n/obj/\n.DS_Store\n",
        };

        Directory.CreateDirectory(projectRoot);
        foreach(var (relativePath, content) in files)
        {
            var outputPath = Path.Combine(projectRoot, relativePath.Replace('/', Path.DirectorySeparatorChar));
            var outputDirectory = Path.GetDirectoryName(outputPath);
            if(!string.IsNullOrEmpty(outputDirectory))
            {
                Directory.CreateDirectory(outputDirectory);
            }
            File.WriteAllText(outputPath, content, Utf8WithoutBom);
        }

        return new GeneratedProject(
            projectRoot,
            projectName,
            files.Keys.Select(path => Path.Combine(projectRoot, path.Replace('/', Path.DirectorySeparatorChar))).ToArray());
    }

    private static void ValidateSdkRoot(string sdkRoot)
    {
        if(!Directory.Exists(sdkRoot))
        {
            throw new DirectoryNotFoundException($"LunaSDK root does not exist: {sdkRoot}");
        }

        var requiredFiles = new[]
        {
            "LunaBuild.csproj",
            "LunaSDK.Project.cs",
            "Tools/LunaBuild/src/LunaBuild.Cli/Program.cs",
            "Tools/LunaBuild/src/LunaBuild.Cli/LunaBuildCli.cs",
            "Tools/LunaBuild/src/LunaBuild.Core/LunaBuild.Core.csproj",
        };
        var missingFiles = requiredFiles
            .Where(path => !File.Exists(Path.Combine(sdkRoot, path.Replace('/', Path.DirectorySeparatorChar))))
            .ToArray();
        if(missingFiles.Length > 0)
        {
            throw new DirectoryNotFoundException(
                $"The selected directory is not a complete LunaSDK checkout: {sdkRoot}. Missing: {string.Join(", ", missingFiles)}");
        }
    }

    private static void ValidateDestination(string projectRoot)
    {
        if(File.Exists(projectRoot))
        {
            throw new IOException($"Project path is an existing file: {projectRoot}");
        }
        if(Directory.Exists(projectRoot) && Directory.EnumerateFileSystemEntries(projectRoot).Any())
        {
            throw new IOException($"Project directory must be empty: {projectRoot}");
        }
    }

    private static void ValidateDisjointTrees(string sdkRoot, string projectRoot)
    {
        if(IsSameOrNested(sdkRoot, projectRoot) || IsSameOrNested(projectRoot, sdkRoot))
        {
            throw new ArgumentException(
                "The new project and LunaSDK must use disjoint directory trees. Choose a project path outside the LunaSDK checkout and outside its parent tree.");
        }
    }

    private static bool IsSameOrNested(string parent, string child)
    {
        var comparison = OperatingSystem.IsWindows() || OperatingSystem.IsMacOS()
            ? StringComparison.OrdinalIgnoreCase
            : StringComparison.Ordinal;
        var normalizedParent = Path.GetFullPath(parent).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
        var normalizedChild = Path.GetFullPath(child).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
        return normalizedChild.Equals(normalizedParent, comparison) ||
            normalizedChild.StartsWith(normalizedParent + Path.DirectorySeparatorChar, comparison);
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

        var root = Path.GetPathRoot(existingPath)
            ?? throw new InvalidOperationException($"Path has no filesystem root: {existingPath}");
        var canonicalPath = root;
        foreach(var component in Path.GetRelativePath(root, existingPath).Split(
            new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
            StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = new DirectoryInfo(Path.Combine(canonicalPath, component));
            canonicalPath = candidate.ResolveLinkTarget(returnFinalTarget: true)?.FullName ?? candidate.FullName;
        }
        while(missingComponents.TryPop(out var component))
        {
            canonicalPath = Path.Combine(canonicalPath, component);
        }
        return Path.GetFullPath(canonicalPath);
    }

    private static void ValidateProjectName(string projectName)
    {
        if(string.IsNullOrWhiteSpace(projectName) ||
            !projectName.All(character =>
                character is >= 'a' and <= 'z' or >= 'A' and <= 'Z' or >= '0' and <= '9' or '_' or '-'))
        {
            throw new ArgumentException(
                "Project name must contain only ASCII letters, digits, underscores, or hyphens.");
        }
    }

    private static string ToIdentifier(string projectName)
    {
        var identifier = projectName.Replace('-', '_');
        return char.IsDigit(identifier[0]) ? "Project_" + identifier : identifier;
    }

    private static string RunnerProject(string sdkReference)
    {
        var sdk = XmlEscape(sdkReference);
        return $$"""
            <Project Sdk="Microsoft.NET.Sdk">
              <PropertyGroup>
                <OutputType>Exe</OutputType>
                <TargetFramework>net9.0</TargetFramework>
                <RollForward>Major</RollForward>
                <ImplicitUsings>enable</ImplicitUsings>
                <Nullable>enable</Nullable>
                <AssemblyName>lunabuild</AssemblyName>
                <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
                <NuGetAudit>false</NuGetAudit>
                <LunaSdkRoot>{{sdk}}</LunaSdkRoot>
              </PropertyGroup>
              <ItemGroup>
                <Compile Include="$(LunaSdkRoot)/Tools/LunaBuild/src/LunaBuild.Cli/Program.cs" Link="Program.cs" />
                <Compile Include="$(LunaSdkRoot)/Tools/LunaBuild/src/LunaBuild.Cli/LunaBuildCli.cs" Link="LunaBuildCli.cs" />
              </ItemGroup>
              <ItemGroup>
                <ProjectReference Include="$(LunaSdkRoot)/Tools/LunaBuild/src/LunaBuild.Core/LunaBuild.Core.csproj" />
              </ItemGroup>
            </Project>
            """ + "\n";
    }

    private static string DotNetSdkSelection()
    {
        return """
            {
              "sdk": {
                "version": "9.0.100",
                "rollForward": "major"
              }
            }
            """ + "\n";
    }

    private static string ProjectRules(string projectName, string typeName, string sdkReference)
    {
        return $$"""
            using LunaBuild.Core;

            public sealed class {{typeName}}ProjectRules : ProjectRules
            {
                public {{typeName}}ProjectRules()
                    : base("{{CSharpEscape(projectName)}}")
                {
                }

                protected override void ConfigureProject(Project project)
                {
                    var lunaSdk = project.ImportProject("{{CSharpEscape(sdkReference)}}");
                    lunaSdk.PrimaryOptions = lunaSdk.DefaultBuildOptions with
                    {
                        Mode = Options.Mode,
                        Platform = Options.Platform,
                        Architecture = Options.Architecture,
                        Shared = Options.Shared,
                        RhiApi = Options.RhiApi,
                    };
                    project.UseActionConfiguration(lunaSdk, "luna.meta");
                    project.UseActionConfiguration(lunaSdk, "cppsl.shader");
                }
            }
            """ + "\n";
    }

    private static string TargetRules(string projectName, string typeName)
    {
        var escapedName = CSharpEscape(projectName);
        return $$"""
            namespace LunaBuild.Core.Targets;

            public sealed class {{typeName}}TargetRules : TargetRules
            {
                public {{typeName}}TargetRules()
                    : base(
                        name: "{{escapedName}}",
                        targetDirectory: ".",
                        rulesPath: "{{escapedName}}.Target.cs")
                {
                    Kind = BuildTargetKind.Executable;
                    Sources("Source/*.cpp");
                    DependsOn("LunaSDK.Runtime");
                }
            }
            """ + "\n";
    }

    private static string MainSource(string projectName)
    {
        return $$"""
            #include <Luna/Runtime/Runtime.hpp>
            #include <cstdio>

            int main()
            {
                if(Luna::failed(Luna::init()))
                {
                    return 1;
                }
                std::puts("{{CppEscape(projectName)}} is running with LunaSDK.");
                Luna::close();
                return 0;
            }
            """ + "\n";
    }

    private static string ProjectReadme(string projectName, string sdkReference)
    {
        return $$"""
            # {{projectName}}

            This project imports LunaSDK from `{{sdkReference}}` and uses the local `LunaBuild.csproj` runner.

            Build:

            ```powershell
            dotnet run --project LunaBuild.csproj -- build --target {{projectName}}
            ```

            Run:

            ```powershell
            dotnet run --project LunaBuild.csproj -- run --target {{projectName}}
            ```

            If the relative location of LunaSDK changes, update `LunaSdkRoot` in `LunaBuild.csproj` and the path passed to `ImportProject` in `{{projectName}}.Project.cs`.
            """ + "\n";
    }

    private static string XmlEscape(string value)
    {
        return value
            .Replace("&", "&amp;")
            .Replace("<", "&lt;")
            .Replace(">", "&gt;")
            .Replace("\"", "&quot;")
            .Replace("'", "&apos;");
    }

    private static string CSharpEscape(string value)
    {
        return value
            .Replace("\\", "\\\\")
            .Replace("\"", "\\\"")
            .Replace("\r", "\\r")
            .Replace("\n", "\\n");
    }

    private static string CppEscape(string value)
    {
        return CSharpEscape(value);
    }
}

internal sealed class ProjectGeneratorCommandLine
{
    public string? SdkRoot { get; private init; }

    public string? ProjectPath { get; private init; }

    public string? ProjectName { get; private init; }

    public bool ShowHelp { get; private init; }

    public static ProjectGeneratorCommandLine Parse(string[] args)
    {
        string? sdkRoot = null;
        string? projectPath = null;
        string? projectName = null;
        var showHelp = false;

        for(var i = 0; i < args.Length; ++i)
        {
            switch(args[i])
            {
                case "-h":
                case "--help":
                    showHelp = true;
                    break;
                case "--sdk-root":
                    sdkRoot = RequireValue(args, ref i, "--sdk-root");
                    break;
                case "--path":
                    projectPath = SetOnce(projectPath, RequireValue(args, ref i, "--path"), "project path");
                    break;
                case "--name":
                    projectName = SetOnce(projectName, RequireValue(args, ref i, "--name"), "project name");
                    break;
                default:
                    if(args[i].StartsWith("-", StringComparison.Ordinal))
                    {
                        throw new ArgumentException($"Unknown option: {args[i]}");
                    }
                    projectPath = SetOnce(projectPath, args[i], "project path");
                    break;
            }
        }

        if(!showHelp && string.IsNullOrWhiteSpace(projectPath))
        {
            throw new ArgumentException("A project path is required.");
        }
        return new ProjectGeneratorCommandLine
        {
            SdkRoot = sdkRoot,
            ProjectPath = projectPath,
            ProjectName = projectName,
            ShowHelp = showHelp,
        };
    }

    private static string RequireValue(string[] args, ref int index, string optionName)
    {
        if(index + 1 >= args.Length)
        {
            throw new ArgumentException($"{optionName} requires a value.");
        }
        return args[++index];
    }

    private static string SetOnce(string? current, string value, string name)
    {
        if(current is not null)
        {
            throw new ArgumentException($"The {name} was specified more than once.");
        }
        return value;
    }
}
