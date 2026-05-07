using CPPSL.Core.Compiler;
using CPPSL.Core.Frontend;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.ShaderModel;

public sealed class CppslShaderModelBuilder
{
    public const string SchemaName = "cppsl.shader_model";
    public const int SchemaVersion = 1;
    private readonly CppslShaderModelBodyLowerer _bodyLowerer = new();

    public CppslShaderModel Build(
        CppslCompileOptions options,
        string sourcePath,
        CppslSemanticModel semanticModel,
        IReadOnlyList<CppslAstNode> astNodes)
    {
        sourcePath = Path.GetFullPath(sourcePath);
        var structures = semanticModel.Structs.ToArray();
        var functions = semanticModel.Functions.ToArray();
        var emittedStructNames = BuildStructNames(structures);
        var emittedFunctionNames = BuildFunctionNames(functions);
        var emittedMethodNames = BuildMethodNames(structures);
        return new CppslShaderModel(
            SchemaName,
            SchemaVersion,
            sourcePath,
            structures.Select(structure => new CppslShaderModelStruct(
                structure.DeclId,
                structure.Name,
                structure.DisplayName,
                EmittedStructName(structure, emittedStructNames),
                structure.IsTemplateInstantiation,
                structure.TemplatePatternDeclId,
                structure.TemplateArguments.Select(static argument => new CppslShaderModelTemplateArgument(
                    argument.Kind,
                    argument.Spelling,
                    argument.Value,
                    argument.TypeInfo is null ? null : ToShaderModelType(argument.TypeInfo))).ToArray(),
                structure.Fields.Select(static field => new CppslShaderModelField(
                    field.Name,
                    field.Type,
                    field.Location,
                    field.IsPosition)).ToArray(),
                structure.Methods.Select(method => new CppslShaderModelMethod(
                    method.DeclId,
                    method.OwnerType,
                    method.Name,
                    EmittedMethodName(method, emittedMethodNames),
                    method.DisplayName,
                    method.ReturnType,
                    method.Parameters.Select(static parameter => new CppslShaderModelParameter(parameter.Name, parameter.Type)).ToArray(),
                    method.IsTemplateInstantiation,
                    method.TemplatePatternDeclId,
                    method.TemplateArguments.Select(static argument => new CppslShaderModelTemplateArgument(
                        argument.Kind,
                        argument.Spelling,
                        argument.Value,
                        argument.TypeInfo is null ? null : ToShaderModelType(argument.TypeInfo))).ToArray(),
                    method.IsConst,
                    _bodyLowerer.LowerMethodBody(astNodes, method.DeclId, structure.Name, method.Name))).ToArray())).ToArray(),
            semanticModel.Globals
                .Where(static global => global.ResourceKind is not null)
                .Select(static global => new CppslShaderModelResource(
                    global.Name,
                    global.Type,
                    global.ResourceKind!,
                    global.DescriptorSet!.Value,
                    global.Binding!.Value)).ToArray(),
            functions
                .Select(function => new CppslShaderModelFunction(
                    function.DeclId,
                    function.Name,
                    EmittedFunctionName(function, emittedFunctionNames),
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslShaderModelParameter(parameter.Name, parameter.Type)).ToArray(),
                    function.IsTemplateInstantiation,
                    function.TemplatePatternDeclId,
                    function.TemplateArguments.Select(static argument => new CppslShaderModelTemplateArgument(
                        argument.Kind,
                        argument.Spelling,
                        argument.Value,
                        argument.TypeInfo is null ? null : new CppslShaderModelType(
                            argument.TypeInfo.Spelling,
                            argument.TypeInfo.CanonicalName,
                            argument.TypeInfo.DesugaredName,
                            argument.TypeInfo.TemplateArguments.Select(ToShaderModelType).ToArray()))).ToArray(),
                    _bodyLowerer.LowerFunctionBody(astNodes, function.DeclId, function.Name))).ToArray(),
            functions
                .Where(static function => function.IsEntryPoint)
                .Select(function => new CppslShaderModelEntryPoint(
                    function.Name,
                    options.Stage.ToString(),
                    function.DeclaredStage,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslShaderModelParameter(parameter.Name, parameter.Type)).ToArray(),
                    _bodyLowerer.LowerFunctionBody(astNodes, function.DeclId, function.Name))).ToArray());
    }

    private static IReadOnlyDictionary<string, string> BuildStructNames(IReadOnlyList<CppslStruct> structures)
    {
        var names = structures
            .Where(static structure => !string.IsNullOrWhiteSpace(structure.DeclId))
            .ToDictionary(
                static structure => structure.DeclId!,
                ShaderStructName,
                StringComparer.Ordinal);

        var duplicatedTemplateNames = structures
            .Where(static structure => structure.IsTemplateInstantiation && !string.IsNullOrWhiteSpace(structure.DeclId))
            .GroupBy(ShaderStructName, StringComparer.Ordinal)
            .Where(static group => group.Count() > 1)
            .Select(static group => group.Key)
            .ToHashSet(StringComparer.Ordinal);
        foreach (var structure in structures.Where(static structure => structure.IsTemplateInstantiation && structure.DeclId is not null))
        {
            var declId = structure.DeclId!;
            if (duplicatedTemplateNames.Contains(names[declId]))
            {
                names[declId] = $"{names[declId]}_{StableIdentifierSuffix(declId)}";
            }
        }

        return names;
    }

    private static IReadOnlyDictionary<string, string> BuildFunctionNames(IReadOnlyList<CppslFunction> functions)
    {
        var names = functions
            .Where(static function => !string.IsNullOrWhiteSpace(function.DeclId))
            .ToDictionary(
                static function => function.DeclId!,
                ShaderFunctionName,
                StringComparer.Ordinal);

        var duplicatedTemplateNames = functions
            .Where(static function => function.IsTemplateInstantiation && !string.IsNullOrWhiteSpace(function.DeclId))
            .GroupBy(ShaderFunctionName, StringComparer.Ordinal)
            .Where(static group => group.Count() > 1)
            .Select(static group => group.Key)
            .ToHashSet(StringComparer.Ordinal);
        foreach (var function in functions.Where(static function => function.IsTemplateInstantiation && function.DeclId is not null))
        {
            var declId = function.DeclId!;
            if (duplicatedTemplateNames.Contains(names[declId]))
            {
                names[declId] = $"{names[declId]}_{StableIdentifierSuffix(declId)}";
            }
        }

        return names;
    }

    private static IReadOnlyDictionary<string, string> BuildMethodNames(IReadOnlyList<CppslStruct> structures)
    {
        var methods = structures
            .SelectMany(static structure => structure.Methods)
            .Where(static method => !string.IsNullOrWhiteSpace(method.DeclId))
            .ToArray();
        var names = methods.ToDictionary(
            static method => method.DeclId!,
            ShaderMethodName,
            StringComparer.Ordinal);

        var duplicatedTemplateNames = methods
            .Where(static method => method.IsTemplateInstantiation && !string.IsNullOrWhiteSpace(method.DeclId))
            .GroupBy(static method => $"{NormalizeMethodOwner(method.OwnerType)}.{ShaderMethodName(method)}", StringComparer.Ordinal)
            .Where(static group => group.Count() > 1)
            .Select(static group => group.Key)
            .ToHashSet(StringComparer.Ordinal);
        foreach (var method in methods.Where(static method => method.IsTemplateInstantiation && method.DeclId is not null))
        {
            var declId = method.DeclId!;
            if (duplicatedTemplateNames.Contains($"{NormalizeMethodOwner(method.OwnerType)}.{names[declId]}"))
            {
                names[declId] = $"{names[declId]}_{StableIdentifierSuffix(declId)}";
            }
        }

        return names;
    }

    private static string EmittedFunctionName(CppslFunction function, IReadOnlyDictionary<string, string> emittedFunctionNames)
    {
        return function.DeclId is not null && emittedFunctionNames.TryGetValue(function.DeclId, out var emittedName)
            ? emittedName
            : ShaderFunctionName(function);
    }

    private static string EmittedStructName(CppslStruct structure, IReadOnlyDictionary<string, string> emittedStructNames)
    {
        return structure.DeclId is not null && emittedStructNames.TryGetValue(structure.DeclId, out var emittedName)
            ? emittedName
            : ShaderStructName(structure);
    }

    private static string EmittedMethodName(CppslMethod method, IReadOnlyDictionary<string, string> emittedMethodNames)
    {
        return method.DeclId is not null && emittedMethodNames.TryGetValue(method.DeclId, out var emittedName)
            ? emittedName
            : ShaderMethodName(method);
    }

    private static string ShaderFunctionName(CppslFunction function)
    {
        return function.IsTemplateInstantiation
            ? $"{function.Name}_tpl_{TemplateArgumentSuffix(function.TemplateArguments, function.DeclId ?? function.DisplayName ?? function.Name)}"
            : function.Name;
    }

    private static string ShaderStructName(CppslStruct structure)
    {
        return structure.IsTemplateInstantiation
            ? $"{structure.Name}_tpl_{TemplateArgumentSuffix(structure.TemplateArguments, structure.DeclId ?? structure.DisplayName ?? structure.Name)}"
            : structure.Name;
    }

    private static string ShaderMethodName(CppslMethod method)
    {
        return method.IsTemplateInstantiation
            ? $"{method.Name}_tpl_{TemplateArgumentSuffix(method.TemplateArguments, method.DeclId ?? method.DisplayName ?? method.Name)}"
            : method.Name;
    }

    private static string NormalizeMethodOwner(string ownerType)
    {
        return SanitizeIdentifierPart(ownerType);
    }

    private static string TemplateArgumentSuffix(IReadOnlyList<CppslTemplateArgumentInfo> arguments, string fallback)
    {
        var readable = arguments
            .Select(static argument => argument.TypeInfo?.Spelling ?? argument.Value ?? argument.Spelling)
            .Where(static value => !string.IsNullOrWhiteSpace(value))
            .Select(SanitizeIdentifierPart)
            .Where(static value => value.Length != 0)
            .ToArray();
        return readable.Length == 0
            ? StableIdentifierSuffix(fallback)
            : string.Join("_", readable);
    }

    private static CppslShaderModelType ToShaderModelType(CppslTypeInfo type)
    {
        return new CppslShaderModelType(
            type.Spelling,
            type.CanonicalName,
            type.DesugaredName,
            type.TemplateArguments.Select(ToShaderModelType).ToArray());
    }

    private static string SanitizeIdentifierPart(string? value)
    {
        if (string.IsNullOrWhiteSpace(value))
        {
            return string.Empty;
        }

        var chars = value
            .Replace("struct ", string.Empty, StringComparison.Ordinal)
            .Replace("class ", string.Empty, StringComparison.Ordinal)
            .Replace("cppsl::", string.Empty, StringComparison.Ordinal)
            .Select(static ch => char.IsLetterOrDigit(ch) ? ch : '_')
            .ToArray();
        var result = new string(chars).Trim('_');
        while (result.Contains("__", StringComparison.Ordinal))
        {
            result = result.Replace("__", "_", StringComparison.Ordinal);
        }
        return result;
    }

    private static string StableIdentifierSuffix(string value)
    {
        const uint offset = 2166136261u;
        const uint prime = 16777619u;
        var hash = offset;
        foreach (var ch in value)
        {
            hash ^= ch;
            hash *= prime;
        }
        return hash.ToString("x8");
    }
}

