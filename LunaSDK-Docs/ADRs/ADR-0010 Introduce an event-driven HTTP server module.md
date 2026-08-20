## Status
Approved.

## Last updated
2026/8/19

## Background
LunaSDK needs HTTP transport support for MCP and for later network-facing features that are unrelated to MCP. Implementing HTTP framing and connection management inside MCP would couple a general transport protocol to one application protocol and make every later HTTP consumer repeat the same work.

ADR-0008 made Network sockets permanently non-blocking, and ADR-0009 added a platform-independent, level-triggered socket poller. These facilities provide the transport and readiness primitives required by an event-driven HTTP server, but they deliberately do not own receive buffers, protocol state, connection persistence, request dispatch, or response framing.

The first HTTP implementation must be useful for local tools and small embedded services without claiming to be a complete web platform. It must parse untrusted input defensively, impose configurable resource limits, preserve HTTP field ordering and duplicates, and avoid binding the public API to MCP-specific routing or JSON processing.

`Name` and `String` have different performance models in Runtime. `Name` stores an immutable, reference-counted, shared copy of equal content and releases that storage when the last reference is released. `String` owns and copies its mutable buffer. HTTP methods and field names are immutable protocol identifiers that are commonly repeated and compared, while request targets, paths, queries, field values, and entity bodies are high-cardinality data whose original representation can be significant.

## Decision
A new `HTTP` module provides a transport-independent HTTP message model and an event-driven HTTP/1.x origin server built on Network TCP sockets and `ISocketPoller`. The module depends on `Runtime` and `Network`; it does not depend on MCP, Frontend, VariantUtils, or any application module.

The initial public message model contains:

```cpp
enum class HTTPVersion : u8
{
    http_1_0,
    http_1_1,
};

struct Header
{
    Name name;
    String value;
};

struct Request
{
    Name method;
    String target;
    String path;
    String query;
    HTTPVersion version;
    Vector<Header> headers;
    Vector<Header> trailers;
    Blob body;
    Network::SocketAddress remote_address;
};

struct Response
{
    u16 status_code = 200;
    Vector<Header> headers;
    Blob body;
    bool close_connection = false;
};
```

`Response::status_code` is one final response status in the range 200 through 599. Informational responses are controlled by the server; in particular, it emits `100 Continue` when an HTTP/1.1 request with the supported expectation is waiting for content.

Parsed field names are validated as HTTP `token` values, normalized to lowercase ASCII, and stored as `Name`. This makes their case-insensitive protocol identity explicit and gives equal field names one shared allocation. Method tokens are validated and stored as `Name` without case normalization because HTTP methods are case-sensitive. The server preserves repeated header and trailer lines in their received order.

`target` preserves the received request-target. `path` and `query` are derived views copied into independent `String` values without percent-decoding or other application-specific normalization. Field values are stored in `String` after removing framing whitespace because HTTP field values can contain arbitrary, high-cardinality octets including `obs-text`. Entity bodies use `Blob`.

The module exposes a synchronous request handler:

```cpp
using RequestHandler = Function<R<Response>(const Request& request)>;
```

The handler is invoked only after the complete request, including its body and trailers, has been parsed. It runs on the thread calling the server's `poll` method. A handler may retain or copy any data it needs after returning, but the request passed to it is valid only for the call. The handler must not reenter the server that is dispatching it. Handler errors produce a `500 Internal Server Error` response and do not make `poll` fail. Listener, poller, and server-internal failures are returned by `poll`; a failure isolated to one accepted client closes that connection without stopping the server.

`new_server` creates the listener, binds it to one supplied `SocketAddress`, starts listening, creates its poller, and retains the handler. `IServer::poll(timeout_ms)` performs one bounded reactor iteration and returns the number of requests dispatched. `timeout_ms == 0` is non-blocking and `U32_MAX` may wait indefinitely. `IServer::get_local_address` exposes the effective listener address, including an automatically assigned port. `IServer::close` removes and closes the listener and all accepted connections immediately and is idempotent.

The server and its sockets are not thread-safe and are owned by the thread that calls `poll`. `IServer::wake` is the only thread-safe method. It interrupts a blocked `poll` so another thread can notify the owner that application work or a shutdown command is pending; it does not mutate server state itself.

The first implementation supports HTTP/1.1 and accepts HTTP/1.0 for basic interoperability. HTTP/1.1 connections are persistent by default, while HTTP/1.0 connections close after each response unless a later revision explicitly adds HTTP/1.0 keep-alive support. A `Connection: close` request token or `Response::close_connection` closes the connection after the serialized response has been sent. Pipelined requests are parsed and answered in order on each connection, subject to the per-poll work quota and output-buffer limit.

Request content is buffered in memory. The parser supports no request body, a valid `Content-Length`, and a final `chunked` transfer coding with optional trailers. Conflicting `Transfer-Encoding` and `Content-Length`, conflicting Content-Length values, invalid numeric framing, unsupported transfer codings, forbidden trailer fields, obsolete line folding, bare line feeds, control characters, and malformed start lines or fields are rejected. A malformed request receives an appropriate fixed error response and the connection is closed.

