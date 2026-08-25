## Functionality

The HTTP module provides a bounded, event-driven HTTP/1.x origin server over the non-blocking sockets and socket poller supplied by Network. It accepts TCP connections, incrementally parses fragmented requests, invokes an application handler after a complete request is available, serializes the returned response, and manages persistent connections without assigning one blocking thread to each client.

The initial server is designed for local tools and small embedded services. It supports HTTP/1.1, basic HTTP/1.0 interoperability, fixed-length and chunked request bodies, trailers, `Expect: 100-continue`, persistent HTTP/1.1 connections, and pipelined requests. It does not provide TLS, HTTP clients, routing, authentication, compression, static files, WebSocket, HTTP/2, HTTP/3, request or response streaming, proxy behavior, or automatic worker dispatch. See [[HTTP Messages]] for the request and response data model.

## Concepts

### Origin server

`HTTP::IServer` owns one TCP listener, one socket poller, all accepted connections, request parsing state, and pending response bytes. `HTTP::new_server` creates and binds the listener immediately, starts listening, creates the poller, and retains the supplied `RequestHandler`.

The HTTP module does not contain a router. One handler receives every request accepted by that server and may dispatch using `Request::method`, `Request::path`, headers, or other application data.

### Poll-driven reactor

`IServer::poll` performs one bounded reactor iteration. It accepts ready clients, receives available bytes, parses complete requests, invokes the handler, queues responses, flushes writable connections, and returns the number of requests dispatched during that call.

The timeout controls only how long that iteration may wait for readiness:

- `0` performs a non-blocking iteration.
- A finite value waits for at most that many milliseconds.
- `U32_MAX` may wait indefinitely.

One call is not required to finish sending every queued response. Continue polling for the lifetime of the service.

### Thread ownership

The server, listener, accepted sockets, parser state, and handler execution are owned by the thread calling `poll`. Except for `IServer::wake`, the server interface is not thread-safe. Do not call `poll`, `get_local_address`, or `close` concurrently, and do not reenter the dispatching server from its handler.

`wake` is thread-safe and interrupts a blocked `poll`. It does not close the server or execute application work. A typical application places a command in its own synchronized queue, calls `wake`, and lets the poll-owner thread consume the command and call `close` if shutdown was requested.

### Synchronous handler

`RequestHandler` receives one completely parsed `Request` and returns `R<Response>`. It runs synchronously inside `poll`, so a long handler delays every connection owned by the same server. Keep handlers bounded or move expensive work to application-owned workers and return only when the buffered response is ready.

A handler failure becomes an HTTP 500 response for that request. It does not cause `poll` to fail. A protocol or socket failure isolated to one client closes that connection without stopping the listener. Listener, poller, or server-wide internal failures are returned from `poll`.

### Connection persistence and pipelining

HTTP/1.1 connections remain open by default. A request containing the `close` connection token or a response with `close_connection` set closes the connection after its queued response is sent. HTTP/1.0 connections close after their response in this revision.

The parser accepts multiple pipelined requests and dispatches them in received order. Responses for one connection are serialized in the same order. Work quotas and output limits prevent one busy connection from monopolizing a poll iteration or allocating unbounded response storage.

### Request framing

The server accepts requests with no body, one consistent `Content-Length`, or `Transfer-Encoding: chunked` as the only transfer coding. Chunk extensions are validated and ignored; decoded trailers are exposed separately from headers.

HTTP/1.1 requires exactly one `Host` field. Conflicting `Content-Length` values, simultaneous `Content-Length` and `Transfer-Encoding`, unsupported transfer codings, forbidden trailer fields, obsolete folded fields, bare line feeds, malformed control characters, and unsupported HTTP versions are rejected before the handler runs.

When an HTTP/1.1 request contains the supported `Expect: 100-continue` value and its body has not arrived, the server sends the interim response before waiting for content. Other expectations receive 417.

### Resource limits

`ServerOptions` bounds retained state and the amount of work performed by one `poll` call:

| Option | Purpose |
| --- | --- |
| `max_connections` | Simultaneously accepted clients |
| `max_request_line_size` | Bytes in one request line |
| `max_header_section_size` | Bytes in one header or trailer section |
| `max_header_count` | Field lines in one header or trailer section |
| `max_body_size` | Decoded request body bytes |
| `max_buffered_output_size` | Unsent serialized response bytes per connection |
| `listen_backlog` | Native listener backlog |
| `max_accepts_per_poll` | New clients accepted per iteration |
| `max_requests_per_poll` | Complete requests dispatched per iteration |
| `max_socket_events_per_poll` | Readiness events consumed per iteration |

`max_body_size` may be zero to accept only empty request bodies. Other capacity and work values must satisfy the validation documented by the public API; notably, the buffered output limit must be at least 1024 bytes and the listen backlog must be positive.

Oversized request lines, field sections, and bodies receive fixed protocol errors such as 414, 431, and 413. Malformed request framing normally receives 400 and closes that connection.

### Deployment boundary

The HTTP server can bind any IPv4 or IPv6 address supplied by the application. Binding `IPV4_ADDRESS_ANY` or `IPV6_ADDRESS_ANY` may expose the service on every matching network interface. The module does not add TLS, authentication, authorization, origin policy, or proxy trust rules. An application must supply those facilities before exposing an HTTP server outside its intended trust boundary.

## Programming guide

### 1. Initialize the module

