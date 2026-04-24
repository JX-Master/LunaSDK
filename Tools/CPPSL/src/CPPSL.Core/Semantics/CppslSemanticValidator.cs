using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;

namespace CPPSL.Core.Semantics;

public sealed class CppslSemanticValidator
{
    private static readonly HashSet<string> StageAttributes = new(StringComparer.Ordinal)
    {
        "vertex",
        "fragment",
        "pixel",
        "compute",
        "raygen",
        "miss",
        "closest_hit",
        "any_hit",
        "intersection",
        "callable"
    };

    private static readonly HashSet<string> KnownAttributes = new(StringComparer.Ordinal)
    {
        "desc_set",
        "binding",
        "location",
        "position",
        "builtin",
        "group_shared",
        "cbuffer",
        "structured_buffer",
        "sbuffer",
        "rwstructured_buffer",
        "rw_structured_buffer",
        "rwsbuffer"
    };

    public IReadOnlyList<CppslDiagnostic> Validate(CppslCompileOptions options, CppslSemanticModel model)
    {
        var diagnostics = new List<CppslDiagnostic>();

        ValidateAttributes(model, diagnostics);
        ValidateEntryPoint(options, model, diagnostics);
        ValidateResources(model, diagnostics);
        ValidateStructLocations(model, diagnostics);

        return diagnostics;
    }

