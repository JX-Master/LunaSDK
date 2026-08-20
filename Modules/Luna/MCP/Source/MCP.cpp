/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MCP.cpp
* @author JXMaster
* @date 2026/8/14
* @brief MCP server message processing and Frontend tool bridge.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_MCP_API LUNA_EXPORT
#include "MCPImpl.hpp"
#include "MCP.meta.generated.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>

namespace Luna
{
    namespace MCP
    {
        namespace
        {
            constexpr i64 PARSE_ERROR = -32700;
            constexpr i64 INVALID_REQUEST = -32600;
            constexpr i64 METHOD_NOT_FOUND = -32601;
            constexpr i64 INVALID_PARAMS = -32602;
            constexpr i64 INTERNAL_ERROR = -32603;
            constexpr i64 UNSUPPORTED_PROTOCOL_VERSION = -32022;

            bool is_request_id(const Variant& value)
            {
                return value.type() == VariantType::string || value.type() == VariantType::number;
            }

            bool is_optional_type(const Variant& object, const Name& key, VariantType type)
            {
                const Variant& value = object.find(key);
                return !value.valid() || value.type() == type;
            }

            bool is_ascii_alpha(c8 ch)
            {
                return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
            }

            bool is_ascii_digit(c8 ch)
            {
                return ch >= '0' && ch <= '9';
            }

            bool is_ascii_alphanumeric(c8 ch)
            {
                return is_ascii_alpha(ch) || is_ascii_digit(ch);
            }

            bool validate_meta_name_segment(const c8* chars, usize size)
            {
                if(!size) return true;
                if(!is_ascii_alphanumeric(chars[0]) ||
                    !is_ascii_alphanumeric(chars[size - 1])) return false;
                for(usize i = 1; i + 1 < size; ++i)
                {
                    c8 ch = chars[i];
                    if(!is_ascii_alphanumeric(ch) && ch != '-' && ch != '_' && ch != '.')
                    {
                        return false;
                    }
                }
                return true;
            }

            bool validate_meta_key(const Name& key, bool require_prefix = false)
            {
                const c8* chars = key.c_str();
                usize slash = key.size();
                for(usize i = 0; i < key.size(); ++i)
                {
                    if(chars[i] != '/') continue;
                    if(slash != key.size()) return false;
                    slash = i;
                }
                if(slash == key.size())
                {
                    return !require_prefix && validate_meta_name_segment(chars, key.size());
                }
                if(!slash || chars[slash - 1] == '.') return false;

                usize label_start = 0;
                while(label_start < slash)
                {
                    usize label_end = label_start;
                    while(label_end < slash && chars[label_end] != '.') ++label_end;
                    usize label_size = label_end - label_start;
                    if(!label_size || !is_ascii_alpha(chars[label_start]) ||
                        !is_ascii_alphanumeric(chars[label_end - 1])) return false;
                    for(usize i = label_start + 1; i + 1 < label_end; ++i)
                    {
                        if(!is_ascii_alphanumeric(chars[i]) && chars[i] != '-') return false;
                    }
                    label_start = label_end + 1;
                }
                return validate_meta_name_segment(
                    chars + slash + 1, key.size() - slash - 1);
            }

            bool validate_meta_object(const Variant& meta)
            {
                if(meta.type() != VariantType::object) return false;
                for(const auto& field : meta.key_values())
                {
                    if(!validate_meta_key(field.first)) return false;
                }
                return true;
            }

            bool validate_object_map(const Variant& map, bool extension_keys)
            {
                if(!map.valid()) return true;
                if(map.type() != VariantType::object) return false;
                for(const auto& field : map.key_values())
                {
                    if(extension_keys && !validate_meta_key(field.first, true)) return false;
                    if(field.second.type() != VariantType::object) return false;
                }
                return true;
            }

