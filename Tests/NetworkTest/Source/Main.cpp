/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/10
*/
#include <Luna/Network/Network.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>

using namespace Luna;
using namespace Luna::Network;

#define lutest luassert_always

namespace
{
    SocketAddress ipv4_loopback(u16 port)
    {
        SocketAddress address = {};
        address.family = AddressFamily::ipv4;
        address.ipv4.address = {127, 0, 0, 1};
        address.ipv4.port = port;
        return address;
    }

    SocketAddress ipv6_loopback(u16 port)
    {
        SocketAddress address = {};
        address.family = AddressFamily::ipv6;
        address.ipv6.address = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
        address.ipv6.port = port;
        address.ipv6.flow_info = 0;
        address.ipv6.scope_id = 0;
        return address;
    }

    SocketAddress loopback_address(AddressFamily family, u16 port)
    {
        return family == AddressFamily::ipv4 ? ipv4_loopback(port) : ipv6_loopback(port);
    }

    u16 socket_port(const SocketAddress& address)
    {
        switch(address.family)
        {
        case AddressFamily::ipv4: return address.ipv4.port;
        case AddressFamily::ipv6: return address.ipv6.port;
        default: return 0;
        }
    }

    bool address_is_any(const SocketAddress& address)
    {
        switch(address.family)
        {
        case AddressFamily::ipv4:
            return address.ipv4.address.bytes[0] == 0 &&
                address.ipv4.address.bytes[1] == 0 &&
                address.ipv4.address.bytes[2] == 0 &&
                address.ipv4.address.bytes[3] == 0;
        case AddressFamily::ipv6:
            for(u8 byte : address.ipv6.address.bytes)
            {
                if(byte != 0) return false;
            }
            return true;
        default:
            return false;
        }
    }

    void expect_socket_create_not_supported()
    {
        auto tcp_result = new_tcp_socket(AddressFamily::unspecified);
        lutest(!tcp_result.valid());
        lutest(tcp_result.errcode() == Network::E_ADDRESS_NOT_SUPPORTED);

        auto udp_result = new_udp_socket(AddressFamily::unspecified);
        lutest(!udp_result.valid());
        lutest(udp_result.errcode() == Network::E_ADDRESS_NOT_SUPPORTED);
    }

    RV read_exact(IStream* stream, void* buffer, usize size)
    {
        usize total_read = 0;
        while(total_read < size)
        {
            usize read_bytes = 0;
            RV r = stream->read((u8*)buffer + total_read, size - total_read, &read_bytes);
            if(failed(r)) return r;
            if(!read_bytes) return E_NO_DATA;
            total_read += read_bytes;
        }
        return ok;
    }

    struct TcpServerContext
    {
        Ref<ITCPSocket> listener;
        AddressFamily family = AddressFamily::unspecified;
        ResultCode error = ResultCode(0);
    };

    void tcp_server_main(void* params)
    {
        TcpServerContext* ctx = (TcpServerContext*)params;
        SocketAddress remote_address = {};
        auto accepted_result = ctx->listener->accept(remote_address);
        if(!accepted_result.valid())
        {
            ctx->error = accepted_result.errcode();
            return;
        }
        Ref<ITCPSocket> socket = accepted_result.get();
        SocketAddress local_address = {};
        RV r = socket->get_local_address(local_address);
        if(failed(r))
        {
            ctx->error = r.errcode();
            return;
        }
        SocketAddress peer_address = {};
        r = socket->get_remote_address(peer_address);
        if(failed(r))
        {
            ctx->error = r.errcode();
            return;
        }
        if(remote_address.family != ctx->family ||
            local_address.family != ctx->family ||
            peer_address.family != ctx->family)
        {
            ctx->error = E_BAD_DATA;
            return;
        }

        c8 input[4];
        r = read_exact(socket.get(), input, sizeof(input));
        if(failed(r))
        {
            ctx->error = r.errcode();
            return;
        }
        if(input[0] != 'p' || input[1] != 'i' || input[2] != 'n' || input[3] != 'g')
        {
            ctx->error = E_BAD_DATA;
            return;
        }
        const c8 output[] = {'p', 'o', 'n', 'g'};
        usize written = 0;
        r = socket->write(output, sizeof(output), &written);
        if(failed(r))
        {
            ctx->error = r.errcode();
            return;
        }
        if(written != sizeof(output))
        {
            ctx->error = E_BAD_DATA;
        }
    }

    void address_info_query_test(AddressFamily family, const c8* node)
    {
        Vector<AddressInfo> result;
        AddressInfo hints = {};
        hints.family = family;
        hints.socktype = SocketType::stream;
        hints.protocol = Protocol::tcp;
        lupanic_if_failed(getaddrinfo(node, "80", &hints, result));
        lutest(!result.empty());
        for(const AddressInfo& info : result)
        {
            lutest(info.family == family);
            lutest(info.socktype == SocketType::stream);
            lutest(info.protocol == Protocol::tcp || info.protocol == Protocol::unspecified);
            lutest(info.addr.family == family);
            lutest(socket_port(info.addr) == 80);
        }
    }

    void address_info_passive_test(AddressFamily family)
    {
        Vector<AddressInfo> result;
        AddressInfo hints = {};
        hints.flags = AddressInfoFlag::passive;
        hints.family = family;
        hints.socktype = SocketType::dgram;
        hints.protocol = Protocol::udp;
        lupanic_if_failed(getaddrinfo(nullptr, "0", &hints, result));
        lutest(!result.empty());
        for(const AddressInfo& info : result)
        {
            lutest(info.family == family);
            lutest(info.socktype == SocketType::dgram);
            lutest(info.protocol == Protocol::udp || info.protocol == Protocol::unspecified);
            lutest(info.addr.family == family);
            lutest(socket_port(info.addr) == 0);
            lutest(address_is_any(info.addr));
        }
    }

