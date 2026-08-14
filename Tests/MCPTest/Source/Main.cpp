/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/14
*/
#include <Luna/MCP/MCP.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/VariantUtils/JSON.hpp>

using namespace Luna;
using namespace Luna::Frontend;
using namespace Luna::MCP;

#define lutest luassert_always

void stdio_test(IMCPServer* server);

namespace
{
    struct Fixture
    {
        Ref<IFrontend> frontend;
        Ref<IMCPServer> server;
    };

    Variant make_request_params(const c8* version = PROTOCOL_VERSION)
    {
        Variant params(VariantType::object);
        params["_meta"]["io.modelcontextprotocol/protocolVersion"] = version;
        params["_meta"]["io.modelcontextprotocol/clientCapabilities"] =
            Variant(VariantType::object);
        return params;
    }

    Variant make_request(const Variant& id, const c8* method, Variant&& params)
    {
        Variant request(VariantType::object);
        request["jsonrpc"] = "2.0";
        request["id"] = id;
        request["method"] = method;
        request["params"] = move(params);
        return request;
    }

    Variant process(IMCPServer* server, const Variant& request)
    {
        MessageResult result = server->process_message(request);
        lutest(result.has_response);
        lutest(result.response.type() == VariantType::object);
        return move(result.response);
    }

    Variant process_json(IMCPServer* server, const c8* json, usize size = USIZE_MAX)
    {
        R<String> output = server->process_json(json, size);
        lupanic_if_failed(output);
        lutest(!output.get().empty());
        R<Variant> response = VariantUtils::read_json(
            output.get().data(), output.get().size(), VariantUtils::JSONReadOptions::strict());
        lupanic_if_failed(response);
        return move(response.get());
    }

    void expect_error(const Variant& response, i64 code)
    {
        lutest(response.find("jsonrpc").str() == Name("2.0"));
        lutest(response.find("error").type() == VariantType::object);
        lutest(response.find("error").find("code").inum() == code);
        lutest(response.find("error").find("message").type() == VariantType::string);
    }

    ToolDesc make_tool(const c8* name, const c8* frontend_url)
    {
        ToolDesc desc;
        desc.name = name;
        desc.frontend_url = frontend_url;
        desc.input_schema = Variant(VariantType::object);
        desc.input_schema["type"] = "object";
        return desc;
    }

    Fixture make_fixture()
    {
        Fixture fixture;
        fixture.frontend = new_frontend();
        lupanic_if_failed(fixture.frontend->set_resource_function(
            "/echo",
            FunctionHandler([](IFrontend*, const Variant& arguments) -> R<Variant>
            {
                return arguments;
            })));
        lupanic_if_failed(fixture.frontend->set_resource_function(
            "/failure",
            FunctionHandler([](IFrontend*, const Variant&) -> R<Variant>
            {
                return set_error(BasicError::bad_arguments(), "handler rejected input");
            })));
        lupanic_if_failed(fixture.frontend->set_resource_function(
            "/bad-result",
            FunctionHandler([](IFrontend*, const Variant&) -> R<Variant>
            {
                const c8 invalid_utf8[] = {(c8)0xC0, (c8)0x80};
                return Variant(Name(invalid_utf8, sizeof(invalid_utf8)));
            })));

        ServerDesc desc;
        desc.name = "luna-mcp-test";
        desc.version = "1.2.3";
        desc.title = "Luna MCP Test";
        desc.description = "Test server";
        desc.website_url = "https://example.com/mcp";
        desc.instructions = "Use the exported tools for tests.";
        desc.discovery_ttl_ms = 123;
        desc.tools_ttl_ms = 456;
        desc.cache_scope = CacheScope::public_cache;
        desc.icons = Variant(VariantType::array);
        Variant icon(VariantType::object);
        icon["src"] = "https://example.com/icon.png";
        icon["mimeType"] = "image/png";
        icon["sizes"] = Variant(VariantType::array);
        icon["sizes"].push_back("32x32");
        icon["theme"] = "dark";
        desc.icons.push_back(move(icon));

        R<Ref<IMCPServer>> server = new_server(fixture.frontend, desc);
        lupanic_if_failed(server);
        fixture.server = server.get();
        return fixture;
    }

