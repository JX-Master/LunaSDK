#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
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
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
struct Options
{
    std::string source_path;
    std::string output_path;
    std::string namespace_name;
    std::string class_name;
    std::string library_name;
    std::string function_prefix;
    std::string record_strip_prefix;
    std::string record_prefix = "Native";
    std::vector<std::string> clang_args;
    std::vector<std::string> include_roots;
    std::unordered_map<std::string, std::string> record_remaps;
    std::unordered_set<std::string> intptr_params;
    std::unordered_map<std::string, std::string> function_param_remaps;
};

struct FieldInfo
{
    std::string managed_type;
    std::string managed_name;
};

struct RecordInfo
{
    std::string managed_name;
    std::vector<FieldInfo> fields;
};

struct ParameterInfo
{
    std::string attribute;
    std::string modifier;
    std::string managed_type;
    std::string managed_name;
};

struct FunctionInfo
{
    std::string native_name;
    std::string managed_name;
    std::string return_type;
    std::vector<ParameterInfo> parameters;
};

class DiagnosticConsumer final : public clang::DiagnosticConsumer
{
public:
    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& diagnostic) override
    {
        clang::SmallString<256> message;
        diagnostic.FormatDiagnostic(message);
        switch (level)
        {
        case clang::DiagnosticsEngine::Warning:
            std::cerr << "warning: " << message.c_str() << "\n";
            break;
        case clang::DiagnosticsEngine::Error:
        case clang::DiagnosticsEngine::Fatal:
            has_error_ = true;
            std::cerr << "error: " << message.c_str() << "\n";
            break;
        default:
            break;
        }
    }

    bool HasError() const
    {
        return has_error_;
    }

private:
    bool has_error_ = false;
};

std::string ReadTextFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("failed to open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void WriteTextFile(const std::string& path, const std::string& content)
{
    std::filesystem::path output(path);
    std::filesystem::create_directories(output.parent_path());
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        throw std::runtime_error("failed to write file: " + path);
    }
    file << content;
}

std::string NormalizePath(const std::string& path)
{
    std::error_code ec;
    auto normalized = std::filesystem::weakly_canonical(std::filesystem::path(path), ec);
    if (ec)
    {
        normalized = std::filesystem::path(path).lexically_normal();
    }
    return normalized.generic_string();
}

bool StartsWith(std::string_view value, std::string_view prefix)
{
    return value.substr(0, prefix.size()) == prefix;
}

std::string StripPrefix(std::string_view value, std::string_view prefix)
{
    if (StartsWith(value, prefix))
    {
        return std::string(value.substr(prefix.size()));
    }
    return std::string(value);
}

bool StartsWithOutName(const std::string& value)
{
    return StartsWith(value, "out") && value.size() > 3;
}

bool IsCSharpKeyword(std::string_view value)
{
    static const std::unordered_set<std::string_view> keywords = {
        "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked",
        "class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else",
        "enum", "event", "explicit", "extern", "false", "finally", "fixed", "float", "for",
        "foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
        "long", "namespace", "new", "null", "object", "operator", "out", "override", "params",
        "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short",
        "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true",
        "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual",
        "void", "volatile", "while"
    };
    return keywords.contains(value);
}

std::string EscapeIdentifier(std::string value)
{
    if (IsCSharpKeyword(value))
    {
        return "@" + value;
    }
    return value;
}

std::string ToPascalCase(std::string_view value)
{
    std::string result;
    bool uppercase = true;
    for (char ch : value)
    {
        if (ch == '_' || ch == '-' || ch == ' ')
        {
            uppercase = true;
            continue;
        }
        if (uppercase)
        {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            uppercase = false;
        }
        else
        {
            result.push_back(ch);
        }
    }
    return result;
}

std::string ToCamelCase(std::string_view value)
{
    auto result = ToPascalCase(value);
    if (!result.empty())
    {
        result[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[0])));
    }
    return result;
}

