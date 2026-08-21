/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HTTPTest.cpp
* @author JXMaster
* @date 2026/8/20
*/
#include <Luna/MCP/StreamableHTTP.hpp>
#include <Luna/Network/Network.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/StringUtils.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <stdio.h>

using namespace Luna;
using namespace Luna::MCP;

#define lutest luassert_always

namespace
{
    constexpr u32 RETRY_LIMIT = 10000;

    struct TestResponse
    {
        u16 status = 0;
        Vector<HTTP::Header> headers;
        String body;
    };

    Network::SocketAddress loopback_address(u16 port = 0)
    {
        Network::SocketAddress address = {};
        address.family = Network::AddressFamily::ipv4;
        address.ipv4.address = {127, 0, 0, 1};
        address.ipv4.port = port;
        return address;
    }

    Network::SocketAddress any_address(u16 port = 0)
    {
        Network::SocketAddress address = {};
        address.family = Network::AddressFamily::ipv4;
        address.ipv4.address = Network::IPV4_ADDRESS_ANY;
        address.ipv4.port = port;
        return address;
    }

    bool ascii_iequal(c8 lhs, c8 rhs)
    {
        if(lhs >= 'A' && lhs <= 'Z') lhs = lhs - 'A' + 'a';
        if(rhs >= 'A' && rhs <= 'Z') rhs = rhs - 'A' + 'a';
        return lhs == rhs;
    }

    Name lowercase_name(const c8* chars, usize size)
    {
        String value;
        value.reserve(size);
        for(usize i = 0; i < size; ++i)
        {
            c8 ch = chars[i];
            if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
            value.push_back(ch);
        }
        return Name(value);
    }

    String header_value(const TestResponse& response, const Name& name, bool* found = nullptr)
    {
        for(const HTTP::Header& header : response.headers)
        {
            if(header.name == name)
            {
                if(found) *found = true;
                return header.value;
            }
        }
        if(found) *found = false;
        return String();
    }

