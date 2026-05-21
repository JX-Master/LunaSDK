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
        struct [[luna::struct("{35d804cf-4249-491f-a3e0-c95944ad5339}")]] Socket : ISocket
        {
            luiimpl();

            AddressFamily m_af;
            int m_socket;

            Socket() :
                m_socket(-1) {}
            ~Socket()
            {
                if(m_socket != -1)
                {
                    ::close(m_socket);
                    m_socket = -1;
                }
            }
            virtual opaque_t get_native_handle() override { return (opaque_t)(usize)m_socket; }
            virtual RV read(void* buffer, usize size, usize* read_bytes) override;
            virtual RV write(const void* buffer, usize size, usize* write_bytes) override;
            virtual RV bind(const SocketAddress& address) override;
            virtual RV listen(i32 len) override;
            virtual RV connect(const SocketAddress& address) override;
            virtual R<Ref<ISocket>> accept(SocketAddress& address) override;
        };
    }
}
