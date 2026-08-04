using System.Diagnostics;
using System.Reflection;
using System.Runtime.Loader;
using System.Security.Cryptography;
using System.Text;
using System.Xml.Linq;

namespace LunaBuild.Core;

public sealed class ProjectTargetRulesProvider : ITargetRulesProvider
{
    private Assembly? _assembly;
    private string? _assemblyWorkspaceRoot;
    private AssemblyLoadContext? _loadContext;

    private static readonly string[] ExcludedTopLevelDirectories =
    {
        ".git",
        "build",
        "Tools/LunaBuild",
    };

    public IReadOnlyList<TargetRules> GetTargetRules(BuildWorkspace workspace)
    {
        var assembly = LoadRulesAssembly(workspace);
        if(assembly is null)
        {
            return Array.Empty<TargetRules>();
        }
        return assembly.GetTypes()
            .Where(type => type is { IsAbstract: false } && typeof(TargetRules).IsAssignableFrom(type))
            .Select(CreateRules)
            .OrderBy(rules => rules.Name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    public IReadOnlyList<ProjectRules> GetProjectRules(BuildWorkspace workspace)
    {
        var assembly = LoadRulesAssembly(workspace);
        if(assembly is null)
        {
            return Array.Empty<ProjectRules>();
        }
        return assembly.GetTypes()
            .Where(type => type is { IsAbstract: false } && typeof(ProjectRules).IsAssignableFrom(type))
            .Select(CreateProjectRules)
            .OrderBy(rules => rules.Name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private Assembly? LoadRulesAssembly(BuildWorkspace workspace)
    {
        if(_assembly is not null && string.Equals(_assemblyWorkspaceRoot, workspace.RootDirectory, StringComparison.OrdinalIgnoreCase))
        {
            return _assembly;
        }

        var ruleFiles = DiscoverRuleFiles(workspace).ToArray();
        if(ruleFiles.Length == 0)
        {
            return null;
        }

        var assemblyPath = CompileRulesAssembly(workspace, ruleFiles);
        _loadContext = new ProjectRulesLoadContext();
        _assembly = _loadContext.LoadFromAssemblyPath(assemblyPath);
        _assemblyWorkspaceRoot = workspace.RootDirectory;
        return _assembly;
    }

    private static IEnumerable<string> DiscoverRuleFiles(BuildWorkspace workspace)
    {
        return DiscoverDirectory(workspace.RootDirectory, isProjectRoot: true);

        IEnumerable<string> DiscoverDirectory(string directory, bool isProjectRoot)
        {
            if(!isProjectRoot && Directory.EnumerateFiles(directory, "*.Project.cs", SearchOption.TopDirectoryOnly).Any())
            {
                yield break;
            }

            foreach(var file in Directory.EnumerateFiles(directory, "*.Target.cs", SearchOption.TopDirectoryOnly)
                .Concat(Directory.EnumerateFiles(directory, "*.Project.cs", SearchOption.TopDirectoryOnly))
                .Order(StringComparer.OrdinalIgnoreCase))
            {
                yield return file;
            }

            foreach(var child in Directory.EnumerateDirectories(directory).Order(StringComparer.OrdinalIgnoreCase))
            {
                var relative = workspace.ToRepositoryRelativePath(child);
                if(ExcludedTopLevelDirectories.Any(excluded =>
                    relative.Equals(excluded, StringComparison.OrdinalIgnoreCase) || IsInDirectory(relative, excluded)))
                {
                    continue;
                }
                foreach(var file in DiscoverDirectory(child, isProjectRoot: false))
                {
                    yield return file;
                }
            }
        }
    }

    private static ProjectRules CreateProjectRules(Type type)
    {
        try
        {
            return (ProjectRules)Activator.CreateInstance(type)!;
        }
        catch(Exception ex)
        {
            throw new InvalidOperationException($"Failed to create project rules: {type.FullName}", ex);
        }
    }

    private static bool IsInDirectory(string relativePath, string directory)
    {
        var normalizedPath = relativePath.Replace('\\', '/');
        var normalizedDirectory = directory.Replace('\\', '/').TrimEnd('/') + "/";
        return normalizedPath.StartsWith(normalizedDirectory, StringComparison.OrdinalIgnoreCase);
    }

    private static TargetRules CreateRules(Type type)
    {
        try
        {
            return (TargetRules)Activator.CreateInstance(type)!;
        }
        catch(Exception ex)
        {
            throw new InvalidOperationException($"Failed to create target rules: {type.FullName}", ex);
        }
    }

    private static string CompileRulesAssembly(BuildWorkspace workspace, IReadOnlyList<string> ruleFiles)
    {
        var projectDirectory = Path.Combine(workspace.BuildDirectory, "Rules");
        Directory.CreateDirectory(projectDirectory);
        using var compileLock = AcquireLock(Path.Combine(projectDirectory, "compile.lock"));

        var projectPath = Path.Combine(projectDirectory, "LunaBuild.ProjectRules.csproj");
        var assemblyName = "LunaBuild.ProjectRules." + StableHash(workspace.RootDirectory) + "." + RulesContentHash(ruleFiles);
        var outputAssembly = Path.Combine(projectDirectory, "bin", "Debug", "net9.0", assemblyName + ".dll");
        WriteProjectFile(projectPath, ruleFiles, assemblyName);

        var dotnet = LocateDotnet();
        var args = new[]
        {
            "build",
            projectPath,
            "-m:1",
            "/nr:false",
            "--nologo",
            "-p:UseSharedCompilation=false",
        };
        var result = RunProcess(dotnet, args, workspace.RootDirectory, TimeSpan.FromMinutes(5));
        if(result.ExitCode != 0)
        {
            throw new InvalidOperationException($"Failed to compile project target rules:{Environment.NewLine}{result.Output}");
        }
        if(!File.Exists(outputAssembly))
        {
            throw new FileNotFoundException("Project target rules build did not produce the expected assembly.", outputAssembly);
        }
        return outputAssembly;
    }

    private static FileStream AcquireLock(string path)
    {
        var deadline = DateTime.UtcNow + TimeSpan.FromSeconds(30);
        while(true)
        {
            try
            {
                return new FileStream(path, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None);
            }
            catch(IOException) when(DateTime.UtcNow < deadline)
            {
                Thread.Sleep(100);
            }
        }
    }

    private static void WriteProjectFile(string projectPath, IReadOnlyList<string> ruleFiles, string assemblyName)
    {
        var coreAssembly = typeof(TargetRules).Assembly.Location;
        var document = new XDocument(
            new XElement("Project",
                new XAttribute("Sdk", "Microsoft.NET.Sdk"),
                new XElement("PropertyGroup",
                    new XElement("TargetFramework", "net9.0"),
                    new XElement("ImplicitUsings", "enable"),
                    new XElement("Nullable", "enable"),
                    new XElement("AssemblyName", assemblyName),
                    new XElement("EnableDefaultCompileItems", "false"),
                    new XElement("NuGetAudit", "false")),
                new XElement("ItemGroup",
                    new XElement("Reference",
                        new XAttribute("Include", "LunaBuild.Core"),
                        new XElement("HintPath", coreAssembly))),
                new XElement("ItemGroup",
                    ruleFiles
                        .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
                        .Select(path => new XElement("Compile", new XAttribute("Include", path))))));

        document.Save(projectPath);
    }

    private static string StableHash(string value)
    {
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(value))).ToLowerInvariant()[..12];
    }

    private static string RulesContentHash(IReadOnlyList<string> ruleFiles)
    {
        var identity = string.Join('\n', ruleFiles
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .Select(path => path + ":" + Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(path)))));
        return StableHash(identity);
    }

