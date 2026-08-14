## Status
Approved.

## Last updated
2026/8/14

## Background
LunaSDK applications can expose protocol-independent operations through the Frontend module. Frontend intentionally does not parse JSON-RPC messages, manage request identifiers, serialize values, or access transports. VariantUtils now provides strict RFC 8259-compatible JSON encoding and decoding, and Runtime exposes process standard input, output, and error as transparent raw-byte operations.

These pieces make an MCP server possible, but they do not define the MCP-specific protocol shell. The shell must select a protocol revision, validate JSON-RPC and MCP request envelopes, advertise server capabilities, describe exported tools, translate tool calls to Frontend invocations, construct protocol responses, and implement transport framing.

MCP revision `2026-07-28` is materially different from earlier revisions. It uses a stateless core, removes the `initialize` and `initialized` exchange, carries the protocol version and client capabilities in every request, requires `server/discover`, and defines stdio as one newline-delimited JSON-RPC message per frame. Supporting legacy initialization-based revisions would therefore add a second lifecycle and compatibility state machine rather than a small compatibility shim.

Frontend has no resource enumeration or protocol metadata API. MCP `tools/list`, however, requires stable tool names, descriptions, JSON Schemas, annotations, and deterministic ordering. Adding MCP fields or enumeration policy to Frontend would couple the protocol-independent invocation kernel to one wire protocol.

## Decision
Introduce a new `MCP` module depending on Runtime, VariantUtils, and Frontend. The first implementation is an MCP server for protocol revision `2026-07-28` with the following scope:

1. The module supports only the modern `2026-07-28` revision. It implements `server/discover`, `tools/list`, and `tools/call`. It does not implement the legacy `initialize` lifecycle or advertise any older revision.
2. The MCP server owns a strong reference to one Frontend instance and is not thread-safe. Applications must synchronize access to both objects when sharing them across threads.
3. The MCP server owns a separate tool export registry. Every export maps one MCP tool descriptor to one Frontend function URL. Tool descriptors include the MCP name, display metadata, input schema, optional output schema, annotations, icons, and custom metadata. Frontend remains unaware of MCP metadata and does not gain resource enumeration APIs.
4. Registering a tool validates its MCP name, requires an object input schema whose root type is `object`, validates optional descriptor fields, and requires the mapped Frontend resource to be a function at registration time. The registry returns tools in deterministic name order.
5. The MCP shell validates the JSON-RPC request object, request identifier, MCP method parameters, required per-request `_meta` fields, and requested protocol version. Unsupported versions return MCP error `-32022`; standard JSON-RPC errors use their standard codes.
6. MCP `2026-07-28` frames contain one request, notification, or response. JSON-RPC batch arrays are rejected as invalid requests rather than unpacked. Client-sent JSON-RPC responses are also rejected because the modern server does not initiate requests.
7. `tools/call` forwards the request `arguments` object to the mapped Frontend function. The MCP layer validates the common envelope and the fact that arguments are an object. The Frontend handler remains responsible for validating tool-specific constraints against its published input schema and enforcing authorization and business rules.
8. Successful Frontend results must be representable by strict JSON. They are returned as `structuredContent` and also as a compact serialized JSON text content block. Frontend errors become MCP tool execution results with `isError: true`; unknown tools and malformed calls are JSON-RPC protocol errors.
9. All wire JSON is decoded and encoded with `JSONReadOptions::strict()` and `JSONWriteOptions::strict()`. Parse failures produce a JSON-RPC parse error response. A notification produces no response.
10. Core message processing operates on `Variant` values and JSON byte ranges without reading a transport. A separate stdio runner performs newline framing, buffers short reads, repeats short writes, writes only protocol frames to standard output, and exits successfully on standard-input EOF after a complete frame. EOF in an unterminated frame is a framing error. Diagnostics are not written to standard output.
11. The stdio runner is synchronous and processes one frame at a time. Cancellation, progress, subscriptions, prompts, resources, Multi Round-Trip Requests, extensions, Streamable HTTP, and legacy compatibility are outside the first implementation.

The protocol contract follows the authoritative MCP `2026-07-28` TypeScript schema and specification:

- https://modelcontextprotocol.io/specification/2026-07-28
- https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/stdio
- https://modelcontextprotocol.io/specification/2026-07-28/server/discover
- https://modelcontextprotocol.io/specification/2026-07-28/server/tools

## Impact
Applications can expose selected Frontend functions as a conforming modern MCP tools server without modifying their service implementation. The protocol core can be tested independently from stdio, and another transport can reuse the same message processor later.

The MCP-owned export registry duplicates the association between an exported tool and a Frontend URL, but it keeps protocol metadata out of Frontend and allows multiple MCP servers to expose different views of the same Frontend instance. Applications must update the MCP registry when an exported Frontend function is removed or replaced.

Only clients supporting the modern revision can use the first implementation. Older clients that start with `initialize` receive a method error and cannot fall forward. Legacy compatibility can be added later behind an explicit version dispatcher without changing the Frontend API.

The module does not contain a general JSON Schema evaluator. Applications are responsible for making handler validation match the published schema. Incorrect or missing application validation is a server bug and may violate MCP security requirements.

The synchronous stdio runner cannot process cancellation while a Frontend handler is executing. Long-running, cancellable, or multi-round-trip operations require a future asynchronous execution design.

## Alternatives considered

### Add MCP metadata and enumeration to Frontend
This was rejected because Frontend is deliberately protocol-independent. Other shells may expose different metadata, naming, filtering, or transport policies, and should not inherit MCP-specific fields.

### Map every Frontend function automatically
This was rejected because Frontend functions do not carry MCP names, descriptions, schemas, safety annotations, or an explicit decision that they are safe to expose to external callers.

### Support both modern and legacy MCP revisions immediately
This was rejected for the first implementation because legacy revisions require an initialization lifecycle, connection-scoped negotiated state, and different message behavior. The modern-only core establishes a smaller, testable baseline.

### Implement MCP inside Frontend
This was rejected because it would combine service invocation, JSON-RPC parsing, MCP semantics, serialization, and transport concerns in one module and contradict Frontend's documented boundary.

### Implement a partial JSON Schema evaluator
This was rejected because accepting only a subset while advertising arbitrary JSON Schema would create misleading security guarantees. A complete validator may be introduced as a reusable VariantUtils feature in a separate decision.

### Represent stdio as an IStream
This was rejected by [[ADR-0006 Expose standard IO as raw byte functions]]. The stdio runner uses the process-global raw-byte functions and owns only its framing buffer.

## Version history
* **2026/8/14** Proposed and approved.
