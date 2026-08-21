/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HTTP.hpp
* @author JXMaster
* @date 2026/8/19
* @brief HTTP message types and event-driven server APIs.
*/
#pragma once
#include <Luna/Network/Network.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Functional.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Vector.hpp>
#include "HTTP.generated.hpp"

#ifndef LUNA_HTTP_API
#define LUNA_HTTP_API
#endif

namespace Luna
{
    //! @addtogroup HTTP HTTP
    //! The HTTP module provides bounded HTTP message parsing, serialization, and an event-driven
    //! origin server over non-blocking Network sockets.
    //! @{

    namespace HTTP
    {
        //! Specifies the HTTP protocol version of a received request.
        enum class HTTPVersion : u8
        {
            //! HTTP/1.0. The initial server closes the connection after its response.
            http_1_0,
            //! HTTP/1.1. The connection is persistent unless either endpoint requests closure.
            http_1_1,
        };

        //! Stores one HTTP header or trailer field line.
        struct Header
        {
            //! The field name.
            //! @details Received names are validated and normalized to lowercase ASCII. Field names
            //! are case-insensitive identifiers, so application-created names should also be lowercase.
            Name name;
            //! The field value after leading and trailing optional whitespace is removed.
            String value;
        };

        //! Stores one completely received HTTP request.
        struct Request
        {
            //! The case-sensitive HTTP method token.
            Name method;
            //! The request-target exactly as received on the request line.
            String target;
            //! The path derived from the request-target without percent-decoding or normalization.
            String path;
            //! The query derived from the request-target without the leading question mark.
            String query;
            //! The received HTTP protocol version.
            HTTPVersion version = HTTPVersion::http_1_1;
            //! Header field lines in received order. Duplicate names are preserved.
            Vector<Header> headers;
            //! Trailer field lines in received order. Duplicate names are preserved.
            Vector<Header> trailers;
            //! The decoded request content. Chunk framing is not included.
            Blob body;
            //! The remote TCP endpoint that sent this request.
            Network::SocketAddress remote_address = {};
        };

        //! Describes one HTTP response returned by a request handler.
        struct Response
        {
            //! The final HTTP status code in the range 200 through 599.
            u16 status_code = 200;
            //! Application response field lines.
            //! @details `content-length`, `transfer-encoding`, and `connection` are controlled by the
            //! server and are ignored when supplied here. Other names and all values are validated
            //! before serialization.
            Vector<Header> headers;
            //! The response content.
            Blob body;
            //! Requests connection closure after this response is completely sent.
            bool close_connection = false;
        };

        //! Handles one completely received request.
        //! @details The handler runs synchronously on the thread calling @ref IServer::poll. Returning
        //! an error generates a `500 Internal Server Error` response for this request. The handler
        //! must not call methods on the server that is dispatching it.
        using RequestHandler = Function<R<Response>(const Request& request)>;

        //! Configures resource limits and per-poll work quotas for one HTTP server.
        struct ServerOptions
        {
            //! Maximum number of simultaneously accepted client connections.
            usize max_connections = 1024;
            //! Maximum number of bytes in one request line, excluding its terminating CRLF.
            usize max_request_line_size = 8 * 1024;
            //! Maximum number of bytes in one header or trailer section, including terminating CRLFs.
            usize max_header_section_size = 64 * 1024;
            //! Maximum number of field lines in one header or trailer section.
            usize max_header_count = 100;
            //! Maximum decoded request content size in bytes.
            usize max_body_size = 16 * 1024 * 1024;
            //! Maximum unsent serialized response bytes buffered for one connection.
            usize max_buffered_output_size = 16 * 1024 * 1024 + 64 * 1024;
            //! Native listen backlog passed to the TCP socket.
            i32 listen_backlog = 128;
            //! Maximum connections accepted by one @ref IServer::poll call.
            u32 max_accepts_per_poll = 64;
            //! Maximum complete requests dispatched by one @ref IServer::poll call.
            u32 max_requests_per_poll = 64;
            //! Maximum readiness events consumed by one @ref IServer::poll call.
            u32 max_socket_events_per_poll = 256;
        };

        //! @interface IServer
        //! Runs one event-driven HTTP origin server over a non-blocking TCP listener.
        //! @details Except for @ref wake, this interface is not thread-safe. The thread calling
        //! @ref poll owns the server, all accepted sockets, and handler execution.
        struct [[Luna::interface("{1F341136-A051-4A5F-9C02-67A0E073D1CC}")]] IServer : virtual Interface
        {
            //! Performs one bounded server reactor iteration.
            //! @param[in] timeout_ms Maximum time to wait for socket readiness. `0` performs a
            //! non-blocking iteration and `U32_MAX` may wait indefinitely.
            //! @return Returns the number of complete requests dispatched during this call.
            //! @details Listener, poller, and internal server failures are returned to the caller.
            //! An individual client connection failure closes that connection without failing the
            //! server. A handler error is converted into an HTTP 500 response.
            virtual R<usize> poll(u32 timeout_ms = 0) = 0;

            //! Gets the effective local listener address.
            //! @param[out] address Receives the bound address, including an automatically assigned port.
            //! @return Returns @ref E_BAD_CALLING_TIME after the server is closed.
            virtual RV get_local_address(Network::SocketAddress& address) = 0;

            //! Tests whether this server has been closed.
            virtual bool is_closed() = 0;

            //! Closes the listener and every accepted connection.
            //! @details This operation is idempotent and must be called on the server owner thread.
            virtual void close() = 0;

            //! Wakes one thread blocked in @ref poll.
            //! @details This is the only thread-safe server method. It does not close or otherwise
            //! mutate the server and may be coalesced with other wake calls.
            virtual void wake() = 0;
        };

        //! Creates, binds, and starts one HTTP server.
        //! @param[in] address The TCP address on which the server listens. A port of zero asks the
        //! platform to select an available port.
        //! @param[in] handler The synchronous request handler retained by the server.
        //! @param[in] options Resource limits and per-poll work quotas copied by the server.
        //! @return Returns the listening server, or an argument, socket, bind, listen, or poller error.
        LUNA_HTTP_API R<Ref<IServer>> new_server(
            const Network::SocketAddress& address,
            RequestHandler&& handler,
            const ServerOptions& options = ServerOptions());

        //! Returns the HTTP module pointer for use with @ref add_module.
        LUNA_HTTP_API Module* module_http();
    }

    //! @}
}