public sealed record CppslShaderModel(
    string Schema,
    int Version,
    string Source,
    IReadOnlyList<CppslShaderModelStruct> Structs,
    IReadOnlyList<CppslShaderModelResource> Resources,
    IReadOnlyList<CppslShaderModelFunction> Functions,
    IReadOnlyList<CppslShaderModelEntryPoint> EntryPoints);

public sealed record CppslShaderModelStruct(
    string? DeclId,
    string Name,
    string? DisplayName,
    string EmittedName,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslShaderModelTemplateArgument> TemplateArguments,
    IReadOnlyList<CppslShaderModelField> Fields,
    IReadOnlyList<CppslShaderModelMethod> Methods);

public sealed record CppslShaderModelField(
    string Name,
    string Type,
    int? Location,
    bool IsPosition);

public sealed record CppslShaderModelResource(
    string Name,
    string Type,
    string ResourceKind,
    int Set,
    int Binding);

public sealed record CppslShaderModelEntryPoint(
    string Name,
    string Stage,
    string? DeclaredStage,
    string? ReturnType,
    IReadOnlyList<CppslShaderModelParameter> Parameters,
    CppslShaderModelNode? Body);

public sealed record CppslShaderModelFunction(
    string? DeclId,
    string Name,
    string EmittedName,
    string? ReturnType,
    IReadOnlyList<CppslShaderModelParameter> Parameters,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslShaderModelTemplateArgument> TemplateArguments,
    CppslShaderModelNode? Body);

