/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HTTP.cpp
* @author JXMaster
* @date 2026/8/19
* @brief HTTP/1.x parsing, serialization, and event-driven server implementation.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HTTP_API LUNA_EXPORT
#include "HTTPImpl.hpp"
#include "HTTP.meta.generated.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <stdio.h>

namespace Luna
{
    namespace HTTP
    {
        namespace
        {
            constexpr usize RECEIVE_CHUNK_SIZE = 16 * 1024;
            constexpr usize RECEIVE_QUOTA_PER_EVENT = 256 * 1024;
            constexpr usize SEND_QUOTA_PER_EVENT = 256 * 1024;

            enum class ParseState : u8
            {
                need_more,
                complete,
                error,
            };

            struct ParseOutcome
            {
                ParseState state = ParseState::need_more;
                Request request;
                usize consumed = 0;
                u16 error_status = 400;
                bool send_continue = false;
            };

            enum class LineState : u8
            {
                need_more,
                found,
                invalid,
            };

            bool is_token_char(u8 ch)
            {
                if((ch >= 'a' && ch <= 'z') ||
                    (ch >= 'A' && ch <= 'Z') ||
                    (ch >= '0' && ch <= '9')) return true;
                switch(ch)
                {
                case '!': case '#': case '$': case '%': case '&': case '\'':
                case '*': case '+': case '-': case '.': case '^': case '_':
                case '`': case '|': case '~': return true;
                default: return false;
                }
            }

            bool is_valid_token(const u8* data, usize size)
            {
                if(!size) return false;
                for(usize i = 0; i < size; ++i)
                {
                    if(!is_token_char(data[i])) return false;
                }
                return true;
            }