std::string EscapeString(std::string_view value)
{
    std::string result;
    result.reserve(value.size() + 8);
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

std::string GetDeclFilePath(const clang::SourceManager& source_manager, clang::SourceLocation location)
{
    auto spelling_location = source_manager.getSpellingLoc(location);
    if (spelling_location.isInvalid())
    {
        return {};
    }
    auto file_id = source_manager.getFileID(spelling_location);
    auto file_entry = source_manager.getFileEntryRefForID(file_id);
    if (!file_entry)
    {
        return {};
    }
    return NormalizePath(std::string(file_entry->getName()));
}

bool IsFromMainFile(const clang::Decl* decl, const std::string& source_path)
{
    return GetDeclFilePath(decl->getASTContext().getSourceManager(), decl->getLocation()) == source_path;
}

std::string MapRecordName(std::string_view native_name, const Options& options)
{
    auto remap = options.record_remaps.find(std::string(native_name));
    if (remap != options.record_remaps.end())
    {
        return remap->second;
    }
    auto stripped = StripPrefix(native_name, options.record_strip_prefix);
    return options.record_prefix + stripped;
}

std::optional<std::string> TryMapAliasType(const std::string& name)
{
    if (name == "luna_errcode_t" || name == "luna_errcat_t" || name == "uintptr_t")
    {
        return "UIntPtr";
    }
    if (name == "luna_handle_t" || name == "luna_type_t")
    {
        return "IntPtr";
    }
    if (name == "int8_t")
    {
        return "sbyte";
    }
    if (name == "uint8_t")
    {
        return "byte";
    }
    if (name == "int16_t")
    {
        return "short";
    }
    if (name == "uint16_t")
    {
        return "ushort";
    }
    if (name == "int32_t")
    {
        return "int";
    }
    if (name == "uint32_t")
    {
        return "uint";
    }
    if (name == "int64_t")
    {
        return "long";
    }
    if (name == "uint64_t")
    {
        return "ulong";
    }
    if (name == "size_t")
    {
        return "nuint";
    }
    if (name == "ptrdiff_t")
    {
        return "nint";
    }
    return std::nullopt;
}

std::string MapBuiltinType(clang::QualType type, const clang::ASTContext& ast_context)
{
    if (type->isVoidType())
    {
        return "void";
    }
    if (type->isBooleanType())
    {
        return "int";
    }
    if (type->isFloatingType())
    {
        auto size = ast_context.getTypeSize(type);
        if (size == 32)
        {
            return "float";
        }
        if (size == 64)
        {
            return "double";
        }
    }
    if (type->isIntegralOrEnumerationType())
    {
        auto size = ast_context.getTypeSize(type);
        auto is_signed = type->isSignedIntegerOrEnumerationType();
        if (size == 8) return is_signed ? "sbyte" : "byte";
        if (size == 16) return is_signed ? "short" : "ushort";
        if (size == 32) return is_signed ? "int" : "uint";
        if (size == 64) return is_signed ? "long" : "ulong";
    }
    throw std::runtime_error("unsupported builtin type: " + type.getAsString());
}

std::string MapValueType(clang::QualType type, const clang::ASTContext& ast_context, const Options& options)
{
    if (type->isPointerType())
    {
        return "IntPtr";
    }
    if (const auto* typedef_type = llvm::dyn_cast<clang::TypedefType>(type.getTypePtrOrNull()))
    {
        auto name = typedef_type->getDecl()->getNameAsString();
        if (auto mapped = TryMapAliasType(name))
        {
            return *mapped;
        }
    }
    if (const auto* elaborated_type = llvm::dyn_cast<clang::ElaboratedType>(type.getTypePtrOrNull()))
    {
        return MapValueType(elaborated_type->getNamedType(), ast_context, options);
    }
    if (const auto* record_type = type->getAs<clang::RecordType>())
    {
        return MapRecordName(record_type->getDecl()->getNameAsString(), options);
    }
    if (const auto* enum_type = type->getAs<clang::EnumType>())
    {
        auto enum_decl = enum_type->getDecl();
        if (enum_decl->getIdentifier())
        {
            return ToPascalCase(enum_decl->getNameAsString());
        }
    }
    return MapBuiltinType(type.getCanonicalType(), ast_context);
}

ParameterInfo MapParameter(
    const clang::FunctionDecl& function,
    const clang::ParmVarDecl& parameter,
    const clang::ASTContext& ast_context,
    const Options& options)
{
    ParameterInfo result;
    result.managed_name = EscapeIdentifier(ToCamelCase(parameter.getNameAsString()));
    auto type = parameter.getType();
    const std::string function_param_key = function.getNameAsString() + ":" + parameter.getNameAsString();
    if (auto remap = options.function_param_remaps.find(function_param_key); remap != options.function_param_remaps.end())
    {
        result.managed_type = remap->second;
        return result;
    }
    if (options.intptr_params.contains(parameter.getNameAsString()))
    {
        result.managed_type = "IntPtr";
        return result;
    }
    if (const auto* pointer_type = type->getAs<clang::PointerType>())
    {
        auto pointee = pointer_type->getPointeeType();
        const bool pointee_is_const = pointee.isConstQualified();
        const bool is_out_name = StartsWithOutName(parameter.getNameAsString());
        if (pointee->isVoidType())
        {
            result.managed_type = "IntPtr";
            return result;
        }
        if (pointee->isAnyCharacterType())
        {
            if (pointee_is_const)
            {
                result.attribute = "[MarshalAs(UnmanagedType.LPUTF8Str)] ";
                result.managed_type = "string";
            }
            else
            {
                result.managed_type = "IntPtr";
            }
            return result;
        }
        if (pointee->isPointerType())
        {
            result.managed_type = "IntPtr";
            if (is_out_name)
            {
                result.modifier = "out";
            }
            return result;
        }
        if (const auto* elaborated_type = llvm::dyn_cast<clang::ElaboratedType>(pointee.getTypePtrOrNull()))
        {
            pointee = elaborated_type->getNamedType();
        }
        if (pointee->getAs<clang::RecordType>())
        {
            result.managed_type = MapValueType(pointee, ast_context, options);
            if (pointee_is_const)
            {
                result.modifier = "in";
            }
            else if (is_out_name)
            {
                result.modifier = "out";
            }
            else
            {
                result.modifier = "ref";
            }
            return result;
        }
        if (const auto* typedef_type = llvm::dyn_cast<clang::TypedefType>(pointee.getTypePtrOrNull()))
        {
            auto alias_name = typedef_type->getDecl()->getNameAsString();
            if (alias_name == "int8_t" || alias_name == "uint8_t")
            {
                result.managed_type = "IntPtr";
                return result;
            }
        }
        if (pointee->isBuiltinType() || pointee->isEnumeralType() || llvm::isa<clang::TypedefType>(pointee.getTypePtrOrNull()))
        {
            if (pointee_is_const)
            {
                result.managed_type = "IntPtr";
            }
            else
            {
                result.managed_type = MapValueType(pointee, ast_context, options);
                result.modifier = is_out_name ? "out" : "ref";
            }
            return result;
        }
        result.managed_type = "IntPtr";
        return result;
    }

    result.managed_type = MapValueType(type, ast_context, options);
    return result;
}

RecordInfo MapRecord(const clang::RecordDecl& record, const clang::ASTContext& ast_context, const Options& options)
{
    RecordInfo result;
    result.managed_name = MapRecordName(record.getNameAsString(), options);
    for (const auto* field : record.fields())
    {
        FieldInfo info;
        info.managed_type = MapValueType(field->getType(), ast_context, options);
        info.managed_name = EscapeIdentifier(ToPascalCase(field->getNameAsString()));
        result.fields.push_back(std::move(info));
    }
    return result;
}

FunctionInfo MapFunction(const clang::FunctionDecl& function, const clang::ASTContext& ast_context, const Options& options)
{
    FunctionInfo result;
    result.native_name = function.getNameAsString();
    result.managed_name = ToPascalCase(StripPrefix(result.native_name, options.function_prefix));
    result.return_type = MapValueType(function.getReturnType(), ast_context, options);
    for (const auto* parameter : function.parameters())
    {
        result.parameters.push_back(MapParameter(function, *parameter, ast_context, options));
    }
    return result;
}

void AppendOptionValue(int& index, int argc, char** argv, std::string& destination)
{
    ++index;
    if (index >= argc)
    {
        throw std::runtime_error("missing value for option");
    }
    destination = argv[index];
}

Options ParseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--source")
        {
            AppendOptionValue(i, argc, argv, options.source_path);
        }
        else if (arg == "--output")
        {
            AppendOptionValue(i, argc, argv, options.output_path);
        }
        else if (arg == "--namespace")
        {
            AppendOptionValue(i, argc, argv, options.namespace_name);
        }
        else if (arg == "--class")
        {
            AppendOptionValue(i, argc, argv, options.class_name);
        }
        else if (arg == "--library")
        {
            AppendOptionValue(i, argc, argv, options.library_name);
        }
        else if (arg == "--function-prefix")
        {
            AppendOptionValue(i, argc, argv, options.function_prefix);
        }
        else if (arg == "--record-strip-prefix")
        {
            AppendOptionValue(i, argc, argv, options.record_strip_prefix);
        }
        else if (arg == "--record-prefix")
        {
            AppendOptionValue(i, argc, argv, options.record_prefix);
        }
        else if (arg == "--record-remap")
        {
            ++i;
            if (i >= argc)
            {
                throw std::runtime_error("missing value for --record-remap");
            }
            std::string value = argv[i];
            auto separator = value.find('=');
            if (separator == std::string::npos || separator == 0 || separator == value.size() - 1)
            {
                throw std::runtime_error("invalid --record-remap, expected NativeName=ManagedName");
            }
            options.record_remaps.emplace(value.substr(0, separator), value.substr(separator + 1));
        }
        else if (arg == "--intptr-param")
        {
            ++i;
            if (i >= argc)
            {
                throw std::runtime_error("missing value for --intptr-param");
            }
            options.intptr_params.emplace(argv[i]);
        }
        else if (arg == "--function-param-remap")
        {
            ++i;
            if (i >= argc)
            {
                throw std::runtime_error("missing value for --function-param-remap");
            }
            std::string value = argv[i];
            auto separator = value.find('=');
            if (separator == std::string::npos || separator == 0 || separator == value.size() - 1)
            {
                throw std::runtime_error("invalid --function-param-remap, expected function:param=ManagedType");
            }
            options.function_param_remaps.emplace(value.substr(0, separator), value.substr(separator + 1));
        }
        else if (arg == "--include")
        {
            ++i;
            if (i >= argc)
            {
                throw std::runtime_error("missing value for --include");
            }
            options.include_roots.push_back(argv[i]);
        }
        else if (arg == "--clang-arg")
        {
            ++i;
            if (i >= argc)
            {
                throw std::runtime_error("missing value for --clang-arg");
            }
            options.clang_args.push_back(argv[i]);
        }
        else
        {
            throw std::runtime_error("unknown option: " + arg);
        }
    }

    if (options.source_path.empty() ||
        options.output_path.empty() ||
        options.namespace_name.empty() ||
        options.class_name.empty() ||
        options.library_name.empty())
    {
        throw std::runtime_error("required options: --source --output --namespace --class --library");
    }
    return options;
}

