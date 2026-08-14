## Status
Approved.

## Last updated
2026/8/14

## Background
LunaSDK applications can expose protocol-independent operations through the Frontend module. Frontend intentionally does not parse JSON-RPC messages, manage request identifiers, serialize values, or access transports. VariantUtils now provides strict RFC 8259-compatible JSON encoding and decoding, and Runtime exposes process standard input, output, and error as transparent raw-byte operations.

These pieces make an MCP server possible, but they do not define the MCP-specific protocol shell. The shell must select a protocol revision, validate JSON-RPC and MCP request envelopes, advertise server capabilities, describe exported tools, translate tool calls to Frontend invocations, construct protocol responses, and implement transport framing.

MCP revision `2026-07-28` is materially different from earlier revisions. It uses a stateless core, removes the `initialize` and `initialized` exchange, carries the protocol version and client capabilities in every request, requires `server/discover`, and defines stdio as one newline-delimited JSON-RPC message per frame. Legacy revisions use an `initialize` and `notifications/initialized` lifecycle with connection-scoped protocol and capability negotiation. Supporting both eras therefore requires two explicit protocol facilities rather than treating missing modern fields as a permissive legacy mode.

Modern-only interoperability testing succeeded with the official MCP TypeScript SDK v2, but Codex CLI `0.147.0-alpha.6.5` opens standard IO MCP servers with `initialize` and requests protocol revision `2025-06-18`. A modern-only server consequently cannot expose tools to the current Codex client even though its transport, JSON, tool registry, and Frontend integration are otherwise correct.

Frontend has no resource enumeration or protocol metadata API. MCP `tools/list`, however, requires stable tool names, descriptions, JSON Schemas, annotations, and deterministic ordering. Adding MCP fields or enumeration policy to Frontend would couple the protocol-independent invocation kernel to one wire protocol.

## Decision
Introduce a new `MCP` module depending on Runtime, VariantUtils, and Frontend. The implementation is a tools server for modern protocol revision `2026-07-28` and legacy protocol revision `2025-06-18` with the following scope:

1. The module provides separate modern and legacy protocol facilities over one shared tool and Frontend core. The modern facility implements `server/discover`, `tools/list`, and `tools/call` for `2026-07-28`. The legacy facility implements `initialize`, `notifications/initialized`, `ping`, `tools/list`, and `tools/call` for `2025-06-18`.
2. The first valid JSON-RPC request selects the facility for that server connection. An `initialize` request selects legacy; every other request selects modern. Notifications received before the first request do not select a facility. After selection, the connection cannot change protocol eras.
3. The legacy facility maintains connection-scoped initialization state. It validates client implementation and capability objects, negotiates `2025-06-18`, advertises only the tools capability, waits for `notifications/initialized` before serving tool operations, and rejects repeated initialization. The modern facility remains stateless with respect to client identity and capabilities and validates them independently on every request.
4. The MCP server owns a strong reference to one Frontend instance and is not thread-safe. Applications must synchronize access to both objects when sharing them between threads.
5. The MCP server owns a separate tool export registry. Every export maps one MCP tool descriptor to one Frontend function URL. Tool descriptors include the MCP name, display metadata, input schema, optional output schema, annotations, icons, and custom metadata. Frontend remains unaware of MCP metadata and does not gain resource enumeration APIs.
6. Registering a tool validates its MCP name, requires an object input schema whose root type is `object`, validates optional descriptor fields, and requires the mapped Frontend resource to be a function at registration time. The registry returns tools in deterministic name order.
7. Each facility projects shared descriptors and invocation results into its exact wire schema. Modern responses include `resultType`, cache fields, modern result metadata, and icons. Legacy responses omit modern-only result, cache, server-information, and icon fields; legacy initialization returns the `2025-06-18` implementation shape. Both facilities return `structuredContent` and a compact serialized JSON text content block for successful tool calls.
8. The MCP shell validates the JSON-RPC request object, request identifier, facility-specific method parameters, and the active protocol lifecycle. The modern facility requires per-request `_meta` fields and returns MCP error `-32022` for an unsupported modern version. Standard JSON-RPC errors use their standard codes.
9. JSON-RPC batch arrays are rejected as invalid requests rather than unpacked. Client-sent JSON-RPC responses are also rejected because neither implemented facility initiates requests.
10. `tools/call` forwards the request `arguments` object to the mapped Frontend function. The MCP layer validates the common envelope and the fact that arguments are an object. The Frontend handler remains responsible for validating tool-specific constraints against its published input schema and enforcing authorization and business rules.
11. Successful Frontend results must be representable by strict JSON. Frontend errors become MCP tool execution results with `isError: true`; unknown tools and malformed calls are JSON-RPC protocol errors.
12. All wire JSON is decoded and encoded with `JSONReadOptions::strict()` and `JSONWriteOptions::strict()`. Parse failures produce a JSON-RPC parse error response. A notification produces no response.
13. Core message processing operates on `Variant` values and JSON byte ranges without reading a transport. A separate stdio runner performs newline framing, buffers short reads, repeats short writes, writes only protocol frames to standard output, and exits successfully on standard-input EOF after a complete frame. EOF in an unterminated frame is a framing error. Diagnostics are not written to standard output.
14. The stdio runner is synchronous and processes one frame at a time. Cancellation, progress, subscriptions, prompts, resources, Multi Round-Trip Requests, extensions, and Streamable HTTP are outside the implementation.