public sealed record CppslShaderModelMethod(
    string? DeclId,
    string OwnerType,
    string Name,
    string EmittedName,
    string? DisplayName,
    string? ReturnType,
    IReadOnlyList<CppslShaderModelParameter> Parameters,
    bool IsTemplateInstantiation,
    string? TemplatePatternDeclId,
    IReadOnlyList<CppslShaderModelTemplateArgument> TemplateArguments,
    bool IsConst,
    CppslShaderModelNode? Body);

public sealed record CppslShaderModelParameter(
    string Name,
    string Type);

public sealed record CppslShaderModelNode(
    CppslShaderModelNodeKind Kind,
    string Spelling,
    string? DisplayName,
    string? Type,
    CppslShaderModelType? TypeInfo,
    string? ReferencedDeclId,
    string? DirectCalleeDeclId,
    string? TemplatePatternDeclId,
    bool IsConstexpr,
    bool IsTemplateInstantiation,
    bool UsesDefaultArgument,
    string? ConstantValue,
    IReadOnlyList<CppslShaderModelTemplateArgument> TemplateArguments,
    IReadOnlyList<CppslShaderModelNode> Children);

public sealed record CppslShaderModelType(
    string Spelling,
    string CanonicalName,
    string DesugaredName,
    IReadOnlyList<CppslShaderModelType> TemplateArguments);

public sealed record CppslShaderModelTemplateArgument(
    string Kind,
    string? Spelling,
    string? Value,
    CppslShaderModelType? TypeInfo);
