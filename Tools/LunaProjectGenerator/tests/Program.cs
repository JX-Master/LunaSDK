using LunaProjectGenerator;

var tests = new (string Name, Action Run)[]
{
    ("creates a standalone LunaSDK project", CreatesProject),
    ("supports an explicit hyphenated project name", ExplicitProjectName),
    ("rejects a non-empty destination", RejectsNonEmptyDestination),
    ("rejects nested SDK and project trees", RejectsNestedTrees),
};

foreach(var test in tests)
{
    try
    {
        test.Run();
        Console.WriteLine($"PASS {test.Name}");
    }
    catch(Exception ex)
    {
        Console.Error.WriteLine($"FAIL {test.Name}");
        Console.Error.WriteLine(ex);
        return 1;
    }
}

Console.WriteLine($"Passed {tests.Length} Luna project generator tests.");
return 0;

static void CreatesProject()
{
    WithFixture((sdkRoot, projectsRoot) =>
    {
        var projectRoot = Path.Combine(projectsRoot, "SampleApp");
        var result = ProjectGenerator.Create(new ProjectGeneratorOptions(sdkRoot, projectRoot));

        Equal("SampleApp", result.ProjectName, "inferred project name");
        Exists(projectRoot, "global.json");
        Exists(projectRoot, "LunaBuild.csproj");
        Exists(projectRoot, "SampleApp.Project.cs");
        Exists(projectRoot, "SampleApp.Target.cs");
        Exists(projectRoot, "Source", "Main.cpp");
        Contains(projectRoot, "SampleApp.Project.cs", "project.ImportProject(\"../../LunaSDK\")");
        Contains(projectRoot, "SampleApp.Target.cs", "DependsOn(\"LunaSDK.Runtime\")");
        Contains(projectRoot, "LunaBuild.csproj", "<LunaSdkRoot>../../LunaSDK</LunaSdkRoot>");
        Contains(projectRoot, "global.json", "\"rollForward\": \"latestFeature\"");
    });
}

static void ExplicitProjectName()
{
    WithFixture((sdkRoot, projectsRoot) =>
    {
        var projectRoot = Path.Combine(projectsRoot, "source-directory");
        ProjectGenerator.Create(new ProjectGeneratorOptions(sdkRoot, projectRoot, "My-Game"));

        Exists(projectRoot, "My-Game.Project.cs");
        Contains(projectRoot, "My-Game.Project.cs", "class My_GameProjectRules");
        Contains(projectRoot, "My-Game.Target.cs", "class My_GameTargetRules");
    });
}

static void RejectsNonEmptyDestination()
{
    WithFixture((sdkRoot, projectsRoot) =>
    {
        var projectRoot = Path.Combine(projectsRoot, "Occupied");
        Directory.CreateDirectory(projectRoot);
        var sentinel = Path.Combine(projectRoot, "keep.txt");
        File.WriteAllText(sentinel, "keep");

        Throws<IOException>(() => ProjectGenerator.Create(new ProjectGeneratorOptions(sdkRoot, projectRoot)));
        Equal("keep", File.ReadAllText(sentinel), "existing destination content");
    });
}

static void RejectsNestedTrees()
{
    var root = NewTemporaryRoot();
    try
    {
        var sdkRoot = CreateFakeSdk(root);
        Throws<ArgumentException>(() => ProjectGenerator.Create(
            new ProjectGeneratorOptions(sdkRoot, Path.Combine(sdkRoot, "Projects", "Nested"))));
    }
    finally
    {
        Directory.Delete(root, recursive: true);
    }
}

static void WithFixture(Action<string, string> test)
{
    var root = NewTemporaryRoot();
    try
    {
        var sdkRoot = CreateFakeSdk(root);
        var projectsRoot = Path.Combine(root, "Projects");
        Directory.CreateDirectory(projectsRoot);
        test(sdkRoot, projectsRoot);
    }
    finally
    {
        Directory.Delete(root, recursive: true);
    }
}

static string NewTemporaryRoot()
{
    var root = Path.Combine(Path.GetTempPath(), "LunaProjectGenerator.Tests", Guid.NewGuid().ToString("N"));
    Directory.CreateDirectory(root);
    return root;
}

static string CreateFakeSdk(string root)
{
    var sdkRoot = Path.Combine(root, "LunaSDK");
    foreach(var path in new[]
    {
        "LunaBuild.csproj",
        "LunaSDK.Project.cs",
        "Tools/LunaBuild/src/LunaBuild.Cli/Program.cs",
        "Tools/LunaBuild/src/LunaBuild.Cli/LunaBuildCli.cs",
        "Tools/LunaBuild/src/LunaBuild.Core/LunaBuild.Core.csproj",
    })
    {
        var fullPath = Path.Combine(sdkRoot, path.Replace('/', Path.DirectorySeparatorChar));
        Directory.CreateDirectory(Path.GetDirectoryName(fullPath)!);
        File.WriteAllText(fullPath, string.Empty);
    }
    return sdkRoot;
}

static void Exists(params string[] path)
{
    var fullPath = Path.Combine(path);
    if(!File.Exists(fullPath))
    {
        throw new InvalidOperationException($"Expected file: {fullPath}");
    }
}

static void Contains(string root, string relativePath, string expected)
{
    var content = File.ReadAllText(Path.Combine(root, relativePath));
    if(!content.Contains(expected, StringComparison.Ordinal))
    {
        throw new InvalidOperationException($"Expected `{relativePath}` to contain `{expected}`.");
    }
}

static void Equal(string expected, string actual, string description)
{
    if(!string.Equals(expected, actual, StringComparison.Ordinal))
    {
        throw new InvalidOperationException($"Expected {description} `{expected}`, got `{actual}`.");
    }
}

static void Throws<T>(Action action)
    where T : Exception
{
    try
    {
        action();
    }
    catch(T)
    {
        return;
    }
    throw new InvalidOperationException($"Expected {typeof(T).Name}.");
}
