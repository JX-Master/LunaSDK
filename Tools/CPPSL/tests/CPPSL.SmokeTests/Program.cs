using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;
using System.Text.RegularExpressions;

var repoRoot = FindRepoRoot(AppContext.BaseDirectory);
var fixturesRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "tests", "fixtures");
var stdRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "std");
var outputDir = Path.Combine(repoRoot, "build", "cppsl-smoke");
var compiler = new CppslCompiler();
var boxShader = Path.Combine(fixturesRoot, "valid", "box", "Box.cxx");
var textureBasicShader = Path.Combine(fixturesRoot, "valid", "texture_basic", "TextureBasic.cxx");

var result = compiler.Compile(new CppslCompileOptions(
    boxShader,
    outputDir,
    new[] { stdRoot },
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
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl)))
{
    Console.Error.WriteLine("error: expected CPPSL smoke artifacts were not generated.");
    return 1;
}

var irText = File.ReadAllText(result.Artifacts.IrPath);
var reflectionText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var hlslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var glslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var mslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!irText.Contains("main_vs", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendProvider\": \"Native\"", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendModelVersion\": 2", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendAst\"", StringComparison.Ordinal) ||
    !irText.Contains("\"outputTargets\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Function\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"Function\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Field\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"Field\"", StringComparison.Ordinal) ||
    !irText.Contains("world_to_proj", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Parameter\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"ParmVar\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Range\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"TypeInfo\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"CanonicalName\": \"struct cppsl::ConstantBuffer", StringComparison.Ordinal) ||
    !irText.Contains("\"TemplateArguments\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Spelling\": \"Camera\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ResultTypeInfo\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"CompoundStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"DeclarationStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"LocalVariable\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Spelling\": \"output\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"CallExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"DisplayName\": \"mul\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"MemberExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"DeclRefExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"ReturnStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("cppslSemanticModel", StringComparison.Ordinal) ||
    !irText.Contains("cppslIr", StringComparison.Ordinal) ||
    !irText.Contains("\"Schema\": \"cppsl.ir\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Version\": 1", StringComparison.Ordinal) ||
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
    Console.Error.WriteLine("error: expected frontend, semantic, or IR facts were not written.");
    return 1;
}
if (!Regex.IsMatch(
        irText,
        "\"cppslIr\"\\s*:\\s*\\{.*?\"Version\"\\s*:\\s*1.*?\"EntryPoints\"\\s*:\\s*\\[.*?\"Name\"\\s*:\\s*\"main_vs\".*?\"Body\"\\s*:\\s*\\{.*?\"Kind\"\\s*:\\s*\"CompoundStatement\".*?\"Kind\"\\s*:\\s*\"LocalVariable\".*?\"DisplayName\"\\s*:\\s*\"mul\".*?\"Kind\"\\s*:\\s*\"ReturnStatement\"",
        RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: expected entry point body AST skeleton was not written to CPPSL IR.");
    return 1;
}
if (!reflectionText.Contains("\"Schema\": \"cppsl.reflection\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Version\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"EntryPoint\": \"main_vs\"", StringComparison.Ordinal) ||
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
if (!hlslText.Contains("struct VSInput", StringComparison.Ordinal) ||
    !hlslText.Contains("float4 position : SV_Position", StringComparison.Ordinal) ||
    !hlslText.Contains("ConstantBuffer<Camera> camera : register(b0, space0);", StringComparison.Ordinal) ||
    !hlslText.Contains("VSOutput main_vs(VSInput input)", StringComparison.Ordinal) ||
    !glslText.Contains("#version 450", StringComparison.Ordinal) ||
    !glslText.Contains("mat4 world_to_proj", StringComparison.Ordinal) ||
    !glslText.Contains("layout(set = 0, binding = 0) uniform camera_Block", StringComparison.Ordinal) ||
    !glslText.Contains("return VSOutput(vec4(0.0), vec2(0.0));", StringComparison.Ordinal) ||
    !mslText.Contains("#include <metal_stdlib>", StringComparison.Ordinal) ||
    !mslText.Contains("float4 position [[position]]", StringComparison.Ordinal) ||
    !mslText.Contains("constant Camera& camera [[buffer(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("vertex VSOutput main_vs", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL shader source targets were not emitted.");
    return 1;
}

var textureResult = compiler.Compile(new CppslCompileOptions(
    textureBasicShader,
    Path.Combine(outputDir, "texture-basic"),
    new[] { stdRoot },
    "main_ps",
    ShaderStage.Fragment));

if (!textureResult.Succeeded || textureResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected texture_basic fixture to compile.");
    foreach (var diagnostic in textureResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var textureReflectionText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var textureHlslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var textureGlslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var textureMslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!textureReflectionText.Contains("\"Name\": \"color_texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"Name\": \"linear_sampler\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"sampler\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture_basic reflection resources were not written.");
    return 1;
}
if (!textureHlslText.Contains("Texture2D<float4> color_texture : register(t0, space0);", StringComparison.Ordinal) ||
    !textureHlslText.Contains("SamplerState linear_sampler : register(s1, space0);", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform texture2D color_texture", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform sampler linear_sampler", StringComparison.Ordinal) ||
    !textureMslText.Contains("fragment PSOutput main_ps", StringComparison.Ordinal) ||
    !textureMslText.Contains("texture2d<float> color_texture [[texture(0)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("sampler linear_sampler [[sampler(1)]]", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture_basic shader source resources were not emitted.");
    return 1;
}

var invalidFixtures = new[]
{
    new InvalidFixture("forbidden_include", "ForbiddenInclude.cxx", "main", ShaderStage.Vertex, "include"),
    new InvalidFixture("stage_mismatch", "StageMismatch.cxx", "main_vs", ShaderStage.Vertex, "declares `compute`"),
    new InvalidFixture("missing_binding", "MissingBinding.cxx", "main_vs", ShaderStage.Vertex, "binding"),
    new InvalidFixture("duplicate_location", "DuplicateLocation.cxx", "main_vs", ShaderStage.Vertex, "duplicate location"),
    new InvalidFixture("unknown_attribute", "UnknownAttribute.cxx", "main_vs", ShaderStage.Vertex, "unknown CPPSL attribute"),
    new InvalidFixture("duplicate_binding", "DuplicateBinding.cxx", "main_vs", ShaderStage.Vertex, "duplicate resource binding"),
    new InvalidFixture("invalid_attribute_target", "InvalidAttributeTarget.cxx", "main_vs", ShaderStage.Vertex, "cannot be used on field")
};

foreach (var fixture in invalidFixtures)
{
    if (!ExpectFixtureFailure(compiler, fixturesRoot, outputDir, stdRoot, fixture))
    {
        return 1;
    }
}

var reflectionOnlyDir = Path.Combine(outputDir, "reflection-only");
var reflectionOnlyResult = compiler.Compile(new CppslCompileOptions(
    boxShader,
    reflectionOnlyDir,
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex,
    new[] { CppslOutputTarget.Reflection }));

if (!reflectionOnlyResult.Succeeded ||
    reflectionOnlyResult.Artifacts is null ||
    !File.Exists(reflectionOnlyResult.Artifacts.IrPath) ||
    !File.Exists(reflectionOnlyResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection)) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Hlsl) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Glsl) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Msl))
{
    Console.Error.WriteLine("error: expected CPPSL reflection-only target emission to work.");
    return 1;
}

