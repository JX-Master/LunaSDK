/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HTTPImpl.hpp
* @author JXMaster
* @date 2026/8/19
* @brief Internal HTTP server declarations.
*/
#pragma once
#include "../HTTP.hpp"
#include <Luna/Network/SocketPoller.hpp>
#include "HTTPImpl.generated.hpp"

namespace Luna
{
    namespace HTTP
    {
        struct Connection
        {
            Ref<Network::ITCPSocket> socket;
            Network::socket_poll_token_t token;
            Network::SocketAddress remote_address = {};
            Vector<u8> input;
            usize input_offset = 0;
            Vector<u8> output;
            usize output_offset = 0;
            bool peer_closed = false;
            bool close_after_write = false;
            bool sent_continue = false;
        };

        struct [[luna::struct("{C3CF9611-10C1-46FD-A789-3A20B3D0B526}")]] Server : IServer
        {
            luiimpl();

            Ref<Network::ITCPSocket> m_listener;
            Ref<Network::ISocketPoller> m_poller;
            Network::socket_poll_token_t m_listener_token;
            RequestHandler m_handler;
            ServerOptions m_options;
            Vector<Connection*> m_connections;
            bool m_closed = true;

            ~Server();
            RV init(
                const Network::SocketAddress& address,
                RequestHandler&& handler,
                const ServerOptions& options);
            virtual R<usize> poll(u32 timeout_ms) override;
            virtual RV get_local_address(Network::SocketAddress& address) override;
            virtual bool is_closed() override { return m_closed; }
            virtual void close() override;
            virtual void wake() override;
        };
    }
}