            bool validate_client_capabilities(const Variant& capabilities)
            {
                if(capabilities.type() != VariantType::object) return false;
                if(!validate_object_map(capabilities.find("experimental"), false)) return false;
                if(!is_optional_type(capabilities, "roots", VariantType::object)) return false;
                if(!is_optional_type(capabilities, "sampling", VariantType::object)) return false;
                if(!is_optional_type(capabilities, "elicitation", VariantType::object)) return false;
                if(!validate_object_map(capabilities.find("extensions"), true)) return false;
                const Variant& sampling = capabilities.find("sampling");
                if(sampling.valid())
                {
                    if(!is_optional_type(sampling, "context", VariantType::object)) return false;
                    if(!is_optional_type(sampling, "tools", VariantType::object)) return false;
                }
                const Variant& elicitation = capabilities.find("elicitation");
                if(elicitation.valid())
                {
                    if(!is_optional_type(elicitation, "form", VariantType::object)) return false;
                    if(!is_optional_type(elicitation, "url", VariantType::object)) return false;
                }
                return true;
            }

            bool validate_icons(const Variant& icons)
            {
                if(!icons.valid()) return true;
                if(icons.type() != VariantType::array) return false;
                for(const Variant& icon : icons.values())
                {
                    if(icon.type() != VariantType::object) return false;
                    if(icon.find("src").type() != VariantType::string) return false;
                    if(!is_optional_type(icon, "mimeType", VariantType::string)) return false;
                    const Variant& sizes = icon.find("sizes");
                    if(sizes.valid())
                    {
                        if(sizes.type() != VariantType::array) return false;
                        for(const Variant& size : sizes.values())
                        {
                            if(size.type() != VariantType::string) return false;
                        }
                    }
                    const Variant& theme = icon.find("theme");
                    if(theme.valid())
                    {
                        if(theme.type() != VariantType::string) return false;
                        if(theme.str() != Name("light") && theme.str() != Name("dark")) return false;
                    }
                }
                return true;
            }

            bool validate_annotations(const Variant& annotations)
            {
                if(!annotations.valid()) return true;
                if(annotations.type() != VariantType::object) return false;
                if(!is_optional_type(annotations, "title", VariantType::string)) return false;
                if(!is_optional_type(annotations, "readOnlyHint", VariantType::boolean)) return false;
                if(!is_optional_type(annotations, "destructiveHint", VariantType::boolean)) return false;
                if(!is_optional_type(annotations, "idempotentHint", VariantType::boolean)) return false;
                if(!is_optional_type(annotations, "openWorldHint", VariantType::boolean)) return false;
                return true;
            }

            bool validate_tool_name(const Name& name)
            {
                if(name.size() < 1 || name.size() > 128) return false;
                for(usize i = 0; i < name.size(); ++i)
                {
                    c8 ch = name.c_str()[i];
                    bool valid = (ch >= 'A' && ch <= 'Z') ||
                        (ch >= 'a' && ch <= 'z') ||
                        (ch >= '0' && ch <= '9') ||
                        ch == '_' || ch == '-' || ch == '.';
                    if(!valid) return false;
                }
                return true;
            }

            bool is_http_token_char(c8 ch)
            {
                return is_ascii_alphanumeric(ch) || ch == '!' || ch == '#' || ch == '$' ||
                    ch == '%' || ch == '&' || ch == '\'' || ch == '*' || ch == '+' ||
                    ch == '-' || ch == '.' || ch == '^' || ch == '_' || ch == '`' ||
                    ch == '|' || ch == '~';
            }

            bool contains_mcp_header_annotation(const Variant& value)
            {
                if(value.type() == VariantType::object)
                {
                    for(const auto& field : value.key_values())
                    {
                        if(field.first == Name("x-mcp-header") ||
                            contains_mcp_header_annotation(field.second))
                        {
                            return true;
                        }
                    }
                }
                else if(value.type() == VariantType::array)
                {
                    for(const Variant& element : value.values())
                    {
                        if(contains_mcp_header_annotation(element)) return true;
                    }
                }
                return false;
            }

