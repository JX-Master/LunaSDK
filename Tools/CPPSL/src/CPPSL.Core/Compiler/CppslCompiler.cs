using System.Text.Json;
using System.Text.Json.Serialization;
using CPPSL.Core.Artifacts;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.Frontend;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Output;
using CPPSL.Core.Reflection;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Compiler;

public sealed class CppslCompiler
{
    private readonly ICppslFrontend _frontend;

    public CppslCompiler()
        : this(new NativeExtractorFrontend())
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
        var targets = CppslOutputTargets.Normalize(options.Targets);

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

        var shaderModel = new CppslShaderModelBuilder().Build(options, sourcePath, semanticModel, frontendResult.AstNodes);
        diagnostics.AddRange(new CppslSemanticValidator().ValidateShaderModel(shaderModel));
        if (diagnostics.Any(static d => d.Severity == DiagnosticSeverity.Error))
        {
            return new CppslCompileResult(false, diagnostics, null);
        }

        Directory.CreateDirectory(outputDirectory);

        var baseName = Path.GetFileNameWithoutExtension(sourcePath);
        var outputs = targets.ToDictionary(
            static target => target,
            target => Path.Combine(outputDirectory, baseName + target.GetFileExtension()));
        var artifacts = new CppslArtifacts(
            Path.Combine(outputDirectory, baseName + ".shader-model.json"),
            outputs);

        WriteArtifacts(options, sourcePath, includeResult.Files, frontendResult, semanticModel, shaderModel, targets, artifacts);
        diagnostics.Add(CppslDiagnostic.Info("CPPSL compilation completed.", sourcePath));
        return new CppslCompileResult(true, diagnostics, artifacts);
    }

    private static void WriteArtifacts(
        CppslCompileOptions options,
        string sourcePath,
        IReadOnlyList<string> files,
        CppslFrontendResult frontendResult,
        CppslSemanticModel semanticModel,
        CppslShaderModel shaderModel,
        IReadOnlyList<CppslOutputTarget> targets,
        CppslArtifacts artifacts)
    {
        var artifactModel = new
        {
            language = "CPPSL",
            source = sourcePath,
            entryPoint = options.EntryPoint,
            stage = options.Stage.ToString(),
            frontendProvider = frontendResult.Provider,
            frontendModelVersion = frontendResult.ModelVersion,
            outputTargets = targets.Select(static target => target.ToString()).ToArray(),
            files,
            frontendDeclarations = frontendResult.Declarations,
            frontendAst = frontendResult.AstNodes,
            cppslShaderModel = shaderModel,
            cppslSemanticModel = semanticModel
        };

        var jsonOptions = CreateJsonOptions();
        File.WriteAllText(artifacts.ShaderModelPath, JsonSerializer.Serialize(artifactModel, jsonOptions));

        foreach (var target in targets)
        {
            File.WriteAllText(artifacts.GetOutputPath(target), EmitTarget(target, options, sourcePath, semanticModel, shaderModel, jsonOptions));
        }
    }

    private static string EmitTarget(
        CppslOutputTarget target,
        CppslCompileOptions options,
        string sourcePath,
        CppslSemanticModel semanticModel,
        CppslShaderModel shaderModel,
        JsonSerializerOptions jsonOptions)
    {
        if (target == CppslOutputTarget.Reflection)
        {
            var reflection = new CppslReflectionBuilder().Build(options, sourcePath, semanticModel);
            return JsonSerializer.Serialize(reflection, jsonOptions);
        }

        return new CppslShaderSourceEmitter().Emit(target, options, semanticModel, shaderModel);
    }

    private static JsonSerializerOptions CreateJsonOptions()
    {
        var options = new JsonSerializerOptions { WriteIndented = true };
        options.Converters.Add(new JsonStringEnumConverter());
        return options;
    }

}
