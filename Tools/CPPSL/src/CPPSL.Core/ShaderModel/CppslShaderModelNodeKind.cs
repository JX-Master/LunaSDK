namespace CPPSL.Core.ShaderModel;

public enum CppslShaderModelNodeKind
{
    Unknown,
    CompoundStatement,
    DeclarationStatement,
    LocalVariable,
    ReturnStatement,
    IfStatement,
    WhileStatement,
    ForStatement,
    ContinueStatement,
    BreakStatement,
    BinaryOperator,
    UnaryOperator,
    ConditionalOperator,
    CallExpression,
    OperatorCallExpression,
    ConstructorCallExpression,
    FunctionalCastExpression,
    CStyleCastExpression,
    MemberExpression,
    DeclRefExpression,
    IntegerLiteral,
    FloatingLiteral,
    BooleanLiteral,
    StringLiteral,
    InitializerListExpression,
    ImplicitCastExpression,
    ParenExpression,
    DefaultArgumentExpression,
    ArraySubscriptExpression
}