            bool collect_tool_header_bindings(
                const Variant& schema,
                bool is_property,
                Vector<Name>& property_path,
                Vector<Name>& used_header_names,
                Vector<ToolHeaderBinding>& bindings)
            {
                if(schema.type() != VariantType::object)
                {
                    return !contains_mcp_header_annotation(schema);
                }

                const Variant& annotation = schema.find("x-mcp-header");
                if(annotation.valid())
                {
                    if(!is_property || annotation.type() != VariantType::string ||
                        !annotation.str()) return false;
                    const Variant& type = schema.find("type");
                    if(type.type() != VariantType::string ||
                        (type.str() != Name("string") && type.str() != Name("integer") &&
                        type.str() != Name("boolean"))) return false;

                    String header_name("mcp-param-");
                    const Name suffix = annotation.str();
                    for(usize i = 0; i < suffix.size(); ++i)
                    {
                        c8 ch = suffix.c_str()[i];
                        if(!is_http_token_char(ch)) return false;
                        if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
                        header_name.push_back(ch);
                    }
                    Name normalized_name(header_name);
                    for(const Name& used : used_header_names)
                    {
                        if(used == normalized_name) return false;
                    }
                    used_header_names.push_back(normalized_name);

                    ToolHeaderBinding binding;
                    binding.property_path = property_path;
                    binding.header_name = move(normalized_name);
                    binding.value_type = type.str();
                    bindings.push_back(move(binding));
                }

                for(const auto& field : schema.key_values())
                {
                    if(field.first == Name("x-mcp-header") ||
                        field.first == Name("properties")) continue;
                    if(contains_mcp_header_annotation(field.second)) return false;
                }

                const Variant& properties = schema.find("properties");
                if(!properties.valid()) return true;
                if(properties.type() != VariantType::object)
                {
                    return !contains_mcp_header_annotation(properties);
                }
                for(const auto& property : properties.key_values())
                {
                    property_path.push_back(property.first);
                    bool valid = collect_tool_header_bindings(
                        property.second,
                        true,
                        property_path,
                        used_header_names,
                        bindings);
                    property_path.pop_back();
                    if(!valid) return false;
                }
                return true;
            }

            Variant make_error_response(
                const Variant* id,
                i64 code,
                const c8* message,
                const Variant* data = nullptr)
            {
                Variant response(VariantType::object);
                response["jsonrpc"] = "2.0";
                if(id) response["id"] = *id;
                Variant error(VariantType::object);
                error["code"] = code;
                error["message"] = message;
                if(data) error["data"] = *data;
                response["error"] = move(error);
                return response;
            }

            MessageResult error_result(
                const Variant* id,
                i64 code,
                const c8* message,
                const Variant* data = nullptr)
            {
                MessageResult result;
                result.has_response = true;
                result.response = make_error_response(id, code, message, data);
                return result;
            }

            MessageResult response_result(const Variant& id, Variant&& body)
            {
                MessageResult result;
                result.has_response = true;
                result.response = Variant(VariantType::object);
                result.response["jsonrpc"] = "2.0";
                result.response["id"] = id;
                result.response["result"] = move(body);
                return result;
            }

            void add_result_fields(Variant& result, const Variant& server_info)
            {
                result["resultType"] = "complete";
                result["_meta"]["io.modelcontextprotocol/serverInfo"] = server_info;
            }

            void add_cache_fields(Variant& result, u64 ttl_ms, CacheScope scope)
            {
                result["ttlMs"] = ttl_ms;
                result["cacheScope"] = scope == CacheScope::public_cache ? "public" : "private";
            }

