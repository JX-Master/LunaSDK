using System.Diagnostics;
using System.Reflection;
using System.Xml.Linq;

namespace LunaBuild.Core;

public sealed class ProjectTargetRulesProvider : ITargetRulesProvider
{
    private Assembly? _assembly;
    private string? _assemblyWorkspaceRoot;

    private static readonly string[] ExcludedTopLevelDirectories =
    {
        ".git",
        ".xmake",
        "build",
        "SDKs",
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
        _assembly = Assembly.LoadFrom(assemblyPath);
        _assemblyWorkspaceRoot = workspace.RootDirectory;
        return _assembly;
    }

    private static IEnumerable<string> DiscoverRuleFiles(BuildWorkspace workspace)
    {
        foreach(var file in Directory.EnumerateFiles(workspace.RootDirectory, "*.Target.cs", SearchOption.AllDirectories)
            .Concat(Directory.EnumerateFiles(workspace.RootDirectory, "*.Project.cs", SearchOption.AllDirectories)))
        {
            var relative = workspace.ToRepositoryRelativePath(file);
            if(ExcludedTopLevelDirectories.Any(excluded => IsInDirectory(relative, excluded)))
            {
                continue;
            }
            yield return file;
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
        var projectDirectory = Path.Combine(workspace.BuildDirectory, "ProjectRules");
        Directory.CreateDirectory(projectDirectory);
        using var compileLock = AcquireLock(Path.Combine(projectDirectory, "compile.lock"));

        var projectPath = Path.Combine(projectDirectory, "LunaBuild.ProjectRules.csproj");
        var outputAssembly = Path.Combine(projectDirectory, "bin", "Debug", "net9.0", "LunaBuild.ProjectRules.dll");
        WriteProjectFile(projectPath, ruleFiles);

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

    private static void WriteProjectFile(string projectPath, IReadOnlyList<string> ruleFiles)
    {
        var coreAssembly = typeof(TargetRules).Assembly.Location;
        var document = new XDocument(
            new XElement("Project",
                new XAttribute("Sdk", "Microsoft.NET.Sdk"),
                new XElement("PropertyGroup",
                    new XElement("TargetFramework", "net9.0"),
                    new XElement("ImplicitUsings", "enable"),
                    new XElement("Nullable", "enable"),
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
}
