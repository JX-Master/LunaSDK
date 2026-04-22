#include <clang/AST/AST.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
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
    std::optional<SourceLocationInfo> location;
    std::optional<SourceRangeInfo> range;
    std::optional<TypeInfo> type_info;
    std::optional<TypeInfo> result_type_info;
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
    if (llvm::isa<clang::FunctionDecl>(decl)) return "Function";
    if (llvm::isa<clang::VarDecl>(decl)) return "GlobalVariable";
    if (llvm::isa<clang::CXXConstructorDecl>(decl)) return "Constructor";
    if (llvm::isa<clang::CXXMethodDecl>(decl)) return "Method";
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

AstNode MakeNode(const clang::Decl* decl, const clang::ASTContext& ast_context);

std::vector<AstNode> MakeChildren(const clang::Decl* decl, const clang::ASTContext& ast_context)
{
    std::vector<AstNode> children;
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        for (const auto* parameter : function->parameters())
        {
            children.push_back(MakeNode(parameter, ast_context));
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

    if (const auto* value_decl = llvm::dyn_cast<clang::ValueDecl>(decl))
    {
        auto type = value_decl->getType();
        node.type_name = type.getAsString();
        node.type_info = MakeTypeInfo(ast_context, type);
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        auto result_type = function->getReturnType();
        node.result_type_name = result_type.getAsString();
        node.result_type_info = MakeTypeInfo(ast_context, result_type);
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
    os << ",\"Location\":";
    WriteLocation(os, node.location);
    os << ",\"Range\":";
    WriteRange(os, node.range);
    os << ",\"TypeInfo\":";
    WriteTypeInfo(os, node.type_info);
    os << ",\"ResultTypeInfo\":";
    WriteTypeInfo(os, node.result_type_info);
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
    os << ",\"Provider\":\"Native\",\"ModelVersion\":1,\"Diagnostics\":[";
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