            bool validate_request_meta(const Variant& params, Name& requested_version)
            {
                const Variant& meta = params.find("_meta");
                if(!validate_meta_object(meta)) return false;
                const Variant& version = meta.find("io.modelcontextprotocol/protocolVersion");
                if(version.type() != VariantType::string) return false;
                if(!validate_client_capabilities(
                    meta.find("io.modelcontextprotocol/clientCapabilities")))
                {
                    return false;
                }
                const Variant& progress_token = meta.find("progressToken");
                if(progress_token.valid() && !is_request_id(progress_token)) return false;
                const Variant& client_info = meta.find("io.modelcontextprotocol/clientInfo");
                if(client_info.valid())
                {
                    if(client_info.type() != VariantType::object) return false;
                    if(client_info.find("name").type() != VariantType::string) return false;
                    if(client_info.find("version").type() != VariantType::string) return false;
                    if(!is_optional_type(client_info, "title", VariantType::string)) return false;
                    if(!is_optional_type(client_info, "description", VariantType::string)) return false;
                    if(!is_optional_type(client_info, "websiteUrl", VariantType::string)) return false;
                    if(!validate_icons(client_info.find("icons"))) return false;
                }
                const Variant& log_level = meta.find("io.modelcontextprotocol/logLevel");
                if(log_level.valid())
                {
                    if(log_level.type() != VariantType::string) return false;
                    Name level = log_level.str();
                    if(level != Name("debug") && level != Name("info") &&
                        level != Name("notice") && level != Name("warning") &&
                        level != Name("error") && level != Name("critical") &&
                        level != Name("alert") && level != Name("emergency")) return false;
                }
                requested_version = version.str();
                return true;
            }

            bool validate_legacy_client_capabilities(const Variant& capabilities)
            {
                if(capabilities.type() != VariantType::object) return false;
                if(!validate_object_map(capabilities.find("experimental"), false)) return false;
                const Variant& roots = capabilities.find("roots");
                if(roots.valid())
                {
                    if(roots.type() != VariantType::object) return false;
                    if(!is_optional_type(roots, "listChanged", VariantType::boolean)) return false;
                }
                if(!is_optional_type(capabilities, "sampling", VariantType::object)) return false;
                if(!is_optional_type(capabilities, "elicitation", VariantType::object)) return false;
                return true;
            }

            bool validate_legacy_client_info(const Variant& client_info)
            {
                if(client_info.type() != VariantType::object) return false;
                if(client_info.find("name").type() != VariantType::string ||
                    !client_info.find("name").str()) return false;
                if(client_info.find("version").type() != VariantType::string ||
                    !client_info.find("version").str()) return false;
                return is_optional_type(client_info, "title", VariantType::string);
            }

            bool validate_legacy_request_meta(const Variant& params)
            {
                const Variant& meta = params.find("_meta");
                if(!meta.valid()) return true;
                if(meta.type() != VariantType::object) return false;
                const Variant& progress_token = meta.find("progressToken");
                return !progress_token.valid() || is_request_id(progress_token);
            }

            MessageResult process_tools_list(
                MCPServer* server,
                const Variant& id,
                const Variant& params,
                bool modern)
            {
                const Variant& cursor = params.find("cursor");
                if(cursor.valid())
                {
                    if(cursor.type() != VariantType::string)
                    {
                        return error_result(&id, INVALID_PARAMS, "Cursor must be a string");
                    }
                    return error_result(&id, INVALID_PARAMS, "Pagination cursor is not valid");
                }

                Vector<const ToolEntry*> sorted_tools;
                sorted_tools.reserve(server->m_tools.size());
                for(const auto& tool : server->m_tools)
                {
                    sorted_tools.push_back(&tool.second);
                }
                sort(sorted_tools.begin(), sorted_tools.end(),
                    [](const ToolEntry* lhs, const ToolEntry* rhs)
                    {
                        return strcmp(lhs->desc.name.c_str(), rhs->desc.name.c_str()) < 0;
                    });

                Variant result(VariantType::object);
                if(modern)
                {
                    add_result_fields(result, server->m_server_info);
                    add_cache_fields(
                        result,
                        server->m_desc.tools_ttl_ms,
                        server->m_desc.cache_scope);
                }
                result["tools"] = Variant(VariantType::array);
                for(const ToolEntry* tool : sorted_tools)
                {
                    result["tools"].push_back(
                        modern ? tool->modern_definition : tool->legacy_definition);
                }
                return response_result(id, move(result));
            }

