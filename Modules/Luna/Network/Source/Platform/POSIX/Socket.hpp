/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "../../../Network.hpp"
#include <unistd.h>
#include "Socket.generated.hpp"

namespace Luna
{
    namespace Network
    {
        struct SocketBase : virtual ISocket
        {
            AddressFamily m_af;
            SocketType m_type;
            int m_socket;

            SocketBase() :
                m_af(AddressFamily::unspecified),
                m_type(SocketType::unspecified),
                m_socket(-1) {}
            ~SocketBase()
            {
                close();
            }
            virtual void close() override;
            virtual opaque_t get_native_handle() override { return (opaque_t)(usize)m_socket; }
            virtual RV get_local_address(SocketAddress& address) override;
            virtual RV bind(const SocketAddress& address) override;
        };

        struct [[luna::struct("{35d804cf-4249-491f-a3e0-c95944ad5339}")]] TCPSocket : SocketBase, ITCPSocket
        {
            luiimpl();

            TCPConnectionState m_status = TCPConnectionState::not_connected;
            ErrCode m_error = ErrCode(0);

            virtual TCPConnectionState get_status() override;
            virtual ErrCode get_error() override { return m_error; }
            virtual RV get_remote_address(SocketAddress& address) override;
            virtual RV receive(void* buffer, usize size, usize* out_received_bytes) override;
            virtual RV send(const void* buffer, usize size, usize* out_sent_bytes) override;
            virtual RV listen(i32 len) override;
            virtual RV connect(const SocketAddress& address) override;
            virtual R<Ref<ITCPSocket>> accept(SocketAddress& address) override;
        };

        struct [[luna::struct("{FC65F3BA-82DA-43C7-A338-9C245EA73F52}")]] UDPSocket : SocketBase, IUDPSocket
        {
            luiimpl();

            virtual RV send_to(const void* buffer, usize size, const SocketAddress& address, usize* out_sent_bytes) override;
            virtual RV receive_from(void* buffer, usize size, SocketAddress* address, usize* out_received_bytes) override;
        };
    }
}