    private static void ValidateAttributes(CppslSemanticModel model, List<CppslDiagnostic> diagnostics)
    {
        var descriptorSetLayoutNames = model.Globals
            .Where(static global => global.IsDescriptorSet)
            .Select(static global => global.Type)
            .ToHashSet(StringComparer.Ordinal);

        foreach (var structure in model.Structs)
        {
            ValidateAttributeSet("struct", structure.Name, structure.Attributes, Array.Empty<string>(), diagnostics);
            foreach (var field in structure.Fields)
            {
                var allowed = descriptorSetLayoutNames.Contains(structure.Name)
                    ? new[] { "binding", "cbuffer", "structured_buffer", "sbuffer", "rwstructured_buffer", "rw_structured_buffer", "rwsbuffer" }
                    : new[] { "location", "position", "builtin" };
                ValidateAttributeSet("field", field.Name, field.Attributes, allowed, diagnostics);
            }
        }

        foreach (var global in model.Globals)
        {
            ValidateAttributeSet(
                "global",
                global.Name,
                global.Attributes,
                new[] { "desc_set", "binding", "group_shared", "cbuffer", "structured_buffer", "sbuffer", "rwstructured_buffer", "rw_structured_buffer", "rwsbuffer" },
                diagnostics);
            if (global.ResourceKind is null &&
                !global.IsDescriptorSet &&
                (global.Attributes.FindAttribute("desc_set") is not null || global.Attributes.FindAttribute("binding") is not null))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL global `{global.Name}` uses resource binding attributes but is not a CPPSL resource type.",
                    global.File,
                    global.Line,
                    global.Column));
            }
        }

        foreach (var function in model.Functions)
        {
            ValidateAttributeSet("function", function.Name, function.Attributes, StageAttributes, diagnostics);
            foreach (var parameter in function.Parameters)
            {
                ValidateAttributeSet("parameter", parameter.Name, parameter.Attributes, new[] { "location", "builtin" }, diagnostics);
            }
        }
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

        var structNames = model.Structs.Select(static structure => structure.Name).ToHashSet(StringComparer.Ordinal);
        if (entryPoint.ReturnType is not null &&
            entryPoint.ReturnType != "void" &&
            !structNames.Contains(entryPoint.ReturnType))
        {
            diagnostics.Add(CppslDiagnostic.Error(
                $"CPPSL entry point `{entryPoint.Name}` return type `{entryPoint.ReturnType}` must be `void` or a CPPSL struct.",
                entryPoint.File,
                entryPoint.Line,
                entryPoint.Column));
        }

        foreach (var parameter in entryPoint.Parameters)
        {
            if (!structNames.Contains(parameter.Type) && !IsAllowedBuiltinParameter(options.Stage, parameter))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL entry point parameter `{parameter.Name}` type `{parameter.Type}` must be a CPPSL struct.",
                    parameter.File,
                    parameter.Line,
                    parameter.Column));
            }
        }
    }

    private static bool IsAllowedBuiltinParameter(ShaderStage stage, CppslParameter parameter)
    {
        if (stage != ShaderStage.Compute)
        {
            return false;
        }

        var builtin = parameter.Attributes.FindAttribute("builtin");
        return builtin?.Arguments.FirstOrDefault() is "dispatch_thread_id" or "group_index";
    }

    private static void ValidateResources(CppslSemanticModel model, List<CppslDiagnostic> diagnostics)
    {
        ValidateDescriptorSetLayouts(model, diagnostics);

        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            if (global.DescriptorSet is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL resource `{global.Name}` must declare `cppsl::desc_set(...)`.",
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

            ValidateResourceType(global, diagnostics);
        }

        foreach (var duplicateGroup in model.Globals
            .Where(static global => global.ResourceKind is not null && global.DescriptorSet is not null && global.Binding is not null)
            .GroupBy(static global => (Set: global.DescriptorSet!.Value, Binding: global.Binding!.Value))
            .Where(static group => group.Count() > 1))
        {
            foreach (var global in duplicateGroup.Skip(1))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL declares duplicate resource binding set `{duplicateGroup.Key.Set}` binding `{duplicateGroup.Key.Binding}`.",
                    global.File,
                    global.Line,
                    global.Column));
            }
        }
    }

    private static void ValidateDescriptorSetLayouts(CppslSemanticModel model, List<CppslDiagnostic> diagnostics)
    {
        foreach (var descriptorSet in model.Globals.Where(static global => global.IsDescriptorSet))
        {
            if (descriptorSet.DescriptorSet is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL descriptor set `{descriptorSet.Name}` must declare `cppsl::desc_set(...)`.",
                    descriptorSet.File,
                    descriptorSet.Line,
                    descriptorSet.Column));
            }
            if (descriptorSet.Binding is not null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL descriptor set `{descriptorSet.Name}` must not declare `cppsl::binding(...)`; bindings belong to descriptor fields.",
                    descriptorSet.File,
                    descriptorSet.Line,
                    descriptorSet.Column));
            }

            var layout = model.Structs.FirstOrDefault(structure => structure.Name == descriptorSet.Type);
            if (layout is null)
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL descriptor set `{descriptorSet.Name}` uses unknown layout type `{descriptorSet.Type}`.",
                    descriptorSet.File,
                    descriptorSet.Line,
                    descriptorSet.Column));
                continue;
            }

            foreach (var field in layout.Fields)
            {
                var resourceKind = DescriptorSetFieldResourceKind(field);
                if (resourceKind is null)
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL descriptor set layout `{layout.Name}` field `{field.Name}` must be a descriptor field.",
                        field.File,
                        field.Line,
                        field.Column));
                    continue;
                }
                if (field.Attributes.FindAttribute("binding") is null)
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL descriptor set layout `{layout.Name}` field `{field.Name}` must declare `cppsl::binding(...)`.",
                        field.File,
                        field.Line,
                        field.Column));
                }

                ValidateDescriptorSetFieldType(layout.Name, field, resourceKind, diagnostics);
            }

            foreach (var duplicateGroup in layout.Fields
                .Where(static field => field.Attributes.FindAttribute("binding").FirstIntArgument() is not null)
                .GroupBy(static field => field.Attributes.FindAttribute("binding").FirstIntArgument()!.Value)
                .Where(static group => group.Count() > 1))
            {
                foreach (var field in duplicateGroup.Skip(1))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL descriptor set layout `{layout.Name}` declares duplicate binding `{duplicateGroup.Key}`.",
                        field.File,
                        field.Line,
                        field.Column));
                }
            }
        }
    }

    private static string? DescriptorSetFieldResourceKind(CppslField field)
    {
        if (field.Attributes.FindAttribute("cbuffer") is not null) return "constant_buffer";
        if (field.Attributes.FindAttribute("structured_buffer") is not null ||
            field.Attributes.FindAttribute("sbuffer") is not null) return "structured_buffer";
        if (field.Attributes.FindAttribute("rwstructured_buffer") is not null ||
            field.Attributes.FindAttribute("rw_structured_buffer") is not null ||
            field.Attributes.FindAttribute("rwsbuffer") is not null) return "rw_structured_buffer";
        if (field.Type.StartsWith("Texture", StringComparison.Ordinal)) return "texture";
        if (field.Type.StartsWith("RWTexture", StringComparison.Ordinal)) return "rw_texture";
        if (field.Type == "SamplerState") return "sampler";
        if (field.Type == "AccelerationStructure") return "acceleration_structure";
        return null;
    }

    private static void ValidateDescriptorSetFieldType(
        string layoutName,
        CppslField field,
        string resourceKind,
        List<CppslDiagnostic> diagnostics)
    {
        switch (resourceKind)
        {
            case "structured_buffer":
                if (!IsConstPointerType(field.Type) && !IsConstArrayType(field.Type))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL descriptor set layout `{layoutName}` structured buffer field `{field.Name}` must be declared as `const T*`.",
                        field.File,
                        field.Line,
                        field.Column));
                }
                break;
            case "rw_structured_buffer":
                if (!IsMutablePointerType(field.Type) && !IsMutableArrayType(field.Type))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL descriptor set layout `{layoutName}` RW structured buffer field `{field.Name}` must be declared as `T*`.",
                        field.File,
                        field.Line,
                        field.Column));
                }
                break;
        }
    }

    private static void ValidateResourceType(CppslGlobal global, List<CppslDiagnostic> diagnostics)
    {
        switch (global.ResourceKind)
        {
            case "structured_buffer":
                if (!IsConstPointerType(global.Type))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL structured buffer `{global.Name}` must be declared as `const T*`.",
                        global.File,
                        global.Line,
                        global.Column));
                }
                break;
            case "rw_structured_buffer":
                if (!IsMutablePointerType(global.Type))
                {
                    diagnostics.Add(CppslDiagnostic.Error(
                        $"CPPSL RW structured buffer `{global.Name}` must be declared as `T*`.",
                        global.File,
                        global.Line,
                        global.Column));
                }
                break;
        }
    }

    private static bool IsConstPointerType(string type)
    {
        var normalized = NormalizeType(type);
        return normalized.EndsWith("*", StringComparison.Ordinal) &&
            (normalized.StartsWith("const ", StringComparison.Ordinal) ||
             normalized.Contains(" const ", StringComparison.Ordinal));
    }

    private static bool IsMutablePointerType(string type)
    {
        var normalized = NormalizeType(type);
        return normalized.EndsWith("*", StringComparison.Ordinal) &&
            !normalized.StartsWith("const ", StringComparison.Ordinal) &&
            !normalized.Contains(" const ", StringComparison.Ordinal);
    }

    private static bool IsConstArrayType(string type)
    {
        var normalized = NormalizeType(type);
        return normalized.EndsWith("[]", StringComparison.Ordinal) &&
            (normalized.StartsWith("const ", StringComparison.Ordinal) ||
             normalized.Contains(" const ", StringComparison.Ordinal));
    }

    private static bool IsMutableArrayType(string type)
    {
        var normalized = NormalizeType(type);
        return normalized.EndsWith("[]", StringComparison.Ordinal) &&
            !normalized.StartsWith("const ", StringComparison.Ordinal) &&
            !normalized.Contains(" const ", StringComparison.Ordinal);
    }

    private static string NormalizeType(string type)
    {
        return string.Join(' ', type.Replace("*", " *", StringComparison.Ordinal).Split(' ', StringSplitOptions.RemoveEmptyEntries));
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

    private static void ValidateAttributeSet(
        string targetKind,
        string targetName,
        IReadOnlyList<CppslAttribute> attributes,
        IEnumerable<string> allowedAttributes,
        List<CppslDiagnostic> diagnostics)
    {
        var allowed = allowedAttributes.ToHashSet(StringComparer.Ordinal);
        foreach (var attribute in attributes)
        {
            if (!KnownAttributes.Contains(attribute.Name) && !StageAttributes.Contains(attribute.Name))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"Unknown CPPSL attribute `cppsl::{attribute.Name}`.",
                    attribute.File,
                    attribute.Line,
                    attribute.Column));
                continue;
            }

            if (!allowed.Contains(attribute.Name))
            {
                diagnostics.Add(CppslDiagnostic.Error(
                    $"CPPSL attribute `cppsl::{attribute.Name}` cannot be used on {targetKind} `{targetName}`.",
                    attribute.File,
                    attribute.Line,
                    attribute.Column));
            }
        }
    }
}
