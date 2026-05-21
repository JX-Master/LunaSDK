#include <clang/AST/AST.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace
{
struct Options
{
    std::vector<std::string> headers;
    std::vector<std::string> header_languages;
    std::vector<std::string> include_roots;
    std::vector<std::string> defines;
    std::vector<std::string> undefines;
    std::string output_dir;
    std::string stamp;
    std::string depfile;
    std::string mode;
    std::string platform;
    std::string arch;
    std::string sysroot;
    std::string resource_dir;
};

enum class ReflectedDeclKind
{
    record,
    enumeration
};

struct ReflectedDecl
{
    ReflectedDeclKind kind;
    std::string declaration_keyword;
    std::string name;
    std::string qualified_name;
    std::string reflection_name;
    std::string guid;
    std::string declaration_attributes;
    std::string enum_underlying_type;
    bool enum_scoped = false;
    std::vector<std::string> namespaces;
    struct Property
    {
        std::string name;
        std::string type;
        uint64_t offset = 0;
    };
    struct Option
    {
        std::string name;
        int64_t value = 0;
    };
    std::vector<Property> properties;
    std::vector<Option> options;
};

struct ParsedAttribute
{
    std::string kind;
    std::string guid;
};

Options ParseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        auto next = [&]() -> std::string {
            if (i + 1 >= argc)
            {
                return {};
            }
            return argv[++i];
        };

        if (arg == "--header")
        {
            options.headers.push_back(next());
        }
        else if (arg == "--header-language")
        {
            options.header_languages.push_back(next());
        }
        else if (arg == "--include")
        {
            options.include_roots.push_back(next());
        }
        else if (arg == "--define")
        {
            options.defines.push_back(next());
        }
        else if (arg == "--undefine")
        {
            options.undefines.push_back(next());
        }
        else if (arg == "--output-dir")
        {
            options.output_dir = next();
        }
        else if (arg == "--stamp")
        {
            options.stamp = next();
        }
        else if (arg == "--depfile")
        {
            options.depfile = next();
        }
        else if (arg == "--mode")
        {
            options.mode = next();
        }
        else if (arg == "--platform")
        {
            options.platform = next();
        }
        else if (arg == "--arch")
        {
            options.arch = next();
        }
        else if (arg == "--isysroot")
        {
            options.sysroot = next();
        }
        else if (arg == "--resource-dir")
        {
            options.resource_dir = next();
        }
    }
    return options;
}

bool ReadTextFile(const std::filesystem::path& path, std::string& text)
{
    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "error: cannot read file: " << path.string() << "\n";
        return false;
    }
    std::stringstream buffer;
    buffer << stream.rdbuf();
    text = buffer.str();
    return true;
}

std::string TrimLeft(std::string_view value)
{
    size_t pos = 0;
    while (pos < value.size() && (value[pos] == ' ' || value[pos] == '\t'))
    {
        ++pos;
    }
    return std::string(value.substr(pos));
}

std::string Trim(std::string_view value)
{
    size_t begin = 0;
    while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t' || value[begin] == '\r' || value[begin] == '\n'))
    {
        ++begin;
    }
    size_t end = value.size();
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n'))
    {
        --end;
    }
    return std::string(value.substr(begin, end - begin));
}

bool EndsWith(std::string_view value, std::string_view suffix)
{
    return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
}

std::string GeneratedHeaderName(const std::filesystem::path& header)
{
    return header.stem().string() + ".generated.hpp";
}

std::filesystem::path GeneratedHeaderPath(const Options& options, const std::filesystem::path& header)
{
    return std::filesystem::path(options.output_dir) / GeneratedHeaderName(header);
}

std::string EscapeCppString(std::string_view value)
{
    std::string result;
    result.reserve(value.size());
    for (char ch : value)
    {
        if (ch == '\\' || ch == '"')
        {
            result.push_back('\\');
        }
        result.push_back(ch);
    }
    return result;
}

std::string TypeReference(const ReflectedDecl& decl)
{
    return "::" + decl.qualified_name;
}

