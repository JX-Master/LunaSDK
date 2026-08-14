/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/14
* @brief Standalone MCP standard IO test server that exports one addition tool.
*/
#include <Luna/MCP/MCP.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/StdIO.hpp>
#include <Luna/Runtime/String.hpp>

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
            if(!write_bytes) return BasicError::io_error();
            offset += write_bytes;
        }
        return ok;
    }

    void report_error(ErrCode error)
    {
        String message("MCPTestServer failed: ");
        message.append(explain(error));
        message.push_back('\n');
        write_all_standard_error(message.data(), message.size());
    }

    R<Variant> add(Frontend::IFrontend*, const Variant& arguments)
    {
        if(arguments.type() != VariantType::object)
        {
            return set_error(BasicError::bad_arguments(), "Arguments must be an object");
        }
        const Variant& a = arguments.find("a");
        const Variant& b = arguments.find("b");
        if(a.type() != VariantType::number || b.type() != VariantType::number)
        {
            return set_error(BasicError::bad_arguments(), "a and b must be numbers");
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

    RV run_test_server()
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
        return MCP::run_stdio_server(server);
    }
}

int main()
{
    init();
    RV result = add_modules({MCP::module_mcp()});
    if(succeeded(result)) result = init_modules();
    if(succeeded(result)) result = run_test_server();
    if(failed(result)) report_error(result.errcode());
    i32 exit_code = failed(result) ? 1 : 0;
    close();
    return exit_code;
}
