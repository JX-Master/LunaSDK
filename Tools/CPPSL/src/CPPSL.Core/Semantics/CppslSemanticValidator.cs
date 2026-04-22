using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;

namespace CPPSL.Core.Semantics;

public sealed class CppslSemanticValidator
{
    public IReadOnlyList<CppslDiagnostic> Validate(CppslCompileOptions options, CppslSemanticModel model)
    {
        var diagnostics = new List<CppslDiagnostic>();

        ValidateEntryPoint(options, model, diagnostics);
        ValidateResources(model, diagnostics);
        ValidateStructLocations(model, diagnostics);

        return diagnostics;
    }

    private static void ValidateEntryPoint(
        CppslCompileOptions options,
        CppslSemanticModel model,
        List<CppslDiagnostic> diagnostics)
    {
        var entryPoint = model.Functions.FirstOrDefault(function => function.Name == options.EntryPoint);
        if (entryPoint is null)
        {
            diagnostics.Add(CppslDiagnostic.Error(
                $"CPPSL entry point `{options.EntryPoint}` was not found."));
            return;
        }

        if (entryPoint.DeclaredStage is null)
        {
            diagnostics.Add(CppslDiagnostic.Error(
                $"CPPSL entry point `{entryPoint.Name}` must declare a shader stage attribute.",
                entryPoint.File,
                entryPoint.Line,
                entryPoint.Column));
            return;
        }

        if (!StageMatches(options.Stage, entryPoint.DeclaredStage))
        {
            diagnostics.Add(CppslDiagnostic.Error(
                $"CPPSL entry point `{entryPoint.Name}` declares `{entryPoint.DeclaredStage}` but compile request uses `{ToAttributeStage(options.Stage)}`.",
                entryPoint.File,
                entryPoint.Line,
                entryPoint.Column));
        }
    }

    private static void ValidateResources(CppslSemanticModel model, List<CppslDiagnostic> diagnostics)
    {
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            if (global.DescriptorSet is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL resource `{global.Name}` must declare `cppsl::set(...)`.",
                    global.File,
                    global.Line,
                    global.Column));
            }
            if (global.Binding is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL resource `{global.Name}` must declare `cppsl::binding(...)`.",
                    global.File,
                    global.Line,
                    global.Column));
            }
        }
    }

    private static void ValidateStructLocations(CppslSemanticModel model, List<CppslDiagnostic> diagnostics)
    {
        foreach (var structure in model.Structs)
        {
            foreach (var duplicateGroup in structure.Fields
                .Where(static field => field.Location is not null)
                .GroupBy(static field => field.Location!.Value)
                .Where(static group => group.Count() > 1))
            {
                foreach (var field in duplicateGroup.Skip(1))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL struct `{structure.Name}` declares duplicate location `{duplicateGroup.Key}`.",
                        field.File,
                        field.Line,
                        field.Column));
                }
            }

            var positionFields = structure.Fields.Where(static field => field.IsPosition).ToArray();
            foreach (var field in positionFields.Skip(1))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL struct `{structure.Name}` may declare only one `cppsl::position` field.",
                    field.File,
                    field.Line,
                    field.Column));
            }
        }
    }

    public static bool StageMatches(ShaderStage requestedStage, string declaredStage)
    {
        var requested = ToAttributeStage(requestedStage);
        return requested == declaredStage ||
            requested == "fragment" && declaredStage == "pixel" ||
            requested == "pixel" && declaredStage == "fragment";
    }

    public static string ToAttributeStage(ShaderStage stage)
    {
        return stage switch
        {
            ShaderStage.Vertex => "vertex",
            ShaderStage.Fragment => "fragment",
            ShaderStage.Pixel => "pixel",
            ShaderStage.Compute => "compute",
            ShaderStage.RayGen => "raygen",
            ShaderStage.Miss => "miss",
            ShaderStage.ClosestHit => "closest_hit",
            ShaderStage.AnyHit => "any_hit",
            ShaderStage.Intersection => "intersection",
            ShaderStage.Callable => "callable",
            _ => stage.ToString().ToLowerInvariant()
        };
    }
}
