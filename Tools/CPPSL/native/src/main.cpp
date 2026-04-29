#include <clang/AST/AST.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Index/USRGeneration.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/Tooling.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
struct Options
{
    std::string source_path;
    std::vector<std::string> include_roots;
};

struct SourceLocationInfo
{
    std::string file;
    unsigned line = 0;
    unsigned column = 0;
};

struct SourceRangeInfo
{
    std::optional<SourceLocationInfo> start;
    std::optional<SourceLocationInfo> end;
};

struct TypeInfo
{
    std::string spelling;
    std::string canonical_name;
    std::string desugared_name;
    std::vector<TypeInfo> template_arguments;
};

struct TemplateArgumentInfo
{
    std::string kind;
    std::string spelling;
    std::string value;
    std::optional<TypeInfo> type_info;
};

struct DiagnosticInfo
{
    std::string severity;
    std::string message;
    std::string file;
    std::optional<unsigned> line;
    std::optional<unsigned> column;
};

std::optional<SourceLocationInfo> GetLocation(const clang::SourceManager& source_manager, clang::SourceLocation location);

struct AstNode
{
    std::string kind;
    std::string provider_kind;
    std::string spelling;
    std::string display_name;
    std::string type_name;
    std::string result_type_name;
    std::string decl_id;
    std::string canonical_decl_id;
    std::string referenced_decl_id;
    std::string direct_callee_decl_id;
    std::string owner_decl_id;
    std::string template_pattern_decl_id;
    bool is_implicit = false;
    bool is_constexpr = false;
    bool is_template_instantiation = false;
    bool uses_default_argument = false;
    std::string constant_value;
    std::optional<SourceLocationInfo> location;
    std::optional<SourceRangeInfo> range;
    std::optional<TypeInfo> type_info;
    std::optional<TypeInfo> result_type_info;
    std::vector<TemplateArgumentInfo> template_arguments;
    std::vector<AstNode> children;
};

class JsonDiagnosticConsumer final : public clang::DiagnosticConsumer
{
public:
    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& diagnostic) override
    {
        clang::SmallString<256> message;
        diagnostic.FormatDiagnostic(message);

        DiagnosticInfo info;
        info.severity = SeverityName(level);
        info.message = std::string(message.str());
        if (diagnostic.hasSourceManager())
        {
            auto location = GetLocation(diagnostic.getSourceManager(), diagnostic.getLocation());
            if (location)
            {
                info.file = location->file;
                info.line = location->line;
                info.column = location->column;
            }
        }
        diagnostics_.push_back(std::move(info));
    }

    const std::vector<DiagnosticInfo>& Diagnostics() const
    {
        return diagnostics_;
    }

private:
    static std::string SeverityName(clang::DiagnosticsEngine::Level level)
    {
        switch (level)
        {
        case clang::DiagnosticsEngine::Warning: return "Warning";
        case clang::DiagnosticsEngine::Error:
        case clang::DiagnosticsEngine::Fatal: return "Error";
        case clang::DiagnosticsEngine::Ignored: return "Info";
        case clang::DiagnosticsEngine::Note:
        case clang::DiagnosticsEngine::Remark: return "Info";
        }
        return "Info";
    }

    std::vector<DiagnosticInfo> diagnostics_;
};

std::string JsonEscape(std::string_view value)
{
    std::string result;
    result.reserve(value.size() + 8);
    for (char ch : value)
    {
        switch (ch)
        {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += ch; break;
        }
    }
    return result;
}

void WriteString(std::ostream& os, std::string_view value)
{
    os << '"' << JsonEscape(value) << '"';
}

void WriteNullableString(std::ostream& os, const std::string& value)
{
    if (value.empty())
    {
        os << "null";
        return;
    }
    WriteString(os, value);
}

void WriteLocation(std::ostream& os, const std::optional<SourceLocationInfo>& location)
{
    if (!location)
    {
        os << "null";
        return;
    }
    os << "{\"File\":";
    WriteString(os, location->file);
    os << ",\"Line\":" << location->line << ",\"Column\":" << location->column << "}";
}

void WriteRange(std::ostream& os, const std::optional<SourceRangeInfo>& range)
{
    if (!range)
    {
        os << "null";
        return;
    }
    os << "{\"Start\":";
    WriteLocation(os, range->start);
    os << ",\"End\":";
    WriteLocation(os, range->end);
    os << "}";
}