            bool ascii_equal_ignore_case(const c8* lhs, usize lhs_size, const c8* rhs, usize rhs_size)
            {
                if(lhs_size != rhs_size) return false;
                for(usize i = 0; i < lhs_size; ++i)
                {
                    u8 a = (u8)lhs[i];
                    u8 b = (u8)rhs[i];
                    if(a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
                    if(b >= 'A' && b <= 'Z') b = b - 'A' + 'a';
                    if(a != b) return false;
                }
                return true;
            }

            bool name_equal_ignore_case(const Name& name, const c8* value)
            {
                return ascii_equal_ignore_case(name.c_str(), name.size(), value, strlen(value));
            }

            bool value_equal_ignore_case(const String& value, const c8* expected)
            {
                return ascii_equal_ignore_case(value.data(), value.size(), expected, strlen(expected));
            }

            LineState find_line(const u8* data, usize size, usize begin, usize& line_end)
            {
                for(usize i = begin; i < size; ++i)
                {
                    if(data[i] == '\n') return LineState::invalid;
                    if(data[i] != '\r') continue;
                    if(i + 1 >= size) return LineState::need_more;
                    if(data[i + 1] != '\n') return LineState::invalid;
                    line_end = i;
                    return LineState::found;
                }
                return LineState::need_more;
            }

            void append_bytes(Vector<u8>& buffer, const void* data, usize size)
            {
                if(!size) return;
                usize old_size = buffer.size();
                buffer.resize(old_size + size);
                memcpy(buffer.data() + old_size, data, size);
            }

            void append_text(Vector<u8>& buffer, const c8* text)
            {
                append_bytes(buffer, text, strlen(text));
            }

            void compact_buffer(Vector<u8>& buffer, usize& offset)
            {
                if(!offset) return;
                if(offset == buffer.size())
                {
                    buffer.clear();
                    offset = 0;
                    return;
                }
                usize remaining = buffer.size() - offset;
                memmove(buffer.data(), buffer.data() + offset, remaining);
                buffer.resize(remaining);
                offset = 0;
            }

            bool validate_field_value(const u8* data, usize size)
            {
                for(usize i = 0; i < size; ++i)
                {
                    u8 ch = data[i];
                    if(ch == '\t' || (ch >= 0x20 && ch != 0x7f)) continue;
                    return false;
                }
                return true;
            }

            bool parse_header_line(
                const u8* data,
                usize size,
                Header& header)
            {
                if(!size || data[0] == ' ' || data[0] == '\t') return false;
                usize colon = size;
                for(usize i = 0; i < size; ++i)
                {
                    if(data[i] == ':')
                    {
                        colon = i;
                        break;
                    }
                }
                if(colon == size || !is_valid_token(data, colon)) return false;

                String lower_name((const c8*)data, colon);
                for(usize i = 0; i < lower_name.size(); ++i)
                {
                    if(lower_name[i] >= 'A' && lower_name[i] <= 'Z')
                    {
                        lower_name[i] = lower_name[i] - 'A' + 'a';
                    }
                }

                usize value_begin = colon + 1;
                while(value_begin < size && (data[value_begin] == ' ' || data[value_begin] == '\t'))
                {
                    ++value_begin;
                }
                usize value_end = size;
                while(value_end > value_begin &&
                    (data[value_end - 1] == ' ' || data[value_end - 1] == '\t'))
                {
                    --value_end;
                }
                if(!validate_field_value(data + value_begin, value_end - value_begin)) return false;

                header.name = lower_name;
                header.value.assign((const c8*)data + value_begin, value_end - value_begin);
                return true;
            }

            bool parse_decimal(const c8* data, usize size, usize& value)
            {
                if(!size) return false;
                usize result = 0;
                for(usize i = 0; i < size; ++i)
                {
                    c8 ch = data[i];
                    if(ch < '0' || ch > '9') return false;
                    usize digit = (usize)(ch - '0');
                    if(result > (USIZE_MAX - digit) / 10) return false;
                    result = result * 10 + digit;
                }
                value = result;
                return true;
            }

            bool parse_content_length_value(
                const String& value,
                bool& has_length,
                usize& content_length)
            {
                usize begin = 0;
                bool found_value = false;
                while(begin <= value.size())
                {
                    usize end = begin;
                    while(end < value.size() && value[end] != ',') ++end;
                    usize trimmed_begin = begin;
                    usize trimmed_end = end;
                    while(trimmed_begin < trimmed_end &&
                        (value[trimmed_begin] == ' ' || value[trimmed_begin] == '\t')) ++trimmed_begin;
                    while(trimmed_end > trimmed_begin &&
                        (value[trimmed_end - 1] == ' ' || value[trimmed_end - 1] == '\t')) --trimmed_end;
                    usize parsed = 0;
                    if(!parse_decimal(value.data() + trimmed_begin, trimmed_end - trimmed_begin, parsed))
                    {
                        return false;
                    }
                    if(has_length && parsed != content_length) return false;
                    has_length = true;
                    found_value = true;
                    content_length = parsed;
                    if(end == value.size()) break;
                    begin = end + 1;
                }
                return found_value;
            }

            bool parse_transfer_encoding(const Vector<Header>& headers, bool& chunked)
            {
                bool found = false;
                usize coding_count = 0;
                bool final_chunked = false;
                for(const Header& header : headers)
                {
                    if(header.name != Name("transfer-encoding")) continue;
                    found = true;
                    usize begin = 0;
                    while(begin <= header.value.size())
                    {
                        usize end = begin;
                        while(end < header.value.size() && header.value[end] != ',') ++end;
                        usize token_begin = begin;
                        usize token_end = end;
                        while(token_begin < token_end &&
                            (header.value[token_begin] == ' ' || header.value[token_begin] == '\t')) ++token_begin;
                        while(token_end > token_begin &&
                            (header.value[token_end - 1] == ' ' || header.value[token_end - 1] == '\t')) --token_end;
                        if(token_begin == token_end ||
                            !is_valid_token((const u8*)header.value.data() + token_begin, token_end - token_begin))
                        {
                            return false;
                        }
                        ++coding_count;
                        final_chunked = ascii_equal_ignore_case(
                            header.value.data() + token_begin,
                            token_end - token_begin,
                            "chunked",
                            7);
                        if(end == header.value.size()) break;
                        begin = end + 1;
                    }
                }
                if(!found)
                {
                    chunked = false;
                    return true;
                }
                chunked = coding_count == 1 && final_chunked;
                return chunked;
            }

            bool field_has_token(
                const Vector<Header>& headers,
                const Name& field_name,
                const c8* expected)
            {
                usize expected_size = strlen(expected);
                for(const Header& header : headers)
                {
                    if(header.name != field_name) continue;
                    usize begin = 0;
                    while(begin <= header.value.size())
                    {
                        usize end = begin;
                        while(end < header.value.size() && header.value[end] != ',') ++end;
                        usize token_begin = begin;
                        usize token_end = end;
                        while(token_begin < token_end &&
                            (header.value[token_begin] == ' ' || header.value[token_begin] == '\t')) ++token_begin;
                        while(token_end > token_begin &&
                            (header.value[token_end - 1] == ' ' || header.value[token_end - 1] == '\t')) --token_end;
                        if(ascii_equal_ignore_case(
                            header.value.data() + token_begin,
                            token_end - token_begin,
                            expected,
                            expected_size)) return true;
                        if(end == header.value.size()) break;
                        begin = end + 1;
                    }
                }
                return false;
            }

            bool parse_hex_size(const u8* data, usize size, usize& value)
            {
                if(!size) return false;
                usize result = 0;
                for(usize i = 0; i < size; ++i)
                {
                    u8 ch = data[i];
                    usize digit = 0;
                    if(ch >= '0' && ch <= '9') digit = ch - '0';
                    else if(ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
                    else if(ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
                    else return false;
                    if(result > (USIZE_MAX - digit) / 16) return false;
                    result = result * 16 + digit;
                }
                value = result;
                return true;
            }

            bool is_forbidden_trailer(const Name& name)
            {
                return name == Name("content-length") ||
                    name == Name("transfer-encoding") ||
                    name == Name("host") ||
                    name == Name("connection") ||
                    name == Name("trailer") ||
                    name == Name("te") ||
                    name == Name("upgrade") ||
                    name == Name("authorization") ||
                    name == Name("proxy-authorization") ||
                    name == Name("proxy-authenticate") ||
                    name == Name("www-authenticate") ||
                    name == Name("cookie") ||
                    name == Name("set-cookie") ||
                    name == Name("content-encoding") ||
                    name == Name("content-range") ||
                    name == Name("content-type");
            }

            ParseState scan_chunked_body(
                const u8* data,
                usize size,
                usize body_begin,
                const ServerOptions& options,
                Vector<Header>& trailers,
                usize& consumed,
                usize& decoded_size,
                u16& error_status)
            {
                usize position = body_begin;
                decoded_size = 0;
                for(;;)
                {
                    usize line_end = 0;
                    LineState line_state = find_line(data, size, position, line_end);
                    if(line_state == LineState::invalid)
                    {
                        error_status = 400;
                        return ParseState::error;
                    }
                    if(line_state == LineState::need_more)
                    {
                        if(size - position > options.max_header_section_size)
                        {
                            error_status = 400;
                            return ParseState::error;
                        }
                        return ParseState::need_more;
                    }
                    usize semicolon = position;
                    while(semicolon < line_end && data[semicolon] != ';') ++semicolon;
                    usize chunk_size = 0;
                    if(!parse_hex_size(data + position, semicolon - position, chunk_size))
                    {
                        error_status = 400;
                        return ParseState::error;
                    }
                    if(semicolon < line_end &&
                        !validate_field_value(data + semicolon + 1, line_end - semicolon - 1))
                    {
                        error_status = 400;
                        return ParseState::error;
                    }
                    position = line_end + 2;
                    if(chunk_size)
                    {
                        if(chunk_size > options.max_body_size - decoded_size)
                        {
                            error_status = 413;
                            return ParseState::error;
                        }
                        if(chunk_size > size - position) return ParseState::need_more;
                        if(size - position - chunk_size < 2) return ParseState::need_more;
                        if(data[position + chunk_size] != '\r' || data[position + chunk_size + 1] != '\n')
                        {
                            error_status = 400;
                            return ParseState::error;
                        }
                        decoded_size += chunk_size;
                        position += chunk_size + 2;
                        continue;
                    }

                    usize trailer_begin = position;
                    usize trailer_count = 0;
                    for(;;)
                    {
                        line_state = find_line(data, size, position, line_end);
                        if(line_state == LineState::invalid)
                        {
                            error_status = 400;
                            return ParseState::error;
                        }
                        if(line_state == LineState::need_more)
                        {
                            if(size - trailer_begin > options.max_header_section_size)
                            {
                                error_status = 431;
                                return ParseState::error;
                            }
                            return ParseState::need_more;
                        }
                        if(line_end + 2 - trailer_begin > options.max_header_section_size)
                        {
                            error_status = 431;
                            return ParseState::error;
                        }
                        if(line_end == position)
                        {
                            consumed = line_end + 2;
                            return ParseState::complete;
                        }
                        if(++trailer_count > options.max_header_count)
                        {
                            error_status = 431;
                            return ParseState::error;
                        }
                        Header trailer;
                        if(!parse_header_line(data + position, line_end - position, trailer) ||
                            is_forbidden_trailer(trailer.name))
                        {
                            error_status = 400;
                            return ParseState::error;
                        }
                        trailers.push_back(move(trailer));
                        position = line_end + 2;
                    }
                }
            }

            void decode_chunked_body(
                const u8* data,
                usize body_begin,
                Blob& body)
            {
                usize position = body_begin;
                usize output_offset = 0;
                for(;;)
                {
                    usize line_end = position;
                    while(data[line_end] != '\r') ++line_end;
                    usize semicolon = position;
                    while(semicolon < line_end && data[semicolon] != ';') ++semicolon;
                    usize chunk_size = 0;
                    parse_hex_size(data + position, semicolon - position, chunk_size);
                    position = line_end + 2;
                    if(!chunk_size) return;
                    memcpy((u8*)body.data() + output_offset, data + position, chunk_size);
                    output_offset += chunk_size;
                    position += chunk_size + 2;
                }
            }

            void derive_target_parts(Request& request)
            {
                const c8* target = request.target.data();
                usize target_size = request.target.size();
                usize path_begin = 0;
                if(target_size >= 7 &&
                    (ascii_equal_ignore_case(target, 7, "http://", 7) ||
                    (target_size >= 8 && ascii_equal_ignore_case(target, 8, "https://", 8))))
                {
                    usize scheme_end = 0;
                    while(scheme_end + 2 < target_size &&
                        !(target[scheme_end] == ':' && target[scheme_end + 1] == '/' && target[scheme_end + 2] == '/'))
                    {
                        ++scheme_end;
                    }
                    path_begin = scheme_end + 3;
                    while(path_begin < target_size && target[path_begin] != '/' && target[path_begin] != '?')
                    {
                        ++path_begin;
                    }
                }
                usize query = path_begin;
                while(query < target_size && target[query] != '?') ++query;
                if(path_begin == target_size || target[path_begin] == '?') request.path = "/";
                else request.path.assign(target + path_begin, query - path_begin);
                if(query < target_size) request.query.assign(target + query + 1, target_size - query - 1);
            }

            ParseOutcome parse_request(
                const u8* data,
                usize size,
                const Network::SocketAddress& remote_address,
                const ServerOptions& options)
            {
                ParseOutcome outcome;
                usize request_line_end = 0;
                LineState line_state = find_line(data, size, 0, request_line_end);
                if(line_state == LineState::invalid)
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }
                if(line_state == LineState::need_more)
                {
                    if(size > options.max_request_line_size)
                    {
                        outcome.state = ParseState::error;
                        outcome.error_status = 414;
                    }
                    return outcome;
                }
                if(request_line_end > options.max_request_line_size)
                {
                    outcome.state = ParseState::error;
                    outcome.error_status = 414;
                    return outcome;
                }

                usize first_space = request_line_end;
                usize second_space = request_line_end;
                for(usize i = 0; i < request_line_end; ++i)
                {
                    if(data[i] != ' ') continue;
                    if(first_space == request_line_end) first_space = i;
                    else
                    {
                        second_space = i;
                        break;
                    }
                }
                if(first_space == request_line_end || second_space == request_line_end ||
                    second_space + 1 >= request_line_end ||
                    !is_valid_token(data, first_space))
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }
                for(usize i = second_space + 1; i < request_line_end; ++i)
                {
                    if(data[i] == ' ')
                    {
                        outcome.state = ParseState::error;
                        return outcome;
                    }
                }
                usize target_begin = first_space + 1;
                usize target_size = second_space - target_begin;
                if(!target_size)
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }
                for(usize i = target_begin; i < second_space; ++i)
                {
                    if(data[i] <= 0x20 || data[i] >= 0x7f)
                    {
                        outcome.state = ParseState::error;
                        return outcome;
                    }
                }
                const u8* version = data + second_space + 1;
                usize version_size = request_line_end - second_space - 1;
                if(version_size == 8 && memcmp(version, "HTTP/1.1", 8) == 0)
                {
                    outcome.request.version = HTTPVersion::http_1_1;
                }
                else if(version_size == 8 && memcmp(version, "HTTP/1.0", 8) == 0)
                {
                    outcome.request.version = HTTPVersion::http_1_0;
                }
                else
                {
                    outcome.state = ParseState::error;
                    outcome.error_status = 505;
                    return outcome;
                }
                outcome.request.method = Name((const c8*)data, first_space);
                outcome.request.target.assign((const c8*)data + target_begin, target_size);
                outcome.request.remote_address = remote_address;
                derive_target_parts(outcome.request);

                usize header_begin = request_line_end + 2;
                usize position = header_begin;
                usize header_count = 0;
                for(;;)
                {
                    usize line_end = 0;
                    line_state = find_line(data, size, position, line_end);
                    if(line_state == LineState::invalid)
                    {
                        outcome.state = ParseState::error;
                        return outcome;
                    }
                    if(line_state == LineState::need_more)
                    {
                        if(size - header_begin > options.max_header_section_size)
                        {
                            outcome.state = ParseState::error;
                            outcome.error_status = 431;
                        }
                        return outcome;
                    }
                    if(line_end + 2 - header_begin > options.max_header_section_size)
                    {
                        outcome.state = ParseState::error;
                        outcome.error_status = 431;
                        return outcome;
                    }
                    if(line_end == position)
                    {
                        position = line_end + 2;
                        break;
                    }
                    if(++header_count > options.max_header_count)
                    {
                        outcome.state = ParseState::error;
                        outcome.error_status = 431;
                        return outcome;
                    }
                    Header header;
                    if(!parse_header_line(data + position, line_end - position, header))
                    {
                        outcome.state = ParseState::error;
                        return outcome;
                    }
                    outcome.request.headers.push_back(move(header));
                    position = line_end + 2;
                }

                bool has_content_length = false;
                usize content_length = 0;
                bool has_transfer_encoding = false;
                usize host_count = 0;
                usize expect_count = 0;
                bool expects_continue = false;
                for(const Header& header : outcome.request.headers)
                {
                    if(header.name == Name("content-length"))
                    {
                        if(!parse_content_length_value(
                            header.value, has_content_length, content_length))
                        {
                            outcome.state = ParseState::error;
                            return outcome;
                        }
                    }
                    else if(header.name == Name("transfer-encoding"))
                    {
                        has_transfer_encoding = true;
                    }
                    else if(header.name == Name("host"))
                    {
                        ++host_count;
                        if(header.value.empty())
                        {
                            outcome.state = ParseState::error;
                            return outcome;
                        }
                    }
                    else if(header.name == Name("expect"))
                    {
                        ++expect_count;
                        expects_continue = value_equal_ignore_case(
                            header.value, "100-continue");
                    }
                }
                if((outcome.request.version == HTTPVersion::http_1_1 && host_count != 1) ||
                    host_count > 1)
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }
                if(expect_count && (expect_count != 1 || !expects_continue))
                {
                    outcome.state = ParseState::error;
                    outcome.error_status = 417;
                    return outcome;
                }
                if(expects_continue && outcome.request.version != HTTPVersion::http_1_1)
                {
                    outcome.state = ParseState::error;
                    outcome.error_status = 417;
                    return outcome;
                }
                if(has_transfer_encoding && has_content_length)
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }
                if(content_length > options.max_body_size)
                {
                    outcome.state = ParseState::error;
                    outcome.error_status = 413;
                    return outcome;
                }