Responses are emitted as HTTP/1.1 messages with server-controlled framing. Application-supplied `Content-Length`, `Transfer-Encoding`, and `Connection` fields do not control framing; the serializer generates the authoritative framing and connection fields. Response field names and values are validated before output so an application cannot inject additional lines. Responses to `HEAD`, 204, and 304 send no body bytes. `Content-Length` is omitted for 204, represents the selected representation for HEAD and 304, and otherwise equals the bytes sent.

`ServerOptions` supplies conservative configurable limits for concurrent connections, request-line bytes, header-section bytes, header count, body bytes, buffered output per connection, accept work, and request dispatch work per `poll` call. Exceeding a syntactic header limit produces `431 Request Header Fields Too Large`; exceeding the body limit produces `413 Content Too Large`; exhausting the connection limit leaves additional connections queued in the listener. These limits prevent one connection from causing unbounded allocation or monopolizing one reactor iteration.

The initial module deliberately excludes TLS, HTTP clients, HTTP/2, HTTP/3, WebSocket and generic protocol upgrades, CONNECT tunneling, response streaming, request streaming, compression, cookies, authentication, routing, static files, proxy behavior, and worker-thread dispatch. Routing and application protocol parsing remain above HTTP. TLS can later be introduced as an explicit transport abstraction rather than inferred from a socket flag.

## Impact
MCP can add Streamable HTTP as a thin protocol adapter that validates its endpoint and content types, processes JSON-RPC messages, and creates HTTP responses without owning TCP or HTTP parsing. Other embedded services can use the same server independently of MCP.

The poll-driven design composes directly with Network and avoids one blocked thread per connection. Applications control where the reactor runs and can integrate it with their own command queues through `wake`. Synchronous handlers keep the initial ownership model simple, but long-running handlers block progress for every connection owned by that server; applications must keep handlers short or move expensive work elsewhere before returning.

Buffering complete requests and responses makes ownership and framing deterministic and is suitable for the initial local-tool workload. It also caps the size of messages the module can handle efficiently. Streaming bodies and asynchronous response completion will require a later API addition rather than hidden behavior changes.

Normalizing field names loses their original capitalization, which has no protocol meaning. Header order, duplicate lines, method spelling, request-target bytes, field values, and body bytes remain observable. `Name` operations add global name-table hashing and synchronization, but methods and field names have sufficient repetition and identifier semantics to justify that cost. High-cardinality message data remains in `String` or `Blob`.

## Alternatives considered
### Implement HTTP directly in MCP
This would reduce the number of new public types for the first consumer, but it would couple TCP lifecycle and HTTP security rules to JSON-RPC and force future services to duplicate them.

### Make the HTTP server blocking or create one thread per connection
This would conflict with the permanent non-blocking socket model established by ADR-0008 and would scale thread count with idle keep-alive connections. A poll-driven server uses the readiness facility already established by ADR-0009.

### Run an internal HTTP thread automatically
An internal thread would make callbacks and shutdown appear convenient but would impose synchronization and scheduling policy on every consumer. Explicit `poll` keeps thread ownership visible and lets applications choose a dedicated thread or integrate the server into an existing loop.

### Store all HTTP text in `String`
This would preserve one container type but repeatedly allocate and copy common method and field-name identifiers. It would also hide field-name case normalization from the type model.

### Store paths, queries, targets, and field values in `Name`
These values are commonly unique, can be large, and often require their exact representation or later parsing. Interning them would add global table work without the repeated comparison and copy benefits expected for identifiers.

### Replace headers with a hash map
A map would lose received ordering and either reject or combine duplicate field lines. Both can change semantics, most notably for `Set-Cookie` and list-valued fields. A vector preserves the wire model; consumers can build indexed views when needed.

### Expose blocking `run` only
A blocking convenience loop would not let an application interleave command queues, multiple services, or controlled shutdown. It can be added later as a helper over `poll` without changing the server interface.

### Support streaming bodies in the first release
Streaming would require pause/resume, backpressure, lifetime, partial-response, and handler reentrancy contracts before the core parser and server have real consumers. Bounded buffering provides a smaller safe foundation and does not prevent adding a separate streaming interface later.

## Remarks
The HTTP/1.1 parsing and framing rules are based on RFC 9110 (HTTP Semantics) and RFC 9112 (HTTP/1.1). Protocol interoperability tests should include fragmented input, multiple requests on one connection, chunked content, duplicate fields, partial sends, malformed framing, and connection closure.

## Version history
* **2026/8/19** Proposed and approved; defined the initial HTTP message model, bounded poll-driven server, HTTP/1.x framing scope, and `Name`/`String` usage.
* **2026/8/19** Implemented the HTTP module and loopback protocol tests.