std::optional<SourceLocationInfo> GetLocation(const clang::SourceManager& source_manager, clang::SourceLocation location)
{
    if (location.isInvalid())
    {
        return std::nullopt;
    }

    auto spelling_location = source_manager.getSpellingLoc(location);
    if (spelling_location.isInvalid())
    {
        return std::nullopt;
    }

    auto file_entry = source_manager.getFileEntryForID(source_manager.getFileID(spelling_location));
    if (!file_entry)
    {
        return std::nullopt;
    }

    return SourceLocationInfo{
        std::string(file_entry->tryGetRealPathName()),
        source_manager.getSpellingLineNumber(spelling_location),
        source_manager.getSpellingColumnNumber(spelling_location)};
}

std::optional<SourceRangeInfo> GetRange(const clang::SourceManager& source_manager, clang::SourceRange range)
{
    if (range.isInvalid())
    {
        return std::nullopt;
    }
    return SourceRangeInfo{
        GetLocation(source_manager, range.getBegin()),
        GetLocation(source_manager, range.getEnd())};
}

bool HasErrorDiagnostics(const std::vector<DiagnosticInfo>& diagnostics)
{
    for (const auto& diagnostic : diagnostics)
    {
        if (diagnostic.severity == "Error")
        {
            return true;
        }
    }
    return false;
}

std::string KindForDecl(const clang::Decl* decl)
{
    if (llvm::isa<clang::NamespaceDecl>(decl)) return "Namespace";
    if (const auto* record = llvm::dyn_cast<clang::RecordDecl>(decl))
    {
        if (record->isStruct()) return "Struct";
        if (record->isClass()) return "Class";
        return "Struct";
    }
    if (llvm::isa<clang::EnumDecl>(decl)) return "Enum";
    if (llvm::isa<clang::FieldDecl>(decl)) return "Field";
    if (llvm::isa<clang::FunctionTemplateDecl>(decl)) return "FunctionTemplate";
    if (llvm::isa<clang::ClassTemplateDecl>(decl)) return "ClassTemplate";
    if (llvm::isa<clang::ParmVarDecl>(decl)) return "Parameter";
    if (llvm::isa<clang::CXXConstructorDecl>(decl)) return "Constructor";
    if (llvm::isa<clang::CXXMethodDecl>(decl)) return "Method";
    if (llvm::isa<clang::FunctionDecl>(decl)) return "Function";
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl))
    {
        const auto* context = variable->getDeclContext();
        return llvm::isa<clang::TranslationUnitDecl>(context) || llvm::isa<clang::NamespaceDecl>(context)
            ? "GlobalVariable"
            : "LocalVariable";
    }
    return "Unknown";
}

bool IsInterestingDecl(const clang::Decl* decl)
{
    return llvm::isa<clang::NamespaceDecl>(decl) ||
        llvm::isa<clang::RecordDecl>(decl) ||
        llvm::isa<clang::EnumDecl>(decl) ||
        llvm::isa<clang::FieldDecl>(decl) ||
        llvm::isa<clang::FunctionTemplateDecl>(decl) ||
        llvm::isa<clang::ClassTemplateDecl>(decl) ||
        llvm::isa<clang::FunctionDecl>(decl) ||
        llvm::isa<clang::VarDecl>(decl) ||
        llvm::isa<clang::ParmVarDecl>(decl);
}

std::string GetDeclName(const clang::NamedDecl* decl)
{
    return decl ? decl->getNameAsString() : std::string();
}

std::string DeclId(const clang::Decl* decl)
{
    if (!decl)
    {
        return {};
    }

    llvm::SmallString<128> usr;
    if (!clang::index::generateUSRForDecl(decl, usr) && !usr.empty())
    {
        return "decl:" + std::string(usr);
    }

    const auto& source_manager = decl->getASTContext().getSourceManager();
    auto location = GetLocation(source_manager, decl->getLocation());
    std::string result;
    llvm::raw_string_ostream stream(result);
    stream << "decl:fallback:";
    if (location)
    {
        stream << location->file << ":" << location->line << ":" << location->column;
    }
    else
    {
        stream << "unknown";
    }
    stream << ":" << decl->getDeclKindName();
    if (const auto* named_decl = llvm::dyn_cast<clang::NamedDecl>(decl))
    {
        stream << ":" << named_decl->getNameAsString();
    }
    return result;
}

std::string CanonicalDeclId(const clang::Decl* decl)
{
    return decl ? DeclId(decl->getCanonicalDecl()) : std::string();
}

std::string OwnerDeclId(const clang::Decl* decl)
{
    if (!decl)
    {
        return {};
    }

    const auto* context_decl = llvm::dyn_cast_or_null<clang::Decl>(decl->getDeclContext());
    return CanonicalDeclId(context_decl);
}