    void server_descriptor_test()
    {
        Ref<IFrontend> frontend = new_frontend();
        ServerDesc desc;
        desc.name = "server";
        desc.version = "1";

        R<Ref<IMCPServer>> null_frontend = new_server(nullptr, desc);
        lutest(!null_frontend.valid());
        lutest(unwrap_errcode(null_frontend.errcode()) == BasicError::bad_arguments());

        ServerDesc missing_name = desc;
        missing_name.name.reset();
        R<Ref<IMCPServer>> missing_name_result = new_server(frontend, missing_name);
        lutest(!missing_name_result.valid());
        lutest(unwrap_errcode(missing_name_result.errcode()) == BasicError::bad_arguments());

        ServerDesc missing_version = desc;
        missing_version.version.reset();
        R<Ref<IMCPServer>> missing_version_result = new_server(frontend, missing_version);
        lutest(!missing_version_result.valid());
        lutest(unwrap_errcode(missing_version_result.errcode()) == BasicError::bad_arguments());

        ServerDesc invalid_icons = desc;
        invalid_icons.icons = Variant(VariantType::object);
        R<Ref<IMCPServer>> invalid_icons_result = new_server(frontend, invalid_icons);
        lutest(!invalid_icons_result.valid());
        lutest(unwrap_errcode(invalid_icons_result.errcode()) == BasicError::bad_arguments());

        ServerDesc invalid_icon_theme = desc;
        invalid_icon_theme.icons = Variant(VariantType::array);
        Variant bad_icon(VariantType::object);
        bad_icon["src"] = "icon.png";
        bad_icon["theme"] = "blue";
        invalid_icon_theme.icons.push_back(move(bad_icon));
        R<Ref<IMCPServer>> invalid_theme_result = new_server(frontend, invalid_icon_theme);
        lutest(!invalid_theme_result.valid());
        lutest(unwrap_errcode(invalid_theme_result.errcode()) == BasicError::bad_arguments());

        ServerDesc invalid_utf8 = desc;
        const c8 invalid_bytes[] = {(c8)0xED, (c8)0xA0, (c8)0x80};
        invalid_utf8.instructions = Name(invalid_bytes, sizeof(invalid_bytes));
        R<Ref<IMCPServer>> invalid_utf8_result = new_server(frontend, invalid_utf8);
        lutest(!invalid_utf8_result.valid());
        lutest(unwrap_errcode(invalid_utf8_result.errcode()) == BasicError::bad_arguments());

        ServerDesc invalid_scope = desc;
        invalid_scope.cache_scope = (CacheScope)99;
        R<Ref<IMCPServer>> invalid_scope_result = new_server(frontend, invalid_scope);
        lutest(!invalid_scope_result.valid());
        lutest(unwrap_errcode(invalid_scope_result.errcode()) == BasicError::bad_arguments());

        R<Ref<IMCPServer>> default_server = new_server(frontend, desc);
        lupanic_if_failed(default_server);
        Variant default_response = process(
            default_server.get(),
            make_request(Variant((i64)1), "server/discover", make_request_params()));
        lutest(default_response.find("result").find("ttlMs").unum() == 0);
        lutest(default_response.find("result").find("cacheScope").str() == Name("private"));
    }