std::string TypeofReference(std::string type)
{
    type = Trim(type);
    return "&::Luna::typeof<" + type + ">";
}

bool WriteGeneratedHeader(const std::filesystem::path& path, const std::vector<ReflectedDecl>& declarations = {})
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::trunc);
    if (!stream)
    {
        std::cerr << "error: cannot write generated header: " << path.string() << "\n";
        return false;
    }
    stream << "// Autogenerated by LunaMetaTool, do not modify.\n";
    stream << "#pragma once\n";
    if (!declarations.empty())
    {
        stream << "#include <Luna/Runtime/TypeInfo.hpp>\n\n";
        for (const auto& declaration : declarations)
        {
            if (!declaration.namespaces.empty())
            {
                stream << "namespace ";
                for (size_t i = 0; i < declaration.namespaces.size(); ++i)
                {
                    if (i)
                    {
                        stream << "::";
                    }
                    stream << declaration.namespaces[i];
                }
                stream << " { ";
            }
            if (declaration.kind == ReflectedDeclKind::record)
            {
                stream << declaration.declaration_keyword;
                if (!declaration.declaration_attributes.empty())
                {
                    stream << " " << declaration.declaration_attributes;
                }
                stream << " " << declaration.name << ";";
            }
            else
            {
                stream << "enum " << (declaration.enum_scoped ? "class " : "") << declaration.name
                    << " : " << declaration.enum_underlying_type << ";";
            }
            if (!declaration.namespaces.empty())
            {
                stream << " }";
            }
            stream << "\n";
        }
        stream << "\nnamespace Luna::Meta\n{\n";
        for (const auto& declaration : declarations)
        {
            if (declaration.kind == ReflectedDeclKind::record)
            {
                stream << "    template <> struct StructMetaData<" << TypeReference(declaration) << ">\n";
                stream << "    {\n";
                stream << "        static constexpr const c8* __name = \"" << EscapeCppString(declaration.reflection_name) << "\";\n";
                stream << "        static constexpr Guid __guid { Guid(\"" << EscapeCppString(declaration.guid) << "\") };\n";
                if (!declaration.properties.empty())
                {
                    stream << "        inline static constexpr StructPropertyMetaData __properties[] =\n";
                    stream << "        {\n";
                    for (const auto& property : declaration.properties)
                    {
                        stream << "            { \"" << EscapeCppString(property.name) << "\", "
                            << TypeofReference(property.type) << ", " << property.offset << " },\n";
                    }
                    stream << "        };\n";
                }
                stream << "    };\n";
            }
            else
            {
                stream << "    template <> struct EnumMetadata<" << TypeReference(declaration) << ">\n";
                stream << "    {\n";
                stream << "        static constexpr const c8* __name = \"" << EscapeCppString(declaration.reflection_name) << "\";\n";
                stream << "        static constexpr Guid __guid { Guid(\"" << EscapeCppString(declaration.guid) << "\") };\n";
                if (!declaration.options.empty())
                {
                    stream << "        inline static constexpr EnumOptionMetaData __options[] =\n";
                    stream << "        {\n";
                    for (const auto& option : declaration.options)
                    {
                        stream << "            { \"" << EscapeCppString(option.name) << "\", " << option.value << " },\n";
                    }
                    stream << "        };\n";
                }
                stream << "    };\n";
            }
        }
        stream << "}\n";
    }
    return true;
}

bool PrecreateGeneratedHeaders(const Options& options)
{
    std::map<std::string, std::string> generated_names;
    for (const auto& header : options.headers)
    {
        auto name = GeneratedHeaderName(header);
        auto [iter, inserted] = generated_names.emplace(name, header);
        if (!inserted)
        {
            std::cerr << "error: multiple meta headers generate the same file: " << name << "\n"
                << "  first: " << iter->second << "\n"
                << "  second: " << header << "\n";
            return false;
        }
        if (!WriteGeneratedHeader(GeneratedHeaderPath(options, header)))
        {
            return false;
        }
    }
    return true;
}