std::string GetDisplayName(const clang::NamedDecl* decl)
{
    if (!decl)
    {
        return {};
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        std::string result;
        llvm::raw_string_ostream stream(result);
        function->printName(stream);
        stream << "(";
        for (unsigned i = 0; i < function->getNumParams(); ++i)
        {
            if (i != 0) stream << ", ";
            stream << function->getParamDecl(i)->getType().getAsString();
        }
        stream << ")";
        return result;
    }
    return decl->getNameAsString();
}

std::string TemplateArgumentKindName(clang::TemplateArgument::ArgKind kind)
{
    switch (kind)
    {
    case clang::TemplateArgument::Null: return "Null";
    case clang::TemplateArgument::Type: return "Type";
    case clang::TemplateArgument::Declaration: return "Declaration";
    case clang::TemplateArgument::NullPtr: return "NullPointer";
    case clang::TemplateArgument::Integral: return "Integral";
    case clang::TemplateArgument::Template: return "Template";
    case clang::TemplateArgument::TemplateExpansion: return "TemplateExpansion";
    case clang::TemplateArgument::Expression: return "Expression";
    case clang::TemplateArgument::Pack: return "Pack";
    case clang::TemplateArgument::StructuralValue: return "StructuralValue";
    }
    return "Unknown";
}

std::string APSIntToString(const llvm::APSInt& value)
{
    llvm::SmallString<64> result;
    value.toString(result, 10);
    return std::string(result);
}

std::string TemplateArgumentValueString(const clang::TemplateArgument& argument)
{
    switch (argument.getKind())
    {
    case clang::TemplateArgument::Integral:
        return APSIntToString(argument.getAsIntegral());
    case clang::TemplateArgument::Declaration:
        return CanonicalDeclId(argument.getAsDecl());
    case clang::TemplateArgument::NullPtr:
        return "nullptr";
    default:
        return {};
    }
}

std::string TemplateArgumentSpellingString(const clang::ASTContext& ast_context, const clang::TemplateArgument& argument)
{
    std::string result;
    llvm::raw_string_ostream stream(result);
    argument.print(ast_context.getPrintingPolicy(), stream, true);
    return result;
}

std::optional<TypeInfo> MakeTypeInfo(const clang::ASTContext& ast_context, clang::QualType type, unsigned depth = 0)
{
    if (type.isNull())
    {
        return std::nullopt;
    }

    TypeInfo info;
    info.spelling = type.getAsString();
    info.canonical_name = type.getCanonicalType().getAsString();
    info.desugared_name = type.getDesugaredType(ast_context).getAsString();

    if (depth >= 4)
    {
        return info;
    }

    if (const auto* specialization_type = type->getAs<clang::TemplateSpecializationType>())
    {
        for (const auto& argument : specialization_type->template_arguments())
        {
            if (argument.getKind() == clang::TemplateArgument::Type)
            {
                if (auto argument_info = MakeTypeInfo(ast_context, argument.getAsType(), depth + 1))
                {
                    info.template_arguments.push_back(std::move(*argument_info));
                }
            }
        }
        return info;
    }

    if (const auto* record = type->getAsCXXRecordDecl())
    {
        if (const auto* specialization = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(record))
        {
            for (const auto& argument : specialization->getTemplateArgs().asArray())
            {
                if (argument.getKind() == clang::TemplateArgument::Type)
                {
                    if (auto argument_info = MakeTypeInfo(ast_context, argument.getAsType(), depth + 1))
                    {
                        info.template_arguments.push_back(std::move(*argument_info));
                    }
                }
            }
        }
    }

    return info;
}

TemplateArgumentInfo MakeTemplateArgumentInfo(const clang::ASTContext& ast_context, const clang::TemplateArgument& argument)
{
    TemplateArgumentInfo info;
    info.kind = TemplateArgumentKindName(argument.getKind());
    info.spelling = TemplateArgumentSpellingString(ast_context, argument);
    info.value = TemplateArgumentValueString(argument);
    if (argument.getKind() == clang::TemplateArgument::Type)
    {
        info.type_info = MakeTypeInfo(ast_context, argument.getAsType());
    }
    return info;
}

void AppendTemplateArguments(
    std::vector<TemplateArgumentInfo>& arguments,
    const clang::ASTContext& ast_context,
    const clang::TemplateArgumentList& argument_list)
{
    for (const auto& argument : argument_list.asArray())
    {
        arguments.push_back(MakeTemplateArgumentInfo(ast_context, argument));
    }
}

