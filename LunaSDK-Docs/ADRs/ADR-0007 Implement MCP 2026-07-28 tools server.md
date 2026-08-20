## Status
Approved.

## Last updated
2026/8/20

## Background
LunaSDK applications can expose protocol-independent operations through the Frontend module. Frontend intentionally does not parse JSON-RPC messages, manage request identifiers, serialize values, or access transports. VariantUtils provides strict RFC 8259-compatible JSON encoding and decoding, Runtime exposes process standard input, output, and error as transparent raw-byte operations, and HTTP provides a bounded event-driven HTTP/1.x origin server.

These pieces make an MCP server possible, but they do not define the MCP-specific protocol shell. The shell must validate JSON-RPC and MCP request envelopes, select a protocol revision, advertise server capabilities, describe exported tools, translate tool calls to Frontend invocations, construct protocol responses, and adapt the protocol to standard IO or Streamable HTTP.

MCP revision `2026-07-28` is materially different from earlier revisions. It uses a stateless core, removes the `initialize` and `initialized` exchange, carries the protocol version and client capabilities in every request, requires `server/discover`, and defines standard mirrored HTTP headers. Legacy revision `2025-06-18` uses an `initialize` and `notifications/initialized` lifecycle and may associate multiple HTTP requests with one logical session. Supporting both eras therefore requires two explicit protocol facilities rather than treating missing modern fields as a permissive legacy mode.

Modern-only interoperability testing succeeded with the official MCP TypeScript SDK v2, but Codex CLI `0.147.0-alpha.6.5` opens standard IO MCP servers with `initialize` and requests protocol revision `2025-06-18`. A modern-only server consequently cannot expose tools to that client even though its transport, JSON, tool registry, and Frontend integration are otherwise correct.

The original MCP implementation placed the tool registry, protocol selection, and legacy lifecycle state in one `IMCPServer`. That model happens to fit one standard IO connection, but it cannot represent a shared HTTP endpoint: modern HTTP requests are stateless, while legacy lifecycle state must survive across POST requests and must not be keyed by a transient TCP connection. The public API has not been released, so the object model can be corrected without preserving the original interface.

Frontend has no resource enumeration or protocol metadata API. MCP `tools/list`, however, requires stable tool names, descriptions, JSON Schemas, annotations, and deterministic ordering. Adding MCP fields or enumeration policy to Frontend would couple the protocol-independent invocation kernel to one wire protocol.

## Decision
The `MCP` module depends on Runtime, VariantUtils, Frontend, and HTTP. It implements a tools server for modern protocol revision `2026-07-28` and legacy protocol revision `2025-06-18` with the following architecture and scope:

1. `IMCPServer` represents a shared MCP service. It retains one Frontend instance, the server descriptor, and one tool export registry. It contains no client protocol selection or initialization state.
2. `IMCPMessageProcessor` represents one ordered protocol conversation for one explicit `ProtocolVersion`. It retains its server. A legacy processor owns the `2025-06-18` initialization lifecycle; a modern processor is stateless. Message processing and strict JSON conversion are exposed by the processor rather than the server.
3. The server and every processor are not thread-safe. Applications must serialize access to a server, all processors created from it, and its retained Frontend object. Tool registration may change while processors exist, but may not race or reenter message processing.
4. The modern facility implements `server/discover`, `tools/list`, and `tools/call` for `2026-07-28`. The legacy facility implements `initialize`, `notifications/initialized`, `ping`, `tools/list`, and `tools/call` for `2025-06-18`.
5. The standard IO runner accepts an `IMCPServer`, owns one selected processor, and performs newline framing. Its first valid JSON-RPC request selects the facility: `initialize` selects legacy and every other request selects modern. Notifications before selection do not select a facility. The selected facility cannot change for the remainder of that standard IO connection.
6. Streamable HTTP is provided by `new_streamable_http_server`, which returns the underlying `HTTP::IServer`. The HTTP handler retains the MCP server and its transport state; a redundant MCP-specific HTTP server interface is not introduced.
7. The initial HTTP transport is local-only. It accepts only IPv4 loopback or IPv6 loopback listener addresses. Requests without `Origin` are accepted for native local clients. Requests containing `Origin` must match an explicitly configured allowlist. TLS, authentication, authorization protocols, proxies, and public listener addresses are outside this revision.
8. Modern HTTP processing is stateless. Every POST independently validates `MCP-Protocol-Version`, the protocol version in request metadata, `Mcp-Method`, conditional `Mcp-Name`, and schema-declared `Mcp-Param-*` mirrored headers before invoking Frontend. Modern HTTP does not create or accept protocol sessions.
9. A successful legacy HTTP `initialize` creates a `2025-06-18` processor and returns an opaque `Mcp-Session-Id`. Subsequent requests locate that processor by session ID rather than TCP connection. Legacy sessions have configurable count and idle-time limits and can be removed by `DELETE`. Session identifiers are routing identifiers for the local transport and are not authentication credentials.
10. The first HTTP implementation buffers complete requests and responses and returns `application/json`. A valid notification returns HTTP 202 without content. GET and server-sent events are not implemented; GET returns 405. Streaming, progress, subscriptions, cancellation, and server-initiated messages require a later streaming HTTP design.
11. JSON is decoded once into an internal validated message envelope before transport validation or method dispatch. The envelope exposes the request identifier, method, parameters, notification state, and protocol metadata so standard IO and HTTP do not duplicate JSON-RPC parsing. HTTP header/body consistency is validated before any Frontend handler can run.
12. The MCP server owns a separate tool export registry. Every export maps one MCP tool descriptor to one Frontend function URL. Tool descriptors include the MCP name, display metadata, input schema, optional output schema, annotations, icons, and custom metadata. Frontend remains unaware of MCP metadata and does not gain resource enumeration APIs.
13. Registering a tool validates its MCP name, requires an object input schema whose root type is `object`, validates optional descriptor fields and HTTP header mapping extensions, and requires the mapped Frontend resource to be a function at registration time. The registry returns tools in deterministic name order and precomputes any HTTP header bindings needed for tool calls.
14. Each facility projects shared descriptors and invocation results into its exact wire schema. Modern responses include `resultType`, cache fields, modern result metadata, and icons. Legacy responses omit modern-only result, cache, server-information, and icon fields; legacy initialization returns the `2025-06-18` implementation shape. Both facilities return `structuredContent` and a compact serialized JSON text content block for successful tool calls.
15. The MCP shell validates the JSON-RPC request object, request identifier, facility-specific method parameters, and active lifecycle. JSON-RPC batch arrays are rejected as invalid requests. Client-sent JSON-RPC responses are rejected because neither implemented facility initiates requests.
16. `tools/call` forwards the request `arguments` object to the mapped Frontend function. The MCP layer validates the common envelope and requires arguments to be an object. The Frontend handler remains responsible for validating tool-specific constraints against its published input schema and enforcing application rules.
17. Successful Frontend results must be representable by strict JSON. Frontend errors become MCP tool execution results with `isError: true`; unknown tools and malformed calls are JSON-RPC protocol errors. All wire JSON uses `JSONReadOptions::strict()` and `JSONWriteOptions::strict()`.
18. The standard IO runner remains synchronous and processes one frame at a time. The HTTP handler runs synchronously on the thread calling `HTTP::IServer::poll`. Long-running, cancellable, streaming, or multi-round-trip operations require a future asynchronous execution design.

The protocol contract follows the authoritative MCP schemas and specifications:

- https://modelcontextprotocol.io/specification/2026-07-28
- https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/stdio
- https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/streamable-http
- https://modelcontextprotocol.io/specification/2026-07-28/server/discover
- https://modelcontextprotocol.io/specification/2026-07-28/server/tools
- https://modelcontextprotocol.io/specification/2025-06-18/basic/lifecycle
- https://modelcontextprotocol.io/specification/2025-06-18/basic/transports
- https://modelcontextprotocol.io/specification/2025-06-18/server/tools

## Impact
One MCP server can be exposed through standard IO, HTTP, or multiple independent protocol processors without sharing client lifecycle state. Applications register tools once, and every active processor observes the same registry. This object model also gives future transports an explicit state owner instead of forcing them to emulate a connection around `IMCPServer`.