Register and initialize HTTP after Runtime. HTTP declares its dependency on Network, so adding `module_http` also initializes the required socket facilities.

```cpp
#include <Luna/HTTP/HTTP.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>

using namespace Luna;

lupanic_if_failed(init());
lupanic_if_failed(add_modules({HTTP::module_http()}));
lupanic_if_failed(init_modules());
```

The LunaBuild target that uses these APIs must depend on `HTTP`:

```csharp
DependsOn("Runtime", "HTTP");
```

### 2. Choose a listener address

Fill a Network `SocketAddress`. Port zero asks the platform to allocate an available port.

```cpp
Network::SocketAddress address = {};
address.family = Network::AddressFamily::ipv4;
address.ipv4.address = {127, 0, 0, 1};
address.ipv4.port = 0;
```

Use a loopback address for local tooling. See [[Sockets]] for address formats and platform-independent socket concepts.

### 3. Create the request handler

The handler below implements two small routes while keeping routing in application code:

```cpp
HTTP::RequestHandler handler(
    [](const HTTP::Request& request) -> R<HTTP::Response>
    {
        HTTP::Response response;
        response.headers.push_back(
            {Name("content-type"), String("text/plain; charset=utf-8")});

        if(request.method == Name("GET") && request.path.compare("/health") == 0)
        {
            const c8 body[] = "ok\n";
            response.body = Blob(body, sizeof(body) - 1);
            return response;
        }
        if(request.method == Name("POST") && request.path.compare("/echo") == 0)
        {
            response.body = request.body;
            return response;
        }

        response.status_code = 404;
        const c8 body[] = "not found\n";
        response.body = Blob(body, sizeof(body) - 1);
        return response;
    });
```

Returning a LunaSDK error asks HTTP to generate a 500 response. Return an explicit 4xx or 5xx `Response` when the error is part of the application protocol and should be visible to the client.

### 4. Configure resource limits

Start from conservative defaults and lower limits when the application protocol has smaller known bounds.

```cpp
HTTP::ServerOptions options;
options.max_connections = 64;
options.max_request_line_size = 4 * 1024;
options.max_header_section_size = 16 * 1024;
options.max_header_count = 64;
options.max_body_size = 1024 * 1024;
options.max_requests_per_poll = 32;
```

The output limit must accommodate the largest serialized response that the handler may return. If serialization would exceed the per-connection limit, the server substitutes a fixed error response and closes the affected connection when necessary.

### 5. Create the server

Create the listener and inspect the effective address when port zero was used.

```cpp
R<Ref<HTTP::IServer>> server_result =
    HTTP::new_server(address, move(handler), options);
lupanic_if_failed(server_result);
Ref<HTTP::IServer> server = move(server_result.get());

Network::SocketAddress local_address = {};
lupanic_if_failed(server->get_local_address(local_address));
u16 selected_port = local_address.ipv4.port;
```

Creation may fail because of invalid arguments, an unsupported address family, socket resource exhaustion, bind conflicts, listen errors, or poller creation errors. Do not enter the poll loop unless the result is valid.

### 6. Run the owner loop

Call `poll` repeatedly on one owner thread. A dedicated server thread may wait indefinitely; a game or tool loop can use a finite or zero timeout to interleave other work.

```cpp
while(!server->is_closed())
{
    R<usize> dispatched = server->poll(U32_MAX);
    if(!dispatched.valid())
    {
        log_error("HTTP", "Server poll failed: %s", explain(dispatched.errcode()));
        break;
    }
}
```

When using a finite timeout, the returned request count can be used for instrumentation, but a zero count is normal and does not indicate failure.

### 7. Wake and stop the server

To notify a blocked owner loop from another thread:

1. Publish a shutdown command through application-owned synchronized state.
2. Call `server->wake()` from the notifying thread.
3. Let the owner thread observe the command after `poll` returns.
4. Call `server->close()` on the owner thread.

`close` is idempotent and immediately closes the listener and accepted connections. It does not drain queued responses. Destroying the server also closes its resources.

Release server and handler-owned LunaSDK objects before shutting down Runtime:

```cpp
server->close();
server.reset();
close();
```

## Complete example

The following minimal program serves one health response on an automatically selected loopback port. A real application should add its own stop condition and error reporting.

```cpp
#include <Luna/HTTP/HTTP.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>

using namespace Luna;

int main()
{
    lupanic_if_failed(init());
    lupanic_if_failed(add_modules({HTTP::module_http()}));
    lupanic_if_failed(init_modules());

    Network::SocketAddress address = {};
    address.family = Network::AddressFamily::ipv4;
    address.ipv4.address = {127, 0, 0, 1};
    address.ipv4.port = 0;

    HTTP::RequestHandler handler(
        [](const HTTP::Request& request) -> R<HTTP::Response>
        {
            HTTP::Response response;
            if(request.method != Name("GET") || request.path.compare("/health") != 0)
            {
                response.status_code = 404;
                return response;
            }
            response.headers.push_back(
                {Name("content-type"), String("text/plain; charset=utf-8")});
            const c8 body[] = "ok\n";
            response.body = Blob(body, sizeof(body) - 1);
            return response;
        });

    R<Ref<HTTP::IServer>> result = HTTP::new_server(address, move(handler));
    lupanic_if_failed(result);
    Ref<HTTP::IServer> server = move(result.get());

    while(!server->is_closed())
    {
        lupanic_if_failed(server->poll(U32_MAX));
    }

    server.reset();
    close();
    return 0;
}
```