                bool chunked = false;
                if(!parse_transfer_encoding(outcome.request.headers, chunked))
                {
                    outcome.state = ParseState::error;
                    return outcome;
                }

                if(chunked)
                {
                    usize decoded_size = 0;
                    usize consumed = 0;
                    u16 error_status = 400;
                    ParseState body_state = scan_chunked_body(
                        data,
                        size,
                        position,
                        options,
                        outcome.request.trailers,
                        consumed,
                        decoded_size,
                        error_status);
                    if(body_state != ParseState::complete)
                    {
                        outcome.state = body_state;
                        outcome.error_status = error_status;
                        outcome.send_continue = body_state == ParseState::need_more && expects_continue;
                        return outcome;
                    }
                    outcome.request.body.resize(decoded_size, false);
                    decode_chunked_body(data, position, outcome.request.body);
                    outcome.consumed = consumed;
                }
                else if(has_content_length)
                {
                    if(size - position < content_length)
                    {
                        outcome.send_continue = expects_continue;
                        return outcome;
                    }
                    outcome.request.body = Blob(data + position, content_length);
                    outcome.consumed = position + content_length;
                }
                else
                {
                    outcome.consumed = position;
                }
                outcome.state = ParseState::complete;
                return outcome;
            }

            const c8* reason_phrase(u16 status)
            {
                switch(status)
                {
                case 100: return "Continue";
                case 200: return "OK";
                case 201: return "Created";
                case 202: return "Accepted";
                case 204: return "No Content";
                case 206: return "Partial Content";
                case 301: return "Moved Permanently";
                case 302: return "Found";
                case 304: return "Not Modified";
                case 307: return "Temporary Redirect";
                case 308: return "Permanent Redirect";
                case 400: return "Bad Request";
                case 401: return "Unauthorized";
                case 403: return "Forbidden";
                case 404: return "Not Found";
                case 405: return "Method Not Allowed";
                case 408: return "Request Timeout";
                case 411: return "Length Required";
                case 413: return "Content Too Large";
                case 414: return "URI Too Long";
                case 417: return "Expectation Failed";
                case 426: return "Upgrade Required";
                case 431: return "Request Header Fields Too Large";
                case 500: return "Internal Server Error";
                case 501: return "Not Implemented";
                case 503: return "Service Unavailable";
                case 505: return "HTTP Version Not Supported";
                default: return "";
                }
            }