std::string TryEvaluateConstantValue(const clang::ASTContext& ast_context, const clang::Expr* expression)
{
    if (!expression || expression->isValueDependent())
    {
        return {};
    }

    clang::Expr::EvalResult result;
    if (!expression->EvaluateAsRValue(result, ast_context))
    {
        return {};
    }

    switch (result.Val.getKind())
    {
    case clang::APValue::Int:
        return APSIntToString(result.Val.getInt());
    case clang::APValue::Float:
    {
        llvm::SmallString<64> value;
        result.Val.getFloat().toString(value);
        return std::string(value);
    }
    case clang::APValue::LValue:
        return result.Val.isNullPointer() ? "nullptr" : std::string();
    default:
        return {};
    }
}

std::string GetSourceText(const clang::ASTContext& ast_context, clang::SourceRange range)
{
    if (range.isInvalid())
    {
        return {};
    }

    const auto& source_manager = ast_context.getSourceManager();
    auto char_range = clang::CharSourceRange::getTokenRange(range);
    bool invalid = false;
    auto text = clang::Lexer::getSourceText(char_range, source_manager, ast_context.getLangOpts(), &invalid);
    return invalid ? std::string() : text.str();
}

std::string KindForStmt(const clang::Stmt* stmt)
{
    if (llvm::isa<clang::CompoundStmt>(stmt)) return "CompoundStatement";
    if (llvm::isa<clang::DeclStmt>(stmt)) return "DeclarationStatement";
    if (llvm::isa<clang::ReturnStmt>(stmt)) return "ReturnStatement";
    if (llvm::isa<clang::IfStmt>(stmt)) return "IfStatement";
    if (llvm::isa<clang::WhileStmt>(stmt)) return "WhileStatement";
    if (llvm::isa<clang::ForStmt>(stmt)) return "ForStatement";
    if (llvm::isa<clang::ContinueStmt>(stmt)) return "ContinueStatement";
    if (llvm::isa<clang::BreakStmt>(stmt)) return "BreakStatement";
    if (llvm::isa<clang::CompoundAssignOperator>(stmt) || llvm::isa<clang::BinaryOperator>(stmt)) return "BinaryOperator";
    if (llvm::isa<clang::UnaryOperator>(stmt)) return "UnaryOperator";
    if (llvm::isa<clang::ConditionalOperator>(stmt)) return "ConditionalOperator";
    if (llvm::isa<clang::CXXOperatorCallExpr>(stmt)) return "OperatorCallExpression";
    if (llvm::isa<clang::CXXConstructExpr>(stmt)) return "ConstructorCallExpression";
    if (llvm::isa<clang::CXXFunctionalCastExpr>(stmt)) return "FunctionalCastExpression";
    if (llvm::isa<clang::CStyleCastExpr>(stmt)) return "CStyleCastExpression";
    if (llvm::isa<clang::CallExpr>(stmt)) return "CallExpression";
    if (llvm::isa<clang::MemberExpr>(stmt)) return "MemberExpression";
    if (llvm::isa<clang::DeclRefExpr>(stmt)) return "DeclRefExpression";
    if (llvm::isa<clang::IntegerLiteral>(stmt)) return "IntegerLiteral";
    if (llvm::isa<clang::FloatingLiteral>(stmt)) return "FloatingLiteral";
    if (llvm::isa<clang::CXXBoolLiteralExpr>(stmt)) return "BooleanLiteral";
    if (llvm::isa<clang::StringLiteral>(stmt)) return "StringLiteral";
    if (llvm::isa<clang::InitListExpr>(stmt)) return "InitializerListExpression";
    if (llvm::isa<clang::ImplicitCastExpr>(stmt)) return "ImplicitCastExpression";
    if (llvm::isa<clang::ParenExpr>(stmt)) return "ParenExpression";
    if (llvm::isa<clang::CXXDefaultArgExpr>(stmt)) return "DefaultArgumentExpression";
    if (llvm::isa<clang::ArraySubscriptExpr>(stmt)) return "ArraySubscriptExpression";
    return "Unknown";
}

