using CPPSL.Core.Frontend;

namespace CPPSL.Core.ShaderModel;

internal sealed class CppslShaderModelBodyLowerer
{
    private static readonly HashSet<CppslAstNodeKind> BodyNodeKinds = new()
    {
        CppslAstNodeKind.CompoundStatement,
        CppslAstNodeKind.DeclarationStatement,
        CppslAstNodeKind.LocalVariable,
        CppslAstNodeKind.ReturnStatement,
        CppslAstNodeKind.IfStatement,
        CppslAstNodeKind.WhileStatement,
        CppslAstNodeKind.ForStatement,
        CppslAstNodeKind.ContinueStatement,
        CppslAstNodeKind.BreakStatement,
        CppslAstNodeKind.BinaryOperator,
        CppslAstNodeKind.UnaryOperator,
        CppslAstNodeKind.ConditionalOperator,
        CppslAstNodeKind.CallExpression,
        CppslAstNodeKind.OperatorCallExpression,
        CppslAstNodeKind.ConstructorCallExpression,
        CppslAstNodeKind.FunctionalCastExpression,
        CppslAstNodeKind.CStyleCastExpression,
        CppslAstNodeKind.MemberExpression,
        CppslAstNodeKind.DeclRefExpression,
        CppslAstNodeKind.IntegerLiteral,
        CppslAstNodeKind.FloatingLiteral,
        CppslAstNodeKind.BooleanLiteral,
        CppslAstNodeKind.StringLiteral,
        CppslAstNodeKind.InitializerListExpression,
        CppslAstNodeKind.ImplicitCastExpression,
        CppslAstNodeKind.ParenExpression,
        CppslAstNodeKind.ArraySubscriptExpression,
        CppslAstNodeKind.Unknown
    };

    public CppslShaderModelNode? LowerFunctionBody(IReadOnlyList<CppslAstNode> astNodes, string functionName)
    {
        var functionNode = astNodes.FirstOrDefault(node =>
            node.Kind == CppslAstNodeKind.Function &&
            node.Spelling == functionName &&
            node.Children.Any(static child => child.Kind == CppslAstNodeKind.CompoundStatement));

        var bodyNode = functionNode?.Children.FirstOrDefault(static child => child.Kind == CppslAstNodeKind.CompoundStatement);
        return bodyNode is null ? null : LowerNode(bodyNode);
    }

    private static CppslShaderModelNode LowerNode(CppslAstNode node)
    {
        return new CppslShaderModelNode(
            LowerKind(node.Kind),
            node.Spelling,
            node.DisplayName,
            node.TypeName,
            node.TypeInfo is null ? null : LowerType(node.TypeInfo),
            node.Children
                .Where(static child => TryLowerKind(child.Kind, out _))
                .Select(LowerNode)
                .ToArray());
    }

    private static CppslShaderModelType LowerType(CppslTypeInfo type)
    {
        return new CppslShaderModelType(
            type.Spelling,
            type.CanonicalName,
            type.DesugaredName,
            type.TemplateArguments.Select(LowerType).ToArray());
    }

    private static CppslShaderModelNodeKind LowerKind(CppslAstNodeKind kind)
    {
        return TryLowerKind(kind, out var shaderModelKind)
            ? shaderModelKind
            : CppslShaderModelNodeKind.Unknown;
    }

    private static bool TryLowerKind(CppslAstNodeKind kind, out CppslShaderModelNodeKind shaderModelKind)
    {
        if (!BodyNodeKinds.Contains(kind))
        {
            shaderModelKind = default;
            return false;
        }

        return Enum.TryParse(kind.ToString(), out shaderModelKind);
    }
}