            MessageResult process_tool_call(
                MCPServer* server,
                const Variant& id,
                const Variant& params,
                bool modern)
            {
                const Variant& tool_name = params.find("name");
                if(tool_name.type() != VariantType::string || !tool_name.str())
                {
                    return error_result(
                        &id, INVALID_PARAMS, "Tool name must be a non-empty string");
                }
                if(params.contains("inputResponses") || params.contains("requestState"))
                {
                    return error_result(
                        &id,
                        INVALID_PARAMS,
                        "Multi Round-Trip Request fields are not supported");
                }

                Variant empty_arguments(VariantType::object);
                const Variant* arguments = &empty_arguments;
                const Variant& specified_arguments = params.find("arguments");
                if(specified_arguments.valid())
                {
                    if(specified_arguments.type() != VariantType::object)
                    {
                        return error_result(
                            &id, INVALID_PARAMS, "Tool arguments must be an object");
                    }
                    arguments = &specified_arguments;
                }

                auto tool_iter = server->m_tools.find(tool_name.str());
                if(tool_iter == server->m_tools.end())
                {
                    return error_result(&id, INVALID_PARAMS, "Unknown tool");
                }
                Name frontend_url = tool_iter->second.desc.frontend_url;
                if(server->m_frontend->get_resource_type(frontend_url) !=
                    Frontend::ResourceType::function)
                {
                    return error_result(
                        &id, INTERNAL_ERROR, "Mapped Frontend function is unavailable");
                }

                R<Variant> invocation = server->m_frontend->invoke(frontend_url, *arguments);
                Variant result(VariantType::object);
                if(modern) add_result_fields(result, server->m_server_info);
                result["content"] = Variant(VariantType::array);
                if(!invocation.valid())
                {
                    Name error_message(explain(invocation.errcode()));
                    R<String> encoded_error = VariantUtils::write_json(
                        Variant(error_message), VariantUtils::JSONWriteOptions::strict());
                    if(!encoded_error.valid()) error_message = "Tool execution failed";
                    Variant content(VariantType::object);
                    content["type"] = "text";
                    content["text"] = error_message;
                    result["content"].push_back(move(content));
                    result["isError"] = true;
                    return response_result(id, move(result));
                }

                R<String> text = VariantUtils::write_json(
                    invocation.get(), VariantUtils::JSONWriteOptions::strict());
                if(!text.valid())
                {
                    return error_result(
                        &id,
                        INTERNAL_ERROR,
                        "Tool result cannot be represented as strict JSON");
                }
                Variant content(VariantType::object);
                content["type"] = "text";
                content["text"] = Name(text.get());
                result["content"].push_back(move(content));
                result["structuredContent"] = invocation.get();
                result["isError"] = false;
                return response_result(id, move(result));
            }

            MessageResult process_modern_request(
                MCPServer* server,
                const Variant& id,
                const Name& method,
                const Variant& message)
            {
                if(method != Name("server/discover") && method != Name("tools/list") &&
                    method != Name("tools/call"))
                {
                    return error_result(&id, METHOD_NOT_FOUND, "Method not found");
                }

                const Variant& params = message.find("params");
                if(params.type() != VariantType::object)
                {
                    return error_result(
                        &id, INVALID_PARAMS, "Request params must be an object");
                }
                Name requested_version;
                if(!validate_request_meta(params, requested_version))
                {
                    return error_result(
                        &id, INVALID_PARAMS, "Invalid MCP request metadata");
                }
                if(requested_version != Name(MODERN_PROTOCOL_VERSION))
                {
                    Variant data(VariantType::object);
                    data["supported"] = Variant(VariantType::array);
                    data["supported"].push_back(MODERN_PROTOCOL_VERSION);
                    data["requested"] = requested_version;
                    return error_result(
                        &id,
                        UNSUPPORTED_PROTOCOL_VERSION,
                        "Unsupported protocol version",
                        &data);
                }

                if(method == Name("server/discover"))
                {
                    Variant result(VariantType::object);
                    add_result_fields(result, server->m_server_info);
                    add_cache_fields(
                        result,
                        server->m_desc.discovery_ttl_ms,
                        server->m_desc.cache_scope);
                    result["supportedVersions"] = Variant(VariantType::array);
                    result["supportedVersions"].push_back(MODERN_PROTOCOL_VERSION);
                    result["capabilities"] = Variant(VariantType::object);
                    result["capabilities"]["tools"] = Variant(VariantType::object);
                    if(server->m_desc.instructions)
                    {
                        result["instructions"] = server->m_desc.instructions;
                    }
                    return response_result(id, move(result));
                }
                if(method == Name("tools/list"))
                {
                    return process_tools_list(server, id, params, true);
                }
                return process_tool_call(server, id, params, true);
            }

