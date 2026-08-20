/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StreamableHTTP.hpp
* @author JXMaster
* @date 2026/8/20
* @brief Local Streamable HTTP transport for MCP servers.
*/
#pragma once
#include "MCP.hpp"
#include <Luna/HTTP/HTTP.hpp>

namespace Luna
{
    namespace MCP
    {
        //! Configures one local MCP Streamable HTTP server.
        struct StreamableHTTPServerOptions
        {
            //! Exact endpoint path accepted by the server.
            //! @details The path must begin with `/` and must not contain a query or fragment.
            String endpoint = "/mcp";
            //! Exact allowed Origin header values. An Origin-bearing request is rejected when its
            //! value is not in this list.
            Vector<String> allowed_origins;
            //! Whether native clients that omit the Origin header are accepted.
            bool allow_requests_without_origin = true;
            //! Maximum number of simultaneously retained `2025-06-18` HTTP sessions.
            usize max_legacy_sessions = 128;
            //! Idle lifetime of one legacy session in milliseconds. Zero disables idle expiry.
            u64 legacy_session_idle_timeout_ms = 30 * 60 * 1000;
            //! Resource limits and work quotas forwarded to the underlying HTTP server.
            HTTP::ServerOptions http;
        };

        //! Creates a local MCP Streamable HTTP server.
        //! @param[in] server The shared MCP server to expose.
        //! @param[in] address The IPv4 or IPv6 loopback address on which to listen. Port zero asks
        //! the platform to select an available port.
        //! @param[in] options MCP endpoint policy and HTTP server options.
        //! @return The underlying poll-driven HTTP server, or an argument, socket, or bind error.
        //! @details This transport supports buffered JSON responses for revisions `2026-07-28`
        //! and `2025-06-18`. It does not implement server-sent events, TLS, authentication, or
        //! non-loopback listeners. Except for @ref HTTP::IServer::wake, the returned object and the
        //! MCP server must be accessed from one owner thread.
        LUNA_MCP_API R<Ref<HTTP::IServer>> new_streamable_http_server(
            IMCPServer* server,
            const Network::SocketAddress& address,
            const StreamableHTTPServerOptions& options = StreamableHTTPServerOptions());
    }
}
