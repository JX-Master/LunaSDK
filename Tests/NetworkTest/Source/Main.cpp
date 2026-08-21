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
#include <Luna/Network/SocketPoller.hpp>
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

    bool is_retryable(ResultCode error)
    {
        return error == E_NOT_READY || error == E_INTERRUPTED;
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
            if(state != TCPConnectionState::connecting) return E_BAD_CALLING_TIME;
            retry_delay();
        }
        return E_TIMEOUT;
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
        return E_TIMEOUT;
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
            if(!sent) return E_NO_DATA;
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
            if(!received) return E_NO_DATA;
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
                if(received) return E_BAD_DATA;
                return socket->get_status() == TCPConnectionState::peer_closed ? RV(ok) : RV(E_BAD_DATA);
            }
            if(!is_retryable(result.errcode())) return result;
            retry_delay();
        }
        return E_TIMEOUT;
    }

    usize poll_events(
        ISocketPoller* poller,
        SocketPollEvent* events,
        usize event_capacity,
        u32 timeout_ms)
    {
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            auto result = poller->poll(
                Span<SocketPollEvent>(events, event_capacity),
                timeout_ms);
            if(result.valid()) return result.get();
            lutest(result.errcode() == E_INTERRUPTED);
        }
        lutest(false);
        return 0;
    }

    const SocketPollEvent* find_poll_event(
        const SocketPollEvent* events,
        usize event_count,
        socket_poll_token_t token)
    {
        for(usize i = 0; i < event_count; ++i)
        {
            if(events[i].token == token) return events + i;
        }
        return nullptr;
    }

    usize count_poll_events(
        const SocketPollEvent* events,
        usize event_count,
        socket_poll_token_t token)
    {
        usize count = 0;
        for(usize i = 0; i < event_count; ++i)
        {
            if(events[i].token == token) ++count;
        }
        return count;
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
        lutest(listener->get_error() == ResultCode(0));
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
        lutest(early_accept_result.errcode() == E_NOT_READY);

        auto client_result = new_tcp_socket(family);
        lutest(client_result.valid());
        Ref<ITCPSocket> client = client_result.get();
        lutest(client->get_status() == TCPConnectionState::not_connected);
        lutest(client->get_error() == ResultCode(0));

        c8 invalid_state_data = 0;
        usize transferred = 1;
        RV invalid_state_receive_result = client->receive(&invalid_state_data, 1, &transferred);
        lutest(failed(invalid_state_receive_result));
        lutest(invalid_state_receive_result.errcode() == E_BAD_CALLING_TIME);
        lutest(transferred == 0);
        transferred = 1;
        RV invalid_state_send_result = client->send(&invalid_state_data, 1, &transferred);
        lutest(failed(invalid_state_send_result));
        lutest(invalid_state_send_result.errcode() == E_BAD_CALLING_TIME);
        lutest(transferred == 0);
        lutest(client->get_status() == TCPConnectionState::not_connected);
        lutest(client->get_error() == ResultCode(0));

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
        lutest(client->get_error() == ResultCode(0));

        auto accepted_result = wait_for_accept(listener.get(), accepted_remote_address);
        lutest(accepted_result.valid());
        Ref<ITCPSocket> accepted = accepted_result.get();
        lutest(accepted->get_status() == TCPConnectionState::connected);
        lutest(accepted->get_error() == ResultCode(0));
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
        lutest(empty_receive_result.errcode() == E_NOT_READY);
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
        lutest(client->get_error() == ResultCode(0));
        usize sent = 1;
        RV closed_send_result = client->send(ping, sizeof(ping), &sent);
        lutest(failed(closed_send_result));
        lutest(closed_send_result.errcode() == E_BAD_CALLING_TIME);
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
        lutest(empty_receive_result.errcode() == E_NOT_READY);
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
        lutest(closed_receive_result.errcode() == E_BAD_CALLING_TIME);
        lutest(received == 0);
    }

    struct PollWakeContext
    {
        Ref<ISocketPoller> poller;
        ResultCode error = ResultCode(0);
        usize event_count = USIZE_MAX;
    };

    void poll_wake_thread(void* parameters)
    {
        PollWakeContext* context = (PollWakeContext*)parameters;
        SocketPollEvent event = {};
        auto result = context->poller->poll(Span<SocketPollEvent>(&event, 1));
        if(result.valid()) context->event_count = result.get();
        else context->error = result.errcode();
    }

    void socket_poller_wake_test()
    {
        auto poller_result = new_socket_poller();
        lutest(poller_result.valid());
        Ref<ISocketPoller> poller = poller_result.get();
        SocketPollEvent event = {};
        lutest(poll_events(poller.get(), &event, 1, 0) == 0);

        poller->wake();
        poller->wake();
        lutest(poll_events(poller.get(), &event, 1, 1000) == 0);

        PollWakeContext context;
        context.poller = poller;
        auto thread_result = new_thread(poll_wake_thread, &context, "NetworkTestPollWake");
        lutest(thread_result.valid());
        Ref<IThread> thread = thread_result.get();
        sleep(10);
        poller->wake();
        thread->wait();
        lutest(context.error == ResultCode(0));
        lutest(context.event_count == 0);

        auto socket_result = new_udp_socket(AddressFamily::ipv4);
        lutest(socket_result.valid());
        Ref<IUDPSocket> socket = move(socket_result.get());
        WeakRef<IUDPSocket> weak_socket(socket);
        auto token_result = poller->add(socket.get(), SocketEventFlag::none);
        lutest(token_result.valid());
        socket.reset();
        lutest(weak_socket.valid());
        Ref<IUDPSocket> retained_socket = weak_socket.pin();
        lutest(retained_socket.valid());
        weak_socket.reset();
        lupanic_if_failed(poller->remove(token_result.get()));
        retained_socket->close();
        retained_socket.reset();
    }

    void socket_poller_tcp_test()
    {
        auto poller_result = new_socket_poller();
        lutest(poller_result.valid());
        Ref<ISocketPoller> poller = poller_result.get();
        SocketPollEvent events[8] = {};

        auto empty_poll_result = poller->poll(Span<SocketPollEvent>());
        lutest(!empty_poll_result.valid());
        lutest(empty_poll_result.errcode() == E_BAD_ARGUMENTS);
        lutest(poll_events(poller.get(), events, 8, 0) == 0);

        auto listener_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(listener_result.valid());
        Ref<ITCPSocket> listener = listener_result.get();
        lupanic_if_failed(listener->bind(ipv4_loopback(0)));
        SocketAddress listener_address = {};
        lupanic_if_failed(listener->get_local_address(listener_address));
        lupanic_if_failed(listener->listen(I32_MAX));

        auto invalid_add_result = poller->add(listener.get(), SocketEventFlag::error);
        lutest(!invalid_add_result.valid());
        lutest(invalid_add_result.errcode() == E_BAD_ARGUMENTS);
        auto listener_token_result = poller->add(
            listener.get(),
            SocketEventFlag::readable,
            (opaque_t)(usize)0x11);
        lutest(listener_token_result.valid());
        socket_poll_token_t listener_token = listener_token_result.get();
        lutest(listener_token);
        auto duplicate_result = poller->add(listener.get(), SocketEventFlag::readable);
        lutest(!duplicate_result.valid());
        lutest(duplicate_result.errcode() == E_ALREADY_EXISTS);
        lutest(poll_events(poller.get(), events, 8, 0) == 0);

        auto client_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(client_result.valid());
        Ref<ITCPSocket> client = client_result.get();
        lupanic_if_failed(client->connect(listener_address));
        auto client_token_result = poller->add(
            client.get(),
            SocketEventFlag::writable,
            (opaque_t)(usize)0x22);
        lutest(client_token_result.valid());
        socket_poll_token_t client_token = client_token_result.get();

        bool listener_ready = false;
        bool client_connected = false;
        for(u32 i = 0; i < RETRY_LIMIT && (!listener_ready || !client_connected); ++i)
        {
            usize event_count = poll_events(poller.get(), events, 8, 1);
            for(usize event_index = 0; event_index < event_count; ++event_index)
            {
                const SocketPollEvent& event = events[event_index];
                if(event.token == listener_token)
                {
                    lutest(event.user_data == (opaque_t)(usize)0x11);
                    lutest(test_flags(event.events, SocketEventFlag::readable));
                    listener_ready = true;
                }
                else if(event.token == client_token)
                {
                    lutest(event.user_data == (opaque_t)(usize)0x22);
                    lutest(test_flags(event.events, SocketEventFlag::writable) ||
                        test_flags(event.events, SocketEventFlag::error) ||
                        test_flags(event.events, SocketEventFlag::hang_up));
                    TCPConnectionState state = client->get_status();
                    lutest(state == TCPConnectionState::connected || state == TCPConnectionState::connecting);
                    client_connected = state == TCPConnectionState::connected;
                }
            }
        }
        lutest(listener_ready);
        lutest(client_connected);

        SocketAddress remote_address = {};
        auto accepted_result = listener->accept(remote_address);
        lutest(accepted_result.valid());
        Ref<ITCPSocket> accepted = accepted_result.get();
        lupanic_if_failed(poller->modify(client_token, SocketEventFlag::none));

        lupanic_if_failed(poller->modify(
            client_token,
            SocketEventFlag::readable | SocketEventFlag::writable));
        const c8 combined_marker = 'c';
        lupanic_if_failed(send_all(accepted.get(), &combined_marker, 1));
        bool combined_event_received = false;
        for(u32 i = 0; i < RETRY_LIMIT; ++i)
        {
            usize event_count = poll_events(poller.get(), events, 8, 1);
            const SocketPollEvent* event = find_poll_event(events, event_count, client_token);
            if(event && test_flags(event->events, SocketEventFlag::readable))
            {
                lutest(test_flags(event->events, SocketEventFlag::writable));
                lutest(count_poll_events(events, event_count, client_token) == 1);
                combined_event_received = true;
                break;
            }
            retry_delay();
        }
        lutest(combined_event_received);
        c8 combined_input = 0;
        lupanic_if_failed(receive_exact(client.get(), &combined_input, 1));
        lutest(combined_input == combined_marker);
        lupanic_if_failed(poller->modify(client_token, SocketEventFlag::none));

        auto accepted_token_result = poller->add(
            accepted.get(),
            SocketEventFlag::readable,
            (opaque_t)(usize)0x33);
        lutest(accepted_token_result.valid());
        socket_poll_token_t accepted_token = accepted_token_result.get();
        RV invalid_modify_result = poller->modify(accepted_token, SocketEventFlag::hang_up);
        lutest(failed(invalid_modify_result));
        lutest(invalid_modify_result.errcode() == E_BAD_ARGUMENTS);
        lutest(poll_events(poller.get(), events, 8, 0) == 0);

        const c8 ping[] = {'p', 'i', 'n', 'g'};
        lupanic_if_failed(send_all(client.get(), ping, sizeof(ping)));
        bool accepted_readable = false;
        for(u32 i = 0; i < RETRY_LIMIT && !accepted_readable; ++i)
        {
            usize event_count = poll_events(poller.get(), events, 8, 1);
            const SocketPollEvent* event = find_poll_event(events, event_count, accepted_token);
            if(event)
            {
                lutest(event->user_data == (opaque_t)(usize)0x33);
                accepted_readable = test_flags(event->events, SocketEventFlag::readable);
            }
        }
        lutest(accepted_readable);
        c8 input[sizeof(ping)] = {};
        lupanic_if_failed(receive_exact(accepted.get(), input, sizeof(input)));
        lutest(input[0] == 'p' && input[1] == 'i' && input[2] == 'n' && input[3] == 'g');

        lupanic_if_failed(poller->modify(accepted_token, SocketEventFlag::writable));
        bool accepted_writable = false;
        for(u32 i = 0; i < RETRY_LIMIT && !accepted_writable; ++i)
        {
            usize event_count = poll_events(poller.get(), events, 8, 1);
            const SocketPollEvent* event = find_poll_event(events, event_count, accepted_token);
            if(event) accepted_writable = test_flags(event->events, SocketEventFlag::writable);
        }
        lutest(accepted_writable);
        lupanic_if_failed(poller->modify(accepted_token, SocketEventFlag::readable));

        lupanic_if_failed(poller->remove(client_token));
        RV stale_remove_result = poller->remove(client_token);
        lutest(failed(stale_remove_result));
        lutest(stale_remove_result.errcode() == E_NOT_FOUND);
        client->close();

        bool accepted_hang_up = false;
        for(u32 i = 0; i < RETRY_LIMIT && !accepted_hang_up; ++i)
        {
            usize event_count = poll_events(poller.get(), events, 8, 1);
            const SocketPollEvent* event = find_poll_event(events, event_count, accepted_token);
            if(event)
            {
                accepted_hang_up = test_flags(event->events, SocketEventFlag::readable) ||
                    test_flags(event->events, SocketEventFlag::hang_up);
            }
        }
        lutest(accepted_hang_up);
        lupanic_if_failed(wait_for_peer_closed(accepted.get()));

        lupanic_if_failed(poller->remove(accepted_token));
        lupanic_if_failed(poller->remove(listener_token));
        auto reused_token_result = poller->add(listener.get(), SocketEventFlag::none);
        lutest(reused_token_result.valid());
        socket_poll_token_t reused_token = reused_token_result.get();
        lutest(reused_token != listener_token);
        RV stale_modify_result = poller->modify(listener_token, SocketEventFlag::readable);
        lutest(failed(stale_modify_result));
        lutest(stale_modify_result.errcode() == E_NOT_FOUND);
        duplicate_result = poller->add(listener.get(), SocketEventFlag::readable);
        lutest(!duplicate_result.valid());
        lutest(duplicate_result.errcode() == E_ALREADY_EXISTS);
        lupanic_if_failed(poller->remove(reused_token));

        accepted->close();
        listener->close();
    }

    void socket_poller_udp_test()
    {
        auto poller_result = new_socket_poller();
        lutest(poller_result.valid());
        Ref<ISocketPoller> poller = poller_result.get();
        auto receiver_result = new_udp_socket(AddressFamily::ipv4);
        lutest(receiver_result.valid());
        Ref<IUDPSocket> receiver = receiver_result.get();
        lupanic_if_failed(receiver->bind(ipv4_loopback(0)));
        SocketAddress receiver_address = {};
        lupanic_if_failed(receiver->get_local_address(receiver_address));
        auto sender_result = new_udp_socket(AddressFamily::ipv4);
        lutest(sender_result.valid());
        Ref<IUDPSocket> sender = sender_result.get();

        auto token_result = poller->add(
            receiver.get(),
            SocketEventFlag::none,
            (opaque_t)(usize)0x44);
        lutest(token_result.valid());
        socket_poll_token_t token = token_result.get();
        const c8 first_datagram = '1';
        usize sent = 0;
        lupanic_if_failed(sender->send_to(&first_datagram, 1, receiver_address, &sent));
        lutest(sent == 1);
        SocketPollEvent events[2] = {};
        lutest(poll_events(poller.get(), events, 2, 0) == 0);

        lupanic_if_failed(poller->modify(token, SocketEventFlag::readable));
        usize event_count = poll_events(poller.get(), events, 2, 1000);
        const SocketPollEvent* event = find_poll_event(events, event_count, token);
        lutest(event);
        lutest(event->user_data == (opaque_t)(usize)0x44);
        lutest(test_flags(event->events, SocketEventFlag::readable));
        c8 input = 0;
        usize received = 0;
        lupanic_if_failed(receiver->receive_from(&input, 1, nullptr, &received));
        lutest(received == 1 && input == first_datagram);

        lupanic_if_failed(poller->modify(token, SocketEventFlag::none));
        const c8 second_datagram = '2';
        lupanic_if_failed(sender->send_to(&second_datagram, 1, receiver_address, &sent));
        lutest(poll_events(poller.get(), events, 2, 0) == 0);
        lupanic_if_failed(poller->modify(token, SocketEventFlag::readable));
        event_count = poll_events(poller.get(), events, 2, 1000);
        event = find_poll_event(events, event_count, token);
        lutest(event && test_flags(event->events, SocketEventFlag::readable));
        lupanic_if_failed(receiver->receive_from(&input, 1, nullptr, &received));
        lutest(received == 1 && input == second_datagram);

        lupanic_if_failed(poller->remove(token));
        RV stale_modify_result = poller->modify(token, SocketEventFlag::readable);
        lutest(failed(stale_modify_result));
        lutest(stale_modify_result.errcode() == E_NOT_FOUND);
        sender->close();
        receiver->close();
    }

    void socket_error_path_test()
    {
        expect_socket_create_not_supported();

        auto tcp_result = new_tcp_socket(AddressFamily::ipv4);
        lutest(tcp_result.valid());
        Ref<ITCPSocket> tcp = tcp_result.get();
        lutest(tcp->get_status() == TCPConnectionState::not_connected);
        lutest(tcp->get_error() == ResultCode(0));
        SocketAddress address = {};
        RV result = tcp->get_remote_address(address);
        lutest(failed(result));
        lutest(result.errcode() == Network::E_NOT_CONNECTED);

        result = tcp->connect(ipv6_loopback(9));
        lutest(failed(result));
        lutest(tcp->get_status() == TCPConnectionState::error);
        lutest(tcp->get_error() == result.errcode());
        ResultCode cached_error = tcp->get_error();
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
        lutest(refused_connection->get_error() != ResultCode(0));
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
    socket_poller_wake_test();
    socket_poller_tcp_test();
    socket_poller_udp_test();
    socket_error_path_test();
    close();
    return 0;
}