The protocol contract follows the authoritative MCP `2026-07-28` TypeScript schema and specification:

- https://modelcontextprotocol.io/specification/2026-07-28
- https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/stdio
- https://modelcontextprotocol.io/specification/2026-07-28/server/discover
- https://modelcontextprotocol.io/specification/2026-07-28/server/tools
- https://modelcontextprotocol.io/specification/2025-06-18/basic/lifecycle
- https://modelcontextprotocol.io/specification/2025-06-18/server/tools

## Impact
Applications can expose selected Frontend functions to both modern MCP clients and initialization-era clients such as the current Codex release without modifying their service implementation. The shared tool core avoids duplicating Frontend registration and invocation behavior, while the two protocol facilities keep incompatible lifecycle and wire rules explicit.

The MCP-owned export registry duplicates the association between an exported tool and a Frontend URL, but it keeps protocol metadata out of Frontend and allows multiple MCP servers to expose different views of the same Frontend instance. Applications must update the MCP registry when an exported Frontend function is removed or replaced.

Protocol selection becomes connection-scoped state. Reusing one `IMCPServer` for messages from multiple independent transports would incorrectly share the selected facility and legacy initialization state; a transport integration must therefore create one server object per connection. The current standard IO model naturally satisfies this rule because one process serves one client connection.

Supporting `2025-06-18` does not imply support for every historical legacy revision. A legacy client requesting another revision receives `2025-06-18` as the server's selected version and disconnects if it does not support that revision, as required by legacy version negotiation.

The module does not contain a general JSON Schema evaluator. Applications are responsible for making handler validation match the published schema. Incorrect or missing application validation is a server bug and may violate MCP security requirements.

The synchronous stdio runner cannot process cancellation while a Frontend handler is executing. Long-running, cancellable, or multi-round-trip operations require a future asynchronous execution design.

## Alternatives considered

### Add MCP metadata and enumeration to Frontend
This was rejected because Frontend is deliberately protocol-independent. Other shells may expose different metadata, naming, filtering, or transport policies, and should not inherit MCP-specific fields.

### Map every Frontend function automatically
This was rejected because Frontend functions do not carry MCP names, descriptions, schemas, safety annotations, or an explicit decision that they are safe to expose to external callers.

### Keep the server modern-only
This was used for the initial implementation because it established a small, testable baseline. It is no longer sufficient after process-level interoperability testing showed that current Codex releases request `2025-06-18`. Keeping modern-only behavior would require every LunaSDK application to ship a separate translation gateway for a common client.

### Treat missing modern metadata as legacy mode
This was rejected because it would conflate two protocol eras, weaken modern validation, and fail to implement the legacy initialization lifecycle and version negotiation. Facility selection is explicit and locked by the first request instead.

### Implement two independent tool registries
This was rejected because protocol version does not change the application operation or Frontend mapping. Separate registries could drift and expose inconsistent tools; only the wire projection and lifecycle are version-specific.

### Implement MCP inside Frontend
This was rejected because it would combine service invocation, JSON-RPC parsing, MCP semantics, serialization, and transport concerns in one module and contradict Frontend's documented boundary.

### Implement a partial JSON Schema evaluator
This was rejected because accepting only a subset while advertising arbitrary JSON Schema would create misleading security guarantees. A complete validator may be introduced as a reusable VariantUtils feature in a separate decision.

### Represent stdio as an IStream
This was rejected by [[ADR-0006 Expose standard IO as raw byte functions]]. The stdio runner uses the process-global raw-byte functions and owns only its framing buffer.

## Version history
* **2026/8/14** Proposed and approved.
* **2026/8/14** Added explicit `2025-06-18` compatibility, first-request protocol facility selection, and Codex interoperability requirements.
