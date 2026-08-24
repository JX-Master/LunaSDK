/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StdIOTest.cpp
* @author JXMaster
* @date 2026/8/14
*/
#include <Luna/MCP/MCP.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/VariantUtils/JSON.hpp>

#ifdef LUNA_PLATFORM_WINDOWS
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#else
#include <unistd.h>
#endif

using namespace Luna;
using namespace Luna::MCP;

#define lutest luassert_always

namespace
{
#ifdef LUNA_PLATFORM_WINDOWS
    void write_pipe(HANDLE pipe, const void* data, usize size)
    {
        DWORD written_bytes = 0;
        lutest(WriteFile(pipe, data, (DWORD)size, &written_bytes, nullptr));
        lutest(written_bytes == size);
    }

    String read_pipe(HANDLE pipe)
    {
        String output;
        c8 buffer[4096];
        while(true)
        {
            DWORD read_bytes = 0;
            BOOL result = ReadFile(pipe, buffer, sizeof(buffer), &read_bytes, nullptr);
            if(!result)
            {
                lutest(GetLastError() == ERROR_BROKEN_PIPE);
                break;
            }
            if(!read_bytes) break;
            output.append(buffer, (usize)read_bytes);
        }
        return output;
    }

    String run_redirected_stdio(
        IMCPServer* server,
        const String& input,
        const StdioServerOptions& options,
        RV& run_result)
    {
        HANDLE input_read = nullptr;
        HANDLE input_write = nullptr;
        HANDLE output_read = nullptr;
        HANDLE output_write = nullptr;
        lutest(CreatePipe(&input_read, &input_write, nullptr, 0));
        lutest(CreatePipe(&output_read, &output_write, nullptr, 0));
        write_pipe(input_write, input.data(), input.size());
        CloseHandle(input_write);

        HANDLE old_input = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE old_output = GetStdHandle(STD_OUTPUT_HANDLE);
        lutest(SetStdHandle(STD_INPUT_HANDLE, input_read));
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, output_write));
        run_result = run_stdio_server(server, options);
        lutest(SetStdHandle(STD_INPUT_HANDLE, old_input));
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, old_output));
        CloseHandle(input_read);
        CloseHandle(output_write);

        String output = read_pipe(output_read);
        CloseHandle(output_read);
        return output;
    }
#else
    void write_pipe(int pipe, const void* data, usize size)
    {
        const c8* bytes = (const c8*)data;
        usize offset = 0;
        while(offset < size)
        {
            ssize_t written_bytes = ::write(pipe, bytes + offset, size - offset);
            lutest(written_bytes > 0);
            offset += (usize)written_bytes;
        }
    }

    String read_pipe(int pipe)
    {
        String output;
        c8 buffer[4096];
        while(true)
        {
            ssize_t read_bytes = ::read(pipe, buffer, sizeof(buffer));
            lutest(read_bytes >= 0);
            if(!read_bytes) break;
            output.append(buffer, (usize)read_bytes);
        }
        return output;
    }

    String run_redirected_stdio(
        IMCPServer* server,
        const String& input,
        const StdioServerOptions& options,
        RV& run_result)
    {
        int input_pipe[2];
        int output_pipe[2];
        lutest(::pipe(input_pipe) == 0);
        lutest(::pipe(output_pipe) == 0);
        write_pipe(input_pipe[1], input.data(), input.size());
        ::close(input_pipe[1]);

        int old_input = ::dup(STDIN_FILENO);
        int old_output = ::dup(STDOUT_FILENO);
        lutest(old_input >= 0);
        lutest(old_output >= 0);
        lutest(::dup2(input_pipe[0], STDIN_FILENO) == STDIN_FILENO);
        lutest(::dup2(output_pipe[1], STDOUT_FILENO) == STDOUT_FILENO);
        ::close(input_pipe[0]);
        ::close(output_pipe[1]);
        run_result = run_stdio_server(server, options);
        lutest(::dup2(old_input, STDIN_FILENO) == STDIN_FILENO);
        lutest(::dup2(old_output, STDOUT_FILENO) == STDOUT_FILENO);
        ::close(old_input);
        ::close(old_output);

        String output = read_pipe(output_pipe[0]);
        ::close(output_pipe[0]);
        return output;
    }