    void address_info_error_test()
    {
        Vector<AddressInfo> result;
        AddressInfo hints = {};
        hints.family = AddressFamily::ipv4;
        hints.socktype = SocketType::stream;
        hints.protocol = Protocol::tcp;
        RV r = getaddrinfo(nullptr, nullptr, &hints, result);
        lutest(failed(r));
        lutest(result.empty());
    }

    void address_info_test()
    {
        address_info_query_test(AddressFamily::ipv4, "127.0.0.1");
        address_info_query_test(AddressFamily::ipv6, "::1");
        address_info_passive_test(AddressFamily::ipv4);
        address_info_passive_test(AddressFamily::ipv6);
        address_info_error_test();
    }

    void tcp_loopback_test(AddressFamily family)
    {
        auto listener_result = new_tcp_socket(family);
        lutest(listener_result.valid());
        Ref<ITCPSocket> listener = listener_result.get();
        lupanic_if_failed(listener->bind(loopback_address(family, 0)));
        SocketAddress listener_address = {};
        lupanic_if_failed(listener->get_local_address(listener_address));
        lutest(listener_address.family == family);
        lutest(socket_port(listener_address) != 0);
        lupanic_if_failed(listener->listen(I32_MAX));

        TcpServerContext ctx;
        ctx.listener = listener;
        ctx.family = family;
        auto thread_result = new_thread(tcp_server_main, &ctx, "NetworkTestTcpServer");
        lutest(thread_result.valid());
        Ref<IThread> thread = thread_result.get();

        auto client_result = new_tcp_socket(family);
        lutest(client_result.valid());
        Ref<ITCPSocket> client = client_result.get();
        lupanic_if_failed(client->connect(listener_address));
        SocketAddress client_local_address = {};
        lupanic_if_failed(client->get_local_address(client_local_address));
        lutest(client_local_address.family == family);
        lutest(socket_port(client_local_address) != 0);
        SocketAddress client_remote_address = {};
        lupanic_if_failed(client->get_remote_address(client_remote_address));
        lutest(client_remote_address.family == family);
        lutest(socket_port(client_remote_address) == socket_port(listener_address));

        const c8 output[] = {'p', 'i', 'n', 'g'};
        usize written = 0;
        lupanic_if_failed(client->write(output, sizeof(output), &written));
        lutest(written == sizeof(output));
        c8 input[4];
        lupanic_if_failed(read_exact(client.get(), input, sizeof(input)));
        lutest(input[0] == 'p' && input[1] == 'o' && input[2] == 'n' && input[3] == 'g');

        thread->wait();
        lutest(ctx.error == ResultCode(0));
    }

    void udp_loopback_test(AddressFamily family)
    {
        auto receiver_result = new_udp_socket(family);
        lutest(receiver_result.valid());
        Ref<IUDPSocket> receiver = receiver_result.get();
        lupanic_if_failed(receiver->bind(loopback_address(family, 0)));
        SocketAddress receiver_address = {};
        lupanic_if_failed(receiver->get_local_address(receiver_address));
        lutest(receiver_address.family == family);
        lutest(socket_port(receiver_address) != 0);

        auto sender_result = new_udp_socket(family);
        lutest(sender_result.valid());
        Ref<IUDPSocket> sender = sender_result.get();
        const c8 output[] = {'u', 'd', 'p'};
        usize sent = 0;
        lupanic_if_failed(sender->send_to(output, sizeof(output), receiver_address, &sent));
        lutest(sent == sizeof(output));

        c8 input[sizeof(output)] = {};
        SocketAddress sender_address = {};
        usize received = 0;
        lupanic_if_failed(receiver->receive_from(input, sizeof(input), &sender_address, &received));
        lutest(received == sizeof(output));
        lutest(input[0] == 'u' && input[1] == 'd' && input[2] == 'p');
        lutest(sender_address.family == family);
        lutest(socket_port(sender_address) != 0);
    }

    void socket_error_path_test()
    {
        expect_socket_create_not_supported();

        auto tcp_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(tcp_result.valid());
        Ref<ITCPSocket> tcp = tcp_result.get();
        SocketAddress address = {};
        RV r = tcp->get_remote_address(address);
        lutest(failed(r));
        lutest(r.errcode() == Network::E_NOT_CONNECTED);

        r = tcp->connect(ipv6_loopback(9));
        lutest(failed(r));

        auto udp_result = new_udp_socket(AddressFamily::ipv4);
        lutest(udp_result.valid());
        Ref<IUDPSocket> udp = udp_result.get();
        const c8 output[] = {'x'};
        usize sent = sizeof(output);
        r = udp->send_to(output, sizeof(output), ipv6_loopback(9), &sent);
        lutest(failed(r));
        lutest(sent == 0);
    }
}

int main()
{
    init();
    lupanic_if_failed(add_modules({module_network()}));
    lupanic_if_failed(init_modules());
    lutest(!strcmp(get_error_category_name(Network::ERROR_CATEGORY), "Network"));
    set_log_to_platform_enabled(true);
    set_log_to_platform_verbosity(LogVerbosity::warning);
    address_info_test();
    tcp_loopback_test(AddressFamily::ipv4);
    tcp_loopback_test(AddressFamily::ipv6);
    udp_loopback_test(AddressFamily::ipv4);
    udp_loopback_test(AddressFamily::ipv6);
    socket_error_path_test();
    close();
    return 0;
}