std::string ParseIncludeTarget(const std::string& line)
{
    auto trimmed = TrimLeft(line);
    if (!trimmed.starts_with("#include"))
    {
        return {};
    }
    auto first_quote = trimmed.find('"');
    if (first_quote != std::string::npos)
    {
        auto second_quote = trimmed.find('"', first_quote + 1);
        if (second_quote != std::string::npos)
        {
            return trimmed.substr(first_quote + 1, second_quote - first_quote - 1);
        }
    }
    auto first_angle = trimmed.find('<');
    if (first_angle != std::string::npos)
    {
        auto second_angle = trimmed.find('>', first_angle + 1);
        if (second_angle != std::string::npos)
        {
            return trimmed.substr(first_angle + 1, second_angle - first_angle - 1);
        }
    }
    return {};
}

bool ValidateGeneratedIncludeOrder(const std::filesystem::path& header, const std::string& text, std::string& error)
{
    auto expected = GeneratedHeaderName(header);
    bool saw_generated_include = false;
    bool saw_expected = false;
    std::istringstream lines(text);
    std::string line;
    unsigned line_number = 0;
    while (std::getline(lines, line))
    {
        ++line_number;
        auto include = ParseIncludeTarget(line);
        if (include.empty())
        {
            continue;
        }
        const bool is_generated = EndsWith(include, ".generated.hpp");
        if (is_generated)
        {
            saw_generated_include = true;
            if (std::filesystem::path(include).filename() == expected)
            {
                saw_expected = true;
            }
            continue;
        }
        if (saw_generated_include)
        {
            error = header.string() + ":" + std::to_string(line_number) +
                ": non-generated include appears after a generated include";
            return false;
        }
    }
    if (!saw_expected)
    {
        error = header.string() + ": missing #include \"" + expected + "\"";
        return false;
    }
    return true;
}

bool IsInTargetFile(
    const clang::SourceManager& sm,
    clang::SourceLocation location,
    const std::filesystem::path& target_file)
{
    if (location.isInvalid())
    {
        return false;
    }
    auto filename = sm.getFilename(sm.getSpellingLoc(location)).str();
    if (filename.empty())
    {
        return false;
    }
    std::error_code error;
    auto actual = std::filesystem::weakly_canonical(filename, error);
    if (error)
    {
        actual = std::filesystem::absolute(filename);
    }
    auto expected = std::filesystem::weakly_canonical(target_file, error);
    if (error)
    {
        expected = std::filesystem::absolute(target_file);
    }
    return actual == expected;
}

std::string DiagnosticLocation(
    const clang::SourceManager& sm,
    clang::SourceLocation location)
{
    auto presumed = sm.getPresumedLoc(sm.getSpellingLoc(location));
    if (!presumed.isValid())
    {
        return "unknown";
    }
    std::stringstream stream;
    stream << presumed.getFilename() << ":" << presumed.getLine() << ":" << presumed.getColumn();
    return stream.str();
}

std::string DeclSourceText(
    const clang::Decl* decl,
    const clang::SourceManager& sm,
    const clang::LangOptions& lang_options)
{
    auto range = clang::CharSourceRange::getTokenRange(decl->getSourceRange());
    if (range.isInvalid())
    {
        return {};
    }
    return clang::Lexer::getSourceText(range, sm, lang_options).str();
}