    Ref<Network::ITCPSocket> connect_client(HTTP::IServer* server)
    {
        Network::SocketAddress address = {};
        lupanic_if_failed(server->get_local_address(address));
        R<Ref<Network::ITCPSocket>> socket_result =
            Network::new_tcp_socket(Network::AddressFamily::ipv4);
        lupanic_if_failed(socket_result);
        Ref<Network::ITCPSocket> socket = move(socket_result.get());
        lupanic_if_failed(socket->connect(address));
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            lupanic_if_failed(server->poll(0));
            Network::TCPConnectionState state = socket->get_status();
            if(state == Network::TCPConnectionState::connected) return socket;
            lutest(state == Network::TCPConnectionState::connecting);
            sleep(1);
        }
        lutest(false);
        return Ref<Network::ITCPSocket>();
    }

    String transact(HTTP::IServer* server, const String& request)
    {
        Ref<Network::ITCPSocket> socket = connect_client(server);
        usize sent_offset = 0;
        String response;
        bool sent_all = false;
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            lupanic_if_failed(server->poll(0));
            if(sent_offset < request.size())
            {
                usize sent = 0;
                RV result = socket->send(
                    request.data() + sent_offset,
                    request.size() - sent_offset,
                    &sent);
                if(result.valid()) sent_offset += sent;
                else lutest(unwrap_errcode(result.errcode()) == E_NOT_READY);
            }
            sent_all = sent_offset == request.size();

            c8 buffer[4096];
            usize received = 0;
            RV receive_result = socket->receive(buffer, sizeof(buffer), &received);
            if(receive_result.valid())
            {
                if(received) response.append(buffer, received);
                else if(sent_all) break;
            }
            else
            {
                lutest(unwrap_errcode(receive_result.errcode()) == E_NOT_READY);
            }
            sleep(1);
        }
        socket->close();
        lutest(sent_all);
        lutest(!response.empty());
        return response;
    }

    TestResponse parse_response(const String& wire)
    {
        TestResponse response;
        usize header_end = wire.find("\r\n\r\n");
        lutest(header_end != String::npos);
        usize first_line_end = wire.find("\r\n");
        lutest(first_line_end != String::npos && first_line_end >= 12);
        lutest(!memcmp(wire.data(), "HTTP/1.1 ", 9));
        c8 status_chars[4] = {
            wire[9], wire[10], wire[11], 0};
        response.status = (u16)strtoi64(status_chars, nullptr, 10);

        usize line_start = first_line_end + 2;
        while(line_start < header_end)
        {
            usize line_end = wire.find("\r\n", line_start);
            lutest(line_end != String::npos && line_end <= header_end);
            usize colon = line_start;
            while(colon < line_end && wire[colon] != ':') ++colon;
            lutest(colon != String::npos && colon < line_end);
            usize value_start = colon + 1;
            while(value_start < line_end &&
                (wire[value_start] == ' ' || wire[value_start] == '\t')) ++value_start;
            usize value_end = line_end;
            while(value_end > value_start &&
                (wire[value_end - 1] == ' ' || wire[value_end - 1] == '\t')) --value_end;
            HTTP::Header header;
            header.name = lowercase_name(wire.data() + line_start, colon - line_start);
            header.value = String(wire.data() + value_start, value_end - value_start);
            response.headers.push_back(move(header));
            line_start = line_end + 2;
        }
        response.body = String(wire.data() + header_end + 4, wire.size() - header_end - 4);
        bool has_length = false;
        String content_length = header_value(response, Name("content-length"), &has_length);
        if(has_length)
        {
            lutest((usize)strtoi64(content_length.c_str(), nullptr, 10) == response.body.size());
        }
        return response;
    }

    String make_request(
        const c8* method,
        const c8* path,
        const String& body,
        const Vector<HTTP::Header>& headers)
    {
        String request(method);
        request.push_back(' ');
        request.append(path);
        request.append(" HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n");
        for(const HTTP::Header& header : headers)
        {
            request.append(header.name.c_str(), header.name.size());
            request.append(": ");
            request.append(header.value);
            request.append("\r\n");
        }
        c8 length[64];
        snprintf(length, sizeof(length), "Content-Length: %llu\r\n\r\n",
            (unsigned long long)body.size());
        request.append(length);
        request.append(body);
        return request;
    }

    TestResponse send(
        HTTP::IServer* server,
        const c8* method,
        const c8* path,
        const String& body,
        Vector<HTTP::Header>&& headers)
    {
        return parse_response(transact(server, make_request(method, path, body, headers)));
    }

    Vector<HTTP::Header> post_headers()
    {
        Vector<HTTP::Header> headers;
        headers.push_back({Name("content-type"), String("application/json")});
        headers.push_back(
            {Name("accept"), String("application/json, text/event-stream")});
        return headers;
    }

    String encode(const Variant& message)
    {
        R<String> json = VariantUtils::write_json(
            message, VariantUtils::JSONWriteOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    Variant modern_message(i64 id, const c8* method)
    {
        Variant message(VariantType::object);
        message["jsonrpc"] = "2.0";
        message["id"] = id;
        message["method"] = method;
        message["params"]["_meta"]["io.modelcontextprotocol/protocolVersion"] =
            MODERN_PROTOCOL_VERSION;
        message["params"]["_meta"]["io.modelcontextprotocol/clientCapabilities"] =
            Variant(VariantType::object);
        return message;
    }

    Vector<HTTP::Header> modern_headers(const c8* method)
    {
        Vector<HTTP::Header> headers = post_headers();
        headers.push_back(
            {Name("mcp-protocol-version"), String(MODERN_PROTOCOL_VERSION)});
        headers.push_back({Name("mcp-method"), String(method)});
        return headers;
    }

    Variant response_json(const TestResponse& response)
    {
        lutest(!response.body.empty());
        R<Variant> json = VariantUtils::read_json(
            response.body.data(),
            response.body.size(),
            VariantUtils::JSONReadOptions::strict());
        lupanic_if_failed(json);
        return move(json.get());
    }

    void expect_error(const TestResponse& response, u16 status, i64 code)
    {
        lutest(response.status == status);
        Variant json = response_json(response);
        lutest(json.find("error").find("code").inum() == code);
    }

    Variant legacy_initialize(i64 id)
    {
        Variant message(VariantType::object);
        message["jsonrpc"] = "2.0";
        message["id"] = id;
        message["method"] = "initialize";
        message["params"]["protocolVersion"] = LEGACY_PROTOCOL_VERSION;
        message["params"]["capabilities"] = Variant(VariantType::object);
        message["params"]["clientInfo"]["name"] = "http-test";
        message["params"]["clientInfo"]["version"] = "1.0";
        return message;
    }

    Vector<HTTP::Header> legacy_headers(const Name& session_id)
    {
        Vector<HTTP::Header> headers = post_headers();
        headers.push_back(
            {Name("mcp-protocol-version"), String(LEGACY_PROTOCOL_VERSION)});
        headers.push_back(
            {Name("mcp-session-id"), String(session_id.c_str(), session_id.size())});
        return headers;
    }

    Name initialize_session(HTTP::IServer* http_server, i64 id)
    {
        TestResponse response = send(
            http_server,
            "POST",
            "/mcp",
            encode(legacy_initialize(id)),
            post_headers());
        lutest(response.status == 200);
        lutest(response_json(response).find("result").find("protocolVersion").str() ==
            Name(LEGACY_PROTOCOL_VERSION));
        bool found = false;
        String session = header_value(response, Name("mcp-session-id"), &found);
        lutest(found && !session.empty());
        return Name(session);
    }
}

void streamable_http_test(IMCPServer* server)
{
    StreamableHTTPServerOptions options;
    options.allowed_origins.push_back("http://localhost:3000");
    R<Ref<HTTP::IServer>> invalid_address = new_streamable_http_server(
        server, any_address(), options);
    lutest(!invalid_address.valid());
    lutest(unwrap_errcode(invalid_address.errcode()) == E_BAD_ARGUMENTS);

    StreamableHTTPServerOptions invalid_endpoint_options = options;
    invalid_endpoint_options.endpoint = "mcp";
    R<Ref<HTTP::IServer>> invalid_endpoint = new_streamable_http_server(
        server, loopback_address(), invalid_endpoint_options);
    lutest(!invalid_endpoint.valid());

    R<Ref<HTTP::IServer>> server_result = new_streamable_http_server(
        server, loopback_address(), options);
    lupanic_if_failed(server_result);
    Ref<HTTP::IServer> http_server = move(server_result.get());

    Variant discover = modern_message(1000, "server/discover");
    TestResponse discover_response = send(
        http_server,
        "POST",
        "/mcp",
        encode(discover),
        modern_headers("server/discover"));
    lutest(discover_response.status == 200);
    lutest(response_json(discover_response).find("result").find("supportedVersions")[0].str() ==
        Name(MODERN_PROTOCOL_VERSION));

    Vector<HTTP::Header> session_ignored = modern_headers("server/discover");
    session_ignored.push_back({Name("mcp-session-id"), String("ignored-modern-session")});
    lutest(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1001, "server/discover")),
        move(session_ignored)).status == 200);

    Vector<HTTP::Header> bad_method = modern_headers("tools/list");
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1002, "server/discover")),
        move(bad_method)), 400, -32020);

    Vector<HTTP::Header> missing_version = post_headers();
    missing_version.push_back({Name("mcp-method"), String("server/discover")});
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1003, "server/discover")),
        move(missing_version)), 400, -32020);

    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1004, "unknown/method")),
        modern_headers("unknown/method")), 404, -32601);

    Vector<HTTP::Header> wrong_content_type;
    wrong_content_type.push_back({Name("content-type"), String("text/plain")});
    wrong_content_type.push_back(
        {Name("accept"), String("application/json, text/event-stream")});
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1005, "server/discover")),
        move(wrong_content_type)), 415, -32600);

    Vector<HTTP::Header> wrong_accept = modern_headers("server/discover");
    for(HTTP::Header& header : wrong_accept)
    {
        if(header.name == Name("accept")) header.value = "application/json";
    }
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1006, "server/discover")),
        move(wrong_accept)), 406, -32600);

    Vector<HTTP::Header> invalid_origin = modern_headers("server/discover");
    invalid_origin.push_back({Name("origin"), String("https://evil.example")});
    lutest(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1007, "server/discover")),
        move(invalid_origin)).status == 403);

    Vector<HTTP::Header> valid_origin = modern_headers("server/discover");
    valid_origin.push_back({Name("origin"), String("http://localhost:3000")});
    lutest(send(
        http_server,
        "POST",
        "/mcp",
        encode(modern_message(1008, "server/discover")),
        move(valid_origin)).status == 200);

    lutest(send(http_server, "GET", "/mcp", String(), Vector<HTTP::Header>()).status == 405);
    lutest(send(http_server, "GET", "/other", String(), Vector<HTTP::Header>()).status == 404);

    Variant notification(VariantType::object);
    notification["jsonrpc"] = "2.0";
    notification["method"] = "notifications/test";
    notification["params"]["_meta"]["io.modelcontextprotocol/protocolVersion"] =
        MODERN_PROTOCOL_VERSION;
    notification["params"]["_meta"]["io.modelcontextprotocol/clientCapabilities"] =
        Variant(VariantType::object);
    TestResponse notification_response = send(
        http_server,
        "POST",
        "/mcp",
        encode(notification),
        modern_headers("notifications/test"));
    lutest(notification_response.status == 202 && notification_response.body.empty());

    Variant call = modern_message(1010, "tools/call");
    call["params"]["name"] = "header_echo";
    call["params"]["arguments"]["region"] = "Hello, 世界";
    call["params"]["arguments"]["count"] = (i64)42;
    call["params"]["arguments"]["context"]["enabled"] = true;
    Vector<HTTP::Header> call_headers = modern_headers("tools/call");
    call_headers.push_back({Name("mcp-name"), String("header_echo")});
    call_headers.push_back(
        {Name("mcp-param-region"), String("=?base64?SGVsbG8sIOS4lueVjA==?=")});
    call_headers.push_back({Name("mcp-param-count"), String("42")});
    call_headers.push_back({Name("mcp-param-enabled"), String("true")});
    TestResponse call_response = send(
        http_server, "POST", "/mcp", encode(call), move(call_headers));
    lutest(call_response.status == 200);
    lutest(response_json(call_response).find("result").find("structuredContent")
        .find("count").inum() == 42);

    Vector<HTTP::Header> missing_param = modern_headers("tools/call");
    missing_param.push_back({Name("mcp-name"), String("header_echo")});
    missing_param.push_back(
        {Name("mcp-param-region"), String("=?base64?SGVsbG8sIOS4lueVjA==?=")});
    missing_param.push_back({Name("mcp-param-count"), String("42")});
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(call),
        move(missing_param)), 400, -32020);

    Name first_session = initialize_session(http_server, 1100);
    Name second_session = initialize_session(http_server, 1101);

    Variant initialized(VariantType::object);
    initialized["jsonrpc"] = "2.0";
    initialized["method"] = "notifications/initialized";
    TestResponse initialized_response = send(
        http_server,
        "POST",
        "/mcp",
        encode(initialized),
        legacy_headers(first_session));
    lutest(initialized_response.status == 202 && initialized_response.body.empty());

    Variant legacy_list(VariantType::object);
    legacy_list["jsonrpc"] = "2.0";
    legacy_list["id"] = (i64)1102;
    legacy_list["method"] = "tools/list";
    legacy_list["params"] = Variant(VariantType::object);
    lutest(send(
        http_server,
        "POST",
        "/mcp",
        encode(legacy_list),
        legacy_headers(first_session)).status == 200);
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(legacy_list),
        legacy_headers(second_session)), 400, -32600);

    Vector<HTTP::Header> no_session = post_headers();
    no_session.push_back(
        {Name("mcp-protocol-version"), String(LEGACY_PROTOCOL_VERSION)});
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(legacy_list),
        move(no_session)), 400, -32600);

    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(legacy_list),
        legacy_headers(Name("unknown-session"))), 404, -32600);

    Vector<HTTP::Header> delete_headers;
    delete_headers.push_back(
        {Name("mcp-protocol-version"), String(LEGACY_PROTOCOL_VERSION)});
    delete_headers.push_back(
        {Name("mcp-session-id"), String(first_session.c_str(), first_session.size())});
    lutest(send(
        http_server,
        "DELETE",
        "/mcp",
        String(),
        move(delete_headers)).status == 204);
    expect_error(send(
        http_server,
        "POST",
        "/mcp",
        encode(legacy_list),
        legacy_headers(first_session)), 404, -32600);

    http_server->close();

    StreamableHTTPServerOptions limited_options;
    limited_options.max_legacy_sessions = 1;
    limited_options.legacy_session_idle_timeout_ms = 100000;
    R<Ref<HTTP::IServer>> limited_result = new_streamable_http_server(
        server, loopback_address(), limited_options);
    lupanic_if_failed(limited_result);
    Ref<HTTP::IServer> limited_server = move(limited_result.get());
    initialize_session(limited_server, 1200);
    expect_error(send(
        limited_server,
        "POST",
        "/mcp",
        encode(legacy_initialize(1201)),
        post_headers()), 503, -32603);
    limited_server->close();

    StreamableHTTPServerOptions expiry_options;
    expiry_options.max_legacy_sessions = 1;
    expiry_options.legacy_session_idle_timeout_ms = 1;
    R<Ref<HTTP::IServer>> expiry_result = new_streamable_http_server(
        server, loopback_address(), expiry_options);
    lupanic_if_failed(expiry_result);
    Ref<HTTP::IServer> expiry_server = move(expiry_result.get());
    initialize_session(expiry_server, 1202);
    sleep(2);
    lutest(send(
        expiry_server,
        "POST",
        "/mcp",
        encode(legacy_initialize(1203)),
        post_headers()).status == 200);
    expiry_server->close();

    StreamableHTTPServerOptions policy_options;
    policy_options.endpoint = "/custom-mcp";
    policy_options.allow_requests_without_origin = false;
    policy_options.allowed_origins.push_back("http://localhost:4000");
    R<Ref<HTTP::IServer>> policy_result = new_streamable_http_server(
        server, loopback_address(), policy_options);
    lupanic_if_failed(policy_result);
    Ref<HTTP::IServer> policy_server = move(policy_result.get());
    lutest(send(
        policy_server,
        "POST",
        "/custom-mcp",
        encode(modern_message(1300, "server/discover")),
        modern_headers("server/discover")).status == 403);
    Vector<HTTP::Header> policy_headers = modern_headers("server/discover");
    policy_headers.push_back({Name("origin"), String("http://localhost:4000")});
    lutest(send(
        policy_server,
        "POST",
        "/custom-mcp",
        encode(modern_message(1301, "server/discover")),
        move(policy_headers)).status == 200);
    lutest(send(
        policy_server,
        "GET",
        "/mcp",
        String(),
        Vector<HTTP::Header>()).status == 404);
    policy_server->close();
}
