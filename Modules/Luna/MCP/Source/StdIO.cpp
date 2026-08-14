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
                        if(unwrap_errcode(result.errcode()) == BasicError::interrupted()) continue;
                        return result.errcode();
                    }
                    if(!write_bytes)
                    {
                        return set_error(
                            BasicError::io_error(),
                            "Standard output accepted no bytes for a non-empty write");
                    }
                    offset += write_bytes;
                }
                return ok;
            }

            RV process_stdio_frame(IMCPServer* server, const String& frame)
            {
                R<String> output = server->process_json(frame.data(), frame.size());
                if(!output.valid()) return output.errcode();
                if(output.get().empty()) return ok;
                RV result = write_all_standard_output(output.get().data(), output.get().size());
                if(failed(result)) return result.errcode();
                return write_all_standard_output("\n", 1);
            }
        }

        LUNA_MCP_API RV run_stdio_server(
            IMCPServer* server,
            const StdioServerOptions& options)
        {
            if(!server || !options.max_message_size) return BasicError::bad_arguments();

            c8 read_buffer[4096];
            String frame;
            while(true)
            {
                usize read_bytes = 0;
                RV result = read_standard_input(read_buffer, sizeof(read_buffer), &read_bytes);
                if(failed(result))
                {
                    if(unwrap_errcode(result.errcode()) == BasicError::interrupted()) continue;
                    return result.errcode();
                }
                if(!read_bytes)
                {
                    if(frame.empty()) return ok;
                    return set_error(
                        BasicError::bad_data(),
                        "MCP standard input ended before the message newline delimiter");
                }

                for(usize i = 0; i < read_bytes; ++i)
                {
                    if(read_buffer[i] == '\n')
                    {
                        result = process_stdio_frame(server, frame);
                        if(failed(result)) return result.errcode();
                        frame.clear();
                    }
                    else
                    {
                        if(frame.size() == options.max_message_size)
                        {
                            return set_error(
                                BasicError::data_too_big(),
                                "MCP standard input message exceeds the configured size limit");
                        }
                        frame.push_back(read_buffer[i]);
                    }
                }
            }
        }
    }
}