bool ExtractLunaAttributes(
    const std::string& source_text,
    std::vector<ParsedAttribute>& attributes,
    std::string& error)
{
    static const std::regex attribute_regex(R"(\[\[\s*luna::(struct|enum)\s*\(([^)]*)\)\s*\]\])");
    static const std::regex string_regex(R"meta("((?:\\.|[^"\\])*)")meta");
    for (std::sregex_iterator iter(source_text.begin(), source_text.end(), attribute_regex), end; iter != end; ++iter)
    {
        ParsedAttribute attribute;
        attribute.kind = (*iter)[1].str();
        auto arguments = (*iter)[2].str();
        std::smatch string_match;
        if (!std::regex_search(arguments, string_match, string_regex))
        {
            error = "missing GUID string in [[luna::" + attribute.kind + "(...)]]";
            return false;
        }
        attribute.guid = string_match[1].str();
        attributes.push_back(attribute);
    }
    return true;
}

bool BuildNamespaceList(
    const clang::Decl* decl,
    std::vector<std::string>& namespaces,
    std::string& error)
{
    std::vector<std::string> reversed;
    const auto* context = decl->getDeclContext();
    while (context && !context->isTranslationUnit())
    {
        if (const auto* namespace_decl = llvm::dyn_cast<clang::NamespaceDecl>(context))
        {
            if (namespace_decl->isAnonymousNamespace() || namespace_decl->getName().empty())
            {
                error = "reflected declarations in anonymous namespaces are not supported";
                return false;
            }
            reversed.push_back(namespace_decl->getNameAsString());
        }
        else if (llvm::isa<clang::CXXRecordDecl>(context))
        {
            error = "nested reflected declarations are not supported in this phase";
            return false;
        }
        context = context->getParent();
    }
    namespaces.assign(reversed.rbegin(), reversed.rend());
    return true;
}

std::string ReflectionNameFromQualifiedName(std::string qualified_name)
{
    constexpr std::string_view luna_prefix = "Luna::";
    if (qualified_name.starts_with(luna_prefix))
    {
        qualified_name.erase(0, luna_prefix.size());
    }
    return qualified_name;
}

std::string RecordDeclarationAttributes(const std::string& source_text, std::string_view declaration_keyword)
{
    const std::regex head_regex("\\b" + std::string(declaration_keyword) + R"(\b\s*((?:\[\[[^\]]+\]\]\s*|alignas\s*\([^)]*\)\s*)*))");
    std::smatch match;
    if (!std::regex_search(source_text, match, head_regex))
    {
        return {};
    }
    auto attributes = match[1].str();
    attributes = std::regex_replace(attributes, std::regex(R"(\[\[\s*luna::(struct|enum)\s*\([^)]*\)\s*\]\]\s*)"), "");
    return Trim(attributes);
}

bool ContainsLunaMarkerAttribute(const std::string& source_text, std::string_view attribute_name)
{
    const std::regex attribute_regex(
        std::string(R"(\[\[\s*(?:Luna|luna)::)") + std::string(attribute_name) + R"(\s*\]\])");
    return std::regex_search(source_text, attribute_regex);
}

bool RecordTextContainsFieldAttribute(
    const std::string& record_text,
    const std::string& field_name,
    std::string_view attribute_name)
{
    const std::regex attribute_regex(
        std::string(R"(\[\[\s*(?:Luna|luna)::)") + std::string(attribute_name) + R"(\s*\]\][^;{}]*\b)" +
        field_name +
        R"(\b)");
    return std::regex_search(record_text, attribute_regex);
}

bool EnumTextContainsOptionAttribute(
    const std::string& enum_text,
    const std::string& option_name,
    std::string_view attribute_name)
{
    const std::regex prefix_attribute_regex(
        std::string(R"(\[\[\s*(?:Luna|luna)::)") + std::string(attribute_name) + R"(\s*\]\]\s*)" +
        option_name +
        R"(\b)");
    const std::regex suffix_attribute_regex(
        std::string(R"(\b)") + option_name +
        R"(\b\s*\[\[\s*(?:Luna|luna)::)" + std::string(attribute_name) + R"(\s*\]\])");
    return std::regex_search(enum_text, prefix_attribute_regex) ||
        std::regex_search(enum_text, suffix_attribute_regex);
}

std::string TypeSourceText(
    const clang::DeclaratorDecl* decl,
    const clang::ASTContext& ast_context)
{
    const auto& sm = ast_context.getSourceManager();
    const auto& lang_options = ast_context.getLangOpts();
    if (auto* type_source_info = decl->getTypeSourceInfo())
    {
        auto type_text = clang::Lexer::getSourceText(
            clang::CharSourceRange::getTokenRange(type_source_info->getTypeLoc().getSourceRange()),
            sm,
            lang_options).str();
        type_text = Trim(type_text);
        if (!type_text.empty())
        {
            return type_text;
        }
    }

    clang::PrintingPolicy policy(ast_context.getLangOpts());
    policy.SuppressScope = false;
    return decl->getType().getAsString(policy);
}

bool CollectRecordProperties(
    const clang::CXXRecordDecl* record,
    const std::string& record_text,
    const clang::ASTContext& ast_context,
    std::vector<ReflectedDecl::Property>& properties,
    std::string& error)
{
    std::set<std::string> property_names;
    auto collect_fields = [&](const clang::CXXRecordDecl* current_record, uint64_t base_bit_offset, const auto& collect_fields_ref) -> bool {
        for (const auto* field : current_record->fields())
        {
            auto field_text = DeclSourceText(field, ast_context.getSourceManager(), ast_context.getLangOpts());
            auto field_name = field->getNameAsString();
            auto bit_offset = base_bit_offset + ast_context.getFieldOffset(field);
            if (field_name.empty())
            {
                if (auto* anonymous_record = field->getType()->getAsCXXRecordDecl())
                {
                    if (!collect_fields_ref(anonymous_record, bit_offset, collect_fields_ref))
                    {
                        return false;
                    }
                }
                continue;
            }
            const bool reflected = ContainsLunaMarkerAttribute(field_text, "property") ||
                RecordTextContainsFieldAttribute(record_text, field_name, "property");
            if (!reflected)
            {
                continue;
            }
            if (field->isBitField())
            {
                error = "[[Luna::property]] does not support bit-fields in this phase: " + field_name;
                return false;
            }
            if (!property_names.insert(field_name).second)
            {
                error = "duplicate reflected property: " + field_name;
                return false;
            }
            if (bit_offset % 8 != 0)
            {
                error = "property offset is not byte-aligned: " + field_name;
                return false;
            }

            ReflectedDecl::Property property;
            property.name = field_name;
            property.type = TypeSourceText(field, ast_context);
            property.offset = bit_offset / 8;
            properties.push_back(std::move(property));
        }
        return true;
    };
    return collect_fields(record, 0, collect_fields);
}

bool CollectEnumOptions(
    const clang::EnumDecl* enum_decl,
    const std::string& enum_text,
    std::vector<ReflectedDecl::Option>& options,
    std::string& error)
{
    std::set<std::string> option_names;
    for (const auto* option_decl : enum_decl->enumerators())
    {
        auto option_name = option_decl->getNameAsString();
        auto option_text = DeclSourceText(option_decl, enum_decl->getASTContext().getSourceManager(), enum_decl->getASTContext().getLangOpts());
        const bool reflected = ContainsLunaMarkerAttribute(option_text, "option") ||
            EnumTextContainsOptionAttribute(enum_text, option_name, "option");
        if (!reflected)
        {
            continue;
        }
        if (option_name.empty())
        {
            error = "[[Luna::option]] cannot be used on unnamed enum options";
            return false;
        }
        if (!option_names.insert(option_name).second)
        {
            error = "duplicate reflected enum option: " + option_name;
            return false;
        }

        ReflectedDecl::Option option;
        option.name = option_name;
        option.value = option_decl->getInitVal().getSExtValue();
        options.push_back(std::move(option));
    }
    return true;
}

std::string EnumUnderlyingType(
    const clang::EnumDecl* enum_decl,
    const clang::ASTContext& ast_context,
    const std::string& source_text)
{
    const std::regex source_underlying_regex("\\b" + enum_decl->getNameAsString() + "\\b\\s*:\\s*([^\\{]+)\\{");
    std::smatch source_underlying_match;
    if (std::regex_search(source_text, source_underlying_match, source_underlying_regex))
    {
        auto source_underlying_type = Trim(source_underlying_match[1].str());
        if (!source_underlying_type.empty())
        {
            return source_underlying_type;
        }
    }

    const auto& sm = ast_context.getSourceManager();
    const auto& lang_options = ast_context.getLangOpts();
    if (auto* type_source_info = enum_decl->getIntegerTypeSourceInfo())
    {
        auto source_text = clang::Lexer::getSourceText(
            clang::CharSourceRange::getTokenRange(type_source_info->getTypeLoc().getSourceRange()),
            sm,
            lang_options).str();
        if (!source_text.empty())
        {
            return source_text;
        }
    }

    clang::PrintingPolicy policy(ast_context.getLangOpts());
    policy.SuppressScope = false;
    return enum_decl->getIntegerType().getAsString(policy);
}

bool ValidateSingleAttribute(
    const std::vector<ParsedAttribute>& attributes,
    std::string_view expected_kind,
    std::string& guid,
    std::string& error)
{
    unsigned expected_count = 0;
    for (const auto& attribute : attributes)
    {
        if (attribute.kind == expected_kind)
        {
            ++expected_count;
            guid = attribute.guid;
        }
        else
        {
            error = "attribute [[luna::" + attribute.kind + "]] cannot be used on this declaration";
            return false;
        }
    }
    if (expected_count > 1)
    {
        error = "duplicate [[luna::" + std::string(expected_kind) + "]] attributes";
        return false;
    }
    return true;
}

bool AddReflectedRecord(
    const clang::CXXRecordDecl* record,
    const std::vector<ParsedAttribute>& attributes,
    const std::string& source_text,
    const clang::ASTContext& ast_context,
    const clang::SourceManager& sm,
    std::vector<ReflectedDecl>& reflected_decls,
    std::set<std::string>& reflected_names,
    std::string& error)
{
    std::string guid;
    if (!ValidateSingleAttribute(attributes, "struct", guid, error))
    {
        return false;
    }
    if (guid.empty())
    {
        return true;
    }
    if (record->isUnion())
    {
        error = "union declarations cannot use [[luna::struct]] in this phase";
        return false;
    }
    if (record->getName().empty())
    {
        error = "anonymous record declarations cannot use [[luna::struct]]";
        return false;
    }
    if (record->getDescribedClassTemplate() || llvm::isa<clang::ClassTemplateSpecializationDecl>(record))
    {
        error = "template record declarations cannot use [[luna::struct]] in this phase";
        return false;
    }

    ReflectedDecl declaration;
    declaration.kind = ReflectedDeclKind::record;
    declaration.declaration_keyword = record->isClass() ? "class" : "struct";
    declaration.name = record->getNameAsString();
    declaration.qualified_name = record->getQualifiedNameAsString();
    declaration.reflection_name = ReflectionNameFromQualifiedName(declaration.qualified_name);
    declaration.guid = guid;
    declaration.declaration_attributes = RecordDeclarationAttributes(source_text, declaration.declaration_keyword);
    if (!CollectRecordProperties(record, source_text, ast_context, declaration.properties, error))
    {
        return false;
    }
    if (!BuildNamespaceList(record, declaration.namespaces, error))
    {
        return false;
    }
    if (!reflected_names.insert(declaration.qualified_name).second)
    {
        error = "duplicate reflected declaration: " + declaration.qualified_name;
        return false;
    }
    reflected_decls.push_back(std::move(declaration));
    return true;
}

bool AddReflectedEnum(
    const clang::EnumDecl* enum_decl,
    const std::vector<ParsedAttribute>& attributes,
    const std::string& source_text,
    const clang::ASTContext& ast_context,
    std::vector<ReflectedDecl>& reflected_decls,
    std::set<std::string>& reflected_names,
    std::string& error)
{
    std::string guid;
    if (!ValidateSingleAttribute(attributes, "enum", guid, error))
    {
        return false;
    }
    if (guid.empty())
    {
        return true;
    }
    if (enum_decl->getName().empty())
    {
        error = "anonymous enum declarations cannot use [[luna::enum]]";
        return false;
    }
    if (!enum_decl->isFixed())
    {
        error = "enum declarations using [[luna::enum]] must have a fixed underlying type";
        return false;
    }

    ReflectedDecl declaration;
    declaration.kind = ReflectedDeclKind::enumeration;
    declaration.name = enum_decl->getNameAsString();
    declaration.qualified_name = enum_decl->getQualifiedNameAsString();
    declaration.reflection_name = ReflectionNameFromQualifiedName(declaration.qualified_name);
    declaration.guid = guid;
    declaration.enum_underlying_type = EnumUnderlyingType(enum_decl, ast_context, source_text);
    declaration.enum_scoped = enum_decl->isScoped();
    if (!CollectEnumOptions(enum_decl, source_text, declaration.options, error))
    {
        return false;
    }
    if (!BuildNamespaceList(enum_decl, declaration.namespaces, error))
    {
        return false;
    }
    if (!reflected_names.insert(declaration.qualified_name).second)
    {
        error = "duplicate reflected declaration: " + declaration.qualified_name;
        return false;
    }
    reflected_decls.push_back(std::move(declaration));
    return true;
}

bool VisitDecls(
    const clang::DeclContext* context,
    const clang::ASTContext& ast_context,
    const std::filesystem::path& target_file,
    std::vector<ReflectedDecl>& reflected_decls,
    std::set<std::string>& reflected_names)
{
    const auto& sm = ast_context.getSourceManager();
    for (const auto* decl : context->decls())
    {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl))
        {
            if (record->isCompleteDefinition() && IsInTargetFile(sm, record->getLocation(), target_file))
            {
                auto decl_text = DeclSourceText(record, sm, ast_context.getLangOpts());
                std::vector<ParsedAttribute> attributes;
                std::string error;
                if (!ExtractLunaAttributes(decl_text, attributes, error) ||
                    !AddReflectedRecord(record, attributes, decl_text, ast_context, sm, reflected_decls, reflected_names, error))
                {
                    std::cerr << "error: " << DiagnosticLocation(sm, record->getLocation()) << ": " << error << "\n";
                    return false;
                }
            }
        }
        else if (const auto* enum_decl = llvm::dyn_cast<clang::EnumDecl>(decl))
        {
            if (enum_decl->isCompleteDefinition() && IsInTargetFile(sm, enum_decl->getLocation(), target_file))
            {
                auto decl_text = DeclSourceText(enum_decl, sm, ast_context.getLangOpts());
                std::vector<ParsedAttribute> attributes;
                std::string error;
                if (!ExtractLunaAttributes(decl_text, attributes, error) ||
                    !AddReflectedEnum(enum_decl, attributes, decl_text, ast_context, reflected_decls, reflected_names, error))
                {
                    std::cerr << "error: " << DiagnosticLocation(sm, enum_decl->getLocation()) << ": " << error << "\n";
                    return false;
                }
            }
        }

        if (const auto* child_context = llvm::dyn_cast<clang::DeclContext>(decl))
        {
            if (!VisitDecls(child_context, ast_context, target_file, reflected_decls, reflected_names))
            {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::string> BuildClangArgs(
    const Options& options,
    const std::filesystem::path& header,
    const std::string& language)
{
    std::vector<std::string> args = {
        "-fsyntax-only",
        "-Wno-unknown-attributes",
        "-Wno-ignored-attributes"
    };
    if (language == "objective-c++20")
    {
        args.push_back("-x");
        args.push_back("objective-c++");
        args.push_back("-std=c++20");
        args.push_back("-fobjc-arc");
    }
    else
    {
        args.push_back("-x");
        args.push_back("c++");
        args.push_back("-std=c++20");
    }
    if (!options.sysroot.empty())
    {
        args.push_back("-isysroot");
        args.push_back(options.sysroot);
    }
    if (!options.resource_dir.empty())
    {
        args.push_back("-resource-dir");
        args.push_back(options.resource_dir);
    }
    for (const auto& include : options.include_roots)
    {
        args.push_back("-I" + include);
    }
    args.push_back("-I" + header.parent_path().string());
    for (const auto& define : options.defines)
    {
        args.push_back("-D" + define);
    }
    for (const auto& undefine : options.undefines)
    {
        args.push_back("-U" + undefine);
    }
    return args;
}

bool ParseHeader(
    const Options& options,
    const std::filesystem::path& header,
    const std::string& language,
    unsigned& reflected_decl_count)
{
    std::string source_text;
    if (!ReadTextFile(header, source_text))
    {
        return false;
    }
    std::string include_error;
    if (!ValidateGeneratedIncludeOrder(header, source_text, include_error))
    {
        std::cerr << "error: " << include_error << "\n";
        return false;
    }

    auto wrapper_source = "#include \"" + EscapeCppString(std::filesystem::absolute(header).string()) + "\"\n";
    auto wrapper_file_name = header.stem().string() + ".luna_meta.cpp";
    auto ast_unit = clang::tooling::buildASTFromCodeWithArgs(
        wrapper_source,
        BuildClangArgs(options, header, language),
        wrapper_file_name,
        "LunaMetaTool",
        std::make_shared<clang::PCHContainerOperations>(),
        clang::tooling::getClangStripDependencyFileAdjuster(),
        clang::tooling::FileContentMappings(),
        nullptr);

    if (!ast_unit)
    {
        std::cerr << "error: failed to parse meta header: " << header.string() << "\n";
        return false;
    }

    std::vector<ReflectedDecl> reflected_decls;
    std::set<std::string> reflected_names;
    if (!VisitDecls(ast_unit->getASTContext().getTranslationUnitDecl(), ast_unit->getASTContext(), header, reflected_decls, reflected_names))
    {
        return false;
    }
    reflected_decl_count += static_cast<unsigned>(reflected_decls.size());
    return WriteGeneratedHeader(GeneratedHeaderPath(options, header), reflected_decls);
}

std::string EscapeDepfilePath(std::string value)
{
    std::replace(value.begin(), value.end(), '\\', '/');
    std::string result;
    result.reserve(value.size());
    for (char ch : value)
    {
        if (ch == ' ')
        {
            result.push_back('\\');
        }
        result.push_back(ch);
    }
    return result;
}

bool WriteDepfile(const Options& options)
{
    std::ofstream stream(options.depfile, std::ios::trunc);
    if (!stream)
    {
        std::cerr << "error: cannot write depfile: " << options.depfile << "\n";
        return false;
    }
    stream << EscapeDepfilePath(options.stamp) << ":";
    for (const auto& header : options.headers)
    {
        stream << " " << EscapeDepfilePath(header);
    }
    stream << "\n";
    return true;
}

bool WriteStamp(const Options& options, unsigned reflected_decl_count)
{
    std::filesystem::create_directories(std::filesystem::path(options.stamp).parent_path());
    std::ofstream stream(options.stamp, std::ios::trunc);
    if (!stream)
    {
        std::cerr << "error: cannot write stamp: " << options.stamp << "\n";
        return false;
    }
    stream << "reflected_declarations=" << reflected_decl_count << "\n";
    return true;
}
}

int main(int argc, char** argv)
{
    auto options = ParseOptions(argc, argv);
    if (options.headers.empty())
    {
        std::cerr << "error: missing --header\n";
        return 2;
    }
    if (options.output_dir.empty() || options.stamp.empty() || options.depfile.empty())
    {
        std::cerr << "error: missing --output-dir, --stamp, or --depfile\n";
        return 2;
    }
    if (!options.header_languages.empty() && options.header_languages.size() != options.headers.size())
    {
        std::cerr << "error: --header-language count must match --header count\n";
        return 2;
    }
    for (const auto& language : options.header_languages)
    {
        if (language != "c++20" && language != "objective-c++20")
        {
            std::cerr << "error: unsupported meta header language: " << language << "\n";
            return 2;
        }
    }

    if (!PrecreateGeneratedHeaders(options))
    {
        return 1;
    }

    unsigned reflected_decl_count = 0;
    bool succeeded = true;
    for (size_t i = 0; i < options.headers.size(); ++i)
    {
        const auto& header = options.headers[i];
        const std::string language = options.header_languages.empty() ? "c++20" : options.header_languages[i];
        if (!ParseHeader(options, header, language, reflected_decl_count))
        {
            succeeded = false;
        }
    }
    if (!succeeded)
    {
        return 1;
    }

    if (!WriteDepfile(options) || !WriteStamp(options, reflected_decl_count))
    {
        return 1;
    }
    return 0;
}
