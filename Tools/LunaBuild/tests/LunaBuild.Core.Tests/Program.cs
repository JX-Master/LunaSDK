using LunaBuild.Core;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Core.Tests;

internal static class Program
{
    private static int _passed;

    private static int Main()
    {
        var tests = new (string Name, Action Body)[]
        {
            ("imports isolate configuration and qualify dependencies", ImportIsolation),
            ("duplicate imports are rejected with chains", DuplicateImport),
            ("project configuration freezes before ConfigureProject", FrozenConfiguration),
            ("import cycles are rejected", ImportCycle),
            ("sibling project targets are not visible", SiblingVisibility),
            ("native link configurations must be compatible", LinkCompatibility),
            ("shared dependencies already beside consumers are not copied onto themselves", SameDirectoryRuntimeStaging),
            ("symlink imports use canonical project identity", CanonicalSymlinkIdentity),
            ("rule edits invalidate the compiled rules cache", RulesCacheInvalidation),
            ("custom imported build roots own the final rules assembly", CustomImportedBuildRoot),
            ("projects cannot share one build root", DuplicateBuildRoot),
            ("dotnet builds honor the requested configuration", DotNetBuildConfiguration),
            ("cpp actions describe their input and output files", CppActionDescriptions),
            ("make system reports indexed task progress", IndexedTaskProgress),
            ("make system releases dependents without batch barriers", AsyncDagScheduling),
            ("aggregate actions execute without task progress", AggregateActionsDoNotReportProgress),
            ("application targets produce native executable graphs", ApplicationTargetGraph),
            ("MSVC UTF-8 compilation is enabled by default and configurable", MsvcUtf8CompilationOption),
            ("apple deployment settings affect layout and commands", AppleDeploymentSettings),
        };

        foreach(var test in tests)
        {
            try
            {
                test.Body();
                ++_passed;
                Console.WriteLine($"PASS {test.Name}");
            }
            catch(Exception exception)
            {
                Console.Error.WriteLine($"FAIL {test.Name}{Environment.NewLine}{exception}");
                return 1;
            }
        }

        Console.WriteLine($"Passed {_passed} LunaBuild Core tests.");
        return 0;
    }

