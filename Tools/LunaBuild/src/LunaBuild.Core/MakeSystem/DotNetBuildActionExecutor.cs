namespace LunaBuild.Core.MakeSystem;

public sealed class DotNetBuildActionExecutor : KnownActionExecutor
{
    private readonly TimeSpan _actionTimeout;

    public DotNetBuildActionExecutor(TimeSpan? actionTimeout = null)
        : base("dotnet.build")
    {
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public override async Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var project = context.Workspace.ResolveRepositoryPath(payload.Required("project"));
        var expectedOutput = context.Workspace.ResolveRepositoryPath(payload.Required("output"));
        if(payload.Contains("generated"))
        {
            WriteGeneratedProject(context.Workspace, payload, project);
        }
        var dotnet = LocateDotnet();
        var args = new List<string>
        {
            "build",
            project,
            "-m:1",
            "/nr:false",
            "--nologo",
            "-p:UseSharedCompilation=false",
        };
        if(payload.Contains("generated"))
        {
            args.Add("-o");
            args.Add(context.Workspace.ResolveRepositoryPath(payload.Required("output_dir")));
        }

        var result = await ProcessRunner.RunAsync(dotnet, args, context.Workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($".NET build failed for {project}:{Environment.NewLine}{result.Output}");
        }
        if(!File.Exists(expectedOutput))
        {
            throw new MakeSystemException($".NET build did not produce expected output: {expectedOutput}");
        }
    }

    private static string LocateDotnet()
    {
        var executable = OperatingSystem.IsWindows() ? "dotnet.exe" : "dotnet";
        var pathValue = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;
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

    private static void WriteGeneratedProject(BuildWorkspace workspace, ActionPayload payload, string project)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(project)!);
        var outputType = payload.Required("output_type");
        var targetFramework = payload.Required("target_framework");
        var nullable = payload.Required("nullable");
        var implicitUsings = payload.Required("implicit_usings");
        var allowUnsafe = payload.Required("allow_unsafe_blocks");
        var assemblyName = payload.Required("assembly_name");

        using var writer = new StreamWriter(project, append: false, new System.Text.UTF8Encoding(false));
        writer.WriteLine("<Project Sdk=\"Microsoft.NET.Sdk\">");
        writer.WriteLine("  <PropertyGroup>");
        writer.WriteLine($"    <OutputType>{Escape(outputType)}</OutputType>");
        writer.WriteLine($"    <TargetFramework>{Escape(targetFramework)}</TargetFramework>");
        writer.WriteLine($"    <AssemblyName>{Escape(assemblyName)}</AssemblyName>");
        writer.WriteLine("    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>");
        writer.WriteLine($"    <Nullable>{Escape(nullable)}</Nullable>");
        writer.WriteLine($"    <ImplicitUsings>{Escape(implicitUsings)}</ImplicitUsings>");
        writer.WriteLine($"    <AllowUnsafeBlocks>{Escape(allowUnsafe)}</AllowUnsafeBlocks>");
        writer.WriteLine("    <NuGetAudit>false</NuGetAudit>");
        writer.WriteLine("    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>");
        writer.WriteLine("    <AppendRuntimeIdentifierToOutputPath>false</AppendRuntimeIdentifierToOutputPath>");
        var defines = payload.All("define");
        if(defines.Count > 0)
        {
            writer.WriteLine($"    <DefineConstants>{Escape(string.Join(';', defines))}</DefineConstants>");
        }
        writer.WriteLine("  </PropertyGroup>");

        WriteCompileItems(workspace, payload, writer);
        WriteReferenceItems(workspace, payload, writer);

        writer.WriteLine("</Project>");
    }

    private static void WriteCompileItems(BuildWorkspace workspace, ActionPayload payload, TextWriter writer)
    {
        var sources = payload.All("source");
        if(sources.Count == 0)
        {
            return;
        }
        writer.WriteLine("  <ItemGroup>");
        foreach(var source in sources)
        {
            writer.WriteLine($"    <Compile Include=\"{Escape(workspace.ResolveRepositoryPath(source))}\" />");
        }
        writer.WriteLine("  </ItemGroup>");
    }

    private static void WriteReferenceItems(BuildWorkspace workspace, ActionPayload payload, TextWriter writer)
    {
        var references = payload.All("reference");
        if(references.Count == 0)
        {
            return;
        }
        writer.WriteLine("  <ItemGroup>");
        foreach(var reference in references)
        {
            var path = workspace.ResolveRepositoryPath(reference);
            var name = Path.GetFileNameWithoutExtension(path);
            writer.WriteLine($"    <Reference Include=\"{Escape(name)}\">");
            writer.WriteLine($"      <HintPath>{Escape(path)}</HintPath>");
            writer.WriteLine("      <Private>true</Private>");
            writer.WriteLine("    </Reference>");
        }
        writer.WriteLine("  </ItemGroup>");
    }

    private static string Escape(string value)
    {
        return System.Security.SecurityElement.Escape(value) ?? string.Empty;
    }
}
