/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/14
* @brief Standalone MCP standard IO and HTTP test server that exports one addition tool.
*/
#include <Luna/MCP/StreamableHTTP.hpp>
#include <Luna/Network/Network.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/StdIO.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/StringUtils.hpp>
#include <stdio.h>

using namespace Luna;

namespace
{
    RV write_all_standard_error(const c8* data, usize size)
    {
        usize offset = 0;
        while(offset < size)
        {
            usize write_bytes = 0;
            RV result = write_standard_error(data + offset, size - offset, &write_bytes);
            if(failed(result)) return result.errcode();
            if(!write_bytes) return E_IO_ERROR;
            offset += write_bytes;
        }
        return ok;
    }

    void report_error(ResultCode error)
    {
        String message("MCPTestServer failed: ");
        message.append(explain(error));
        message.push_back('\n');
        write_all_standard_error(message.data(), message.size());
    }

    ResultCode wrap_error(ResultCode error, const c8* context)
    {
        String cause(explain(error));
        return set_error(unwrap_errcode(error), "%s: %s", context, cause.c_str());
    }

    R<Variant> add(Frontend::IFrontend*, const Variant& arguments)
    {
        if(arguments.type() != VariantType::object)
        {
            return set_error(E_BAD_ARGUMENTS, "Arguments must be an object");
        }
        const Variant& a = arguments.find("a");
        const Variant& b = arguments.find("b");
        if(a.type() != VariantType::number || b.type() != VariantType::number)
        {
            return set_error(E_BAD_ARGUMENTS, "a and b must be numbers");
        }
        Variant result(VariantType::object);
        result["sum"] = a.fnum() + b.fnum();
        return result;
    }

    MCP::ToolDesc make_add_tool()
    {
        MCP::ToolDesc tool;
        tool.name = "add";
        tool.frontend_url = "/math/add";
        tool.title = "Add numbers";
        tool.description = "Adds two numbers and returns their sum.";

        tool.input_schema = Variant(VariantType::object);
        tool.input_schema["type"] = "object";
        tool.input_schema["properties"]["a"]["type"] = "number";
        tool.input_schema["properties"]["a"]["description"] = "The first addend.";
        tool.input_schema["properties"]["b"]["type"] = "number";
        tool.input_schema["properties"]["b"]["description"] = "The second addend.";
        tool.input_schema["required"] = Variant(VariantType::array);
        tool.input_schema["required"].push_back("a");
        tool.input_schema["required"].push_back("b");
        tool.input_schema["additionalProperties"] = false;

        tool.output_schema = Variant(VariantType::object);
        tool.output_schema["type"] = "object";
        tool.output_schema["properties"]["sum"]["type"] = "number";
        tool.output_schema["properties"]["sum"]["description"] = "The sum of a and b.";
        tool.output_schema["required"] = Variant(VariantType::array);
        tool.output_schema["required"].push_back("sum");
        tool.output_schema["additionalProperties"] = false;

        tool.annotations = Variant(VariantType::object);
        tool.annotations["readOnlyHint"] = true;
        tool.annotations["destructiveHint"] = false;
        tool.annotations["idempotentHint"] = true;
        tool.annotations["openWorldHint"] = false;
        return tool;
    }

    Network::SocketAddress loopback_address(u16 port)
    {
        Network::SocketAddress address = {};
        address.family = Network::AddressFamily::ipv4;
        address.ipv4.address = {127, 0, 0, 1};
        address.ipv4.port = port;
        return address;
    }

    RV run_test_server(bool use_http, u16 port)
    {
        Ref<Frontend::IFrontend> frontend = Frontend::new_frontend();
        RV result = frontend->set_resource_function(
            "/math/add", Frontend::FunctionHandler(add));
        if(failed(result)) return result.errcode();

        MCP::ServerDesc server_desc;
        server_desc.name = "lunasdk-mcp-test-server";
        server_desc.version = "1.0.0";
        server_desc.title = "LunaSDK MCP Test Server";
        server_desc.description = "A standalone test server for the LunaSDK MCP module.";
        server_desc.instructions =
            "Use the add tool to add two numbers. Read the result from the sum field.";
        server_desc.cache_scope = MCP::CacheScope::public_cache;

        R<Ref<MCP::IMCPServer>> server_result = MCP::new_server(frontend, server_desc);
        if(!server_result.valid()) return server_result.errcode();
        Ref<MCP::IMCPServer> server = move(server_result.get());

        result = server->set_tool(make_add_tool());
        if(failed(result)) return result.errcode();
        if(!use_http) return MCP::run_stdio_server(server);

        R<Ref<HTTP::IServer>> http_result = MCP::new_streamable_http_server(
            server, loopback_address(port));
        if(!http_result.valid())
        {
            return wrap_error(http_result.errcode(), "Failed to create the HTTP server");
        }
        Ref<HTTP::IServer> http_server = move(http_result.get());
        Network::SocketAddress local_address = {};
        result = http_server->get_local_address(local_address);
        if(failed(result))
        {
            return wrap_error(result.errcode(), "Failed to query the HTTP listener address");
        }
        c8 endpoint[128];
        int endpoint_size = snprintf(
            endpoint,
            sizeof(endpoint),
            "MCPTestServer listening at http://127.0.0.1:%u/mcp\n",
            (u32)local_address.ipv4.port);
        if(endpoint_size <= 0 || (usize)endpoint_size >= sizeof(endpoint))
        {
            return E_FAILURE;
        }
        result = write_all_standard_error(endpoint, (usize)endpoint_size);
        if(failed(result))
        {
            return wrap_error(result.errcode(), "Failed to report the HTTP endpoint");
        }
        while(!http_server->is_closed())
        {
            R<usize> poll_result = http_server->poll(U32_MAX);
            if(!poll_result.valid())
            {
                return wrap_error(poll_result.errcode(), "Failed to poll the HTTP server");
            }
        }
        return ok;
    }
}

int main(int argc, char** argv)
{
    lupanic_if_failed(init());
    bool use_http = false;
    u16 port = 0;
    RV result = ok;
    if(argc == 2 && !strcmp(argv[1], "--stdio"))
    {
    }
    else if(argc == 3 && !strcmp(argv[1], "--http"))
    {
        c8* end = nullptr;
        i64 specified_port = strtoi64(argv[2], &end, 10);
        if(end != argv[2] + strlen(argv[2]) || specified_port < 0 || specified_port > 65535)
        {
            result = set_error(E_BAD_ARGUMENTS, "Invalid HTTP port");
        }
        else
        {
            use_http = true;
            port = (u16)specified_port;
        }
    }
    else if(argc != 1)
    {
        result = set_error(
            E_BAD_ARGUMENTS,
            "Usage: MCPTestServer [--stdio | --http <port>]");
    }
    if(succeeded(result)) result = add_modules({MCP::module_mcp()});
    if(succeeded(result)) result = init_modules();
    if(succeeded(result)) result = run_test_server(use_http, port);
    if(failed(result)) report_error(result.errcode());
    i32 exit_code = failed(result) ? 1 : 0;
    close();
    return exit_code;
}