var nativeSyntaxErrorShader = Path.Combine(fixturesRoot, "invalid", "syntax_error", "SyntaxError.cxx");
var nativeSyntaxErrorResult = compiler.Compile(new CppslCompileOptions(
    nativeSyntaxErrorShader,
    Path.Combine(outputDir, "native-syntax-error"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex,
    new[] { CppslOutputTarget.Reflection }));

if (nativeSyntaxErrorResult.Succeeded ||
    !nativeSyntaxErrorResult.Diagnostics.Any(static diagnostic =>
        diagnostic.Severity == DiagnosticSeverity.Error &&
        diagnostic.Message.Contains("expected", StringComparison.OrdinalIgnoreCase) &&
        diagnostic.File is not null &&
        diagnostic.Line is not null &&
        diagnostic.Column is not null))
{
    Console.Error.WriteLine("error: expected native extractor frontend to report structured Clang diagnostics.");
    foreach (var diagnostic in nativeSyntaxErrorResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

Console.WriteLine("CPPSL smoke tests passed.");
return 0;

static bool ExpectFixtureFailure(
    CppslCompiler compiler,
    string fixturesRoot,
    string outputDir,
    string stdRoot,
    InvalidFixture fixture)
{
    var shaderPath = Path.Combine(fixturesRoot, "invalid", fixture.Directory, fixture.FileName);
    var result = compiler.Compile(new CppslCompileOptions(
        shaderPath,
        Path.Combine(outputDir, "invalid", fixture.Directory),
        new[] { stdRoot },
        fixture.EntryPoint,
        fixture.Stage));

    if (result.Succeeded ||
        !result.Diagnostics.Any(d => d.Severity == DiagnosticSeverity.Error))
    {
        Console.Error.WriteLine($"error: expected fixture `{fixture.Directory}` to fail CPPSL validation.");
        return false;
    }

    if (!result.Diagnostics.Any(d => d.Message.Contains(fixture.ExpectedDiagnosticFragment, StringComparison.OrdinalIgnoreCase)))
    {
        Console.Error.WriteLine($"error: fixture `{fixture.Directory}` did not report expected diagnostic fragment `{fixture.ExpectedDiagnosticFragment}`.");
        foreach (var diagnostic in result.Diagnostics)
        {
            Console.Error.WriteLine(diagnostic.ToDisplayString());
        }
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

internal sealed record InvalidFixture(
    string Directory,
    string FileName,
    string EntryPoint,
    ShaderStage Stage,
    string ExpectedDiagnosticFragment);
