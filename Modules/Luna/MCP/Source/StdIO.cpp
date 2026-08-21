/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StdIO.cpp
* @author JXMaster
* @date 2026/8/14
* @brief Newline-delimited MCP standard IO transport.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_MCP_API LUNA_EXPORT
#include "../MCP.hpp"
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/StdIO.hpp>
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    namespace MCP
    {
        namespace
        {
            RV write_all_standard_output(const c8* data, usize size)
            {
                usize offset = 0;
                while(offset < size)
                {
                    usize write_bytes = 0;
                    RV result = write_standard_output(data + offset, size - offset, &write_bytes);
                    if(failed(result))
                    {
                        if(unwrap_errcode(result.errcode()) == E_INTERRUPTED) continue;
                        return result.errcode();
                    }
                    if(!write_bytes)
                    {
                        return set_error(
                            E_IO_ERROR,
                            "Standard output accepted no bytes for a non-empty write");
                    }
                    offset += write_bytes;
                }
                return ok;
            }

            bool is_selecting_request(const Variant& message, Name& method)
            {
                if(message.type() != VariantType::object || !message.contains("id") ||
                    message.find("jsonrpc").type() != VariantType::string ||
                    message.find("jsonrpc").str() != Name("2.0") ||
                    message.find("method").type() != VariantType::string ||
                    message.contains("result") || message.contains("error"))
                {
                    return false;
                }
                const Variant& id = message.find("id");
                if(id.type() != VariantType::string && id.type() != VariantType::number)
                {
                    return false;
                }
                method = message.find("method").str();
                return true;
            }

            RV process_stdio_frame(
                IMCPServer* server,
                Ref<IMCPMessageProcessor>& processor,
                const String& frame)
            {
                R<Variant> parsed = VariantUtils::read_json(
                    frame.data(), frame.size(), VariantUtils::JSONReadOptions::strict());
                if(parsed.valid() && !processor)
                {
                    Name method;
                    if(is_selecting_request(parsed.get(), method))
                    {
                        ProtocolVersion version = method == Name("initialize") ?
                            ProtocolVersion::v2025_06_18 : ProtocolVersion::v2026_07_28;
                        R<Ref<IMCPMessageProcessor>> created =
                            server->new_message_processor(version);
                        if(!created.valid()) return created.errcode();
                        processor = move(created.get());
                    }
                }

                Ref<IMCPMessageProcessor> temporary_processor;
                IMCPMessageProcessor* target = processor;
                if(!target)
                {
                    R<Ref<IMCPMessageProcessor>> created = server->new_message_processor(
                        ProtocolVersion::v2026_07_28);
                    if(!created.valid()) return created.errcode();
                    temporary_processor = move(created.get());
                    target = temporary_processor;
                }

                String output;
                if(parsed.valid())
                {
                    MessageResult message_result = target->process_message(parsed.get());
                    if(!message_result.has_response) return ok;
                    R<String> encoded = VariantUtils::write_json(
                        message_result.response, VariantUtils::JSONWriteOptions::strict());
                    if(!encoded.valid()) return encoded.errcode();
                    output = move(encoded.get());
                }
                else
                {
                    R<String> encoded = target->process_json(frame.data(), frame.size());
                    if(!encoded.valid()) return encoded.errcode();
                    output = move(encoded.get());
                }
                if(output.empty()) return ok;
                RV result = write_all_standard_output(output.data(), output.size());
                if(failed(result)) return result.errcode();
                return write_all_standard_output("\n", 1);
            }
        }

        LUNA_MCP_API RV run_stdio_server(
            IMCPServer* server,
            const StdioServerOptions& options)
        {
            if(!server || !options.max_message_size) return E_BAD_ARGUMENTS;

            c8 read_buffer[4096];
            String frame;
            Ref<IMCPMessageProcessor> processor;
            while(true)
            {
                usize read_bytes = 0;
                RV result = read_standard_input(read_buffer, sizeof(read_buffer), &read_bytes);
                if(failed(result))
                {
                    if(unwrap_errcode(result.errcode()) == E_INTERRUPTED) continue;
                    return result.errcode();
                }
                if(!read_bytes)
                {
                    if(frame.empty()) return ok;
                    return set_error(
                        E_BAD_DATA,
                        "MCP standard input ended before the message newline delimiter");
                }

                for(usize i = 0; i < read_bytes; ++i)
                {
                    if(read_buffer[i] == '\n')
                    {
                        result = process_stdio_frame(server, processor, frame);
                        if(failed(result)) return result.errcode();
                        frame.clear();
                    }
                    else
                    {
                        if(frame.size() == options.max_message_size)
                        {
                            return set_error(
                                E_DATA_TOO_BIG,
                                "MCP standard input message exceeds the configured size limit");
                        }
                        frame.push_back(read_buffer[i]);
                    }
                }
            }
        }
    }
}