std::string EmitBindingFile(
    const Options& options,
    const std::vector<RecordInfo>& records,
    const std::vector<FunctionInfo>& functions)
{
    std::ostringstream output;
    output << "// Auto-generated by csharp-binding-generator. Do not edit by hand.\n";
    output << "using System;\n";
    output << "using System.Runtime.InteropServices;\n\n";
    output << "namespace " << options.namespace_name << ";\n\n";
    for (const auto& record : records)
    {
        output << "[StructLayout(LayoutKind.Sequential)]\n";
        output << "internal partial struct " << record.managed_name << "\n";
        output << "{\n";
        for (const auto& field : record.fields)
        {
            output << "    public " << field.managed_type << " " << field.managed_name << ";\n";
        }
        output << "}\n\n";
    }

    output << "internal static partial class " << options.class_name << "\n";
    output << "{\n";
    for (const auto& function : functions)
    {
        output << "    [DllImport(\"" << EscapeString(options.library_name) << "\", EntryPoint = \"" << EscapeString(function.native_name) << "\", CallingConvention = CallingConvention.Cdecl)]\n";
        output << "    internal static extern " << function.return_type << " " << function.managed_name << "(";
        for (size_t i = 0; i < function.parameters.size(); ++i)
        {
            const auto& parameter = function.parameters[i];
            if (i != 0)
            {
                output << ", ";
            }
            output << parameter.attribute;
            if (!parameter.modifier.empty())
            {
                output << parameter.modifier << " ";
            }
            output << parameter.managed_type << " " << parameter.managed_name;
        }
        output << ");\n\n";
    }
    output << "}\n";
    return output.str();
}
}

