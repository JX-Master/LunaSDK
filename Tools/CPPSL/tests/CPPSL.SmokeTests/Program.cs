using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;

var repoRoot = FindRepoRoot(AppContext.BaseDirectory);
var outputDir = Path.Combine(repoRoot, "build", "cppsl-smoke");
var compiler = new CppslCompiler();

var result = compiler.Compile(new CppslCompileOptions(
    Path.Combine(repoRoot, "Tools", "CPPSL", "samples", "Box.cxx"),
    outputDir,
    new[] { Path.Combine(repoRoot, "Tools", "CPPSL", "std") },
    "main_vs",
    ShaderStage.Vertex));

foreach (var diagnostic in result.Diagnostics)
{
    Console.Error.WriteLine(diagnostic.ToDisplayString());
}

if (!result.Succeeded)
{
    return 1;
}

if (result.Artifacts is null ||
    !File.Exists(result.Artifacts.IrPath) ||
    !File.Exists(result.Artifacts.ReflectionPath) ||
    !File.Exists(result.Artifacts.HlslPath) ||
    !File.Exists(result.Artifacts.GlslPath) ||
    !File.Exists(result.Artifacts.MslPath))
{
    Console.Error.WriteLine("error: expected CPPSL smoke artifacts were not generated.");
    return 1;
}

var irText = File.ReadAllText(result.Artifacts.IrPath);
if (!irText.Contains("main_vs", StringComparison.Ordinal) ||
    !irText.Contains("CXCursor_FunctionDecl", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected ClangSharp declarations were not written to the IR placeholder.");
    return 1;
}

var badIncludeDir = Path.Combine(outputDir, "bad");
Directory.CreateDirectory(badIncludeDir);
var badShader = Path.Combine(badIncludeDir, "Bad.cxx");
File.WriteAllText(badShader, "#include <Luna/RHI/RHI.hpp>\n");
var badResult = compiler.Compile(new CppslCompileOptions(
    badShader,
    Path.Combine(outputDir, "bad-out"),
    new[] { Path.Combine(repoRoot, "Tools", "CPPSL", "std") },
    "main",
    ShaderStage.Vertex));

if (badResult.Succeeded ||
    !badResult.Diagnostics.Any(d => d.Severity == DiagnosticSeverity.Error))
{
    Console.Error.WriteLine("error: expected bad include to fail CPPSL validation.");
    return 1;
}

Console.WriteLine("CPPSL smoke tests passed.");
return 0;

static string FindRepoRoot(string start)
{
    var dir = new DirectoryInfo(start);
    while (dir is not null)
    {
        if (File.Exists(Path.Combine(dir.FullName, "xmake.lua")) &&
            Directory.Exists(Path.Combine(dir.FullName, "Tools")))
        {
            return dir.FullName;
        }
        dir = dir.Parent;
    }
    throw new InvalidOperationException("Could not find LunaSDK repository root.");
}
