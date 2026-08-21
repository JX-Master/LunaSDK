/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "../../../SocketPoller.hpp"
#include <Luna/Runtime/Vector.hpp>
#include <WinSock2.h>
#include "Socket.generated.hpp"

namespace Luna
{
    namespace Network
    {
        struct SocketBase : virtual ISocket
        {
            AddressFamily m_af;
            SocketType m_type;
            SOCKET m_socket;

            SocketBase() :
                m_af(AddressFamily::unspecified),
                m_type(SocketType::unspecified),
                m_socket(INVALID_SOCKET) {}
            ~SocketBase()
            {
                close();
            }
            virtual void close() override;
            virtual opaque_t get_native_handle() override { return (opaque_t)m_socket; }
            virtual RV get_local_address(SocketAddress& address) override;
            virtual RV bind(const SocketAddress& address) override;
        };

        struct [[luna::struct("{42EF7CB8-B292-4837-88A4-D2E8AC156BA2}")]] TCPSocket : SocketBase, ITCPSocket
        {
            luiimpl();

            TCPConnectionState m_status = TCPConnectionState::not_connected;
            ResultCode m_error = ResultCode(0);

            virtual TCPConnectionState get_status() override;
            virtual ResultCode get_error() override { return m_error; }
            virtual RV get_remote_address(SocketAddress& address) override;
            virtual RV receive(void* buffer, usize size, usize* out_received_bytes) override;
            virtual RV send(const void* buffer, usize size, usize* out_sent_bytes) override;
            virtual RV listen(i32 len) override;
            virtual RV connect(const SocketAddress& address) override;
            virtual R<Ref<ITCPSocket>> accept(SocketAddress& address) override;
        };

        struct [[luna::struct("{1078150E-3383-450F-A307-C2E6538F80F1}")]] UDPSocket : SocketBase, IUDPSocket
        {
            luiimpl();

            virtual RV send_to(const void* buffer, usize size, const SocketAddress& address, usize* out_sent_bytes) override;
            virtual RV receive_from(void* buffer, usize size, SocketAddress* address, usize* out_received_bytes) override;
        };

        struct SocketPollRegistration
        {
            Ref<ISocket> socket;
            SocketEventFlag interests = SocketEventFlag::none;
            opaque_t user_data = nullptr;
            u32 generation = 1;
            bool active = false;
        };

        struct [[luna::struct("{1E9AC64A-CFAB-4895-813C-8F935E011CD6}")]] SocketPoller : ISocketPoller
        {
            luiimpl();

            SOCKET m_wake_receiver = INVALID_SOCKET;
            SOCKET m_wake_sender = INVALID_SOCKET;
            Vector<SocketPollRegistration> m_registrations;
            Vector<u32> m_free_slots;
            Vector<WSAPOLLFD> m_poll_descriptors;
            Vector<socket_poll_token_t> m_poll_tokens;

            ~SocketPoller();
            RV init();
            virtual R<socket_poll_token_t> add(
                ISocket* socket,
                SocketEventFlag interests,
                opaque_t user_data) override;
            virtual RV modify(socket_poll_token_t token, SocketEventFlag interests) override;
            virtual RV remove(socket_poll_token_t token) override;
            virtual R<usize> poll(Span<SocketPollEvent> events, u32 timeout_ms) override;
            virtual void wake() override;
        };
    }
}