            bool is_response_control_header(const Name& name)
            {
                return name_equal_ignore_case(name, "content-length") ||
                    name_equal_ignore_case(name, "transfer-encoding") ||
                    name_equal_ignore_case(name, "connection");
            }

            bool response_has_no_content(u16 status)
            {
                return (status >= 100 && status < 200) || status == 204 || status == 304;
            }

            bool serialize_response(
                const Response& response,
                const Name& request_method,
                bool close_connection,
                Vector<u8>& output)
            {
                if(response.status_code < 200 || response.status_code > 599) return false;
                c8 status_line[128];
                int status_line_size = snprintf(
                    status_line,
                    sizeof(status_line),
                    "HTTP/1.1 %u %s\r\n",
                    (u32)response.status_code,
                    reason_phrase(response.status_code));
                if(status_line_size <= 0 || (usize)status_line_size >= sizeof(status_line)) return false;
                append_bytes(output, status_line, (usize)status_line_size);

                for(const Header& header : response.headers)
                {
                    if(is_response_control_header(header.name)) continue;
                    if(!is_valid_token((const u8*)header.name.c_str(), header.name.size()) ||
                        !validate_field_value((const u8*)header.value.data(), header.value.size()))
                    {
                        return false;
                    }
                    for(usize i = 0; i < header.name.size(); ++i)
                    {
                        u8 ch = (u8)header.name.c_str()[i];
                        if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
                        append_bytes(output, &ch, 1);
                    }
                    append_text(output, ": ");
                    append_bytes(output, header.value.data(), header.value.size());
                    append_text(output, "\r\n");
                }

                usize representation_length = response.body.size();
                if(response.status_code >= 100 && response.status_code < 200 || response.status_code == 204)
                {
                    representation_length = 0;
                }
                if(response.status_code != 204)
                {
                    c8 length_line[64];
                    int length_line_size = snprintf(
                        length_line,
                        sizeof(length_line),
                        "content-length: %llu\r\n",
                        (unsigned long long)representation_length);
                    if(length_line_size <= 0 || (usize)length_line_size >= sizeof(length_line)) return false;
                    append_bytes(output, length_line, (usize)length_line_size);
                }
                if(close_connection) append_text(output, "connection: close\r\n");
                append_text(output, "\r\n");

                bool head_request = request_method == Name("HEAD");
                if(!head_request && !response_has_no_content(response.status_code))
                {
                    append_bytes(output, response.body.data(), response.body.size());
                }
                return true;
            }