#endif

    String request_json(i64 id, const c8* method)
    {
        Variant request(VariantType::object);
        request["jsonrpc"] = "2.0";
        request["id"] = id;
        request["method"] = method;
        request["params"]["_meta"]["io.modelcontextprotocol/protocolVersion"] =
            PROTOCOL_VERSION;
        request["params"]["_meta"]["io.modelcontextprotocol/clientCapabilities"] =
            Variant(VariantType::object);
        R<String> json = VariantUtils::write_json(
            request, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    String notification_json()
    {
        Variant notification(VariantType::object);
        notification["jsonrpc"] = "2.0";
        notification["method"] = "notifications/cancelled";
        notification["params"] = Variant(VariantType::object);
        R<String> json = VariantUtils::write_json(
            notification, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    String legacy_initialize_json(i64 id)
    {
        Variant request(VariantType::object);
        request["jsonrpc"] = "2.0";
        request["id"] = id;
        request["method"] = "initialize";
        request["params"]["protocolVersion"] = LEGACY_PROTOCOL_VERSION;
        request["params"]["capabilities"]["elicitation"]["form"] =
            Variant(VariantType::object);
        request["params"]["clientInfo"]["name"] = "stdio-legacy-client";
        request["params"]["clientInfo"]["version"] = "1.0.0";
        R<String> json = VariantUtils::write_json(
            request, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    String legacy_initialized_json()
    {
        Variant notification(VariantType::object);
        notification["jsonrpc"] = "2.0";
        notification["method"] = "notifications/initialized";
        R<String> json = VariantUtils::write_json(
            notification, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    String legacy_list_json(i64 id)
    {
        Variant request(VariantType::object);
        request["jsonrpc"] = "2.0";
        request["id"] = id;
        request["method"] = "tools/list";
        request["params"] = Variant(VariantType::object);
        R<String> json = VariantUtils::write_json(
            request, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }
}

void stdio_test(IMCPServer* server)
{
    StdioServerOptions options;
    RV run_result;
    String input = request_json(100, "server/discover");
    input.push_back('\n');
    input.append(notification_json());
    input.push_back('\n');
    input.append(request_json(101, "tools/list"));
    input.append("\r\n");
    String output = run_redirected_stdio(server, input, options, run_result);
    lupanic_if_failed(run_result);

    usize response_count = 0;
    usize line_start = 0;
    for(usize i = 0; i < output.size(); ++i)
    {
        if(output[i] != '\n') continue;
        lutest(i > line_start);
        R<Variant> response = VariantUtils::read_json(
            output.data() + line_start,
            i - line_start,
            VariantUtils::JSONReadOptions::strict());
        lupanic_if_failed(response);
        lutest(response.get().find("id").inum() == (i64)(100 + response_count));
        ++response_count;
        line_start = i + 1;
    }
    lutest(response_count == 2);
    lutest(line_start == output.size());

    StdioServerOptions small_options;
    small_options.max_message_size = 4;
    String too_large_output = run_redirected_stdio(
        server, String("12345\n"), small_options, run_result);
    lutest(!run_result.valid());
    lutest(unwrap_errcode(run_result.errcode()) == E_DATA_TOO_BIG);
    lutest(too_large_output.empty());

    StdioServerOptions exact_options;
    exact_options.max_message_size = 2;
    String exact_output = run_redirected_stdio(
        server, String("{}\n"), exact_options, run_result);
    lupanic_if_failed(run_result);
    lutest(!exact_output.empty());
    lutest(exact_output[exact_output.size() - 1] == '\n');

    String incomplete_output = run_redirected_stdio(
        server, String("{}"), options, run_result);
    lutest(!run_result.valid());
    lutest(unwrap_errcode(run_result.errcode()) == E_BAD_DATA);
    lutest(incomplete_output.empty());

    StdioServerOptions zero_options;
    zero_options.max_message_size = 0;
    RV zero_result = run_stdio_server(server, zero_options);
    lutest(!zero_result.valid());
    lutest(zero_result.errcode() == E_BAD_ARGUMENTS);
    RV null_result = run_stdio_server(nullptr, options);
    lutest(!null_result.valid());
    lutest(null_result.errcode() == E_BAD_ARGUMENTS);
}

void stdio_legacy_test(IMCPServer* server)
{
    StdioServerOptions options;
    RV run_result;
    String input = legacy_initialize_json(200);
    input.push_back('\n');
    input.append(legacy_initialized_json());
    input.push_back('\n');
    input.append(legacy_list_json(201));
    input.push_back('\n');
    String output = run_redirected_stdio(server, input, options, run_result);
    lupanic_if_failed(run_result);

    usize response_count = 0;
    usize line_start = 0;
    for(usize i = 0; i < output.size(); ++i)
    {
        if(output[i] != '\n') continue;
        R<Variant> response = VariantUtils::read_json(
            output.data() + line_start,
            i - line_start,
            VariantUtils::JSONReadOptions::strict());
        lupanic_if_failed(response);
        lutest(response.get().find("id").inum() == (i64)(200 + response_count));
        if(!response_count)
        {
            lutest(response.get().find("result").find("protocolVersion").str() ==
                Name(LEGACY_PROTOCOL_VERSION));
        }
        else
        {
            lutest(response.get().find("result").find("tools").size() == 1);
        }
        ++response_count;
        line_start = i + 1;
    }
    lutest(response_count == 2);
    lutest(line_start == output.size());
}
