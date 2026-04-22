#include <clang/AST/AST.h>
#include <clang/AST/DeclCXX.h>
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

struct AstNode
{
    std::string kind;
    std::string provider_kind;
    std::string spelling;
    std::string display_name;
    std::string type_name;
    std::string result_type_name;
    std::optional<SourceLocationInfo> location;
    std::vector<AstNode> children;
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

AstNode MakeNode(const clang::Decl* decl, const clang::SourceManager& source_manager);

std::vector<AstNode> MakeChildren(const clang::Decl* decl, const clang::SourceManager& source_manager)
{
    std::vector<AstNode> children;
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        for (const auto* parameter : function->parameters())
        {
            children.push_back(MakeNode(parameter, source_manager));
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
            children.push_back(MakeNode(child, source_manager));
        }
    }
    return children;
}

AstNode MakeNode(const clang::Decl* decl, const clang::SourceManager& source_manager)
{
    const auto* named_decl = llvm::dyn_cast<clang::NamedDecl>(decl);
    AstNode node;
    node.kind = KindForDecl(decl);
    node.provider_kind = decl->getDeclKindName();
    node.spelling = GetDeclName(named_decl);
    node.display_name = GetDisplayName(named_decl);
    node.location = GetLocation(source_manager, decl->getLocation());

    if (const auto* value_decl = llvm::dyn_cast<clang::ValueDecl>(decl))
    {
        node.type_name = value_decl->getType().getAsString();
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        node.result_type_name = function->getReturnType().getAsString();
    }

    node.children = MakeChildren(decl, source_manager);
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
    os << ",\"Range\":null,\"Attributes\":[],\"Children\":[";
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

void WriteDiagnostic(std::ostream& os, std::string_view severity, std::string_view message, std::string_view file)
{
    os << "{\"Severity\":";
    WriteString(os, severity);
    os << ",\"Message\":";
    WriteString(os, message);
    os << ",\"File\":";
    WriteString(os, file);
    os << ",\"Line\":null,\"Column\":null}";
}

void WriteResult(
    std::ostream& os,
    bool succeeded,
    const std::string& source_path,
    const std::vector<AstNode>& declarations,
    const std::vector<AstNode>& ast_nodes,
    const std::vector<std::string>& errors)
{
    os << "{\"Succeeded\":" << (succeeded ? "true" : "false");
    os << ",\"Provider\":\"Native\",\"ModelVersion\":0,\"Diagnostics\":[";
    for (size_t i = 0; i < errors.size(); ++i)
    {
        if (i != 0) os << ",";
        WriteDiagnostic(os, "Error", errors[i], source_path);
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
}

int main(int argc, char** argv)
{
    auto options = ParseOptions(argc, argv);
    if (options.source_path.empty())
    {
        WriteResult(std::cout, false, {}, {}, {}, {"missing --source"});
        return 2;
    }

    std::ifstream source_stream(options.source_path);
    if (!source_stream)
    {
        WriteResult(std::cout, false, options.source_path, {}, {}, {"source file does not exist"});
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

    auto ast_unit = clang::tooling::buildASTFromCodeWithArgs(
        source_buffer.str(),
        args,
        options.source_path);
    if (!ast_unit)
    {
        WriteResult(std::cout, false, options.source_path, {}, {}, {"Clang native extractor failed to build AST"});
        return 1;
    }

    std::vector<AstNode> ast_nodes;
    const auto& source_manager = ast_unit->getSourceManager();
    for (const auto* decl : ast_unit->getASTContext().getTranslationUnitDecl()->decls())
    {
        if (decl->isImplicit() || !IsInterestingDecl(decl))
        {
            continue;
        }
        if (const auto* record = llvm::dyn_cast<clang::RecordDecl>(decl); record && !record->isCompleteDefinition())
        {
            continue;
        }
        ast_nodes.push_back(MakeNode(decl, source_manager));
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

    WriteResult(std::cout, true, options.source_path, declarations, ast_nodes, {});
    return 0;
}