            MessageResult process_legacy_initialize(
                MCPMessageProcessor* processor,
                const Variant& id,
                const Variant& message)
            {
                MCPServer* server = processor->m_server;
                if(processor->m_legacy_lifecycle != LegacyLifecycle::uninitialized)
                {
                    return error_result(&id, INVALID_REQUEST, "Server is already initialized");
                }
                const Variant& params = message.find("params");
                if(params.type() != VariantType::object ||
                    params.find("protocolVersion").type() != VariantType::string ||
                    !params.find("protocolVersion").str() ||
                    !validate_legacy_client_capabilities(params.find("capabilities")) ||
                    !validate_legacy_client_info(params.find("clientInfo")) ||
                    !validate_legacy_request_meta(params))
                {
                    return error_result(&id, INVALID_PARAMS, "Invalid initialize parameters");
                }

                Variant result(VariantType::object);
                result["protocolVersion"] = LEGACY_PROTOCOL_VERSION;
                result["capabilities"] = Variant(VariantType::object);
                result["capabilities"]["tools"] = Variant(VariantType::object);
                result["serverInfo"] = server->m_legacy_server_info;
                if(server->m_desc.instructions)
                {
                    result["instructions"] = server->m_desc.instructions;
                }
                processor->m_legacy_lifecycle = LegacyLifecycle::wait_initialized;
                return response_result(id, move(result));
            }

            MessageResult process_legacy_request(
                MCPMessageProcessor* processor,
                const Variant& id,
                const Name& method,
                const Variant& message)
            {
                MCPServer* server = processor->m_server;
                if(method == Name("initialize"))
                {
                    return process_legacy_initialize(processor, id, message);
                }
                if(method == Name("ping"))
                {
                    const Variant& params = message.find("params");
                    if(params.valid() && params.type() != VariantType::object)
                    {
                        return error_result(&id, INVALID_PARAMS, "Ping params must be an object");
                    }
                    return response_result(id, Variant(VariantType::object));
                }
                if(processor->m_legacy_lifecycle != LegacyLifecycle::initialized)
                {
                    return error_result(&id, INVALID_REQUEST, "Server is not initialized");
                }
                if(method != Name("tools/list") && method != Name("tools/call"))
                {
                    return error_result(&id, METHOD_NOT_FOUND, "Method not found");
                }

                Variant empty_params(VariantType::object);
                const Variant* params = &empty_params;
                const Variant& specified_params = message.find("params");
                if(specified_params.valid())
                {
                    if(specified_params.type() != VariantType::object)
                    {
                        return error_result(
                            &id, INVALID_PARAMS, "Request params must be an object");
                    }
                    params = &specified_params;
                }
                if(!validate_legacy_request_meta(*params))
                {
                    return error_result(&id, INVALID_PARAMS, "Invalid legacy request metadata");
                }
                if(method == Name("tools/list"))
                {
                    return process_tools_list(server, id, *params, false);
                }
                if(!specified_params.valid())
                {
                    return error_result(
                        &id, INVALID_PARAMS, "Tool call params must be an object");
                }
                return process_tool_call(server, id, *params, false);
            }