    void tool_registry_test(Fixture& fixture)
    {
        ToolDesc zulu = make_tool("zulu", "/echo");
        zulu.title = "Zulu title";
        zulu.description = "Zulu description";
        zulu.output_schema = Variant(VariantType::object);
        zulu.output_schema["type"] = "object";
        zulu.annotations = Variant(VariantType::object);
        zulu.annotations["title"] = "Annotated title";
        zulu.annotations["readOnlyHint"] = true;
        zulu.annotations["destructiveHint"] = false;
        zulu.annotations["idempotentHint"] = true;
        zulu.annotations["openWorldHint"] = false;
        zulu.icons = Variant(VariantType::array);
        Variant icon(VariantType::object);
        icon["src"] = "data:image/png;base64,AA==";
        icon["sizes"] = Variant(VariantType::array);
        icon["sizes"].push_back("any");
        zulu.icons.push_back(move(icon));
        zulu.metadata = Variant(VariantType::object);
        zulu.metadata["com.example/enabled"] = true;
        lupanic_if_failed(fixture.server->set_tool(move(zulu)));
        lupanic_if_failed(fixture.server->set_tool(make_tool("alpha", "/echo")));
        lupanic_if_failed(fixture.server->set_tool(make_tool("failure", "/failure")));
        lupanic_if_failed(fixture.server->set_tool(make_tool("bad_result", "/bad-result")));
        lutest(fixture.server->get_tool_count() == 4);

        RV duplicate = fixture.server->set_tool(make_tool("alpha", "/echo"));
        lutest(!duplicate.valid());
        lutest(duplicate.errcode() == BasicError::already_exists());
        lupanic_if_failed(fixture.server->set_tool(make_tool("alpha", "/echo"), true));

        c8 boundary_name_bytes[128];
        for(usize i = 0; i < sizeof(boundary_name_bytes); ++i) boundary_name_bytes[i] = 'b';
        ToolDesc boundary_name = make_tool("temporary", "/echo");
        boundary_name.name = Name(boundary_name_bytes, sizeof(boundary_name_bytes));
        Name boundary_name_value = boundary_name.name;
        lupanic_if_failed(fixture.server->set_tool(move(boundary_name)));
        lupanic_if_failed(fixture.server->remove_tool(boundary_name_value));
        lutest(fixture.server->get_tool_count() == 4);

        ToolDesc empty_name = make_tool("temporary", "/echo");
        empty_name.name.reset();
        RV empty_name_result = fixture.server->set_tool(move(empty_name));
        lutest(!empty_name_result.valid());
        lutest(unwrap_errcode(empty_name_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_char = make_tool("bad/name", "/echo");
        RV bad_char_result = fixture.server->set_tool(move(bad_char));
        lutest(!bad_char_result.valid());
        lutest(unwrap_errcode(bad_char_result.errcode()) == BasicError::bad_arguments());

        c8 long_name_bytes[129];
        for(usize i = 0; i < sizeof(long_name_bytes); ++i) long_name_bytes[i] = 'a';
        ToolDesc long_name = make_tool("temporary", "/echo");
        long_name.name = Name(long_name_bytes, sizeof(long_name_bytes));
        RV long_name_result = fixture.server->set_tool(move(long_name));
        lutest(!long_name_result.valid());
        lutest(unwrap_errcode(long_name_result.errcode()) == BasicError::bad_arguments());

        ToolDesc missing_mapping = make_tool("missing", "/missing");
        RV missing_mapping_result = fixture.server->set_tool(move(missing_mapping));
        lutest(!missing_mapping_result.valid());
        lutest(unwrap_errcode(missing_mapping_result.errcode()) == BasicError::bad_arguments());

        lupanic_if_failed(fixture.frontend->set_resource_data("/not-function", Variant()));
        ToolDesc wrong_mapping_type = make_tool("wrong_mapping", "/not-function");
        RV wrong_mapping_result = fixture.server->set_tool(move(wrong_mapping_type));
        lutest(!wrong_mapping_result.valid());
        lutest(unwrap_errcode(wrong_mapping_result.errcode()) == BasicError::bad_arguments());
        lupanic_if_failed(fixture.frontend->remove_resource("/not-function"));

        ToolDesc bad_input_schema = make_tool("bad_input", "/echo");
        bad_input_schema.input_schema["type"] = "array";
        RV bad_input_result = fixture.server->set_tool(move(bad_input_schema));
        lutest(!bad_input_result.valid());
        lutest(unwrap_errcode(bad_input_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_output_schema = make_tool("bad_output", "/echo");
        bad_output_schema.output_schema = Variant(VariantType::array);
        RV bad_output_result = fixture.server->set_tool(move(bad_output_schema));
        lutest(!bad_output_result.valid());
        lutest(unwrap_errcode(bad_output_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_annotations = make_tool("bad_annotations", "/echo");
        bad_annotations.annotations = Variant(VariantType::object);
        bad_annotations.annotations["readOnlyHint"] = "yes";
        RV bad_annotations_result = fixture.server->set_tool(move(bad_annotations));
        lutest(!bad_annotations_result.valid());
        lutest(unwrap_errcode(bad_annotations_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_icons = make_tool("bad_icons", "/echo");
        bad_icons.icons = Variant(VariantType::array);
        bad_icons.icons.push_back(Variant(VariantType::object));
        RV bad_icons_result = fixture.server->set_tool(move(bad_icons));
        lutest(!bad_icons_result.valid());
        lutest(unwrap_errcode(bad_icons_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_metadata = make_tool("bad_metadata", "/echo");
        bad_metadata.metadata = Variant(VariantType::array);
        RV bad_metadata_result = fixture.server->set_tool(move(bad_metadata));
        lutest(!bad_metadata_result.valid());
        lutest(unwrap_errcode(bad_metadata_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_metadata_key = make_tool("bad_metadata_key", "/echo");
        bad_metadata_key.metadata = Variant(VariantType::object);
        bad_metadata_key.metadata["bad key"] = true;
        RV bad_metadata_key_result = fixture.server->set_tool(move(bad_metadata_key));
        lutest(!bad_metadata_key_result.valid());
        lutest(unwrap_errcode(bad_metadata_key_result.errcode()) == BasicError::bad_arguments());

        ToolDesc bad_utf8 = make_tool("bad_utf8", "/echo");
        const c8 invalid_utf8[] = {(c8)0xF4, (c8)0x90, (c8)0x80, (c8)0x80};
        bad_utf8.description = Name(invalid_utf8, sizeof(invalid_utf8));
        RV bad_utf8_result = fixture.server->set_tool(move(bad_utf8));
        lutest(!bad_utf8_result.valid());
        lutest(unwrap_errcode(bad_utf8_result.errcode()) == BasicError::bad_arguments());

        RV empty_remove = fixture.server->remove_tool(Name());
        lutest(!empty_remove.valid());
        lutest(empty_remove.errcode() == BasicError::bad_arguments());
        lupanic_if_failed(fixture.server->remove_tool("absent"));
    }

    void discovery_test(Fixture& fixture)
    {
        Variant params = make_request_params();
        params["_meta"]["io.modelcontextprotocol/clientInfo"] = Variant(VariantType::object);
        params["_meta"]["io.modelcontextprotocol/clientInfo"]["name"] = "client";
        params["_meta"]["io.modelcontextprotocol/clientInfo"]["version"] = "1";
        params["_meta"]["io.modelcontextprotocol/clientInfo"]["description"] = "Client";
        params["_meta"]["io.modelcontextprotocol/clientInfo"]["websiteUrl"] =
            "https://example.com/client";
        params["_meta"]["progressToken"] = "token";
        params["_meta"]["io.modelcontextprotocol/logLevel"] = "info";
        Variant& capabilities =
            params["_meta"]["io.modelcontextprotocol/clientCapabilities"];
        capabilities["experimental"]["feature"] = Variant(VariantType::object);
        capabilities["roots"] = Variant(VariantType::object);
        capabilities["sampling"]["tools"] = Variant(VariantType::object);
        capabilities["elicitation"]["form"] = Variant(VariantType::object);
        capabilities["extensions"]["com.example/feature"] = Variant(VariantType::object);
        Variant response = process(
            fixture.server,
            make_request(Variant((i64)7), "server/discover", move(params)));
        lutest(response.find("id").inum() == 7);
        const Variant& result = response.find("result");
        lutest(result.find("resultType").str() == Name("complete"));
        lutest(result.find("ttlMs").unum() == 123);
        lutest(result.find("cacheScope").str() == Name("public"));
        lutest(result.find("supportedVersions").size() == 1);
        lutest(result.find("supportedVersions")[0].str() == Name(PROTOCOL_VERSION));
        lutest(result.find("capabilities").find("tools").type() == VariantType::object);
        lutest(result.find("instructions").str() == Name("Use the exported tools for tests."));
        const Variant& server_info = result.find("_meta").find(
            "io.modelcontextprotocol/serverInfo");
        lutest(server_info.find("name").str() == Name("luna-mcp-test"));
        lutest(server_info.find("version").str() == Name("1.2.3"));
        lutest(server_info.find("title").str() == Name("Luna MCP Test"));
        lutest(server_info.find("description").str() == Name("Test server"));
        lutest(server_info.find("websiteUrl").str() == Name("https://example.com/mcp"));
        lutest(server_info.find("icons").size() == 1);
    }

    void tools_list_test(Fixture& fixture)
    {
        Variant response = process(
            fixture.server,
            make_request(Variant("list-id"), "tools/list", make_request_params()));
        lutest(response.find("id").str() == Name("list-id"));
        const Variant& result = response.find("result");
        lutest(result.find("ttlMs").unum() == 456);
        lutest(result.find("cacheScope").str() == Name("public"));
        const Variant& tools = result.find("tools");
        lutest(tools.type() == VariantType::array);
        lutest(tools.size() == 4);
        lutest(tools[0].find("name").str() == Name("alpha"));
        lutest(tools[1].find("name").str() == Name("bad_result"));
        lutest(tools[2].find("name").str() == Name("failure"));
        lutest(tools[3].find("name").str() == Name("zulu"));
        const Variant& zulu = tools[3];
        lutest(zulu.find("title").str() == Name("Zulu title"));
        lutest(zulu.find("description").str() == Name("Zulu description"));
        lutest(zulu.find("inputSchema").find("type").str() == Name("object"));
        lutest(zulu.find("outputSchema").find("type").str() == Name("object"));
        lutest(zulu.find("annotations").find("readOnlyHint").boolean());
        lutest(!zulu.find("annotations").find("destructiveHint").boolean(true));
        lutest(zulu.find("annotations").find("idempotentHint").boolean());
        lutest(!zulu.find("annotations").find("openWorldHint").boolean(true));
        lutest(zulu.find("icons").size() == 1);
        lutest(zulu.find("_meta").find("com.example/enabled").boolean());
    }

    void tool_call_test(Fixture& fixture)
    {
        Variant params = make_request_params();
        params["name"] = "alpha";
        params["arguments"]["value"] = (i64)42;
        Variant response = process(
            fixture.server,
            make_request(Variant((i64)8), "tools/call", move(params)));
        const Variant& result = response.find("result");
        lutest(result.find("resultType").str() == Name("complete"));
        lutest(!result.find("isError").boolean(true));
        lutest(result.find("structuredContent").find("value").inum() == 42);
        lutest(result.find("content").size() == 1);
        lutest(result.find("content")[0].find("type").str() == Name("text"));
        R<Variant> text_value = VariantUtils::read_json(
            result.find("content")[0].find("text").c_str(),
            VariantUtils::JSONReadOptions::strict());
        lupanic_if_failed(text_value);
        lutest(text_value.get().find("value").inum() ==
            result.find("structuredContent").find("value").inum());

        Variant no_arguments = make_request_params();
        no_arguments["name"] = "alpha";
        Variant no_arguments_response = process(
            fixture.server,
            make_request(Variant((i64)9), "tools/call", move(no_arguments)));
        lutest(no_arguments_response.find("result").find("structuredContent").type() ==
            VariantType::object);
        lutest(no_arguments_response.find("result").find("structuredContent").empty());

        Variant failure_params = make_request_params();
        failure_params["name"] = "failure";
        Variant failure_response = process(
            fixture.server,
            make_request(Variant((i64)10), "tools/call", move(failure_params)));
        lutest(failure_response.find("result").find("isError").boolean());
        lutest(failure_response.find("result").find("content")[0].find("text").str() ==
            Name("handler rejected input"));

        Variant bad_result_params = make_request_params();
        bad_result_params["name"] = "bad_result";
        Variant bad_result_response = process(
            fixture.server,
            make_request(Variant((i64)11), "tools/call", move(bad_result_params)));
        expect_error(bad_result_response, -32603);
    }

    void protocol_error_test(Fixture& fixture)
    {
        Variant invalid_object(VariantType::object);
        expect_error(process(fixture.server, invalid_object), -32600);

        Variant batch(VariantType::array);
        batch.push_back(Variant(VariantType::object));
        expect_error(process(fixture.server, batch), -32600);

        Variant wrong_version = make_request_params("2099-01-01");
        Variant wrong_version_response = process(
            fixture.server,
            make_request(Variant((i64)20), "server/discover", move(wrong_version)));
        expect_error(wrong_version_response, -32022);
        lutest(wrong_version_response.find("error").find("data").find("requested").str() ==
            Name("2099-01-01"));
        lutest(wrong_version_response.find("error").find("data").find("supported")[0].str() ==
            Name(PROTOCOL_VERSION));

        Variant no_meta(VariantType::object);
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)21), "server/discover", move(no_meta))), -32602);

        Variant no_capabilities = make_request_params();
        no_capabilities["_meta"].erase("io.modelcontextprotocol/clientCapabilities");
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)22), "server/discover", move(no_capabilities))), -32602);

        Variant bad_client_info = make_request_params();
        bad_client_info["_meta"]["io.modelcontextprotocol/clientInfo"] =
            Variant(VariantType::object);
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)23), "server/discover", move(bad_client_info))), -32602);

        Variant unknown_method = process(
            fixture.server,
            make_request(Variant((i64)24), "unknown/method", make_request_params()));
        expect_error(unknown_method, -32601);

        Variant legacy_initialize(VariantType::object);
        legacy_initialize["jsonrpc"] = "2.0";
        legacy_initialize["id"] = (i64)241;
        legacy_initialize["method"] = "initialize";
        expect_error(process(fixture.server, legacy_initialize), -32601);

        Variant bad_meta_key = make_request_params();
        bad_meta_key["_meta"]["bad key"] = true;
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)242), "server/discover", move(bad_meta_key))), -32602);

        Variant bad_log_level = make_request_params();
        bad_log_level["_meta"]["io.modelcontextprotocol/logLevel"] = "verbose";
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)243), "server/discover", move(bad_log_level))), -32602);

        Variant bad_capability = make_request_params();
        bad_capability["_meta"]["io.modelcontextprotocol/clientCapabilities"]["sampling"]
            ["tools"] = true;
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)244), "server/discover", move(bad_capability))), -32602);

        Variant cursor_params = make_request_params();
        cursor_params["cursor"] = "unknown";
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)25), "tools/list", move(cursor_params))), -32602);

        Variant cursor_type = make_request_params();
        cursor_type["cursor"] = (i64)1;
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)26), "tools/list", move(cursor_type))), -32602);

        Variant unknown_tool = make_request_params();
        unknown_tool["name"] = "does_not_exist";
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)27), "tools/call", move(unknown_tool))), -32602);

        Variant missing_name = make_request_params();
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)28), "tools/call", move(missing_name))), -32602);

        Variant bad_arguments = make_request_params();
        bad_arguments["name"] = "alpha";
        bad_arguments["arguments"] = Variant(VariantType::array);
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)29), "tools/call", move(bad_arguments))), -32602);

        Variant bad_input_responses = make_request_params();
        bad_input_responses["name"] = "alpha";
        bad_input_responses["inputResponses"] = Variant(VariantType::array);
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)30), "tools/call", move(bad_input_responses))), -32602);

        Variant bad_request_state = make_request_params();
        bad_request_state["name"] = "alpha";
        bad_request_state["requestState"] = true;
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)31), "tools/call", move(bad_request_state))), -32602);

        Variant unsupported_mrtr = make_request_params();
        unsupported_mrtr["name"] = "alpha";
        unsupported_mrtr["inputResponses"] = Variant(VariantType::object);
        expect_error(process(
            fixture.server,
            make_request(Variant((i64)311), "tools/call", move(unsupported_mrtr))), -32602);

        Variant null_id = make_request_params();
        Variant null_id_request(VariantType::object);
        null_id_request["jsonrpc"] = "2.0";
        null_id_request["id"] = Variant();
        null_id_request["method"] = "server/discover";
        null_id_request["params"] = move(null_id);
        Variant null_id_response = process(fixture.server, null_id_request);
        expect_error(null_id_response, -32600);
        lutest(!null_id_response.contains("id"));

        Variant wrong_jsonrpc = make_request(
            Variant((i64)32), "server/discover", make_request_params());
        wrong_jsonrpc["jsonrpc"] = "1.0";
        expect_error(process(fixture.server, wrong_jsonrpc), -32600);

        Variant client_response(VariantType::object);
        client_response["jsonrpc"] = "2.0";
        client_response["id"] = (i64)33;
        client_response["result"] = Variant(VariantType::object);
        expect_error(process(fixture.server, client_response), -32600);

        Variant non_json_message = make_request(
            Variant((i64)34), "server/discover", make_request_params());
        const c8 invalid_message_utf8[] = {(c8)0xC1, (c8)0xBF};
        non_json_message["extension"] = Name(
            invalid_message_utf8, sizeof(invalid_message_utf8));
        Variant non_json_response = process(fixture.server, non_json_message);
        expect_error(non_json_response, -32600);
        lutest(!non_json_response.contains("id"));

        Variant notification(VariantType::object);
        notification["jsonrpc"] = "2.0";
        notification["method"] = "notifications/cancelled";
        notification["params"] = Variant(VariantType::object);
        MessageResult notification_result = fixture.server->process_message(notification);
        lutest(!notification_result.has_response);
        R<String> notification_json = VariantUtils::write_json(
            notification, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(notification_json);
        R<String> notification_output = fixture.server->process_json(
            notification_json.get().data(), notification_json.get().size());
        lupanic_if_failed(notification_output);
        lutest(notification_output.get().empty());
    }

    void strict_json_test(Fixture& fixture)
    {
        Variant parse_error = process_json(fixture.server, "{invalid");
        expect_error(parse_error, -32700);
        lutest(!parse_error.contains("id"));

        Variant comment_error = process_json(
            fixture.server,
            "{/*comment*/\"jsonrpc\":\"2.0\"}");
        expect_error(comment_error, -32700);

        Variant duplicate_key_error = process_json(fixture.server, "{\"a\":1,\"a\":2}");
        expect_error(duplicate_key_error, -32700);

        const c8 invalid_utf8[] = {'{', '"', 'x', '"', ':', '"', (c8)0xC0, (c8)0x80, '"', '}'};
        Variant utf8_error = process_json(fixture.server, invalid_utf8, sizeof(invalid_utf8));
        expect_error(utf8_error, -32700);

        Variant batch_error = process_json(fixture.server, "[]");
        expect_error(batch_error, -32600);

        R<String> compact = fixture.server->process_json(
            "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"server/discover\","
            "\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":"
            "\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}}");
        lupanic_if_failed(compact);
        for(c8 ch : compact.get()) lutest(ch != '\n');
    }

    void unavailable_mapping_test(Fixture& fixture)
    {
        lupanic_if_failed(fixture.frontend->set_resource_function(
            "/temporary",
            FunctionHandler([](IFrontend*, const Variant& value) -> R<Variant>
            {
                return value;
            })));
        lupanic_if_failed(fixture.server->set_tool(make_tool("temporary", "/temporary")));
        lupanic_if_failed(fixture.frontend->remove_resource("/temporary"));
        Variant params = make_request_params();
        params["name"] = "temporary";
        Variant response = process(
            fixture.server,
            make_request(Variant((i64)40), "tools/call", move(params)));
        expect_error(response, -32603);
        lupanic_if_failed(fixture.server->remove_tool("temporary"));
    }
}

int main()
{
    init();
    lupanic_if_failed(add_modules({module_mcp()}));
    lupanic_if_failed(init_modules());
    server_descriptor_test();
    {
        Fixture fixture = make_fixture();
        tool_registry_test(fixture);
        discovery_test(fixture);
        tools_list_test(fixture);
        tool_call_test(fixture);
        protocol_error_test(fixture);
        strict_json_test(fixture);
        unavailable_mapping_test(fixture);
        stdio_test(fixture.server);
    }
    close();
    return 0;
}