    private static string LocateDotnet()
    {
        var pathValue = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;
        var executable = OperatingSystem.IsWindows() ? "dotnet.exe" : "dotnet";
        foreach(var pathEntry in pathValue.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = Path.Combine(pathEntry, executable);
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        return "dotnet";
    }

    private static ProcessResult RunProcess(string fileName, IReadOnlyList<string> arguments, string workingDirectory, TimeSpan timeout)
    {
        using var timeoutCts = new CancellationTokenSource(timeout);
        var output = new System.Text.StringBuilder();
        using var process = new Process
        {
            StartInfo = new ProcessStartInfo
            {
                FileName = fileName,
                WorkingDirectory = workingDirectory,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
            },
        };
        foreach(var argument in arguments)
        {
            process.StartInfo.ArgumentList.Add(argument);
        }
        process.OutputDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                output.AppendLine(e.Data);
            }
        };
        process.ErrorDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                output.AppendLine(e.Data);
            }
        };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();
        try
        {
            process.WaitForExitAsync(timeoutCts.Token).GetAwaiter().GetResult();
        }
        catch(OperationCanceledException)
        {
            TryKill(process);
            throw new TimeoutException($"Compiling project target rules timed out after {timeout.TotalSeconds:0}s.");
        }

        return new ProcessResult(process.ExitCode, output.ToString());
    }

    private static void TryKill(Process process)
    {
        try
        {
            if(!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
            }
        }
        catch
        {
            // Best effort cleanup after timeout.
        }
    }

    private sealed record ProcessResult(int ExitCode, string Output);

    private sealed class ProjectRulesLoadContext : AssemblyLoadContext
    {
        public ProjectRulesLoadContext()
            : base(isCollectible: false)
        {
        }

        protected override Assembly? Load(AssemblyName assemblyName)
        {
            var coreAssembly = typeof(TargetRules).Assembly;
            return assemblyName.Name == coreAssembly.GetName().Name ? coreAssembly : null;
        }
    }
}