    private static void ImportIsolation()
    {
        using var scope = new TestScope();
        var host = scope.Project("Host");
        var imported = scope.Project("Host/Imported");
        Write(imported, "Imported.Project.cs", """
            using LunaBuild.Core;
            public sealed class CommonProjectRules : ProjectRules
            {
                public CommonProjectRules() : base("Imported") {}
                protected override void ConfigureProperties(BuildWorkspace workspace)
                {
                    BooleanProperty("feature", false);
                }
                protected override void Configure(BuildWorkspace workspace, BuildOptions options)
                {
                    ActionConfiguration(
                        "test.action",
                        values: new Dictionary<string, string> { ["value"] = "imported" });
                }
            }
            """);
        Write(imported, "Core.Target.cs", HeaderOnlyTarget("Core"));

        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class CommonProjectRules : ProjectRules
            {
                public CommonProjectRules() : base("Host") {}
                protected override void ConfigureProperties(BuildWorkspace workspace)
                {
                    StringProperty("host_value", "host");
                }
                protected override void ConfigureProject(Project project)
                {
                    var imported = project.ImportProject("Imported");
                    imported.PrimaryOptions = imported.DefaultBuildOptions with
                    {
                        Mode = BuildMode.Release,
                        Platform = project.PrimaryOptions.Platform,
                        Architecture = project.PrimaryOptions.Architecture,
                        Shared = project.PrimaryOptions.Shared,
                        RhiApi = project.PrimaryOptions.RhiApi,
                        Properties = imported.ResolveProperties(
                            new Dictionary<string, string?> { ["feature"] = "true" }),
                    };
                    project.UseActionConfiguration(imported, "test.action");
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App", "Imported.Core"));
        Write(host, "Core.Target.cs", HeaderOnlyTarget("Core"));

        var session = CreateSession(host);
        Equal(2, session.Projects.Count, "project count");
        Equal("Host", session.HostProject.Name, "host name");
        var importedProject = session.Projects.Single(project => project.Name == "Imported");
        True(importedProject.PrimaryOptions.Properties.GetBoolean("feature"), "imported property mapping");
        Equal(BuildMode.Release, importedProject.PrimaryOptions.Mode, "imported mode isolation");
        Equal(BuildMode.Debug, session.HostProject.PrimaryOptions.Mode, "host mode isolation");
        True(!session.HostProject.PrimaryOptions.Properties.TryGetBoolean("feature", out _), "host property isolation");
        True(importedProject.BuildDirectory.StartsWith(session.HostWorkspace.BuildDirectory, StringComparison.OrdinalIgnoreCase), "imported build root");

        var app = session.Targets.Single(target => target.QualifiedName == "Host.App");
        Equal("Imported.Core", app.Dependencies.Single(), "qualified dependency");
        Equal("Host.Core", session.ResolveTargetName("Core"), "bare target selects host project");
        Equal("Imported.Core", session.ResolveTargetName("Imported.Core"), "qualified imported target selection");
        Equal("Imported", app.Options.FindActionConfiguration("test.action")?.ProviderProjectName, "adopted action provider");
        var graph = new CppTargetGraphGenerator().Generate(
            session.HostWorkspace,
            session.HostProject.PrimaryOptions,
            session.Targets,
            "App");
        Equal(2, graph.Version, "graph version");
        True(graph.Projects.Any(project => project.Name == "Imported" && !project.IsHost), "graph project table");
        True(graph.Nodes.Any(node => node.Id == "target://Host.App" && node.ProjectName == "Host"), "node project identity");

        var ideDirectory = Path.Combine(host, "ide");
        VSCodeWorkspaceWriter.Write(
            session.HostWorkspace,
            session.HostProject.PrimaryOptions,
            graph,
            session.Targets,
            ideDirectory);
        var tasks = File.ReadAllText(Path.Combine(ideDirectory, "tasks.json"));
        True(!tasks.Contains("Release", StringComparison.Ordinal), "IDE tasks must replay host options, not imported options");
    }

    private static void DuplicateImport()
    {
        using var scope = new TestScope();
        var imported = SimpleProject(scope, "Imported", "Imported", "Core");
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    project.ImportProject("{{CSharp(imported)}}");
                    project.ImportProject("{{CSharp(imported)}}");
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App"));
        Throws<InvalidOperationException>(() => CreateSession(host), "imported more than once", "duplicate import");
    }

    private static void FrozenConfiguration()
    {
        using var scope = new TestScope();
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", """
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    project.PrimaryOptions = project.DefaultBuildOptions;
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App"));
        Throws<InvalidOperationException>(() => CreateSession(host), "configuration is frozen", "frozen host configuration");
    }

    private static void ImportCycle()
    {
        using var scope = new TestScope();
        var host = scope.Project("Host");
        var child = scope.Project("Child");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project) => project.ImportProject("{{CSharp(child)}}");
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App"));
        Write(child, "Child.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class ChildRules : ProjectRules
            {
                public ChildRules() : base("Child") {}
                protected override void ConfigureProject(Project project) => project.ImportProject("{{CSharp(host)}}");
            }
            """);
        Write(child, "Core.Target.cs", HeaderOnlyTarget("Core"));
        Throws<InvalidOperationException>(() => CreateSession(host), "imported more than once", "cycle import");
    }

    private static void SiblingVisibility()
    {
        using var scope = new TestScope();
        var left = SimpleProject(scope, "Left", "Left", "Core", "Right.Core");
        var right = SimpleProject(scope, "Right", "Right", "Core");
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    project.ImportProject("{{CSharp(left)}}");
                    project.ImportProject("{{CSharp(right)}}");
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App", "Left.Core"));
        Throws<InvalidOperationException>(() => CreateSession(host), "not in `Left`'s import subtree", "sibling visibility");
    }

    private static void LinkCompatibility()
    {
        using var scope = new TestScope();
        var imported = scope.Project("Imported");
        Write(imported, "Imported.Project.cs", BasicProjectRules("Imported", "ImportedRules"));
        Write(imported, "Core.Target.cs", NativeTarget("Core", BuildTargetKind.SharedLibrary));
        Write(imported, "core.cpp", "int imported_core() { return 0; }");

        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    var imported = project.ImportProject("{{CSharp(imported)}}");
                    imported.PrimaryOptions = imported.DefaultBuildOptions with
                    {
                        Mode = BuildMode.Release,
                        Platform = project.PrimaryOptions.Platform,
                        Architecture = project.PrimaryOptions.Architecture,
                        Shared = project.PrimaryOptions.Shared,
                        RhiApi = project.PrimaryOptions.RhiApi,
                        Properties = imported.ResolveProperties(new Dictionary<string, string?>()),
                    };
                }
            }
            """);
        Write(host, "App.Target.cs", NativeTarget("App", BuildTargetKind.Executable, "Imported.Core"));
        Write(host, "app.cpp", "int main() { return 0; }");
        var session = CreateSession(host);
        Throws<InvalidOperationException>(
            () => new CppTargetGraphGenerator().Generate(
                session.HostWorkspace,
                session.HostProject.PrimaryOptions,
                session.Targets,
                "App"),
            "mode: consumer=Debug, dependency=Release",
            "link compatibility");
    }

    private static void SameDirectoryRuntimeStaging()
    {
        using var scope = new TestScope();
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", BasicProjectRules("Host", "HostRules"));
        Write(host, "Core.Target.cs", NativeTarget("Core", BuildTargetKind.SharedLibrary));
        Write(host, "core.cpp", "int host_core() { return 0; }");
        Write(host, "App.Target.cs", NativeTarget("App", BuildTargetKind.Executable, "Core"));
        Write(host, "app.cpp", "int main() { return 0; }");

        var session = CreateSession(host);
        var graph = new CppTargetGraphGenerator().Generate(
            session.HostWorkspace,
            session.HostProject.PrimaryOptions,
            session.Targets,
            "App");
        True(
            !graph.Nodes.Any(node => node.Command is not null &&
                node.Command.StartsWith("kind=file.copy", StringComparison.Ordinal) &&
                CopySourceEqualsOutput(node.Command)),
            "runtime staging must not emit a self-copy action");
    }

    private static bool CopySourceEqualsOutput(string command)
    {
        var values = command.Split('\n')
            .Select(line => line.Split('=', 2))
            .Where(parts => parts.Length == 2)
            .ToDictionary(parts => parts[0], parts => parts[1], StringComparer.Ordinal);
        return values.TryGetValue("source", out var source) &&
            values.TryGetValue("output", out var output) &&
            source.Equals(output, StringComparison.OrdinalIgnoreCase);
    }

    private static void CanonicalSymlinkIdentity()
    {
        if(OperatingSystem.IsWindows())
        {
            return;
        }

        using var scope = new TestScope();
        var imported = SimpleProject(scope, "RealParent/Imported", "Imported", "Core");
        var scopeRoot = Path.GetDirectoryName(Path.GetDirectoryName(imported)!)!;
        var link = Path.Combine(scopeRoot, "ParentLink");
        Directory.CreateSymbolicLink(link, Path.GetDirectoryName(imported)!);
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", """
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    project.ImportProject("../RealParent/Imported");
                    project.ImportProject("../ParentLink/Imported");
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App"));
        Throws<InvalidOperationException>(() => CreateSession(host), "imported more than once", "canonical symlink identity");
    }

    private static void RulesCacheInvalidation()
    {
        using var scope = new TestScope();
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", BasicProjectRules("Host", "HostRules"));
        Write(host, "Rule.Target.cs", HeaderOnlyTarget("Before"));
        var before = CreateSession(host);
        True(before.Targets.Any(target => target.QualifiedName == "Host.Before"), "initial rule assembly");

        Write(host, "Rule.Target.cs", HeaderOnlyTarget("After"));
        var after = CreateSession(host);
        True(after.Targets.Any(target => target.QualifiedName == "Host.After"), "recompiled rule assembly");
        True(after.Targets.All(target => target.QualifiedName != "Host.Before"), "stale rule type must not survive");
    }

    private static void CustomImportedBuildRoot()
    {
        using var scope = new TestScope();
        SimpleProject(scope, "Imported", "Imported", "Core");
        var customBuildRoot = scope.Project("CustomBuild");
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    var imported = project.ImportProject("../Imported");
                    imported.BuildDirectory = "{{CSharp(customBuildRoot)}}";
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App", "Imported.Core"));

        var session = CreateSession(host);
        var importedProject = session.Projects.Single(project => project.Name == "Imported");
        var expectedBuildRoot = new BuildWorkspace(importedProject.RootDirectory, customBuildRoot).BuildDirectory;
        Equal(expectedBuildRoot, importedProject.BuildDirectory, "custom build directory");
        True(
            Directory.EnumerateFiles(Path.Combine(customBuildRoot, "Rules"), "LunaBuild.ProjectRules.*.dll", SearchOption.AllDirectories).Any(),
            "rules assembly under custom build root");
    }

    private static void DuplicateBuildRoot()
    {
        using var scope = new TestScope();
        SimpleProject(scope, "Left", "Left", "Core");
        SimpleProject(scope, "Right", "Right", "Core");
        var sharedBuildRoot = scope.Project("SharedBuild");
        var host = scope.Project("Host");
        Write(host, "Host.Project.cs", $$"""
            using LunaBuild.Core;
            public sealed class HostRules : ProjectRules
            {
                public HostRules() : base("Host") {}
                protected override void ConfigureProject(Project project)
                {
                    var left = project.ImportProject("../Left");
                    var right = project.ImportProject("../Right");
                    left.BuildDirectory = "{{CSharp(sharedBuildRoot)}}";
                    right.BuildDirectory = "{{CSharp(sharedBuildRoot)}}";
                }
            }
            """);
        Write(host, "App.Target.cs", HeaderOnlyTarget("App", "Left.Core", "Right.Core"));
        Throws<InvalidOperationException>(() => CreateSession(host), "same build directory", "duplicate build root");
    }

    private static void DotNetBuildConfiguration()
    {
        using var scope = new TestScope();
        var root = scope.Project("DotNetRelease");
        Write(root, "global.json", """
            {
              "sdk": {
                "version": "9.0.100",
                "rollForward": "major"
              }
            }
            """);
        Write(root, "Probe.csproj", """
            <Project Sdk="Microsoft.NET.Sdk">
              <PropertyGroup>
                <OutputType>Exe</OutputType>
                <TargetFramework>net9.0</TargetFramework>
                <AssemblyName>releaseprobe</AssemblyName>
                <ImplicitUsings>enable</ImplicitUsings>
                <NuGetAudit>false</NuGetAudit>
              </PropertyGroup>
            </Project>
            """);
        Write(root, "Program.cs", "Console.WriteLine(\"release probe\");");

        var executable = OperatingSystem.IsWindows() ? "releaseprobe.exe" : "releaseprobe";
        var expectedOutput = Path.Combine("artifacts", "bin", "Probe", "release", executable);
        var options = BuildOptions.HostDefault() with { Mode = BuildMode.Release };
        var payload = string.Join('\n',
            "kind=dotnet.build",
            "name=Probe",
            "project=Probe.csproj",
            $"output={expectedOutput.Replace('\\', '/')}",
            "artifacts_dir=artifacts",
            "mode=Release");
        var node = new BuildGraphNode(
            Id: "file://" + expectedOutput.Replace('\\', '/'),
            Kind: BuildGraphNodeKind.File,
            Path: expectedOutput,
            Command: payload,
            Dependencies: Array.Empty<string>(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>());
        var graph = new BuildGraph(2, options, new[] { node }, new[] { node.Id });
        var context = new MakeActionContext(
            new BuildWorkspace(root),
            graph,
            node,
            "dotnet.build",
            payload,
            Array.Empty<BuildGraphNode>(),
            Array.Empty<BuildGraphNode>(),
            Array.Empty<BuildGraphNode>());

        new DotNetBuildActionExecutor(TimeSpan.FromMinutes(1))
            .ExecuteAsync(context, CancellationToken.None)
            .GetAwaiter()
            .GetResult();
        True(File.Exists(Path.Combine(root, expectedOutput)), "release .NET output exists");
    }

    private static void CppActionDescriptions()
    {
        using var scope = new TestScope();
        var root = scope.Project("CppActionDescriptions");
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault();
        var executor = new CppActionExecutor();

        var compilePayload = string.Join('\n',
            "kind=cpp.compile",
            "source=Source/Nested/alpha.cpp");
        var compileContext = ActionContext(workspace, options, "cpp.compile", compilePayload, "obj/alpha.o");
        Equal("compiling alpha.cpp", executor.GetDescription(compileContext), "compile action description");

        var linkPayload = string.Join('\n',
            "kind=cpp.link.shared",
            "output=bin/alpha.dll");
        var linkContext = ActionContext(workspace, options, "cpp.link.shared", linkPayload, "bin/alpha.dll");
        Equal("linking alpha.dll", executor.GetDescription(linkContext), "link action description");
    }

    private static void IndexedTaskProgress()
    {
        using var scope = new TestScope();
        var root = scope.Project("IndexedTaskProgress");
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault();
        var firstPath = "out/first.txt";
        var secondPath = "out/second.txt";
        var first = TestFileActionNode(firstPath, "compiling alpha.cpp", delayMilliseconds: 40);
        var second = TestFileActionNode(secondPath, "linking alpha.dll", delayMilliseconds: 1);
        var target = new BuildGraphNode(
            Id: "target://Test",
            Kind: BuildGraphNodeKind.Virtual,
            Path: null,
            Command: "kind=target.inspect\nname=Test",
            Dependencies: new[] { first.Id, second.Id },
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>());
        var graph = new BuildGraph(2, options, new[] { first, second, target }, new[] { target.Id });
        var progress = new List<string>();
        var makeSystem = new MakeSystemBackend(
            new IMakeActionExecutor[] { new TestFileActionExecutor(), new AggregateActionExecutor() },
            maxParallelism: 2);
        var buildOptions = new MakeSystemBuildOptions
        {
            Progress = progress.Add,
        };

        var result = makeSystem.BuildAsync(
                workspace,
                graph,
                options: buildOptions)
            .GetAwaiter()
            .GetResult();

        Equal(3, result.ActionsExecuted, "executed command count includes the aggregate action");
        Equal(2, progress.Count, "progress line count");
        Equal("[1/2]compiling alpha.cpp", progress[0], "first progress line");
        Equal("[2/2]linking alpha.dll", progress[1], "second progress line");
        True(File.Exists(workspace.ResolveRepositoryPath(firstPath)), "first test action output");
        True(File.Exists(workspace.ResolveRepositoryPath(secondPath)), "second test action output");

        progress.Clear();
        var upToDate = makeSystem.BuildAsync(workspace, graph, options: buildOptions).GetAwaiter().GetResult();
        True(upToDate.UpToDate, "second test action build is up to date");
        Equal(0, upToDate.ActionsExecuted, "up-to-date action count");
        Equal(0, progress.Count, "up-to-date progress line count");
    }

    private static void AggregateActionsDoNotReportProgress()
    {
        using var scope = new TestScope();
        var root = scope.Project("AggregateActions");
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault();
        var target = new BuildGraphNode(
            Id: "target://AggregateOnly",
            Kind: BuildGraphNodeKind.Virtual,
            Path: null,
            Command: "kind=target.inspect\nname=AggregateOnly",
            Dependencies: Array.Empty<string>(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>());
        var graph = new BuildGraph(2, options, new[] { target }, new[] { target.Id });
        var progress = new List<string>();
        var makeSystem = new MakeSystemBackend(new[] { new AggregateActionExecutor() });
        var buildOptions = new MakeSystemBuildOptions
        {
            Progress = progress.Add,
        };

        var first = makeSystem.BuildAsync(workspace, graph, options: buildOptions).GetAwaiter().GetResult();
        True(!first.UpToDate, "aggregate-only first build updates the cache");
        Equal(1, first.ActionsExecuted, "aggregate-only executed command count");
        Equal(0, progress.Count, "aggregate-only progress line count");

        var second = makeSystem.BuildAsync(workspace, graph, options: buildOptions).GetAwaiter().GetResult();
        True(second.UpToDate, "aggregate-only second build is up to date");
        Equal(0, progress.Count, "up-to-date aggregate progress line count");
    }

    private static MakeActionContext ActionContext(
        BuildWorkspace workspace,
        BuildOptions options,
        string actionKind,
        string payload,
        string path)
    {
        var node = new BuildGraphNode(
            Id: BuildGraphIds.File(path),
            Kind: BuildGraphNodeKind.File,
            Path: path,
            Command: payload,
            Dependencies: Array.Empty<string>(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>());
        var graph = new BuildGraph(2, options, new[] { node }, new[] { node.Id });
        return new MakeActionContext(
            workspace,
            graph,
            node,
            actionKind,
            payload,
            Array.Empty<BuildGraphNode>(),
            Array.Empty<BuildGraphNode>(),
            Array.Empty<BuildGraphNode>());
    }

    private static BuildGraphNode TestFileActionNode(string path, string description, int delayMilliseconds)
    {
        return new BuildGraphNode(
            Id: BuildGraphIds.File(path),
            Kind: BuildGraphNodeKind.File,
            Path: path,
            Command: string.Join('\n',
                "kind=test.file",
                $"output={path}",
                $"description={description}",
                $"delay={delayMilliseconds}"),
            Dependencies: Array.Empty<string>(),
            OrderOnlyDependencies: Array.Empty<string>(),
            Outputs: Array.Empty<string>(),
            Depfiles: Array.Empty<string>());
    }

    private static BuildSession CreateSession(string root)
    {
        return BuildSession.Create(new BuildWorkspace(root), definition =>
        {
            var defaults = BuildOptions.HostDefault();
            return defaults with
            {
                Properties = definition.ResolveProperties(new Dictionary<string, string?>()),
            };
        });
    }

    private static void ApplicationTargetGraph()
    {
        using var scope = new TestScope();
        var root = scope.Project("ApplicationGraph");
        Write(root, "app.cpp", "int main() { return 0; }");
        Write(root, "App.Target.cs", string.Empty);
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault() with
        {
            Platform = BuildPlatform.MacOS,
            Architecture = "arm64",
            RhiApi = RhiApi.Metal,
            Apple = BuildOptions.HostDefault().Apple with { MacOSDeploymentTarget = "12.0" },
        };
        var target = new ApplicationTargetRules().ToDefinition(workspace, options, "Host", "host", isHostProject: true);
        var graph = new CppTargetGraphGenerator().Generate(workspace, options, new[] { target }, target.QualifiedName);
        var link = graph.Nodes.Single(node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.link.executable");
        True(link.Command!.Contains("application=true", StringComparison.Ordinal), "application link marker");
        True(link.Command.Contains("target=Host.App", StringComparison.Ordinal), "qualified application target name");
        True(target.Kind.ProducesNativeExecutable(), "application is a native executable producer");

        var windowsOptions = options with
        {
            Platform = BuildPlatform.Windows,
            Architecture = "x64",
            RhiApi = RhiApi.D3D12,
        };
        var windowsTarget = new ApplicationTargetRules().ToDefinition(workspace, windowsOptions, "Host", "windows", isHostProject: true);
        var windowsGraph = new CppTargetGraphGenerator().Generate(workspace, windowsOptions, new[] { windowsTarget }, windowsTarget.QualifiedName);
        var windowsLink = windowsGraph.Nodes.Single(node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.link.executable");
        True(windowsLink.Path!.EndsWith("App.exe", StringComparison.OrdinalIgnoreCase), "Windows application executable extension");
        True(windowsLink.Command!.Contains("application=true", StringComparison.Ordinal), "Windows application link marker");
    }

    private static void AsyncDagScheduling()
    {
        using var scope = new TestScope();
        var root = scope.Project("AsyncDagScheduling");
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault();
        var slow = TestFileActionNode("out/slow.txt", "slow", delayMilliseconds: 300);
        var fast = TestFileActionNode("out/fast.txt", "fast", delayMilliseconds: 30);
        var dependent = TestFileActionNode("out/dependent.txt", "dependent", delayMilliseconds: 10) with
        {
            Dependencies = new[] { fast.Id },
        };
        var graph = new BuildGraph(2, options, new[] { slow, fast, dependent }, new[] { slow.Id, dependent.Id });

        var events = new List<string>();
        var eventsLock = new object();
        void Record(string value)
        {
            lock(eventsLock)
            {
                events.Add(value);
            }
        }

        var executor = new TestFileActionExecutor(
            onStarted: description => Record("start:" + description),
            onFinished: description => Record("finish:" + description));
        var makeSystem = new MakeSystemBackend(new[] { executor }, maxParallelism: 2);
        makeSystem.BuildAsync(workspace, graph).GetAwaiter().GetResult();

        int EventIndex(string value)
        {
            lock(eventsLock)
            {
                return events.IndexOf(value);
            }
        }

        True(EventIndex("finish:fast") < EventIndex("start:dependent"), "dependent waits for its own prerequisite");
        True(EventIndex("start:dependent") < EventIndex("finish:slow"), "dependent starts before an unrelated slow task finishes");
    }

    private static void MsvcUtf8CompilationOption()
    {
        using var scope = new TestScope();
        var root = scope.Project("MsvcUtf8");
        Write(root, "app.cpp", "const char* text = \"中文\"; int main() { return text[0] == 0; }");
        Write(root, "App.Target.cs", string.Empty);
        var workspace = new BuildWorkspace(root);
        var options = BuildOptions.HostDefault() with
        {
            Platform = BuildPlatform.Windows,
            Architecture = "x64",
            RhiApi = RhiApi.D3D12,
        };

        var defaultTarget = new ApplicationTargetRules().ToDefinition(
            workspace, options, "Host", "utf8-default", isHostProject: true);
        True(defaultTarget.EnableMsvcUtf8, "MSVC UTF-8 default");
        var defaultGraph = new CppTargetGraphGenerator().Generate(
            workspace, options, new[] { defaultTarget }, defaultTarget.QualifiedName);
        var defaultCompile = defaultGraph.Nodes.Single(
            node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.compile");
        True(defaultCompile.Command!.Contains("msvc_utf8=True", StringComparison.Ordinal),
            "enabled MSVC UTF-8 action identity");
        var defaultCommandsPath = Path.Combine(root, "compile_commands.default.json");
        CompileCommandsWriter.Write(workspace, defaultGraph, defaultCommandsPath);
        True(File.ReadAllText(defaultCommandsPath).Contains("/utf-8", StringComparison.Ordinal),
            "default MSVC UTF-8 compiler argument");

        var disabledTarget = new ApplicationTargetRules(enableMsvcUtf8: false).ToDefinition(
            workspace, options, "Host", "utf8-disabled", isHostProject: true);
        True(!disabledTarget.EnableMsvcUtf8, "disabled MSVC UTF-8 target option");
        var disabledGraph = new CppTargetGraphGenerator().Generate(
            workspace, options, new[] { disabledTarget }, disabledTarget.QualifiedName);
        var disabledCompile = disabledGraph.Nodes.Single(
            node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.compile");
        True(disabledCompile.Command!.Contains("msvc_utf8=False", StringComparison.Ordinal),
            "disabled MSVC UTF-8 action identity");
        var disabledCommandsPath = Path.Combine(root, "compile_commands.disabled.json");
        CompileCommandsWriter.Write(workspace, disabledGraph, disabledCommandsPath);
        True(!File.ReadAllText(disabledCommandsPath).Contains("/utf-8", StringComparison.Ordinal),
            "disabled MSVC UTF-8 compiler argument");
    }

    private static void AppleDeploymentSettings()
    {
        using var scope = new TestScope();
        var root = scope.Project("AppleOptions");
        Write(root, "app.cpp", "int main() { return 0; }");
        Write(root, "App.Target.cs", string.Empty);
        var workspace = new BuildWorkspace(root);
        var macOptions = BuildOptions.HostDefault() with
        {
            Platform = BuildPlatform.MacOS,
            Architecture = "arm64",
            RhiApi = RhiApi.Metal,
            Apple = BuildOptions.HostDefault().Apple with { MacOSDeploymentTarget = "12.0" },
        };
        var macTarget = new ApplicationTargetRules().ToDefinition(workspace, macOptions, "Host", "mac", isHostProject: true);
        var macGraph = new CppTargetGraphGenerator().Generate(workspace, macOptions, new[] { macTarget }, macTarget.QualifiedName);
        var macCompile = macGraph.Nodes.Single(node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.compile");
        True(macCompile.Command!.Contains("macos_deployment_target=12.0", StringComparison.Ordinal), "macOS deployment target in action identity");

        var iosOptions = macOptions with
        {
            Platform = BuildPlatform.IOS,
            Apple = macOptions.Apple with { SdkName = "iphonesimulator", IOSDeploymentTarget = "14.0" },
        };
        var segments = BuildOutputLayout.ConfigurationSegments(iosOptions);
        True(segments.Contains("iphonesimulator"), "iOS SDK isolates output directory");
        var iosTarget = new ApplicationTargetRules().ToDefinition(workspace, iosOptions, "Host", "ios", isHostProject: true);
        var iosGraph = new CppTargetGraphGenerator().Generate(workspace, iosOptions, new[] { iosTarget }, iosTarget.QualifiedName);
        var iosCompile = iosGraph.Nodes.Single(node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.compile");
        True(iosCompile.Path!.Replace('\\', '/').Contains("/IOS/iphonesimulator/arm64/Debug/", StringComparison.Ordinal), "iOS simulator object layout");
        True(iosCompile.Command!.Contains("ios_deployment_target=14.0", StringComparison.Ordinal), "iOS deployment target in action identity");
    }

    private static string SimpleProject(
        TestScope scope,
        string directory,
        string projectName,
        string targetName,
        params string[] dependencies)
    {
        var root = scope.Project(directory);
        Write(root, projectName + ".Project.cs", BasicProjectRules(projectName, projectName + "Rules"));
        Write(root, targetName + ".Target.cs", HeaderOnlyTarget(targetName, dependencies));
        return root;
    }

    private static string BasicProjectRules(string projectName, string typeName)
    {
        return $$"""
            using LunaBuild.Core;
            public sealed class {{typeName}} : ProjectRules
            {
                public {{typeName}}() : base("{{projectName}}") {}
            }
            """;
    }

    private static string HeaderOnlyTarget(string name, params string[] dependencies)
    {
        var dependencyLine = dependencies.Length == 0
            ? string.Empty
            : $"DependsOn({string.Join(", ", dependencies.Select(dependency => "\"" + dependency + "\""))});";
        return $$"""
            using LunaBuild.Core;
            public sealed class {{name}}Rules : TargetRules
            {
                public {{name}}Rules() : base("{{name}}", ".", "{{name}}.Target.cs")
                {
                    Kind = BuildTargetKind.HeaderOnly;
                    {{dependencyLine}}
                }
            }
            """;
    }

    private static string NativeTarget(string name, BuildTargetKind kind, params string[] dependencies)
    {
        var dependencyLine = dependencies.Length == 0
            ? string.Empty
            : $"DependsOn({string.Join(", ", dependencies.Select(dependency => "\"" + dependency + "\""))});";
        var source = name == "App" ? "app.cpp" : "core.cpp";
        return $$"""
            using LunaBuild.Core;
            public sealed class {{name}}Rules : TargetRules
            {
                public {{name}}Rules() : base("{{name}}", ".", "{{name}}.Target.cs")
                {
                    Kind = BuildTargetKind.{{kind}};
                    Sources("{{source}}");
                    {{dependencyLine}}
                }
            }
            """;
    }

    private static void Write(string root, string relativePath, string contents)
    {
        var path = Path.Combine(root, relativePath);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        File.WriteAllText(path, contents);
    }

    private static string CSharp(string value)
    {
        return value.Replace("\\", "\\\\", StringComparison.Ordinal).Replace("\"", "\\\"", StringComparison.Ordinal);
    }

    private static void True(bool condition, string message)
    {
        if(!condition)
        {
            throw new InvalidOperationException("Assertion failed: " + message);
        }
    }

    private static void Equal<T>(T expected, T actual, string message)
    {
        if(!EqualityComparer<T>.Default.Equals(expected, actual))
        {
            throw new InvalidOperationException($"Assertion failed: {message}. Expected `{expected}`, got `{actual}`.");
        }
    }

    private static void Throws<T>(Action action, string messageFragment, string message)
        where T : Exception
    {
        try
        {
            action();
        }
        catch(T exception)
        {
            True(exception.Message.Contains(messageFragment, StringComparison.OrdinalIgnoreCase),
                $"{message}: exception did not contain `{messageFragment}`: {exception.Message}");
            return;
        }
        throw new InvalidOperationException($"Assertion failed: {message}. Expected {typeof(T).Name}.");
    }

    private sealed class TestScope : IDisposable
    {
        private readonly string _root = Path.Combine(Path.GetTempPath(), "LunaBuild.Core.Tests", Guid.NewGuid().ToString("N"));

        public string Project(string name)
        {
            var path = Path.Combine(_root, name);
            Directory.CreateDirectory(path);
            return path;
        }

        public void Dispose()
        {
            try
            {
                if(Directory.Exists(_root))
                {
                    Directory.Delete(_root, recursive: true);
                }
            }
            catch(IOException)
            {
                // Rules assemblies may remain mapped until process exit on some platforms.
            }
            catch(UnauthorizedAccessException)
            {
                // Best-effort cleanup of temporary test fixtures.
            }
        }
    }

    private sealed class ApplicationTargetRules : TargetRules
    {
        public ApplicationTargetRules(bool? enableMsvcUtf8 = null)
            : base("App", ".", "App.Target.cs")
        {
            Kind = BuildTargetKind.Application;
            Sources("app.cpp");
            if(enableMsvcUtf8.HasValue)
            {
                MsvcUtf8(enableMsvcUtf8.Value);
            }
        }
    }

    private sealed class TestFileActionExecutor : KnownActionExecutor
    {
        private readonly Action<string>? _onStarted;
        private readonly Action<string>? _onFinished;

        public TestFileActionExecutor(Action<string>? onStarted = null, Action<string>? onFinished = null)
            : base("test.file")
        {
            _onStarted = onStarted;
            _onFinished = onFinished;
        }

        public override string GetDescription(MakeActionContext context)
        {
            return RequiredPayloadValue(context.ActionPayload, "description");
        }

        public override async Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
        {
            var description = RequiredPayloadValue(context.ActionPayload, "description");
            var delay = int.Parse(RequiredPayloadValue(context.ActionPayload, "delay"));
            var output = context.Workspace.ResolveRepositoryPath(RequiredPayloadValue(context.ActionPayload, "output"));
            _onStarted?.Invoke(description);
            await Task.Delay(delay, cancellationToken);
            Directory.CreateDirectory(Path.GetDirectoryName(output)!);
            await File.WriteAllTextAsync(output, "generated", cancellationToken);
            _onFinished?.Invoke(description);
        }

        private static string RequiredPayloadValue(string payload, string name)
        {
            var prefix = name + "=";
            foreach(var line in payload.Split('\n'))
            {
                if(line.StartsWith(prefix, StringComparison.Ordinal))
                {
                    return line[prefix.Length..];
                }
            }
            throw new InvalidOperationException($"Missing test action payload value: {name}");
        }
    }
}
