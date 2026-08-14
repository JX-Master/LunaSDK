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
    constexpr u32 RETRY_LIMIT = 5000;

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

    bool is_retryable(ErrCode error)
    {
        return error == BasicError::not_ready() || error == BasicError::interrupted();
    }

    void retry_delay()
    {
        sleep(1);
    }

    RV wait_for_connected(ITCPSocket* socket)
    {
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            TCPConnectionState state = socket->get_status();
            if(state == TCPConnectionState::connected) return ok;
            if(state == TCPConnectionState::error) return socket->get_error();
            if(state != TCPConnectionState::connecting) return BasicError::bad_calling_time();
            retry_delay();
        }
        return BasicError::timeout();
    }

    R<Ref<ITCPSocket>> wait_for_accept(ITCPSocket* listener, SocketAddress& address)
    {
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            auto result = listener->accept(address);
            if(result.valid()) return result;
            if(!is_retryable(result.errcode())) return result.errcode();
            retry_delay();
        }
        return BasicError::timeout();
    }

    RV send_all(ITCPSocket* socket, const void* buffer, usize size)
    {
        usize total_sent = 0;
        u32 retries = 0;
        while(total_sent < size)
        {
            usize sent = 0;
            RV result = socket->send((const u8*)buffer + total_sent, size - total_sent, &sent);
            if(failed(result))
            {
                if(is_retryable(result.errcode()) && retries++ < RETRY_LIMIT)
                {
                    retry_delay();
                    continue;
                }
                return result;
            }
            if(!sent) return BasicError::no_data();
            total_sent += sent;
            retries = 0;
        }
        return ok;
    }

    RV receive_exact(ITCPSocket* socket, void* buffer, usize size)
    {
        usize total_received = 0;
        u32 retries = 0;
        while(total_received < size)
        {
            usize received = 0;
            RV result = socket->receive((u8*)buffer + total_received, size - total_received, &received);
            if(failed(result))
            {
                if(is_retryable(result.errcode()) && retries++ < RETRY_LIMIT)
                {
                    retry_delay();
                    continue;
                }
                return result;
            }
            if(!received) return BasicError::no_data();
            total_received += received;
            retries = 0;
        }
        return ok;
    }

    RV wait_for_peer_closed(ITCPSocket* socket)
    {
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            c8 data = 0;
            usize received = 0;
            RV result = socket->receive(&data, 1, &received);
            if(succeeded(result))
            {
                if(received) return BasicError::bad_data();
                return socket->get_status() == TCPConnectionState::peer_closed ? RV(ok) : RV(BasicError::bad_data());
            }
            if(!is_retryable(result.errcode())) return result;
            retry_delay();
        }
        return BasicError::timeout();
    }

    void expect_socket_create_not_supported()
    {
        auto tcp_result = new_tcp_socket(AddressFamily::unspecified);
        lutest(!tcp_result.valid());
        lutest(tcp_result.errcode() == NetworkError::address_not_supported());

        auto udp_result = new_udp_socket(AddressFamily::unspecified);
        lutest(!udp_result.valid());
        lutest(udp_result.errcode() == NetworkError::address_not_supported());
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
        RV result_code = getaddrinfo(nullptr, nullptr, &hints, result);
        lutest(failed(result_code));
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
        lutest(listener->get_status() == TCPConnectionState::not_connected);
        lutest(listener->get_error() == ErrCode(0));
        lupanic_if_failed(listener->bind(loopback_address(family, 0)));
        lutest(listener->get_status() == TCPConnectionState::not_connected);

        SocketAddress listener_address = {};
        lupanic_if_failed(listener->get_local_address(listener_address));
        lutest(listener_address.family == family);
        lutest(socket_port(listener_address) != 0);
        lupanic_if_failed(listener->listen(I32_MAX));
        lutest(listener->get_status() == TCPConnectionState::listening);

        SocketAddress accepted_remote_address = {};
        auto early_accept_result = listener->accept(accepted_remote_address);
        lutest(!early_accept_result.valid());
        lutest(early_accept_result.errcode() == BasicError::not_ready());

        auto client_result = new_tcp_socket(family);
        lutest(client_result.valid());
        Ref<ITCPSocket> client = client_result.get();
        lutest(client->get_status() == TCPConnectionState::not_connected);
        lutest(client->get_error() == ErrCode(0));

        c8 invalid_state_data = 0;
        usize transferred = 1;
        RV invalid_state_receive_result = client->receive(&invalid_state_data, 1, &transferred);
        lutest(failed(invalid_state_receive_result));
        lutest(invalid_state_receive_result.errcode() == BasicError::bad_calling_time());
        lutest(transferred == 0);
        transferred = 1;
        RV invalid_state_send_result = client->send(&invalid_state_data, 1, &transferred);
        lutest(failed(invalid_state_send_result));
        lutest(invalid_state_send_result.errcode() == BasicError::bad_calling_time());
        lutest(transferred == 0);
        lutest(client->get_status() == TCPConnectionState::not_connected);
        lutest(client->get_error() == ErrCode(0));

        transferred = 1;
        lupanic_if_failed(client->receive(nullptr, 0, &transferred));
        lutest(transferred == 0);
        transferred = 1;
        lupanic_if_failed(client->send(nullptr, 0, &transferred));
        lutest(transferred == 0);

        lupanic_if_failed(client->connect(listener_address));
        TCPConnectionState initial_connect_state = client->get_status();
        lutest(initial_connect_state == TCPConnectionState::connecting ||
            initial_connect_state == TCPConnectionState::connected);
        lupanic_if_failed(wait_for_connected(client.get()));
        lutest(client->get_error() == ErrCode(0));

        auto accepted_result = wait_for_accept(listener.get(), accepted_remote_address);
        lutest(accepted_result.valid());
        Ref<ITCPSocket> accepted = accepted_result.get();
        lutest(accepted->get_status() == TCPConnectionState::connected);
        lutest(accepted->get_error() == ErrCode(0));
        lutest(accepted_remote_address.family == family);

        SocketAddress client_local_address = {};
        lupanic_if_failed(client->get_local_address(client_local_address));
        lutest(client_local_address.family == family);
        lutest(socket_port(client_local_address) != 0);
        SocketAddress client_remote_address = {};
        lupanic_if_failed(client->get_remote_address(client_remote_address));
        lutest(client_remote_address.family == family);
        lutest(socket_port(client_remote_address) == socket_port(listener_address));
        SocketAddress accepted_local_address = {};
        lupanic_if_failed(accepted->get_local_address(accepted_local_address));
        lutest(accepted_local_address.family == family);
        lutest(socket_port(accepted_local_address) == socket_port(listener_address));

        c8 empty_buffer[1] = {};
        usize received = 1;
        RV empty_receive_result = accepted->receive(empty_buffer, sizeof(empty_buffer), &received);
        lutest(failed(empty_receive_result));
        lutest(empty_receive_result.errcode() == BasicError::not_ready());
        lutest(received == 0);
        lutest(accepted->get_status() == TCPConnectionState::connected);

        const c8 ping[] = {'p', 'i', 'n', 'g'};
        lupanic_if_failed(send_all(client.get(), ping, sizeof(ping)));
        c8 ping_input[sizeof(ping)] = {};
        lupanic_if_failed(receive_exact(accepted.get(), ping_input, sizeof(ping_input)));
        lutest(ping_input[0] == 'p' && ping_input[1] == 'i' && ping_input[2] == 'n' && ping_input[3] == 'g');

        const c8 pong[] = {'p', 'o', 'n', 'g'};
        lupanic_if_failed(send_all(accepted.get(), pong, sizeof(pong)));
        c8 pong_input[sizeof(pong)] = {};
        lupanic_if_failed(receive_exact(client.get(), pong_input, sizeof(pong_input)));
        lutest(pong_input[0] == 'p' && pong_input[1] == 'o' && pong_input[2] == 'n' && pong_input[3] == 'g');

        client->close();
        client->close();
        lutest(client->get_status() == TCPConnectionState::closed);
        lutest(client->get_error() == ErrCode(0));
        usize sent = 1;
        RV closed_send_result = client->send(ping, sizeof(ping), &sent);
        lutest(failed(closed_send_result));
        lutest(closed_send_result.errcode() == BasicError::bad_calling_time());
        lutest(sent == 0);
        lupanic_if_failed(wait_for_peer_closed(accepted.get()));
        lutest(accepted->get_status() == TCPConnectionState::peer_closed);

        accepted->close();
        listener->close();
        listener->close();
        lutest(accepted->get_status() == TCPConnectionState::closed);
        lutest(listener->get_status() == TCPConnectionState::closed);
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

        c8 input[3] = {};
        SocketAddress sender_address = {};
        usize received = 1;
        RV empty_receive_result = receiver->receive_from(input, sizeof(input), &sender_address, &received);
        lutest(failed(empty_receive_result));
        lutest(empty_receive_result.errcode() == BasicError::not_ready());
        lutest(received == 0);

        auto sender_result = new_udp_socket(family);
        lutest(sender_result.valid());
        Ref<IUDPSocket> sender = sender_result.get();
        const c8 output[] = {'u', 'd', 'p'};
        usize sent = 0;
        lupanic_if_failed(sender->send_to(output, sizeof(output), receiver_address, &sent));
        lutest(sent == sizeof(output));

        bool received_datagram = false;
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            received = 0;
            RV result = receiver->receive_from(input, sizeof(input), &sender_address, &received);
            if(succeeded(result))
            {
                received_datagram = true;
                break;
            }
            lutest(is_retryable(result.errcode()));
            retry_delay();
        }
        lutest(received_datagram);
        lutest(received == sizeof(output));
        lutest(input[0] == 'u' && input[1] == 'd' && input[2] == 'p');
        lutest(sender_address.family == family);
        lutest(socket_port(sender_address) != 0);

        sender->close();
        receiver->close();
        receiver->close();
        received = 1;
        RV closed_receive_result = receiver->receive_from(input, sizeof(input), nullptr, &received);
        lutest(failed(closed_receive_result));
        lutest(closed_receive_result.errcode() == BasicError::bad_calling_time());
        lutest(received == 0);
    }

    void socket_error_path_test()
    {
        expect_socket_create_not_supported();

        auto tcp_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(tcp_result.valid());
        Ref<ITCPSocket> tcp = tcp_result.get();
        lutest(tcp->get_status() == TCPConnectionState::not_connected);
        lutest(tcp->get_error() == ErrCode(0));
        SocketAddress address = {};
        RV result = tcp->get_remote_address(address);
        lutest(failed(result));
        lutest(result.errcode() == NetworkError::not_connected());

        result = tcp->connect(ipv6_loopback(9));
        lutest(failed(result));
        lutest(tcp->get_status() == TCPConnectionState::error);
        lutest(tcp->get_error() == result.errcode());
        ErrCode cached_error = tcp->get_error();
        tcp->close();
        lutest(tcp->get_status() == TCPConnectionState::closed);
        lutest(tcp->get_error() == cached_error);

        auto unavailable_endpoint_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(unavailable_endpoint_result.valid());
        Ref<ITCPSocket> unavailable_endpoint = unavailable_endpoint_result.get();
        lupanic_if_failed(unavailable_endpoint->bind(ipv4_loopback(0)));
        SocketAddress unavailable_address = {};
        lupanic_if_failed(unavailable_endpoint->get_local_address(unavailable_address));
        unavailable_endpoint->close();

        auto refused_connection_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(refused_connection_result.valid());
        Ref<ITCPSocket> refused_connection = refused_connection_result.get();
        result = refused_connection->connect(unavailable_address);
        if(succeeded(result))
        {
            bool observed_error = false;
            for(u32 i = 0; i < RETRY_LIMIT; ++i)
            {
                TCPConnectionState state = refused_connection->get_status();
                if(state == TCPConnectionState::error)
                {
                    observed_error = true;
                    break;
                }
                lutest(state == TCPConnectionState::connecting);
                retry_delay();
            }
            lutest(observed_error);
        }
        lutest(refused_connection->get_status() == TCPConnectionState::error);
        lutest(refused_connection->get_error() != ErrCode(0));
        cached_error = refused_connection->get_error();
        refused_connection->close();
        lutest(refused_connection->get_status() == TCPConnectionState::closed);
        lutest(refused_connection->get_error() == cached_error);

        auto udp_result = new_udp_socket(AddressFamily::ipv4);
        lutest(udp_result.valid());
        Ref<IUDPSocket> udp = udp_result.get();
        const c8 output[] = {'x'};
        usize sent = sizeof(output);
        result = udp->send_to(output, sizeof(output), ipv6_loopback(9), &sent);
        lutest(failed(result));
        lutest(sent == 0);
    }
}

int main()
{
    init();
    lupanic_if_failed(add_modules({module_network()}));
    lupanic_if_failed(init_modules());
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
