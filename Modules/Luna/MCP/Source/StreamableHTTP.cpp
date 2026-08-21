/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StreamableHTTP.cpp
* @author JXMaster
* @date 2026/8/20
* @brief Local MCP Streamable HTTP transport.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_MCP_API LUNA_EXPORT
#include "MCPImpl.hpp"
#include <Luna/Runtime/Base64.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/StringUtils.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <math.h>
#include <stdio.h>

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
            constexpr i64 HEADER_MISMATCH = -32020;
            constexpr i64 UNSUPPORTED_PROTOCOL_VERSION = -32022;
            constexpr i64 MAX_SAFE_INTEGER = 9007199254740991LL;

            bool ascii_iequal(c8 lhs, c8 rhs)
            {
                if(lhs >= 'A' && lhs <= 'Z') lhs = lhs - 'A' + 'a';
                if(rhs >= 'A' && rhs <= 'Z') rhs = rhs - 'A' + 'a';
                return lhs == rhs;
            }

            bool ascii_iequal(const c8* lhs, usize lhs_size, const c8* rhs, usize rhs_size)
            {
                if(lhs_size != rhs_size) return false;
                for(usize i = 0; i < lhs_size; ++i)
                {
                    if(!ascii_iequal(lhs[i], rhs[i])) return false;
                }
                return true;
            }

            bool string_equal(const String& value, const Name& expected)
            {
                return value.size() == expected.size() &&
                    !memcmp(value.data(), expected.c_str(), value.size());
            }

            bool string_equal(const String& lhs, const String& rhs)
            {
                return lhs.size() == rhs.size() &&
                    !memcmp(lhs.data(), rhs.data(), lhs.size());
            }

            bool is_request_id(const Variant& value)
            {
                return value.type() == VariantType::string || value.type() == VariantType::number;
            }

            const String* find_unique_header(
                const HTTP::Request& request,
                const Name& name,
                bool& duplicate)
            {
                duplicate = false;
                const String* result = nullptr;
                for(const HTTP::Header& header : request.headers)
                {
                    if(header.name != name) continue;
                    if(result)
                    {
                        duplicate = true;
                        return nullptr;
                    }
                    result = &header.value;
                }
                return result;
            }

            bool media_type_is(const c8* begin, usize size, const c8* expected)
            {
                while(size && (*begin == ' ' || *begin == '\t'))
                {
                    ++begin;
                    --size;
                }
                while(size && (begin[size - 1] == ' ' || begin[size - 1] == '\t')) --size;
                const c8* semicolon = (const c8*)memchr(begin, ';', size);
                usize media_size = semicolon ? (usize)(semicolon - begin) : size;
                while(media_size &&
                    (begin[media_size - 1] == ' ' || begin[media_size - 1] == '\t'))
                {
                    --media_size;
                }
                usize expected_size = strlen(expected);
                return ascii_iequal(begin, media_size, expected, expected_size);
            }

            bool accept_contains(const String& accept, const c8* expected)
            {
                usize start = 0;
                while(start <= accept.size())
                {
                    usize end = start;
                    while(end < accept.size() && accept[end] != ',') ++end;
                    if(media_type_is(accept.data() + start, end - start, expected)) return true;
                    if(end == accept.size()) break;
                    start = end + 1;
                }
                return false;
            }

            bool is_loopback(const Network::SocketAddress& address)
            {
                if(address.family == Network::AddressFamily::ipv4)
                {
                    return address.ipv4.address.bytes[0] == 127;
                }
                if(address.family != Network::AddressFamily::ipv6) return false;
                for(usize i = 0; i < 15; ++i)
                {
                    if(address.ipv6.address.bytes[i]) return false;
                }
                return address.ipv6.address.bytes[15] == 1;
            }

            bool valid_endpoint(const String& endpoint)
            {
                if(endpoint.empty() || endpoint[0] != '/') return false;
                for(c8 ch : endpoint)
                {
                    if(ch == '?' || ch == '#' || ch == '\r' || ch == '\n') return false;
                }
                return true;
            }

            HTTP::Response empty_response(u16 status)
            {
                HTTP::Response response;
                response.status_code = status;
                return response;
            }

            Variant error_response(
                const Variant* id,
                i64 code,
                const c8* message,
                const Variant* data = nullptr)
            {
                Variant response(VariantType::object);
                response["jsonrpc"] = "2.0";
                if(id) response["id"] = *id;
                response["error"]["code"] = code;
                response["error"]["message"] = message;
                if(data) response["error"]["data"] = *data;
                return response;
            }

            R<HTTP::Response> json_response(u16 status, const Variant& body)
            {
                R<String> json = VariantUtils::write_json(
                    body, VariantUtils::JSONWriteOptions::strict());
                if(!json.valid()) return json.errcode();
                HTTP::Response response;
                response.status_code = status;
                response.headers.push_back(
                    {Name("content-type"), String("application/json")});
                response.headers.push_back(
                    {Name("cache-control"), String("no-store")});
                response.body = Blob(json.get().data(), json.get().size());
                return response;
            }

            R<HTTP::Response> json_error(
                u16 status,
                const Variant* id,
                i64 code,
                const c8* message,
                const Variant* data = nullptr)
            {
                Variant body = error_response(id, code, message, data);
                return json_response(status, body);
            }

            i64 response_error_code(const MessageResult& result)
            {
                if(!result.has_response) return 0;
                const Variant& error = result.response.find("error");
                if(error.type() != VariantType::object ||
                    error.find("code").type() != VariantType::number) return 0;
                return error.find("code").inum();
            }

            u16 response_status(const MessageResult& result, bool modern)
            {
                i64 code = response_error_code(result);
                if(!code) return 200;
                if(modern && code == METHOD_NOT_FOUND) return 404;
                if(code == PARSE_ERROR || code == INVALID_REQUEST || code == INVALID_PARAMS ||
                    code == HEADER_MISMATCH || code == UNSUPPORTED_PROTOCOL_VERSION) return 400;
                if(code == INTERNAL_ERROR) return 500;
                return 200;
            }

            R<HTTP::Response> message_response(
                const MessageResult& result,
                bool modern)
            {
                if(!result.has_response) return empty_response(202);
                return json_response(response_status(result, modern), result.response);
            }

            bool valid_base64(const c8* chars, usize size)
            {
                if(size % 4) return false;
                usize padding = 0;
                if(size && chars[size - 1] == '=') ++padding;
                if(size > 1 && chars[size - 2] == '=') ++padding;
                for(usize i = 0; i < size; ++i)
                {
                    c8 ch = chars[i];
                    bool valid = (ch >= 'A' && ch <= 'Z') ||
                        (ch >= 'a' && ch <= 'z') ||
                        (ch >= '0' && ch <= '9') || ch == '+' || ch == '/';
                    if(valid) continue;
                    if(ch != '=' || i < size - padding) return false;
                }
                return true;
            }

            bool decode_mirrored_value(const String& header, String& decoded)
            {
                constexpr c8 prefix[] = "=?base64?";
                constexpr c8 suffix[] = "?=";
                constexpr usize prefix_size = sizeof(prefix) - 1;
                constexpr usize suffix_size = sizeof(suffix) - 1;
                bool has_prefix = header.size() >= prefix_size &&
                    !memcmp(header.data(), prefix, prefix_size);
                bool has_suffix = header.size() >= suffix_size &&
                    !memcmp(header.data() + header.size() - suffix_size, suffix, suffix_size);
                if(has_prefix || has_suffix)
                {
                    if(!has_prefix || !has_suffix ||
                        header.size() < prefix_size + suffix_size) return false;
                    const c8* encoded = header.data() + prefix_size;
                    usize encoded_size = header.size() - prefix_size - suffix_size;
                    if(!valid_base64(encoded, encoded_size)) return false;
                    decoded.resize(base64_get_decoded_size(encoded_size), 0);
                    usize decoded_size = encoded_size ? base64_decode(
                        decoded.data(), decoded.size(), encoded, encoded_size) : 0;
                    decoded.resize(decoded_size, 0);
                    return true;
                }

                for(c8 ch : header)
                {
                    u8 byte = (u8)ch;
                    if(byte != 0x09 && (byte < 0x20 || byte > 0x7E)) return false;
                }
                decoded = header;
                return true;
            }

            bool mirrored_value_equals(const String& header, const Name& expected)
            {
                String decoded;
                return decode_mirrored_value(header, decoded) &&
                    decoded.size() == expected.size() &&
                    !memcmp(decoded.data(), expected.c_str(), decoded.size());
            }

            const Variant* find_argument(
                const Variant& arguments,
                const Vector<Name>& property_path)
            {
                const Variant* value = &arguments;
                for(const Name& property : property_path)
                {
                    if(value->type() != VariantType::object || !value->contains(property))
                    {
                        return nullptr;
                    }
                    value = &value->find(property);
                }
                return value;
            }

            bool integer_value(const Variant& value, i64& result)
            {
                if(value.type() != VariantType::number) return false;
                switch(value.number_type())
                {
                case VariantNumberType::number_i64:
                    result = value.inum();
                    return result >= -MAX_SAFE_INTEGER && result <= MAX_SAFE_INTEGER;
                case VariantNumberType::number_u64:
                    if(value.unum() > (u64)MAX_SAFE_INTEGER) return false;
                    result = (i64)value.unum();
                    return true;
                case VariantNumberType::number_f64:
                {
                    f64 number = value.fnum();
                    if(!isfinite(number) || floor(number) != number ||
                        number < (f64)-MAX_SAFE_INTEGER || number > (f64)MAX_SAFE_INTEGER)
                    {
                        return false;
                    }
                    result = (i64)number;
                    return true;
                }
                default: return false;
                }
            }

            bool binding_value_matches(
                const String& header,
                const Variant& value,
                const Name& value_type)
            {
                String decoded;
                if(!decode_mirrored_value(header, decoded)) return false;
                if(value_type == Name("string"))
                {
                    return value.type() == VariantType::string &&
                        decoded.size() == value.str().size() &&
                        !memcmp(decoded.data(), value.str().c_str(), decoded.size());
                }
                if(value_type == Name("boolean"))
                {
                    if(value.type() != VariantType::boolean) return false;
                    const c8* expected = value.boolean() ? "true" : "false";
                    usize expected_size = value.boolean() ? 4 : 5;
                    return decoded.size() == expected_size &&
                        !memcmp(decoded.data(), expected, expected_size);
                }
                i64 number = 0;
                if(!integer_value(value, number) || decoded.empty()) return false;
                c8* end = nullptr;
                i64 header_number = strtoi64(decoded.c_str(), &end, 10);
                return end == decoded.c_str() + decoded.size() && header_number == number;
            }

            R<HTTP::Response> header_mismatch(
                const Variant* id,
                const c8* message)
            {
                return json_error(400, id, HEADER_MISMATCH, message);
            }

            R<HTTP::Response> validate_modern_headers(
                StreamableHTTPState* state,
                const HTTP::Request& request,
                const Variant& message,
                const Variant* id,
                const Name& method,
                const String& version_header)
            {
                const Variant& params = message.find("params");
                const Variant& body_version = params.find("_meta").find(
                    "io.modelcontextprotocol/protocolVersion");
                if(body_version.type() != VariantType::string ||
                    !string_equal(version_header, body_version.str()))
                {
                    return header_mismatch(
                        id, "MCP-Protocol-Version does not match request metadata");
                }

                bool duplicate = false;
                const String* method_header = find_unique_header(
                    request, Name("mcp-method"), duplicate);
                if(duplicate || !method_header || !string_equal(*method_header, method))
                {
                    return header_mismatch(id, "Mcp-Method does not match the request method");
                }

                if(method != Name("tools/call")) return HTTP::Response();
                const Variant& tool_name = params.find("name");
                const String* name_header = find_unique_header(
                    request, Name("mcp-name"), duplicate);
                if(duplicate || tool_name.type() != VariantType::string || !name_header ||
                    !mirrored_value_equals(*name_header, tool_name.str()))
                {
                    return header_mismatch(id, "Mcp-Name does not match the requested tool");
                }

                auto tool = state->server->m_tools.find(tool_name.str());
                if(tool == state->server->m_tools.end()) return HTTP::Response();
                const Variant& arguments = params.find("arguments");
                Variant empty_arguments(VariantType::object);
                const Variant& argument_object = arguments.valid() ? arguments : empty_arguments;
                for(const ToolHeaderBinding& binding : tool->second.header_bindings)
                {
                    const String* header = find_unique_header(
                        request, binding.header_name, duplicate);
                    if(duplicate)
                    {
                        return header_mismatch(id, "A mirrored tool parameter header is repeated");
                    }
                    const Variant* value = find_argument(
                        argument_object, binding.property_path);
                    if(!value || !value->valid())
                    {
                        if(header)
                        {
                            return header_mismatch(
                                id, "A mirrored tool parameter header has no body value");
                        }
                        continue;
                    }
                    if(!header || !binding_value_matches(*header, *value, binding.value_type))
                    {
                        return header_mismatch(
                            id, "A mirrored tool parameter header does not match the body value");
                    }
                }
                return HTTP::Response();
            }

            bool response_is_error(const HTTP::Response& response)
            {
                return response.status_code != 200;
            }

            void expire_legacy_sessions(StreamableHTTPState* state)
            {
                if(!state->options.legacy_session_idle_timeout_ms) return;
                u64 now = get_ticks();
                f64 ticks_per_ms = get_ticks_per_second() / 1000.0;
                Vector<Name> expired;
                for(const auto& session : state->legacy_sessions)
                {
                    f64 elapsed_ms = (f64)(now - session.second.last_activity_ticks) /
                        ticks_per_ms;
                    if(elapsed_ms >= (f64)state->options.legacy_session_idle_timeout_ms)
                    {
                        expired.push_back(session.first);
                    }
                }
                for(const Name& id : expired) state->legacy_sessions.erase(id);
            }

            Name new_session_id(StreamableHTTPState* state)
            {
                while(true)
                {
                    c8 buffer[80];
                    snprintf(
                        buffer,
                        sizeof(buffer),
                        "luna-%016llx-%016llx-%016llx",
                        (unsigned long long)random_u64(),
                        (unsigned long long)random_u64(),
                        (unsigned long long)state->next_session_id++);
                    Name id(buffer);
                    if(state->legacy_sessions.find(id) == state->legacy_sessions.end())
                    {
                        return id;
                    }
                }
            }

            bool allowed_origin(StreamableHTTPState* state, const HTTP::Request& request)
            {
                bool duplicate = false;
                const String* origin = find_unique_header(
                    request, Name("origin"), duplicate);
                if(duplicate) return false;
                if(!origin) return state->options.allow_requests_without_origin;
                for(const String& allowed : state->options.allowed_origins)
                {
                    if(string_equal(*origin, allowed)) return true;
                }
                return false;
            }

            R<HTTP::Response> handle_delete(
                StreamableHTTPState* state,
                const HTTP::Request& request)
            {
                bool duplicate = false;
                const String* version = find_unique_header(
                    request, Name("mcp-protocol-version"), duplicate);
                if(duplicate || !version ||
                    !ascii_iequal(
                        version->data(), version->size(),
                        LEGACY_PROTOCOL_VERSION, strlen(LEGACY_PROTOCOL_VERSION)))
                {
                    return json_error(
                        400, nullptr, INVALID_REQUEST, "DELETE requires the legacy protocol version");
                }
                const String* session_header = find_unique_header(
                    request, Name("mcp-session-id"), duplicate);
                if(duplicate || !session_header || session_header->empty())
                {
                    return json_error(
                        400, nullptr, INVALID_REQUEST, "DELETE requires an MCP session ID");
                }
                Name session_id(*session_header);
                auto session = state->legacy_sessions.find(session_id);
                if(session == state->legacy_sessions.end())
                {
                    return json_error(
                        404, nullptr, INVALID_REQUEST, "MCP session was not found");
                }
                state->legacy_sessions.erase(session_id);
                return empty_response(204);
            }

            R<HTTP::Response> handle_legacy_initialize(
                StreamableHTTPState* state,
                const HTTP::Request& request,
                const Variant& message)
            {
                const Variant& id = message.find("id");
                bool duplicate = false;
                const String* session_header = find_unique_header(
                    request, Name("mcp-session-id"), duplicate);
                if(duplicate || session_header)
                {
                    return json_error(
                        400, &id, INVALID_REQUEST, "Initialize must not carry an MCP session ID");
                }
                const String* version_header = find_unique_header(
                    request, Name("mcp-protocol-version"), duplicate);
                if(duplicate || (version_header &&
                    !ascii_iequal(
                        version_header->data(), version_header->size(),
                        LEGACY_PROTOCOL_VERSION, strlen(LEGACY_PROTOCOL_VERSION))))
                {
                    return json_error(
                        400, &id, INVALID_REQUEST, "Invalid legacy protocol version header");
                }

                expire_legacy_sessions(state);
                if(state->legacy_sessions.size() >= state->options.max_legacy_sessions)
                {
                    return json_error(
                        503, &id, INTERNAL_ERROR, "Legacy MCP session limit reached");
                }
                R<Ref<IMCPMessageProcessor>> processor =
                    state->server->new_message_processor(ProtocolVersion::v2025_06_18);
                if(!processor.valid()) return processor.errcode();
                MessageResult result = processor.get()->process_message(message);
                R<HTTP::Response> response = message_response(result, false);
                if(!response.valid()) return response.errcode();
                if(response_error_code(result)) return response;

                Name session_id = new_session_id(state);
                LegacyHTTPSession session;
                session.processor = move(processor.get());
                session.last_activity_ticks = get_ticks();
                state->legacy_sessions.insert_or_assign(session_id, move(session));
                response.get().headers.push_back(
                    {Name("mcp-session-id"), String(session_id.c_str(), session_id.size())});
                return response;
            }

            R<HTTP::Response> handle_legacy_request(
                StreamableHTTPState* state,
                const HTTP::Request& request,
                const Variant& message,
                const String& version_header)
            {
                const Variant* id = message.contains("id") ? &message.find("id") : nullptr;
                if(!ascii_iequal(
                    version_header.data(), version_header.size(),
                    LEGACY_PROTOCOL_VERSION, strlen(LEGACY_PROTOCOL_VERSION)))
                {
                    return json_error(
                        400, id, UNSUPPORTED_PROTOCOL_VERSION, "Unsupported protocol version");
                }
                bool duplicate = false;
                const String* session_header = find_unique_header(
                    request, Name("mcp-session-id"), duplicate);
                if(duplicate || !session_header || session_header->empty())
                {
                    return json_error(
                        400, id, INVALID_REQUEST, "Legacy request requires an MCP session ID");
                }
                expire_legacy_sessions(state);
                auto session = state->legacy_sessions.find(Name(*session_header));
                if(session == state->legacy_sessions.end())
                {
                    return json_error(404, id, INVALID_REQUEST, "MCP session was not found");
                }
                session->second.last_activity_ticks = get_ticks();
                MessageResult result = session->second.processor->process_message(message);
                return message_response(result, false);
            }

            R<HTTP::Response> handle_modern_request(
                StreamableHTTPState* state,
                const HTTP::Request& request,
                const Variant& message,
                const Name& method,
                const String& version_header)
            {
                const Variant* id = message.contains("id") ? &message.find("id") : nullptr;
                if(!ascii_iequal(
                    version_header.data(), version_header.size(),
                    MODERN_PROTOCOL_VERSION, strlen(MODERN_PROTOCOL_VERSION)))
                {
                    Variant data(VariantType::object);
                    data["supported"] = Variant(VariantType::array);
                    data["supported"].push_back(MODERN_PROTOCOL_VERSION);
                    data["requested"] = Name(version_header);
                    return json_error(
                        400,
                        id,
                        UNSUPPORTED_PROTOCOL_VERSION,
                        "Unsupported protocol version",
                        &data);
                }
                R<HTTP::Response> header_result = validate_modern_headers(
                    state, request, message, id, method, version_header);
                if(!header_result.valid()) return header_result.errcode();
                if(response_is_error(header_result.get())) return header_result;

                R<Ref<IMCPMessageProcessor>> processor =
                    state->server->new_message_processor(ProtocolVersion::v2026_07_28);
                if(!processor.valid()) return processor.errcode();
                MessageResult result = processor.get()->process_message(message);
                return message_response(result, true);
            }

            R<HTTP::Response> handle_post(
                StreamableHTTPState* state,
                const HTTP::Request& request)
            {
                bool duplicate = false;
                const String* content_type = find_unique_header(
                    request, Name("content-type"), duplicate);
                if(duplicate || !content_type ||
                    !media_type_is(
                        content_type->data(), content_type->size(), "application/json"))
                {
                    return json_error(
                        415, nullptr, INVALID_REQUEST, "Content-Type must be application/json");
                }
                const String* accept = find_unique_header(
                    request, Name("accept"), duplicate);
                if(duplicate || !accept ||
                    !accept_contains(*accept, "application/json") ||
                    !accept_contains(*accept, "text/event-stream"))
                {
                    return json_error(
                        406,
                        nullptr,
                        INVALID_REQUEST,
                        "Accept must include application/json and text/event-stream");
                }

                R<Variant> parsed = VariantUtils::read_json(
                    (const c8*)request.body.data(),
                    request.body.size(),
                    VariantUtils::JSONReadOptions::strict());
                if(!parsed.valid())
                {
                    return json_error(400, nullptr, PARSE_ERROR, "Parse error");
                }
                const Variant& message = parsed.get();
                const bool has_id = message.type() == VariantType::object &&
                    message.contains("id");
                const Variant* id = has_id && is_request_id(message.find("id")) ?
                    &message.find("id") : nullptr;
                if(message.type() != VariantType::object ||
                    (has_id && !is_request_id(message.find("id"))) ||
                    message.find("jsonrpc").type() != VariantType::string ||
                    message.find("jsonrpc").str() != Name("2.0") ||
                    message.find("method").type() != VariantType::string ||
                    message.contains("result") || message.contains("error"))
                {
                    return json_error(400, id, INVALID_REQUEST, "Invalid Request");
                }
                const Name method = message.find("method").str();
                if(method == Name("initialize"))
                {
                    if(!has_id)
                    {
                        return json_error(
                            400, nullptr, INVALID_REQUEST, "Initialize must be a request");
                    }
                    return handle_legacy_initialize(state, request, message);
                }

                const String* version_header = find_unique_header(
                    request, Name("mcp-protocol-version"), duplicate);
                if(duplicate || !version_header || version_header->empty())
                {
                    return header_mismatch(id, "MCP-Protocol-Version is required");
                }
                if(ascii_iequal(
                    version_header->data(), version_header->size(),
                    LEGACY_PROTOCOL_VERSION, strlen(LEGACY_PROTOCOL_VERSION)))
                {
                    return handle_legacy_request(
                        state, request, message, *version_header);
                }
                return handle_modern_request(
                    state, request, message, method, *version_header);
            }

            R<HTTP::Response> handle_request(
                StreamableHTTPState* state,
                const HTTP::Request& request)
            {
                if(!string_equal(request.path, state->options.endpoint))
                {
                    return empty_response(404);
                }
                if(!allowed_origin(state, request)) return empty_response(403);
                expire_legacy_sessions(state);
                if(request.method == Name("POST")) return handle_post(state, request);
                if(request.method == Name("DELETE")) return handle_delete(state, request);
                HTTP::Response response = empty_response(405);
                response.headers.push_back({Name("allow"), String("POST, DELETE")});
                return response;
            }
        }

        LUNA_MCP_API R<Ref<HTTP::IServer>> new_streamable_http_server(
            IMCPServer* server,
            const Network::SocketAddress& address,
            const StreamableHTTPServerOptions& options)
        {
            if(!server || !is_loopback(address) || !valid_endpoint(options.endpoint) ||
                !options.max_legacy_sessions)
            {
                return set_error(
                    E_BAD_ARGUMENTS,
                    "Streamable HTTP requires a server, loopback address, valid endpoint, and session capacity");
            }
            MCPServer* concrete_pointer = cast_object<MCPServer>(server->get_object());
            Ref<MCPServer> concrete_server(concrete_pointer);
            if(!concrete_server)
            {
                return set_error(E_BAD_ARGUMENTS, "Invalid MCP server implementation");
            }
            for(const String& origin : options.allowed_origins)
            {
                if(origin.empty() || origin.find('\r') != String::npos ||
                    origin.find('\n') != String::npos)
                {
                    return set_error(E_BAD_ARGUMENTS, "Invalid allowed Origin value");
                }
            }

            Ref<StreamableHTTPState> state = new_object<StreamableHTTPState>();
            state->server = move(concrete_server);
            state->options = options;
            HTTP::RequestHandler handler(
                [state](const HTTP::Request& request) -> R<HTTP::Response>
                {
                    return handle_request(state, request);
                });
            return HTTP::new_server(address, move(handler), options.http);
        }
    }
}
