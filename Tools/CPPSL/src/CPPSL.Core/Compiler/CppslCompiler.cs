using System.Text.Json;
using CPPSL.Core.Artifacts;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.Frontend;
using CPPSL.Core.Reflection;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Compiler;

public sealed class CppslCompiler
{
    private readonly ICppslFrontend _frontend;

    public CppslCompiler()
        : this(new ClangSharpFrontend())
    {
    }

    public CppslCompiler(ICppslFrontend frontend)
    {
        _frontend = frontend;
    }

    public CppslCompileResult Compile(CppslCompileOptions options)
    {
        var diagnostics = new List<CppslDiagnostic>();
        var sourcePath = Path.GetFullPath(options.SourcePath);
        var outputDirectory = Path.GetFullPath(options.OutputDirectory);
        var includeRoots = options.IncludeRoots.Select(Path.GetFullPath).ToArray();

        if (Path.GetExtension(sourcePath) != ".cxx")
        {
            diagnostics.Add(CppslDiagnostic.Error("CPPSL entry files must use the `.cxx` extension.", sourcePath));
        }
        if (!File.Exists(sourcePath))
        {
            diagnostics.Add(CppslDiagnostic.Error("CPPSL source file does not exist.", sourcePath));
        }
        foreach (var includeRoot in includeRoots)
        {
            if (!Directory.Exists(includeRoot))
            {
                diagnostics.Add(CppslDiagnostic.Error("CPPSL include root does not exist.", includeRoot));
            }
        }
        if (diagnostics.Count != 0)
        {
            return new CppslCompileResult(false, diagnostics, null);
        }

        var scanner = new IncludeScanner(includeRoots);
        var includeResult = scanner.ScanTransitive(sourcePath);
        diagnostics.AddRange(includeResult.Diagnostics);
        if (diagnostics.Any(static d => d.Severity == DiagnosticSeverity.Error))
        {
            return new CppslCompileResult(false, diagnostics, null);
        }

        var frontendResult = _frontend.Parse(new CppslFrontendOptions(sourcePath, includeRoots));
        diagnostics.AddRange(frontendResult.Diagnostics);
        if (!frontendResult.Succeeded)
        {
            return new CppslCompileResult(false, diagnostics, null);
        }
        var semanticModel = new CppslSemanticModelBuilder().Build(options, sourcePath, frontendResult.AstNodes);
        diagnostics.AddRange(new CppslSemanticValidator().Validate(options, semanticModel));
        if (diagnostics.Any(static d => d.Severity == DiagnosticSeverity.Error))
        {
            return new CppslCompileResult(false, diagnostics, null);
        }

        Directory.CreateDirectory(outputDirectory);

        var baseName = Path.GetFileNameWithoutExtension(sourcePath);
        var artifacts = new CppslArtifacts(
            Path.Combine(outputDirectory, baseName + ".ir.json"),
            Path.Combine(outputDirectory, baseName + ".reflection.json"),
            Path.Combine(outputDirectory, baseName + ".hlsl"),
            Path.Combine(outputDirectory, baseName + ".glsl"),
            Path.Combine(outputDirectory, baseName + ".msl"));

        var reflection = new CppslReflectionBuilder().Build(options, sourcePath, semanticModel);
        WritePlaceholderArtifacts(options, sourcePath, includeResult.Files, frontendResult, semanticModel, reflection, artifacts);
        diagnostics.Add(CppslDiagnostic.Info("CPPSL phase 0 validation completed.", sourcePath));
        return new CppslCompileResult(true, diagnostics, artifacts);
    }

    private static void WritePlaceholderArtifacts(
        CppslCompileOptions options,
        string sourcePath,
        IReadOnlyList<string> files,
        CppslFrontendResult frontendResult,
        CppslSemanticModel semanticModel,
        CppslReflectionModel reflection,
        CppslArtifacts artifacts)
    {
        var artifactModel = new
        {
            language = "CPPSL",
            phase = 0,
            source = sourcePath,
            entryPoint = options.EntryPoint,
            stage = options.Stage.ToString(),
            frontendProvider = frontendResult.Provider,
            files,
            frontendDeclarations = frontendResult.Declarations,
            frontendAst = frontendResult.AstNodes,
            cppslSemanticModel = semanticModel,
            note = "Placeholder artifact. Luna Shader IR lowering is not implemented yet."
        };

        var jsonOptions = new JsonSerializerOptions { WriteIndented = true };
        File.WriteAllText(artifacts.IrPath, JsonSerializer.Serialize(artifactModel, jsonOptions));
        File.WriteAllText(artifacts.ReflectionPath, JsonSerializer.Serialize(reflection, jsonOptions));

        File.WriteAllText(artifacts.HlslPath, MakePlaceholderSource("HLSL", options));
        File.WriteAllText(artifacts.GlslPath, MakePlaceholderSource("GLSL", options));
        File.WriteAllText(artifacts.MslPath, MakePlaceholderSource("MSL", options));
    }

    private static string MakePlaceholderSource(string target, CppslCompileOptions options)
    {
        return $"""
        // Generated by cppslc phase 0.
        // Target: {target}
        // Entry: {options.EntryPoint}
        // Stage: {options.Stage}
        // TODO: lower Luna Shader IR to {target}.
        """;
    }
}
