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
        struct [[luna::struct("{42EF7CB8-B292-4837-88A4-D2E8AC156BA2}")]] Socket : ISocket
        {
            luiimpl();

            AddressFamily m_af;
            SOCKET m_socket;

            Socket() :
                m_socket(INVALID_SOCKET) {}
            ~Socket()
            {
                if (m_socket != INVALID_SOCKET)
                {
                    closesocket(m_socket);
                    m_socket = INVALID_SOCKET;
                }
            }
            virtual opaque_t get_native_handle() override { return (opaque_t)m_socket; }
            virtual RV read(void* buffer, usize size, usize* read_bytes) override;
            virtual RV write(const void* buffer, usize size, usize* write_bytes) override;
            virtual RV bind(const SocketAddress& address) override;
            virtual RV listen(i32 len) override;
            virtual RV connect(const SocketAddress& address) override;
            virtual R<Ref<ISocket>> accept(SocketAddress& address) override;
        };
    }
}
