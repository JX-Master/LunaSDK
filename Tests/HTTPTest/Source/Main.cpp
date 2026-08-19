/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/19
*/
#include <Luna/HTTP/HTTP.hpp>
#include <Luna/Network/Network.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <stdio.h>
#include <string.h>

using namespace Luna;
using namespace Luna::HTTP;

#define lutest luassert_always

namespace
{
    constexpr u32 RETRY_LIMIT = 10000;

    Network::SocketAddress loopback_address(u16 port = 0)
    {
        Network::SocketAddress address = {};
        address.family = Network::AddressFamily::ipv4;
        address.ipv4.address = {127, 0, 0, 1};
        address.ipv4.port = port;
        return address;
    }

    Blob text_blob(const c8* text)
    {
        return Blob(text, strlen(text));
    }

    Response text_response(const c8* text)
    {
        Response response;
        response.headers.push_back({Name("content-type"), String("text/plain")});
        response.body = text_blob(text);
        return response;
    }

    usize count_text(const String& text, const c8* needle)
    {
        usize count = 0;
        usize needle_size = strlen(needle);
        usize position = 0;
        while(position <= text.size())
        {
            usize found = text.find(needle, position);
            if(found == String::npos) break;
            ++count;
            position = found + needle_size;
        }
        return count;
    }

    bool string_equal(const String& value, const c8* expected)
    {
        usize expected_size = strlen(expected);
        return value.size() == expected_size &&
            memcmp(value.data(), expected, expected_size) == 0;
    }