bool IsInterestingStmt(const clang::Stmt* stmt)
{
    return llvm::isa<clang::CompoundStmt>(stmt) ||
        llvm::isa<clang::DeclStmt>(stmt) ||
        llvm::isa<clang::ReturnStmt>(stmt) ||
        llvm::isa<clang::IfStmt>(stmt) ||
        llvm::isa<clang::WhileStmt>(stmt) ||
        llvm::isa<clang::ForStmt>(stmt) ||
        llvm::isa<clang::ContinueStmt>(stmt) ||
        llvm::isa<clang::BreakStmt>(stmt) ||
        llvm::isa<clang::BinaryOperator>(stmt) ||
        llvm::isa<clang::UnaryOperator>(stmt) ||
        llvm::isa<clang::ConditionalOperator>(stmt) ||
        llvm::isa<clang::CallExpr>(stmt) ||
        llvm::isa<clang::CXXConstructExpr>(stmt) ||
        llvm::isa<clang::CXXFunctionalCastExpr>(stmt) ||
        llvm::isa<clang::CStyleCastExpr>(stmt) ||
        llvm::isa<clang::MemberExpr>(stmt) ||
        llvm::isa<clang::DeclRefExpr>(stmt) ||
        llvm::isa<clang::IntegerLiteral>(stmt) ||
        llvm::isa<clang::FloatingLiteral>(stmt) ||
        llvm::isa<clang::CXXBoolLiteralExpr>(stmt) ||
        llvm::isa<clang::StringLiteral>(stmt) ||
        llvm::isa<clang::InitListExpr>(stmt) ||
        llvm::isa<clang::ImplicitCastExpr>(stmt) ||
        llvm::isa<clang::ParenExpr>(stmt) ||
        llvm::isa<clang::CXXDefaultArgExpr>(stmt) ||
        llvm::isa<clang::ArraySubscriptExpr>(stmt);
}

std::string GetStmtDisplayName(const clang::Stmt* stmt)
{
    if (const auto* binary = llvm::dyn_cast<clang::BinaryOperator>(stmt))
    {
        return binary->getOpcodeStr().str();
    }
    if (const auto* unary = llvm::dyn_cast<clang::UnaryOperator>(stmt))
    {
        return std::string(clang::UnaryOperator::getOpcodeStr(unary->getOpcode()));
    }
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(stmt))
    {
        return member->getMemberNameInfo().getAsString();
    }
    if (const auto* decl_ref = llvm::dyn_cast<clang::DeclRefExpr>(stmt))
    {
        return decl_ref->getNameInfo().getAsString();
    }
    if (const auto* call = llvm::dyn_cast<clang::CallExpr>(stmt))
    {
        if (const auto* callee = call->getDirectCallee())
        {
            return callee->getNameAsString();
        }
    }
    return {};
}

void WriteTypeInfo(std::ostream& os, const std::optional<TypeInfo>& type_info)
{
    if (!type_info)
    {
        os << "null";
        return;
    }

    os << "{\"Spelling\":";
    WriteString(os, type_info->spelling);
    os << ",\"CanonicalName\":";
    WriteString(os, type_info->canonical_name);
    os << ",\"DesugaredName\":";
    WriteString(os, type_info->desugared_name);
    os << ",\"TemplateArguments\":[";
    for (size_t i = 0; i < type_info->template_arguments.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteTypeInfo(os, type_info->template_arguments[i]);
    }
    os << "]}";
}

void WriteTemplateArgumentInfo(std::ostream& os, const TemplateArgumentInfo& argument)
{
    os << "{\"Kind\":";
    WriteString(os, argument.kind);
    os << ",\"Spelling\":";
    WriteNullableString(os, argument.spelling);
    os << ",\"Value\":";
    WriteNullableString(os, argument.value);
    os << ",\"TypeInfo\":";
    WriteTypeInfo(os, argument.type_info);
    os << "}";
}

AstNode MakeNode(const clang::Decl* decl, const clang::ASTContext& ast_context);
AstNode MakeStmtNode(const clang::Stmt* stmt, const clang::ASTContext& ast_context);

void AppendStmtChildren(std::vector<AstNode>& children, const clang::Stmt* stmt, const clang::ASTContext& ast_context)
{
    for (const auto* child : stmt->children())
    {
        if (!child)
        {
            continue;
        }

        if (IsInterestingStmt(child))
        {
            children.push_back(MakeStmtNode(child, ast_context));
        }
        else
        {
            AppendStmtChildren(children, child, ast_context);
        }
    }
}

std::vector<AstNode> MakeChildren(const clang::Decl* decl, const clang::ASTContext& ast_context)
{
    std::vector<AstNode> children;
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        for (const auto* parameter : function->parameters())
        {
            children.push_back(MakeNode(parameter, ast_context));
        }
        if (const auto* body = function->getBody())
        {
            children.push_back(MakeStmtNode(body, ast_context));
        }
        return children;
    }

    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl))
    {
        if (const auto* initializer = variable->getInit())
        {
            children.push_back(MakeStmtNode(initializer, ast_context));
        }
        return children;
    }

    if (const auto* context = llvm::dyn_cast<clang::DeclContext>(decl))
    {
        for (const auto* child : context->decls())
        {
            if (child->isImplicit() || !IsInterestingDecl(child))
            {
                continue;
            }
            if (const auto* record = llvm::dyn_cast<clang::RecordDecl>(child); record && !record->isCompleteDefinition())
            {
                continue;
            }
            children.push_back(MakeNode(child, ast_context));
        }
    }
    return children;
}

