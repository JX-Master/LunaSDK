/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "../../../Network.hpp"
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
                if (m_socket != INVALID_SOCKET)
                {
                    closesocket(m_socket);
                    m_socket = INVALID_SOCKET;
                }
            }
            virtual opaque_t get_native_handle() override { return (opaque_t)m_socket; }
            virtual RV get_local_address(SocketAddress& address) override;
            virtual RV bind(const SocketAddress& address) override;
        };

        struct [[luna::struct("{42EF7CB8-B292-4837-88A4-D2E8AC156BA2}")]] TCPSocket : SocketBase, ITCPSocket
        {
            luiimpl();

            virtual RV get_remote_address(SocketAddress& address) override;
            virtual RV read(void* buffer, usize size, usize* read_bytes) override;
            virtual RV write(const void* buffer, usize size, usize* write_bytes) override;
            virtual RV listen(i32 len) override;
            virtual RV connect(const SocketAddress& address) override;
            virtual R<Ref<ITCPSocket>> accept(SocketAddress& address) override;
        };

        struct [[luna::struct("{1078150E-3383-450F-A307-C2E6538F80F1}")]] UDPSocket : SocketBase, IUDPSocket
        {
            luiimpl();

            virtual RV send_to(const void* buffer, usize size, const SocketAddress& address, usize* sent_bytes) override;
            virtual RV receive_from(void* buffer, usize size, SocketAddress* address, usize* received_bytes) override;
        };
    }
}