    Ref<Network::ITCPSocket> connect_client(IServer* server)
    {
        Network::SocketAddress address = {};
        lupanic_if_failed(server->get_local_address(address));
        auto socket_result = Network::new_tcp_socket(Network::AddressFamily::ipv4);
        lupanic_if_failed(socket_result);
        Ref<Network::ITCPSocket> socket = socket_result.get();
        lupanic_if_failed(socket->connect(address));
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            auto poll_result = server->poll(0);
            lupanic_if_failed(poll_result);
            Network::TCPConnectionState state = socket->get_status();
            if(state == Network::TCPConnectionState::connected) return socket;
            if(state == Network::TCPConnectionState::error)
            {
                lutest(false);
            }
            lutest(state == Network::TCPConnectionState::connecting);
            sleep(1);
        }
        lutest(false);
        return Ref<Network::ITCPSocket>();
    }

    void send_data(
        IServer* server,
        Network::ITCPSocket* socket,
        const void* data,
        usize size,
        usize fragment_size = USIZE_MAX)
    {
        usize offset = 0;
        u32 retries = 0;
        while(offset < size)
        {
            usize available = min(size - offset, fragment_size);
            usize sent = 0;
            RV result = socket->send((const u8*)data + offset, available, &sent);
            if(failed(result))
            {
                if((result.errcode() == BasicError::not_ready() ||
                    result.errcode() == BasicError::interrupted()) && retries++ < RETRY_LIMIT)
                {
                    lupanic_if_failed(server->poll(1));
                    continue;
                }
                lupanic_if_failed(result);
            }
            offset += sent;
            lupanic_if_failed(server->poll(0));
        }
    }

    void send_text(
        IServer* server,
        Network::ITCPSocket* socket,
        const c8* text,
        usize fragment_size = USIZE_MAX)
    {
        send_data(server, socket, text, strlen(text), fragment_size);
    }

    bool receive_available(Network::ITCPSocket* socket, String& output)
    {
        bool peer_closed = false;
        for(;;)
        {
            c8 buffer[4096];
            usize received = 0;
            RV result = socket->receive(buffer, sizeof(buffer), &received);
            if(failed(result))
            {
                if(result.errcode() == BasicError::not_ready()) break;
                if(result.errcode() == BasicError::interrupted()) continue;
                lupanic_if_failed(result);
            }
            if(!received)
            {
                peer_closed = true;
                break;
            }
            output.append(buffer, received);
        }
        return peer_closed;
    }

    bool pump_until(
        IServer* server,
        Network::ITCPSocket* socket,
        String& output,
        const c8* needle,
        usize required_count = 1,
        bool require_close = false)
    {
        bool peer_closed = false;
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            lupanic_if_failed(server->poll(1));
            peer_closed = receive_available(socket, output) || peer_closed;
            bool found = count_text(output, needle) >= required_count;
            if(found && (!require_close || peer_closed)) return peer_closed;
            sleep(1);
        }
        fprintf(stderr, "Timed out waiting for '%s' x%llu (close=%d). Received %llu bytes:\n%.*s\n",
            needle,
            (unsigned long long)required_count,
            require_close ? 1 : 0,
            (unsigned long long)output.size(),
            (int)output.size(),
            output.c_str());
        lutest(false);
        return peer_closed;
    }

    void close_client(IServer* server, Network::ITCPSocket* socket)
    {
        socket->close();
        for(u32 i = 0; i < 8; ++i) lupanic_if_failed(server->poll(0));
    }

    struct WakeContext
    {
        IServer* server = nullptr;
        ErrCode error;
        usize dispatched = USIZE_MAX;
    };

    void wake_thread(void* parameter)
    {
        WakeContext* context = (WakeContext*)parameter;
        R<usize> result = context->server->poll(U32_MAX);
        if(!result.valid()) context->error = result.errcode();
        else context->dispatched = result.get();
    }

    void wake_test()
    {
        auto server_result = new_server(
            loopback_address(),
            RequestHandler([](const Request&) -> R<Response>
            {
                return Response();
            }));
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();
        WakeContext context;
        context.server = server.get();
        auto thread_result = new_thread(wake_thread, &context, "HTTPTestWake");
        lupanic_if_failed(thread_result);
        Ref<IThread> thread = thread_result.get();
        sleep(10);
        server->wake();
        thread->wait();
        lutest(context.error == ErrCode(0));
        lutest(context.dispatched == 0);
        server->close();
    }

    void message_and_pipeline_test()
    {
        usize handled = 0;
        RequestHandler handler([&handled](const Request& request) -> R<Response>
        {
            ++handled;
            lutest(request.remote_address.family == Network::AddressFamily::ipv4);
            if(string_equal(request.path, "/basic"))
            {
                lutest(request.method == Name("GET"));
                lutest(string_equal(request.target, "/basic?a=1"));
                lutest(string_equal(request.query, "a=1"));
                lutest(request.version == HTTPVersion::http_1_1);
                lutest(request.headers.size() == 3);
                lutest(request.headers[0].name == Name("host"));
                lutest(request.headers[1].name == Name("x-test"));
                lutest(request.headers[2].name == Name("x-test"));
                Response response = text_response("basic-body");
                response.headers.push_back({Name("Content-Length"), String("999")});
                response.headers.push_back({Name("Connection"), String("close")});
                return response;
            }
            if(string_equal(request.path, "/head"))
            {
                lutest(request.method == Name("HEAD"));
                return text_response("hidden");
            }
            if(string_equal(request.path, "/echo"))
            {
                lutest(request.method == Name("POST"));
                lutest(string_equal(request.query, "x=2"));
                lutest(request.body.size() == 5);
                lutest(memcmp(request.body.data(), "abcde", 5) == 0);
                return text_response("echo-ok");
            }
            if(string_equal(request.path, "/chunk"))
            {
                lutest(request.body.size() == 9);
                lutest(memcmp(request.body.data(), "Wikipedia", 9) == 0);
                lutest(request.trailers.size() == 1);
                lutest(request.trailers[0].name == Name("x-end"));
                lutest(string_equal(request.trailers[0].value, "yes"));
                return text_response("chunk-ok");
            }
            return text_response("unexpected");
        });

        auto server_result = new_server(loopback_address(), move(handler));
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();
        Ref<Network::ITCPSocket> client = connect_client(server.get());

        const c8 requests[] =
            "GET /basic?a=1 HTTP/1.1\r\n"
            "HoSt: localhost\r\n"
            "X-Test: one\r\n"
            "x-test: two\r\n\r\n"
            "HEAD /head HTTP/1.1\r\nHost: localhost\r\n\r\n"
            "POST /echo?x=2 HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nabcde"
            "POST /chunk HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n"
            "4\r\nWiki\r\n5;ext=value\r\npedia\r\n0\r\nX-End: yes\r\n\r\n";
        send_data(server.get(), client.get(), requests, sizeof(requests) - 1, 3);

        String output;
        pump_until(server.get(), client.get(), output, "HTTP/1.1 200 ", 4);
        lutest(handled == 4);
        lutest(output.find("basic-body") != String::npos);
        lutest(output.find("content-length: 10") != String::npos);
        lutest(output.find("content-length: 6") != String::npos);
        lutest(output.find("hidden") == String::npos);
        lutest(output.find("echo-ok") != String::npos);
        lutest(output.find("chunk-ok") != String::npos);
        lutest(output.find("content-length: 999") == String::npos);
        lutest(output.find("connection: close") == String::npos);

        close_client(server.get(), client.get());
        server->close();
        lutest(server->is_closed());
        Network::SocketAddress address = {};
        RV address_result = server->get_local_address(address);
        lutest(failed(address_result));
        lutest(address_result.errcode() == BasicError::bad_calling_time());
    }

    void expect_continue_test()
    {
        usize handled = 0;
        RequestHandler handler([&handled](const Request& request) -> R<Response>
        {
            ++handled;
            lutest(string_equal(request.path, "/expect"));
            lutest(request.body.size() == 4);
            lutest(memcmp(request.body.data(), "data", 4) == 0);
            return text_response("expect-ok");
        });
        auto server_result = new_server(loopback_address(), move(handler));
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();
        Ref<Network::ITCPSocket> client = connect_client(server.get());

        send_text(
            server.get(),
            client.get(),
            "POST /expect HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n"
            "Expect: 100-continue\r\n\r\n",
            1);
        String output;
        pump_until(server.get(), client.get(), output, "HTTP/1.1 100 Continue");
        lutest(handled == 0);
        send_text(server.get(), client.get(), "data", 1);
        pump_until(server.get(), client.get(), output, "expect-ok");
        lutest(handled == 1);

        close_client(server.get(), client.get());
        server->close();
    }

    void protocol_error_test()
    {
        usize handled = 0;
        RequestHandler handler([&handled](const Request&) -> R<Response>
        {
            ++handled;
            return text_response("should-not-run");
        });
        auto server_result = new_server(loopback_address(), move(handler));
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();

        struct ErrorCase
        {
            const c8* request;
            const c8* response;
        };
        const ErrorCase cases[] =
        {
            {
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1\r\n"
                "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
                "HTTP/1.1 400 "
            },
            {
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1, 2\r\n\r\nx",
                "HTTP/1.1 400 "
            },
            {
                "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n"
                "0\r\nAuthorization: secret\r\n\r\n",
                "HTTP/1.1 400 "
            },
            {"GET / HTTP/1.1\r\n\r\n", "HTTP/1.1 400 "},
            {"GET / HTTP/1.1\nHost: localhost\n\n", "HTTP/1.1 400 "},
            {
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1\r\n"
                "Expect: something-else\r\n\r\nx",
                "HTTP/1.1 417 "
            },
            {"GET / HTTP/2.0\r\nHost: localhost\r\n\r\n", "HTTP/1.1 505 "},
        };
        for(const ErrorCase& error_case : cases)
        {
            Ref<Network::ITCPSocket> client = connect_client(server.get());
            send_text(server.get(), client.get(), error_case.request);
            String output;
            bool closed = pump_until(
                server.get(), client.get(), output, error_case.response, 1, true);
            lutest(closed);
            client->close();
        }
        lutest(handled == 0);
        server->close();
    }

    void resource_limit_test()
    {
        usize handled = 0;
        RequestHandler handler([&handled](const Request&) -> R<Response>
        {
            ++handled;
            return text_response("unexpected");
        });
        ServerOptions options;
        options.max_header_section_size = 128;
        options.max_body_size = 4;
        auto server_result = new_server(loopback_address(), move(handler), options);
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();

        Ref<Network::ITCPSocket> header_client = connect_client(server.get());
        send_text(
            server.get(),
            header_client.get(),
            "GET / HTTP/1.1\r\nHost: localhost\r\n"
            "X-Long: 012345678901234567890123456789012345678901234567890123456789"
            "012345678901234567890123456789012345678901234567890123456789\r\n\r\n");
        String header_output;
        pump_until(server.get(), header_client.get(), header_output, "HTTP/1.1 431 ", 1, true);
        header_client->close();

        Ref<Network::ITCPSocket> body_client = connect_client(server.get());
        send_text(
            server.get(),
            body_client.get(),
            "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n");
        String body_output;
        pump_until(server.get(), body_client.get(), body_output, "HTTP/1.1 413 ", 1, true);
        body_client->close();
        lutest(handled == 0);
        server->close();
    }

    void response_error_and_close_test()
    {
        RequestHandler handler([](const Request& request) -> R<Response>
        {
            if(string_equal(request.path, "/handler-error"))
            {
                return BasicError::bad_arguments();
            }
            if(string_equal(request.path, "/bad-header"))
            {
                Response response = text_response("invalid");
                response.headers.push_back({Name("x-test"), String("ok\r\ninjected: yes")});
                return response;
            }
            if(string_equal(request.path, "/no-content"))
            {
                Response response;
                response.status_code = 204;
                response.body = text_blob("must-not-be-sent");
                return response;
            }
            Response response = text_response("closing");
            response.close_connection = true;
            return response;
        });
        auto server_result = new_server(loopback_address(), move(handler));
        lupanic_if_failed(server_result);
        Ref<IServer> server = server_result.get();

        Ref<Network::ITCPSocket> client = connect_client(server.get());
        send_text(
            server.get(), client.get(),
            "GET /handler-error HTTP/1.1\r\nHost: localhost\r\n\r\n"
            "GET /bad-header HTTP/1.1\r\nHost: localhost\r\n\r\n"
            "GET /no-content HTTP/1.1\r\nHost: localhost\r\n\r\n"
            "GET /close HTTP/1.1\r\nHost: localhost\r\n\r\n");
        String output;
        pump_until(server.get(), client.get(), output, "HTTP/1.1 500 ", 2);
        pump_until(server.get(), client.get(), output, "HTTP/1.1 204 No Content\r\n\r\n");
        bool closed = pump_until(server.get(), client.get(), output, "closing", 1, true);
        lutest(closed);
        lutest(output.find("injected: yes") == String::npos);
        lutest(output.find("must-not-be-sent") == String::npos);
        lutest(output.find("connection: close") != String::npos);
        client->close();

        Ref<Network::ITCPSocket> http_1_0_client = connect_client(server.get());
        send_text(server.get(), http_1_0_client.get(), "GET /legacy HTTP/1.0\r\n\r\n");
        String legacy_output;
        closed = pump_until(server.get(), http_1_0_client.get(), legacy_output, "closing", 1, true);
        lutest(closed);
        lutest(legacy_output.find("connection: close") != String::npos);
        http_1_0_client->close();
        server->close();
    }

    void argument_validation_test()
    {
        ServerOptions options;
        options.max_connections = 0;
        auto invalid_options = new_server(
            loopback_address(),
            RequestHandler([](const Request&) -> R<Response>
            {
                return Response();
            }),
            options);
        lutest(!invalid_options.valid());
        lutest(invalid_options.errcode() == BasicError::bad_arguments());

        auto invalid_handler = new_server(
            loopback_address(), RequestHandler(), ServerOptions());
        lutest(!invalid_handler.valid());
        lutest(invalid_handler.errcode() == BasicError::bad_arguments());

        options = ServerOptions();
        options.max_body_size = 0;
        auto zero_body_server = new_server(
            loopback_address(),
            RequestHandler([](const Request&) -> R<Response>
            {
                return Response();
            }),
            options);
        lupanic_if_failed(zero_body_server);
        zero_body_server.get()->close();
    }
}

int main()
{
    init();
    lupanic_if_failed(add_modules({module_http()}));
    lupanic_if_failed(init_modules());
    argument_validation_test();
    wake_test();
    message_and_pipeline_test();
    expect_continue_test();
    protocol_error_test();
    resource_limit_test();
    response_error_and_close_test();
    close();
    return 0;
}
