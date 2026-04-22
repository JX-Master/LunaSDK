using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;
using System.Text.RegularExpressions;

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
var reflectionText = File.ReadAllText(result.Artifacts.ReflectionPath);
if (!irText.Contains("main_vs", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendProvider\": \"ClangSharp\"", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendAst\"", StringComparison.Ordinal) ||
    !irText.Contains("CXCursor_FunctionDecl", StringComparison.Ordinal) ||
    !irText.Contains("CXCursor_FieldDecl", StringComparison.Ordinal) ||
    !irText.Contains("world_to_proj", StringComparison.Ordinal) ||
    !irText.Contains("CXCursor_ParmDecl", StringComparison.Ordinal) ||
    !irText.Contains("cppslSemanticModel", StringComparison.Ordinal) ||
    !irText.Contains("constant_buffer", StringComparison.Ordinal) ||
    !irText.Contains("\"IsEntryPoint\": true", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"set\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"binding\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"location\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"position\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"vertex\"", StringComparison.Ordinal) ||
    !irText.Contains("\"DescriptorSet\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"Binding\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"Location\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"IsPosition\": true", StringComparison.Ordinal) ||
    !irText.Contains("\"DeclaredStage\": \"vertex\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected frontend AST facts were not written to the IR placeholder.");
    return 1;
}
if (!reflectionText.Contains("\"EntryPoint\": \"main_vs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Descriptors\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Name\": \"camera\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"ResourceKind\": \"constant_buffer\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Set\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Binding\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"StageInputs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Struct\": \"VSInput\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"StageOutputs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Struct\": \"VSOutput\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"IsPosition\": true", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL reflection facts were not written.");
    return 1;
}
if (irText.Contains("\"Name\": \"output\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: local variables must not be classified as CPPSL globals.");
    return 1;
}
if (!Regex.IsMatch(irText, "\"Name\": \"input\".*?\"Attributes\": \\[\\s*\\]", RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: function parameters must not inherit function attributes.");
    return 1;
}

var stdRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "std");
if (!ExpectFailure(
        compiler,
        outputDir,
        stdRoot,
        "BadInclude.cxx",
        "#include <Luna/RHI/RHI.hpp>\n",
        "main",
        ShaderStage.Vertex,
        "bad include"))
{
    return 1;
}

if (!ExpectFailure(
        compiler,
        outputDir,
        stdRoot,
        "StageMismatch.cxx",
        """
        #include <cppsl/core.hxx>

        using namespace cppsl;

        [[cppsl::compute]]
        void main_vs()
        {
        }
        """,
        "main_vs",
        ShaderStage.Vertex,
        "stage mismatch"))
{
    return 1;
}

if (!ExpectFailure(
        compiler,
        outputDir,
        stdRoot,
        "MissingBinding.cxx",
        """
        #include <cppsl/core.hxx>
        #include <cppsl/resource.hxx>

        using namespace cppsl;

        struct Camera
        {
            float4x4 world_to_proj;
        };

        [[cppsl::set(0)]]
        ConstantBuffer<Camera> camera;

        [[cppsl::vertex]]
        void main_vs()
        {
        }
        """,
        "main_vs",
        ShaderStage.Vertex,
        "missing resource binding"))
{
    return 1;
}

if (!ExpectFailure(
        compiler,
        outputDir,
        stdRoot,
        "DuplicateLocation.cxx",
        """
        #include <cppsl/core.hxx>
        #include <cppsl/math.hxx>

        using namespace cppsl;

        struct VSInput
        {
            [[cppsl::location(0)]] float3 position;
            [[cppsl::location(0)]] float2 texcoord;
        };

        [[cppsl::vertex]]
        void main_vs(VSInput input)
        {
        }
        """,
        "main_vs",
        ShaderStage.Vertex,
        "duplicate struct location"))
{
    return 1;
}

Console.WriteLine("CPPSL smoke tests passed.");
return 0;

static bool ExpectFailure(
    CppslCompiler compiler,
    string outputDir,
    string stdRoot,
    string fileName,
    string source,
    string entryPoint,
    ShaderStage stage,
    string caseName)
{
    var badCaseDir = Path.Combine(outputDir, "bad");
    Directory.CreateDirectory(badCaseDir);
    var shaderPath = Path.Combine(badCaseDir, fileName);
    File.WriteAllText(shaderPath, source);

    var result = compiler.Compile(new CppslCompileOptions(
        shaderPath,
        Path.Combine(outputDir, "bad-out", Path.GetFileNameWithoutExtension(fileName)),
        new[] { stdRoot },
        entryPoint,
        stage));

    if (result.Succeeded ||
        !result.Diagnostics.Any(d => d.Severity == DiagnosticSeverity.Error))
    {
        Console.Error.WriteLine($"error: expected {caseName} to fail CPPSL validation.");
        return false;
    }

    return true;
}

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