            void make_error_response(u16 status, bool close_connection, Vector<u8>& output)
            {
                const c8* phrase = reason_phrase(status);
                String body;
                c8 status_text[16];
                int status_size = snprintf(status_text, sizeof(status_text), "%u ", (u32)status);
                if(status_size > 0) body.append(status_text, (usize)status_size);
                body.append(phrase);
                body.push_back('\n');
                Response response;
                response.status_code = status;
                response.headers.push_back({Name("content-type"), String("text/plain; charset=utf-8")});
                response.body = Blob(body.data(), body.size());
                serialize_response(response, Name(), close_connection, output);
            }

            usize maximum_input_size(const ServerOptions& options)
            {
                usize value = options.max_request_line_size;
                if(value > USIZE_MAX - options.max_header_section_size) return USIZE_MAX;
                value += options.max_header_section_size;
                if(value > USIZE_MAX - options.max_body_size) return USIZE_MAX;
                value += options.max_body_size;
                if(value > USIZE_MAX - options.max_header_section_size) return USIZE_MAX;
                value += options.max_header_section_size;
                return value;
            }

            bool contains_connection(const Vector<Connection*>& connections, Connection* connection)
            {
                for(Connection* candidate : connections)
                {
                    if(candidate == connection) return true;
                }
                return false;
            }
        }

        Server::~Server()
        {
            close();
        }

        RV Server::init(
            const Network::SocketAddress& address,
            RequestHandler&& handler,
            const ServerOptions& options)
        {
            if(!handler ||
                (address.family != Network::AddressFamily::ipv4 &&
                    address.family != Network::AddressFamily::ipv6) ||
                !options.max_connections ||
                !options.max_request_line_size ||
                !options.max_header_section_size ||
                !options.max_header_count ||
                options.max_buffered_output_size < 1024 ||
                options.listen_backlog <= 0 ||
                !options.max_accepts_per_poll ||
                !options.max_requests_per_poll ||
                !options.max_socket_events_per_poll)
            {
                return BasicError::bad_arguments();
            }

            m_handler = move(handler);
            m_options = options;
            auto listener_result = Network::new_tcp_socket(address.family);
            if(!listener_result.valid()) return listener_result.errcode();
            m_listener = listener_result.get();
            RV result = m_listener->bind(address);
            if(failed(result))
            {
                m_listener->close();
                m_listener.reset();
                return result;
            }
            result = m_listener->listen(options.listen_backlog);
            if(failed(result))
            {
                m_listener->close();
                m_listener.reset();
                return result;
            }
            auto poller_result = Network::new_socket_poller();
            if(!poller_result.valid())
            {
                m_listener->close();
                m_listener.reset();
                return poller_result.errcode();
            }
            m_poller = poller_result.get();
            auto token_result = m_poller->add(
                m_listener.get(), Network::SocketEventFlag::readable, nullptr);
            if(!token_result.valid())
            {
                m_listener->close();
                m_listener.reset();
                m_poller.reset();
                return token_result.errcode();
            }
            m_listener_token = token_result.get();
            m_closed = false;
            return ok;
        }

        namespace
        {
            void remove_connection(Server* server, Connection* connection)
            {
                if(connection->token)
                {
                    RV remove_result = server->m_poller->remove(connection->token);
                    (void)remove_result;
                }
                connection->socket->close();
                for(usize i = 0; i < server->m_connections.size(); ++i)
                {
                    if(server->m_connections[i] != connection) continue;
                    server->m_connections.swap_erase(server->m_connections.begin() + i);
                    break;
                }
                memdelete(connection);
            }

            RV update_connection_interests(Server* server, Connection* connection)
            {
                Network::SocketEventFlag interests = Network::SocketEventFlag::none;
                if(!connection->close_after_write &&
                    !connection->peer_closed &&
                    connection->output_offset == connection->output.size())
                {
                    interests |= Network::SocketEventFlag::readable;
                }
                if(connection->output_offset < connection->output.size())
                {
                    interests |= Network::SocketEventFlag::writable;
                }
                return server->m_poller->modify(connection->token, interests);
            }

            bool append_output_with_limit(
                Connection* connection,
                const Vector<u8>& response,
                usize limit)
            {
                compact_buffer(connection->output, connection->output_offset);
                if(response.size() > limit - connection->output.size()) return false;
                append_bytes(connection->output, response.data(), response.size());
                return true;
            }

            RV queue_error(
                Server* server,
                Connection* connection,
                u16 status,
                bool close_connection)
            {
                Vector<u8> response;
                make_error_response(status, close_connection, response);
                if(!append_output_with_limit(
                    connection, response, server->m_options.max_buffered_output_size))
                {
                    return BasicError::out_of_resource();
                }
                if(close_connection) connection->close_after_write = true;
                return update_connection_interests(server, connection);
            }

            RV process_connection_input(
                Server* server,
                Connection* connection,
                usize& dispatched,
                usize dispatch_limit)
            {
                bool needs_more_input = false;
                while(dispatched < dispatch_limit && !connection->close_after_write)
                {
                    if(connection->output_offset < connection->output.size()) break;
                    usize available = connection->input.size() - connection->input_offset;
                    if(!available) break;
                    ParseOutcome outcome = parse_request(
                        connection->input.data() + connection->input_offset,
                        available,
                        connection->remote_address,
                        server->m_options);
                    if(outcome.state == ParseState::need_more)
                    {
                        if(outcome.send_continue && !connection->sent_continue)
                        {
                            static const c8 CONTINUE_RESPONSE[] = "HTTP/1.1 100 Continue\r\n\r\n";
                            Vector<u8> interim;
                            append_bytes(interim, CONTINUE_RESPONSE, sizeof(CONTINUE_RESPONSE) - 1);
                            if(!append_output_with_limit(
                                connection,
                                interim,
                                server->m_options.max_buffered_output_size))
                            {
                                return BasicError::out_of_resource();
                            }
                            connection->sent_continue = true;
                            return update_connection_interests(server, connection);
                        }
                        needs_more_input = true;
                        break;
                    }
                    if(outcome.state == ParseState::error)
                    {
                        connection->input.clear();
                        connection->input_offset = 0;
                        return queue_error(
                            server, connection, outcome.error_status, true);
                    }

                    bool request_close = outcome.request.version == HTTPVersion::http_1_0 ||
                        field_has_token(outcome.request.headers, Name("connection"), "close");
                    R<Response> handler_result = server->m_handler(outcome.request);
                    Response response;
                    bool handler_failed = !handler_result.valid();
                    if(!handler_failed) response = move(handler_result.get());
                    else response.status_code = 500;
                    bool close_after_response = request_close || response.close_connection;

                    Vector<u8> serialized;
                    bool serialized_ok = serialize_response(
                        response,
                        outcome.request.method,
                        close_after_response,
                        serialized);
                    if(!serialized_ok)
                    {
                        serialized.clear();
                        make_error_response(500, false, serialized);
                    }
                    if(!append_output_with_limit(
                        connection,
                        serialized,
                        server->m_options.max_buffered_output_size))
                    {
                        serialized.clear();
                        make_error_response(500, true, serialized);
                        append_output_with_limit(
                            connection,
                            serialized,
                            server->m_options.max_buffered_output_size);
                        connection->close_after_write = true;
                    }
                    connection->input_offset += outcome.consumed;
                    connection->sent_continue = false;
                    ++dispatched;
                    if(close_after_response) connection->close_after_write = true;
                    compact_buffer(connection->input, connection->input_offset);
                    if(connection->output.size() >= server->m_options.max_buffered_output_size)
                    {
                        break;
                    }
                }
                if(connection->peer_closed)
                {
                    usize remaining_input = connection->input.size() - connection->input_offset;
                    if(needs_more_input && remaining_input && !connection->close_after_write)
                    {
                        return queue_error(server, connection, 400, true);
                    }
                    if(!remaining_input) connection->close_after_write = true;
                }
                return update_connection_interests(server, connection);
            }

            RV receive_connection(
                Server* server,
                Connection* connection,
                usize& dispatched,
                usize dispatch_limit)
            {
                if(connection->output_offset < connection->output.size())
                {
                    return update_connection_interests(server, connection);
                }
                usize input_limit = maximum_input_size(server->m_options);
                usize received_this_event = 0;
                while(received_this_event < RECEIVE_QUOTA_PER_EVENT)
                {
                    if(connection->input_offset) compact_buffer(
                        connection->input, connection->input_offset);
                    if(connection->input.size() == input_limit)
                    {
                        return queue_error(server, connection, 413, true);
                    }
                    usize receive_size = min(
                        RECEIVE_CHUNK_SIZE,
                        min(
                            input_limit - connection->input.size(),
                            RECEIVE_QUOTA_PER_EVENT - received_this_event));
                    usize old_size = connection->input.size();
                    connection->input.resize(old_size + receive_size);
                    usize received = 0;
                    RV result = connection->socket->receive(
                        connection->input.data() + old_size,
                        receive_size,
                        &received);
                    if(failed(result))
                    {
                        connection->input.resize(old_size);
                        if(result.errcode() == BasicError::not_ready()) break;
                        if(result.errcode() == BasicError::interrupted()) continue;
                        return result;
                    }
                    connection->input.resize(old_size + received);
                    received_this_event += received;
                    if(!received)
                    {
                        connection->peer_closed = true;
                        break;
                    }
                    result = process_connection_input(
                        server, connection, dispatched, dispatch_limit);
                    if(failed(result) ||
                        dispatched >= dispatch_limit ||
                        connection->close_after_write ||
                        connection->output_offset < connection->output.size())
                    {
                        return result;
                    }
                }
                return process_connection_input(server, connection, dispatched, dispatch_limit);
            }

            RV flush_connection(Server* server, Connection* connection)
            {
                usize sent_this_event = 0;
                while(connection->output_offset < connection->output.size() &&
                    sent_this_event < SEND_QUOTA_PER_EVENT)
                {
                    usize available = connection->output.size() - connection->output_offset;
                    usize send_size = min(available, SEND_QUOTA_PER_EVENT - sent_this_event);
                    usize sent = 0;
                    RV result = connection->socket->send(
                        connection->output.data() + connection->output_offset,
                        send_size,
                        &sent);
                    if(failed(result))
                    {
                        if(result.errcode() == BasicError::not_ready()) break;
                        if(result.errcode() == BasicError::interrupted()) continue;
                        return result;
                    }
                    connection->output_offset += sent;
                    sent_this_event += sent;
                    if(!sent) break;
                }
                if(connection->output_offset == connection->output.size())
                {
                    connection->output.clear();
                    connection->output_offset = 0;
                }
                return update_connection_interests(server, connection);
            }

            RV accept_connections(Server* server, usize& accepted)
            {
                while(accepted < server->m_options.max_accepts_per_poll &&
                    server->m_connections.size() < server->m_options.max_connections)
                {
                    Network::SocketAddress remote_address = {};
                    auto accepted_result = server->m_listener->accept(remote_address);
                    if(!accepted_result.valid())
                    {
                        if(accepted_result.errcode() == BasicError::not_ready()) return ok;
                        if(accepted_result.errcode() == BasicError::interrupted()) continue;
                        return accepted_result.errcode();
                    }
                    Connection* connection = memnew<Connection>();
                    connection->socket = accepted_result.get();
                    connection->remote_address = remote_address;
                    auto token_result = server->m_poller->add(
                        connection->socket.get(),
                        Network::SocketEventFlag::readable,
                        connection);
                    if(!token_result.valid())
                    {
                        connection->socket->close();
                        memdelete(connection);
                        return token_result.errcode();
                    }
                    connection->token = token_result.get();
                    server->m_connections.push_back(connection);
                    ++accepted;
                }
                return ok;
            }
        }

        R<usize> Server::poll(u32 timeout_ms)
        {
            if(m_closed) return BasicError::bad_calling_time();
            usize dispatched = 0;

            for(usize i = 0;
                i < m_connections.size() && dispatched < m_options.max_requests_per_poll;)
            {
                Connection* connection = m_connections[i];
                RV result = process_connection_input(
                    this, connection, dispatched, m_options.max_requests_per_poll);
                if(failed(result))
                {
                    remove_connection(this, connection);
                    continue;
                }
                if(connection->close_after_write && connection->output.empty())
                {
                    remove_connection(this, connection);
                    continue;
                }
                ++i;
            }

            Vector<Network::SocketPollEvent> events(m_options.max_socket_events_per_poll);
            auto poll_result = m_poller->poll(
                Span<Network::SocketPollEvent>(events.data(), events.size()),
                dispatched ? 0 : timeout_ms);
            if(!poll_result.valid()) return poll_result.errcode();

            usize accepted = 0;
            usize event_count = poll_result.get();
            for(usize i = 0; i < event_count; ++i)
            {
                const Network::SocketPollEvent& event = events[i];
                if(event.token == m_listener_token)
                {
                    RV result = accept_connections(this, accepted);
                    if(failed(result)) return result.errcode();
                    continue;
                }

                Connection* connection = (Connection*)event.user_data;
                if(!connection || !contains_connection(m_connections, connection)) continue;
                bool remove = false;
                if(test_flags(event.events, Network::SocketEventFlag::readable) &&
                    !connection->close_after_write)
                {
                    RV result = receive_connection(
                        this,
                        connection,
                        dispatched,
                        m_options.max_requests_per_poll);
                    if(failed(result)) remove = true;
                }
                if(!remove && test_flags(event.events, Network::SocketEventFlag::writable) &&
                    connection->output_offset < connection->output.size())
                {
                    RV result = flush_connection(this, connection);
                    if(failed(result)) remove = true;
                }
                if(!remove && test_flags(event.events, Network::SocketEventFlag::error))
                {
                    Network::TCPConnectionState state = connection->socket->get_status();
                    if(state == Network::TCPConnectionState::error ||
                        state == Network::TCPConnectionState::closed) remove = true;
                }
                if(!remove && test_flags(event.events, Network::SocketEventFlag::hang_up))
                {
                    connection->peer_closed = true;
                }
                if(!remove && connection->close_after_write && connection->output.empty()) remove = true;
                if(remove) remove_connection(this, connection);
            }
            return dispatched;
        }

        RV Server::get_local_address(Network::SocketAddress& address)
        {
            if(m_closed) return BasicError::bad_calling_time();
            return m_listener->get_local_address(address);
        }

        void Server::close()
        {
            if(m_closed) return;
            if(m_listener_token)
            {
                RV remove_result = m_poller->remove(m_listener_token);
                (void)remove_result;
            }
            m_listener_token = Network::NULL_SOCKET_POLL_TOKEN;
            if(m_listener)
            {
                m_listener->close();
                m_listener.reset();
            }
            while(!m_connections.empty())
            {
                remove_connection(this, m_connections.back());
            }
            m_handler.reset();
            m_closed = true;
        }

        void Server::wake()
        {
            Ref<Network::ISocketPoller> poller = m_poller;
            if(poller) poller->wake();
        }

        LUNA_HTTP_API R<Ref<IServer>> new_server(
            const Network::SocketAddress& address,
            RequestHandler&& handler,
            const ServerOptions& options)
        {
            Ref<Server> server = new_object<Server>();
            RV result = server->init(address, move(handler), options);
            if(failed(result)) return result.errcode();
            return Ref<IServer>(server);
        }

        struct HTTPModule : public Module
        {
            virtual const c8* get_name() override { return "HTTP"; }

            virtual RV on_register() override
            {
                Meta::register_HTTP_types();
                return add_dependency_module(this, module_network());
            }
        };

        LUNA_HTTP_API Module* module_http()
        {
            static HTTPModule module;
            return &module;
        }
    }
}