            MessageResult process_valid_message(
                MCPMessageProcessor* processor,
                const Variant& message)
            {
                MCPServer* server = processor->m_server;
                if(message.type() != VariantType::object)
                {
                    return error_result(nullptr, INVALID_REQUEST, "Invalid Request");
                }

                const bool has_id = message.contains("id");
                const Variant& id = message.find("id");
                if(has_id && !is_request_id(id))
                {
                    return error_result(nullptr, INVALID_REQUEST, "Invalid Request");
                }
                const Variant* response_id = has_id ? &id : nullptr;
                if(message.find("jsonrpc").type() != VariantType::string ||
                    message.find("jsonrpc").str() != Name("2.0") ||
                    message.find("method").type() != VariantType::string ||
                    message.contains("result") || message.contains("error"))
                {
                    return error_result(response_id, INVALID_REQUEST, "Invalid Request");
                }

                const Name method = message.find("method").str();
                if(!has_id)
                {
                    if(method == Name("notifications/initialized") &&
                        processor->m_protocol_version == ProtocolVersion::v2025_06_18 &&
                        processor->m_legacy_lifecycle == LegacyLifecycle::wait_initialized)
                    {
                        processor->m_legacy_lifecycle = LegacyLifecycle::initialized;
                    }
                    return MessageResult();
                }

                if(processor->m_protocol_version == ProtocolVersion::v2026_07_28)
                {
                    return process_modern_request(server, id, method, message);
                }
                return process_legacy_request(processor, id, method, message);
            }
        }