AstNode MakeStmtNode(const clang::Stmt* stmt, const clang::ASTContext& ast_context)
{
    const auto& source_manager = ast_context.getSourceManager();
    AstNode node;
    node.kind = KindForStmt(stmt);
    node.provider_kind = stmt->getStmtClassName();
    node.spelling = GetSourceText(ast_context, stmt->getSourceRange());
    node.display_name = GetStmtDisplayName(stmt);
    node.location = GetLocation(source_manager, stmt->getBeginLoc());
    node.range = GetRange(source_manager, stmt->getSourceRange());
    node.uses_default_argument = llvm::isa<clang::CXXDefaultArgExpr>(stmt);

    if (const auto* expression = llvm::dyn_cast<clang::Expr>(stmt))
    {
        auto type = expression->getType();
        node.type_name = type.getAsString();
        node.type_info = MakeTypeInfo(ast_context, type);
        node.constant_value = TryEvaluateConstantValue(ast_context, expression);
    }

    if (const auto* decl_ref = llvm::dyn_cast<clang::DeclRefExpr>(stmt))
    {
        node.referenced_decl_id = CanonicalDeclId(decl_ref->getDecl());
    }
    if (const auto* member = llvm::dyn_cast<clang::MemberExpr>(stmt))
    {
        node.referenced_decl_id = CanonicalDeclId(member->getMemberDecl());
    }
    if (const auto* call = llvm::dyn_cast<clang::CallExpr>(stmt))
    {
        if (const auto* callee = call->getDirectCallee())
        {
            node.direct_callee_decl_id = CanonicalDeclId(callee);
            if (const auto* specialization = callee->getTemplateSpecializationInfo())
            {
                node.is_template_instantiation = true;
                node.template_pattern_decl_id = CanonicalDeclId(specialization->getTemplate());
                if (const auto* arguments = specialization->TemplateArguments)
                {
                    AppendTemplateArguments(node.template_arguments, ast_context, *arguments);
                }
            }
        }
    }
    if (const auto* constructor = llvm::dyn_cast<clang::CXXConstructExpr>(stmt))
    {
        node.direct_callee_decl_id = CanonicalDeclId(constructor->getConstructor());
    }

    if (const auto* declaration_statement = llvm::dyn_cast<clang::DeclStmt>(stmt))
    {
        for (const auto* declaration : declaration_statement->decls())
        {
            if (declaration && IsInterestingDecl(declaration))
            {
                node.children.push_back(MakeNode(declaration, ast_context));
            }
        }
        return node;
    }

    AppendStmtChildren(node.children, stmt, ast_context);
    return node;
}

AstNode MakeNode(const clang::Decl* decl, const clang::ASTContext& ast_context)
{
    const auto& source_manager = ast_context.getSourceManager();
    const auto* named_decl = llvm::dyn_cast<clang::NamedDecl>(decl);
    AstNode node;
    node.kind = KindForDecl(decl);
    node.provider_kind = decl->getDeclKindName();
    node.spelling = GetDeclName(named_decl);
    node.display_name = GetDisplayName(named_decl);
    node.location = GetLocation(source_manager, decl->getLocation());
    node.range = GetRange(source_manager, decl->getSourceRange());
    node.decl_id = DeclId(decl);
    node.canonical_decl_id = CanonicalDeclId(decl);
    node.owner_decl_id = OwnerDeclId(decl);
    node.is_implicit = decl->isImplicit();

    if (const auto* value_decl = llvm::dyn_cast<clang::ValueDecl>(decl))
    {
        auto type = value_decl->getType();
        node.type_name = type.getAsString();
        node.type_info = MakeTypeInfo(ast_context, type);
    }
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl))
    {
        node.is_constexpr = variable->isConstexpr();
        if (const auto* initializer = variable->getInit())
        {
            node.constant_value = TryEvaluateConstantValue(ast_context, initializer);
        }
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        auto result_type = function->getReturnType();
        node.result_type_name = result_type.getAsString();
        node.result_type_info = MakeTypeInfo(ast_context, result_type);
        node.is_constexpr = function->isConstexpr();
        if (const auto* specialization = function->getTemplateSpecializationInfo())
        {
            node.is_template_instantiation = true;
            node.template_pattern_decl_id = CanonicalDeclId(specialization->getTemplate());
            if (const auto* arguments = specialization->TemplateArguments)
            {
                AppendTemplateArguments(node.template_arguments, ast_context, *arguments);
            }
        }
    }
    if (const auto* specialization = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl))
    {
        node.is_template_instantiation = true;
        node.template_pattern_decl_id = CanonicalDeclId(specialization->getSpecializedTemplate());
        AppendTemplateArguments(node.template_arguments, ast_context, specialization->getTemplateArgs());
    }

    node.children = MakeChildren(decl, ast_context);
    return node;
}