int main(int argc, char** argv)
{
    try
    {
        auto options = ParseOptions(argc, argv);
        options.source_path = NormalizePath(options.source_path);
        options.output_path = NormalizePath(options.output_path);
        for (auto& include_root : options.include_roots)
        {
            include_root = NormalizePath(include_root);
        }

        auto source_text = ReadTextFile(options.source_path);

        std::vector<std::string> args = {
            "-x",
            "c-header",
            "-std=c17",
            "-fsyntax-only",
            "-Wno-unknown-attributes",
            "-Wno-ignored-attributes"
        };
        for (const auto& include_root : options.include_roots)
        {
            args.push_back("-I" + include_root);
        }
        for (const auto& clang_arg : options.clang_args)
        {
            args.push_back(clang_arg);
        }

        DiagnosticConsumer diagnostic_consumer;
        auto ast_unit = clang::tooling::buildASTFromCodeWithArgs(
            source_text,
            args,
            options.source_path,
            "csharp-binding-generator",
            std::make_shared<clang::PCHContainerOperations>(),
            clang::tooling::getClangStripDependencyFileAdjuster(),
            clang::tooling::FileContentMappings(),
            &diagnostic_consumer);
        if (!ast_unit || diagnostic_consumer.HasError())
        {
            return 1;
        }

        std::vector<RecordInfo> records;
        std::vector<FunctionInfo> functions;
        std::unordered_set<std::string> seen_records;
        std::unordered_set<std::string> seen_functions;

        const auto& ast_context = ast_unit->getASTContext();
        for (const auto* decl : ast_context.getTranslationUnitDecl()->decls())
        {
            if (!IsFromMainFile(decl, options.source_path))
            {
                continue;
            }
            if (const auto* record = llvm::dyn_cast<clang::RecordDecl>(decl))
            {
                if (!record->isStruct() || !record->isCompleteDefinition() || record->getName().empty())
                {
                    continue;
                }
                if (options.record_remaps.contains(record->getNameAsString()))
                {
                    continue;
                }
                auto managed_name = MapRecordName(record->getNameAsString(), options);
                if (seen_records.insert(managed_name).second)
                {
                    records.push_back(MapRecord(*record, ast_context, options));
                }
                continue;
            }
            if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(decl))
            {
                if (function->isImplicit() || function->getName().empty())
                {
                    continue;
                }
                auto managed_name = ToPascalCase(StripPrefix(function->getNameAsString(), options.function_prefix));
                if (seen_functions.insert(managed_name).second)
                {
                    functions.push_back(MapFunction(*function, ast_context, options));
                }
            }
        }

        auto output = EmitBindingFile(options, records, functions);
        WriteTextFile(options.output_path, output);
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "csharp-binding-generator: " << error.what() << "\n";
        return 1;
    }
}