        RV MCPServer::set_tool(ToolDesc&& desc, bool overwrite)
        {
            if(!validate_tool_name(desc.name))
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP tool name");
            }
            if(!desc.frontend_url ||
                m_frontend->get_resource_type(desc.frontend_url) != Frontend::ResourceType::function)
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "The mapped Frontend resource must be a function");
            }
            if(desc.input_schema.type() != VariantType::object ||
                desc.input_schema.find("type").type() != VariantType::string ||
                desc.input_schema.find("type").str() != Name("object"))
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "The MCP input schema root type must be object");
            }
            if(desc.output_schema.valid() && desc.output_schema.type() != VariantType::object)
            {
                return set_error(BasicError::bad_arguments(), "The MCP output schema must be an object");
            }
            if(!validate_annotations(desc.annotations))
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP tool annotations");
            }
            if(!validate_icons(desc.icons))
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP tool icons");
            }
            if(desc.metadata.valid() && !validate_meta_object(desc.metadata))
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP tool metadata");
            }

            Vector<ToolHeaderBinding> header_bindings;
            Vector<Name> property_path;
            Vector<Name> used_header_names;
            if(!collect_tool_header_bindings(
                desc.input_schema,
                false,
                property_path,
                used_header_names,
                header_bindings))
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "Invalid x-mcp-header annotation in MCP tool input schema");
            }

            auto existing = m_tools.find(desc.name);
            if(existing != m_tools.end() && !overwrite) return BasicError::already_exists();

            ToolEntry entry;
            entry.modern_definition = Variant(VariantType::object);
            entry.modern_definition["name"] = desc.name;
            if(desc.title) entry.modern_definition["title"] = desc.title;
            if(desc.description) entry.modern_definition["description"] = desc.description;
            entry.modern_definition["inputSchema"] = desc.input_schema;
            if(desc.output_schema.valid())
            {
                entry.modern_definition["outputSchema"] = desc.output_schema;
            }
            if(desc.annotations.valid()) entry.modern_definition["annotations"] = desc.annotations;
            if(desc.icons.valid()) entry.modern_definition["icons"] = desc.icons;
            if(desc.metadata.valid()) entry.modern_definition["_meta"] = desc.metadata;

            entry.legacy_definition = entry.modern_definition;
            entry.legacy_definition.erase("icons");
            entry.header_bindings = move(header_bindings);

            R<String> encoded = VariantUtils::write_json(
                entry.modern_definition, VariantUtils::JSONWriteOptions::strict());
            if(!encoded.valid())
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "The MCP tool descriptor is not representable as strict JSON");
            }
            entry.desc = move(desc);
            m_tools.insert_or_assign(entry.desc.name, move(entry));
            return ok;
        }

        RV MCPServer::remove_tool(const Name& name)
        {
            if(!name) return BasicError::bad_arguments();
            m_tools.erase(name);
            return ok;
        }

        usize MCPServer::get_tool_count()
        {
            return m_tools.size();
        }

        R<Ref<IMCPMessageProcessor>> MCPServer::new_message_processor(
            ProtocolVersion version)
        {
            if(version != ProtocolVersion::v2025_06_18 &&
                version != ProtocolVersion::v2026_07_28)
            {
                return BasicError::bad_arguments();
            }
            Ref<MCPMessageProcessor> processor = new_object<MCPMessageProcessor>();
            processor->m_server = Ref<MCPServer>(this);
            processor->m_protocol_version = version;
            return Ref<IMCPMessageProcessor>(processor);
        }

        ProtocolVersion MCPMessageProcessor::get_protocol_version()
        {
            return m_protocol_version;
        }

        MessageResult MCPMessageProcessor::process_message(const Variant& message)
        {
            R<String> validation = VariantUtils::write_json(
                message, VariantUtils::JSONWriteOptions::strict());
            if(!validation.valid())
            {
                return error_result(nullptr, INVALID_REQUEST, "Message is not valid strict JSON data");
            }
            return process_valid_message(this, message);
        }

        R<String> MCPMessageProcessor::process_json(const c8* json, usize json_size)
        {
            R<Variant> parsed = json ?
                VariantUtils::read_json(json, json_size, VariantUtils::JSONReadOptions::strict()) :
                R<Variant>(BasicError::bad_arguments());
            MessageResult result;
            if(!parsed.valid())
            {
                result = error_result(nullptr, PARSE_ERROR, "Parse error");
            }
            else
            {
                result = process_valid_message(this, parsed.get());
            }
            if(!result.has_response) return String();
            return VariantUtils::write_json(
                result.response, VariantUtils::JSONWriteOptions::strict());
        }

        LUNA_MCP_API R<Ref<IMCPServer>> new_server(
            Frontend::IFrontend* frontend,
            const ServerDesc& desc)
        {
            if(!frontend || !desc.name || !desc.version)
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "An MCP server requires a Frontend, name, and version");
            }
            if(desc.cache_scope != CacheScope::public_cache &&
                desc.cache_scope != CacheScope::private_cache)
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP cache scope");
            }
            if(!validate_icons(desc.icons))
            {
                return set_error(BasicError::bad_arguments(), "Invalid MCP server icons");
            }

            Ref<MCPServer> server = new_object<MCPServer>();
            server->m_frontend = Ref<Frontend::IFrontend>(frontend);
            server->m_desc = desc;
            server->m_server_info = Variant(VariantType::object);
            server->m_server_info["name"] = desc.name;
            server->m_server_info["version"] = desc.version;
            if(desc.title) server->m_server_info["title"] = desc.title;
            if(desc.description) server->m_server_info["description"] = desc.description;
            if(desc.website_url) server->m_server_info["websiteUrl"] = desc.website_url;
            if(desc.icons.valid()) server->m_server_info["icons"] = desc.icons;

            server->m_legacy_server_info = Variant(VariantType::object);
            server->m_legacy_server_info["name"] = desc.name;
            server->m_legacy_server_info["version"] = desc.version;
            if(desc.title) server->m_legacy_server_info["title"] = desc.title;

            R<String> encoded = VariantUtils::write_json(
                server->m_server_info, VariantUtils::JSONWriteOptions::strict());
            if(!encoded.valid())
            {
                return set_error(
                    BasicError::bad_arguments(),
                    "The MCP server descriptor is not representable as strict JSON");
            }
            if(desc.instructions)
            {
                R<String> instructions = VariantUtils::write_json(
                    Variant(desc.instructions), VariantUtils::JSONWriteOptions::strict());
                if(!instructions.valid())
                {
                    return set_error(
                        BasicError::bad_arguments(),
                        "The MCP server instructions are not valid UTF-8");
                }
            }
            return Ref<IMCPServer>(server);
        }

        struct MCPModule : public Module
        {
            virtual const c8* get_name() override { return "MCP"; }

            virtual RV on_register() override
            {
                Meta::register_MCP_types();
                return add_dependency_modules(
                    this,
                    {module_variant_utils(), Frontend::module_frontend(), HTTP::module_http()});
            }
        };

        LUNA_MCP_API Module* module_mcp()
        {
            static MCPModule m;
            return &m;
        }
    }
}