void WriteNode(std::ostream& os, const AstNode& node)
{
    os << "{\"Kind\":";
    WriteString(os, node.kind);
    os << ",\"ProviderKind\":";
    WriteString(os, node.provider_kind);
    os << ",\"Spelling\":";
    WriteString(os, node.spelling);
    os << ",\"DisplayName\":";
    WriteNullableString(os, node.display_name);
    os << ",\"TypeName\":";
    WriteNullableString(os, node.type_name);
    os << ",\"ResultTypeName\":";
    WriteNullableString(os, node.result_type_name);
    os << ",\"DeclId\":";
    WriteNullableString(os, node.decl_id);
    os << ",\"CanonicalDeclId\":";
    WriteNullableString(os, node.canonical_decl_id);
    os << ",\"ReferencedDeclId\":";
    WriteNullableString(os, node.referenced_decl_id);
    os << ",\"DirectCalleeDeclId\":";
    WriteNullableString(os, node.direct_callee_decl_id);
    os << ",\"OwnerDeclId\":";
    WriteNullableString(os, node.owner_decl_id);
    os << ",\"TemplatePatternDeclId\":";
    WriteNullableString(os, node.template_pattern_decl_id);
    os << ",\"IsImplicit\":" << (node.is_implicit ? "true" : "false");
    os << ",\"IsConstexpr\":" << (node.is_constexpr ? "true" : "false");
    os << ",\"IsTemplateInstantiation\":" << (node.is_template_instantiation ? "true" : "false");
    os << ",\"UsesDefaultArgument\":" << (node.uses_default_argument ? "true" : "false");
    os << ",\"ConstantValue\":";
    WriteNullableString(os, node.constant_value);
    os << ",\"Location\":";
    WriteLocation(os, node.location);
    os << ",\"Range\":";
    WriteRange(os, node.range);
    os << ",\"TypeInfo\":";
    WriteTypeInfo(os, node.type_info);
    os << ",\"ResultTypeInfo\":";
    WriteTypeInfo(os, node.result_type_info);
    os << ",\"TemplateArguments\":[";
    for (size_t i = 0; i < node.template_arguments.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteTemplateArgumentInfo(os, node.template_arguments[i]);
    }
    os << "]";
    os << ",\"Attributes\":[],\"Children\":[";
    for (size_t i = 0; i < node.children.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteNode(os, node.children[i]);
    }
    os << "]}";
}

void WriteDeclaration(std::ostream& os, const AstNode& node)
{
    os << "{\"Kind\":";
    WriteString(os, node.kind);
    os << ",\"ProviderKind\":";
    WriteString(os, node.provider_kind);
    os << ",\"Spelling\":";
    WriteString(os, node.spelling);
    os << ",\"DisplayName\":";
    WriteNullableString(os, node.display_name);
    os << ",\"DeclId\":";
    WriteNullableString(os, node.decl_id);
    os << ",\"CanonicalDeclId\":";
    WriteNullableString(os, node.canonical_decl_id);
    os << ",\"OwnerDeclId\":";
    WriteNullableString(os, node.owner_decl_id);
    os << ",\"IsImplicit\":" << (node.is_implicit ? "true" : "false");
    os << ",\"IsConstexpr\":" << (node.is_constexpr ? "true" : "false");
    os << ",\"IsTemplateInstantiation\":" << (node.is_template_instantiation ? "true" : "false");
    os << ",\"TemplatePatternDeclId\":";
    WriteNullableString(os, node.template_pattern_decl_id);
    os << ",\"TemplateArguments\":[";
    for (size_t i = 0; i < node.template_arguments.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteTemplateArgumentInfo(os, node.template_arguments[i]);
    }
    os << "]";
    os << ",\"Location\":";
    WriteLocation(os, node.location);
    os << "}";
}

void WriteDiagnostic(std::ostream& os, const DiagnosticInfo& diagnostic)
{
    os << "{\"Severity\":";
    WriteString(os, diagnostic.severity);
    os << ",\"Message\":";
    WriteString(os, diagnostic.message);
    os << ",\"File\":";
    if (!diagnostic.file.empty())
    {
        WriteString(os, diagnostic.file);
    }
    else
    {
        os << "null";
    }
    os << ",\"Line\":";
    if (diagnostic.line) os << *diagnostic.line;
    else os << "null";
    os << ",\"Column\":";
    if (diagnostic.column) os << *diagnostic.column;
    else os << "null";
    os << "}";
}

