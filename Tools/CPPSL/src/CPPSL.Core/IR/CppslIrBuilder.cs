using CPPSL.Core.Compiler;
using CPPSL.Core.Frontend;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.IR;

public sealed class CppslIrBuilder
{
    public const string SchemaName = "cppsl.ir";
    public const int SchemaVersion = 1;

    public CppslIrModule Build(
        CppslCompileOptions options,
        string sourcePath,
        CppslSemanticModel semanticModel,
        IReadOnlyList<CppslAstNode> astNodes)
    {
        sourcePath = Path.GetFullPath(sourcePath);
        return new CppslIrModule(
            SchemaName,
            SchemaVersion,
            sourcePath,
            semanticModel.Structs.Select(static structure => new CppslIrStruct(
                structure.Name,
                structure.Fields.Select(static field => new CppslIrField(
                    field.Name,
                    field.Type,
                    field.Location,
                    field.IsPosition)).ToArray())).ToArray(),
            semanticModel.Globals
                .Where(static global => global.ResourceKind is not null)
                .Select(static global => new CppslIrResource(
                    global.Name,
                    global.Type,
                    global.ResourceKind!,
                    global.DescriptorSet!.Value,
                    global.Binding!.Value)).ToArray(),
            semanticModel.Functions
                .Select(function => new CppslIrFunction(
                    function.Name,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslIrParameter(parameter.Name, parameter.Type)).ToArray(),
                    FindFunctionBody(astNodes, function.Name))).ToArray(),
            semanticModel.Functions
                .Where(static function => function.IsEntryPoint)
                .Select(function => new CppslIrEntryPoint(
                    function.Name,
                    options.Stage.ToString(),
                    function.DeclaredStage,
                    function.ReturnType,
                    function.Parameters.Select(static parameter => new CppslIrParameter(parameter.Name, parameter.Type)).ToArray(),
                    FindFunctionBody(astNodes, function.Name))).ToArray());
    }

    private static CppslIrNode? FindFunctionBody(IReadOnlyList<CppslAstNode> astNodes, string functionName)
    {
        var functionNode = astNodes.FirstOrDefault(node =>
            node.Kind == CppslAstNodeKind.Function &&
            node.Spelling == functionName &&
            node.Children.Any(static child => child.Kind == CppslAstNodeKind.CompoundStatement));

        var bodyNode = functionNode?.Children.FirstOrDefault(static child => child.Kind == CppslAstNodeKind.CompoundStatement);
        return bodyNode is null ? null : ToIrNode(bodyNode);
    }

    private static CppslIrNode ToIrNode(CppslAstNode node)
    {
        return new CppslIrNode(
            node.Kind.ToString(),
            node.Spelling,
            node.DisplayName,
            node.TypeName,
            node.TypeInfo is null ? null : ToIrType(node.TypeInfo),
            node.Children
                .Where(static child => IsBodyNode(child.Kind))
                .Select(ToIrNode)
                .ToArray());
    }

    private static CppslIrType ToIrType(CppslTypeInfo type)
    {
        return new CppslIrType(
            type.Spelling,
            type.CanonicalName,
            type.DesugaredName,
            type.TemplateArguments.Select(ToIrType).ToArray());
    }

    private static bool IsBodyNode(CppslAstNodeKind kind)
    {
        return kind is
            CppslAstNodeKind.CompoundStatement or
            CppslAstNodeKind.DeclarationStatement or
            CppslAstNodeKind.LocalVariable or
            CppslAstNodeKind.ReturnStatement or
            CppslAstNodeKind.IfStatement or
            CppslAstNodeKind.WhileStatement or
            CppslAstNodeKind.ForStatement or
            CppslAstNodeKind.ContinueStatement or
            CppslAstNodeKind.BreakStatement or
            CppslAstNodeKind.BinaryOperator or
            CppslAstNodeKind.UnaryOperator or
            CppslAstNodeKind.ConditionalOperator or
            CppslAstNodeKind.CallExpression or
            CppslAstNodeKind.OperatorCallExpression or
            CppslAstNodeKind.ConstructorCallExpression or
            CppslAstNodeKind.FunctionalCastExpression or
            CppslAstNodeKind.CStyleCastExpression or
            CppslAstNodeKind.MemberExpression or
            CppslAstNodeKind.DeclRefExpression or
            CppslAstNodeKind.IntegerLiteral or
            CppslAstNodeKind.FloatingLiteral or
            CppslAstNodeKind.BooleanLiteral or
            CppslAstNodeKind.StringLiteral or
            CppslAstNodeKind.InitializerListExpression or
            CppslAstNodeKind.ImplicitCastExpression or
            CppslAstNodeKind.ParenExpression or
            CppslAstNodeKind.ArraySubscriptExpression;
    }
}

public sealed record CppslIrModule(
    string Schema,
    int Version,
    string Source,
    IReadOnlyList<CppslIrStruct> Structs,
    IReadOnlyList<CppslIrResource> Resources,
    IReadOnlyList<CppslIrFunction> Functions,
    IReadOnlyList<CppslIrEntryPoint> EntryPoints);

public sealed record CppslIrStruct(
    string Name,
    IReadOnlyList<CppslIrField> Fields);

public sealed record CppslIrField(
    string Name,
    string Type,
    int? Location,
    bool IsPosition);

public sealed record CppslIrResource(
    string Name,
    string Type,
    string ResourceKind,
    int Set,
    int Binding);

public sealed record CppslIrEntryPoint(
    string Name,
    string Stage,
    string? DeclaredStage,
    string? ReturnType,
    IReadOnlyList<CppslIrParameter> Parameters,
    CppslIrNode? Body);

public sealed record CppslIrFunction(
    string Name,
    string? ReturnType,
    IReadOnlyList<CppslIrParameter> Parameters,
    CppslIrNode? Body);

public sealed record CppslIrParameter(
    string Name,
    string Type);

public sealed record CppslIrNode(
    string Kind,
    string Spelling,
    string? DisplayName,
    string? Type,
    CppslIrType? TypeInfo,
    IReadOnlyList<CppslIrNode> Children);

public sealed record CppslIrType(
    string Spelling,
    string CanonicalName,
    string DesugaredName,
    IReadOnlyList<CppslIrType> TemplateArguments);