The HTTP adapter relies on the general HTTP module for TCP, parsing, connection persistence, bounds, and response framing. MCP owns only endpoint routing, media-type policy, Origin policy, version and mirrored-header validation, legacy session association, JSON-RPC processing, and MCP-specific status mapping.

Restricting this transport to loopback makes its intended deployment boundary executable rather than documentary. A future public deployment design must add secure randomness, authentication and authorization, TLS termination policy, proxy trust rules, and denial-of-service controls before non-loopback binding is permitted.

The MCP-owned export registry duplicates the association between an exported tool and a Frontend URL, but it keeps protocol metadata out of Frontend and allows multiple MCP servers to expose different views of the same Frontend instance. Applications must update the MCP registry when an exported Frontend function is removed or replaced.

The module does not contain a general JSON Schema evaluator. Applications are responsible for making handler validation match the published schema. The module validates only schema structure required by MCP itself and the header-mirroring extension needed by the HTTP transport.

The synchronous transports cannot process cancellation while a Frontend handler is executing. Long-running tool work may also block other HTTP connections owned by the same HTTP server. Applications should keep handlers bounded until asynchronous dispatch is introduced.

## Alternatives considered

### Keep client state in `IMCPServer`
This was the initial implementation. It was rejected because one shared HTTP endpoint serves multiple logical clients, and neither the first request received by the endpoint nor its TCP connections define one MCP protocol conversation.

### Use the TCP connection as a legacy HTTP session
This was rejected because HTTP clients may reconnect between requests, use multiple connections, or be separated from the server by transport infrastructure. `Mcp-Session-Id` is the protocol-level association mechanism.

### Hide version selection inside every message processor
This was rejected because the transport already has authoritative selection information. Standard IO selects from its first request, modern HTTP carries an explicit version on every POST, and legacy HTTP uses initialization plus a session identifier. Fixed-version processors have fewer invalid states.

### Add a separate MCP HTTP server interface
This was rejected because it would duplicate `HTTP::IServer::poll`, address inspection, wake, close, and ownership rules without adding MCP-specific lifecycle operations. Returning `HTTP::IServer` keeps the adapter thin.

### Add a separate MCPHTTP module
This would keep Network and HTTP out of standard-IO-only dependency graphs, but would split one protocol implementation and its standard transports across two registration units. The initial implementation keeps one MCP module and separates HTTP declarations and implementation into dedicated files.

### Add MCP metadata and enumeration to Frontend
This was rejected because Frontend is deliberately protocol-independent. Other shells may expose different metadata, naming, filtering, or transport policies and should not inherit MCP-specific fields.

### Map every Frontend function automatically
This was rejected because Frontend functions do not carry MCP names, descriptions, schemas, safety annotations, or an explicit decision that they are safe to expose.

### Implement two independent tool registries
This was rejected because protocol version does not change the application operation or Frontend mapping. Separate registries could drift and expose inconsistent tools; only the wire projection and lifecycle are version-specific.

### Implement HTTP directly on Network sockets
This was rejected by [[ADR-0010 Introduce an event-driven HTTP server module]]. MCP uses the common HTTP parser and server rather than repeating framing and connection management.

### Implement SSE in the first HTTP revision
The currently implemented tools methods can complete with one JSON response. SSE would require long-lived responses, backpressure, asynchronous completion, cancellation, and server-initiated notification ownership that the buffered synchronous HTTP API does not yet expose.

### Implement a partial JSON Schema evaluator
This was rejected because accepting only a subset while advertising arbitrary JSON Schema would create misleading security guarantees. A complete validator may be introduced as a reusable VariantUtils feature in a separate decision.

## Version history
* **2026/8/14** Proposed and approved.
* **2026/8/14** Added explicit `2025-06-18` compatibility, first-request standard IO facility selection, and Codex interoperability requirements.
* **2026/8/20** Replaced connection-scoped `IMCPServer` state with shared servers and fixed-version message processors; added local Streamable HTTP, modern stateless requests, and legacy HTTP sessions.