void WriteResult(
    std::ostream& os,
    bool succeeded,
    const std::string& source_path,
    const std::vector<AstNode>& declarations,
    const std::vector<AstNode>& ast_nodes,
    const std::vector<DiagnosticInfo>& diagnostics)
{
    os << "{\"Succeeded\":" << (succeeded ? "true" : "false");
    os << ",\"Provider\":\"Native\",\"ModelVersion\":3,\"Diagnostics\":[";
    for (size_t i = 0; i < diagnostics.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteDiagnostic(os, diagnostics[i]);
    }
    os << "],\"Declarations\":[";
    for (size_t i = 0; i < declarations.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteDeclaration(os, declarations[i]);
    }
    os << "],\"AstNodes\":[";
    for (size_t i = 0; i < ast_nodes.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteNode(os, ast_nodes[i]);
    }
    os << "]}";
}

Options ParseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        if (arg == "--source" && i + 1 < argc)
        {
            options.source_path = argv[++i];
        }
        else if (arg == "--include" && i + 1 < argc)
        {
            options.include_roots.emplace_back(argv[++i]);
        }
    }
    return options;
}

DiagnosticInfo ErrorDiagnostic(std::string message, const std::string& source_path = {})
{
    DiagnosticInfo diagnostic;
    diagnostic.severity = "Error";
    diagnostic.message = std::move(message);
    diagnostic.file = source_path;
    return diagnostic;
}
}

int main(int argc, char** argv)
{
    auto options = ParseOptions(argc, argv);
    if (options.source_path.empty())
    {
        WriteResult(std::cout, false, {}, {}, {}, {ErrorDiagnostic("missing --source")});
        return 2;
    }

    std::ifstream source_stream(options.source_path);
    if (!source_stream)
    {
        WriteResult(std::cout, false, options.source_path, {}, {}, {ErrorDiagnostic("source file does not exist", options.source_path)});
        return 2;
    }
    std::stringstream source_buffer;
    source_buffer << source_stream.rdbuf();

    std::vector<std::string> args = {
        "-x",
        "c++",
        "-std=c++20",
        "-fsyntax-only",
        "-Wno-unknown-attributes",
        "-Wno-ignored-attributes",
        "-D__CPPSL__=1"
    };
    for (const auto& include_root : options.include_roots)
    {
        args.push_back("-I" + include_root);
    }

    JsonDiagnosticConsumer diagnostic_consumer;
    auto ast_unit = clang::tooling::buildASTFromCodeWithArgs(
        source_buffer.str(),
        args,
        options.source_path,
        "cppsl-native-extractor",
        std::make_shared<clang::PCHContainerOperations>(),
        clang::tooling::getClangStripDependencyFileAdjuster(),
        clang::tooling::FileContentMappings(),
        &diagnostic_consumer);
    if (!ast_unit)
    {
        auto diagnostics = diagnostic_consumer.Diagnostics();
        std::vector<DiagnosticInfo> fallback_diagnostics(diagnostics.begin(), diagnostics.end());
        if (fallback_diagnostics.empty())
        {
            fallback_diagnostics.push_back(ErrorDiagnostic("Clang native extractor failed to build AST", options.source_path));
        }
        WriteResult(std::cout, false, options.source_path, {}, {}, fallback_diagnostics);
        return 1;
    }

    std::vector<AstNode> ast_nodes;
    const auto& ast_context = ast_unit->getASTContext();
    for (const auto* decl : ast_context.getTranslationUnitDecl()->decls())
    {
        if (decl->isImplicit() || !IsInterestingDecl(decl))
        {
            continue;
        }
        if (const auto* record = llvm::dyn_cast<clang::RecordDecl>(decl); record && !record->isCompleteDefinition())
        {
            continue;
        }
        ast_nodes.push_back(MakeNode(decl, ast_context));
    }

    std::vector<AstNode> declarations;
    for (const auto& node : ast_nodes)
    {
        if (node.kind == "Namespace" ||
            node.kind == "Struct" ||
            node.kind == "Class" ||
            node.kind == "Enum" ||
            node.kind == "Function" ||
            node.kind == "FunctionTemplate" ||
            node.kind == "ClassTemplate" ||
            node.kind == "GlobalVariable")
        {
            declarations.push_back(node);
        }
    }

    const auto& diagnostics = diagnostic_consumer.Diagnostics();
    const bool has_errors = HasErrorDiagnostics(diagnostics);
    WriteResult(std::cout, !has_errors, options.source_path, declarations, ast_nodes, diagnostics);
    return has_errors ? 1 : 0;
}
