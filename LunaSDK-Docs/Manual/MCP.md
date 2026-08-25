The MCP module exposes selected Frontend functions as tools through Model Context Protocol (MCP) revisions `2026-07-28` and `2025-06-18`. It supplies the protocol shell that Frontend deliberately omits: JSON-RPC validation, protocol selection and lifecycle, version and capability checks, server discovery, tool metadata, tool invocation responses, strict JSON serialization, and standard IO and local Streamable HTTP transports.

The implementation is a synchronous tools server with two protocol facilities over one shared Frontend and tool registry. The modern `2026-07-28` facility implements `server/discover`, `tools/list`, and `tools/call`. The legacy `2025-06-18` facility implements `initialize`, `notifications/initialized`, `ping`, `tools/list`, and `tools/call`. Prompts, resources, subscriptions, cancellation, progress, Multi Round-Trip Requests, server-sent events, public-network deployment, TLS, and authentication are outside the current scope.

## Concepts

### MCP server

An MCP server is the shared service definition. It retains one `Frontend::IFrontend` instance and publishes selected Frontend functions in an MCP tool registry. It contains no client protocol selection or initialization state.

One server may be used by standard IO, Streamable HTTP, or application-defined transports. Each ordered protocol conversation uses a separate `IMCPMessageProcessor`. The standard IO runner creates and owns its selected processor, while the HTTP adapter creates stateless modern processors per request and retains legacy processors per MCP session.

The MCP server, all processors created from it, and its retained Frontend object are not thread-safe. An application that shares any of them between threads must synchronize all access to the entire service.

### Server descriptor

`ServerDesc` describes the server implementation returned during discovery or initialization and controls modern discovery and tool-list cache hints. Its required `name` and `version` fields identify the implementation. Optional display fields and instructions improve client presentation and model guidance.

`discovery_ttl_ms` and `tools_ttl_ms` control the `ttlMs` value of `server/discover` and `tools/list`. `cache_scope` is `private_cache` by default so that a response is not reused across authorization contexts unless the application explicitly declares it public.

### Tool export

`ToolDesc` associates one MCP tool name and descriptor with one Frontend function URL. Tool registration is explicit: a Frontend function is never exposed merely because it exists.

The MCP tool name must be between 1 and 128 ASCII characters and may contain letters, digits, `_`, `-`, and `.`. `input_schema` must be a JSON object whose root `type` is `object`. Optional output schema, annotations, icons, and metadata must have the types required by the MCP schema. Metadata keys follow the MCP `_meta` key syntax. All descriptor data must be representable as strict JSON.

The MCP module validates the shared request envelope and requires `tools/call` arguments to be an object. Multi Round-Trip Request fields are rejected because that feature is outside the implementation. The Frontend handler must validate tool-specific values against the published input schema, ensure successful output conforms to any published output schema, and enforce authorization, security, and business rules. The MCP module does not contain a partial JSON Schema evaluator.

Tool descriptors are projected into the schema of the selected protocol. The modern definition includes supported presentation fields such as icons. The `2025-06-18` definition omits fields that did not exist in that revision while retaining title, description, input and output schemas, annotations, and custom metadata.

For modern HTTP calls, a property schema may contain an `x-mcp-header` string. The value is the suffix of the required `Mcp-Param-*` request header for that property. Header mappings may appear on root-property chains and support `string`, `integer`, and `boolean` values. Mapped header names must be valid and unique without regard to ASCII case. The HTTP adapter checks every mapped header against the JSON body before calling Frontend.

### Message processor

`IMCPServer::new_message_processor` creates an independent processor for one explicit `ProtocolVersion`. `IMCPMessageProcessor::process_message` accepts one already-decoded `Variant` message and returns either one response object or no response for a notification. `IMCPMessageProcessor::process_json` performs the same operation on one complete JSON byte range using strict JSON decoding and encoding. Invalid JSON produces a JSON-RPC parse-error response rather than a LunaSDK error.

The processor accepts one JSON-RPC object per call. JSON arrays are not treated as batches and are rejected as invalid requests.

Modern requests must contain both `io.modelcontextprotocol/protocolVersion` and `io.modelcontextprotocol/clientCapabilities` in their `_meta` object. Each request is validated independently against `2026-07-28`.

Legacy clients begin with `initialize`. The server responds with protocol version `2025-06-18`, the tools capability, implementation information, and optional instructions. Tool operations are rejected until the client sends `notifications/initialized`. The `ping` request is accepted during and after initialization. Repeated initialization is rejected. Protocol facilities cannot switch because each processor has a fixed version.

Frontend handler errors are tool results with `isError: true`, which lets a model inspect and potentially correct the call. An unknown tool, malformed request, unsupported method, missing Frontend function, or other protocol/server failure is a JSON-RPC error response. Modern results include modern result metadata and cache fields where applicable; legacy results omit those fields.

### Standard IO runner

`run_stdio_server` continuously reads raw bytes from standard input, divides them at newline characters, processes each complete frame, and writes each response followed by one newline to standard output. It handles short reads and short writes and exits successfully when standard input reaches EOF after a complete frame.

Every input message must be terminated by a newline and must not exceed `StdioServerOptions::max_message_size`. Embedded newlines are not allowed in one message. A process using this transport must reserve standard output exclusively for MCP frames; diagnostics should go to standard error or another sink.

The first valid JSON-RPC request selects the processor for the standard IO connection. `initialize` selects legacy `2025-06-18`; every other request selects modern `2026-07-28`. Notifications before the first request do not select a processor.

### Local Streamable HTTP server

`new_streamable_http_server` adapts a shared MCP server to the event-driven HTTP module and returns `HTTP::IServer`. The caller owns the poll loop and may inspect the selected local address, wake the poller, or close the server through that common interface.

The listener address must be IPv4 loopback or IPv6 loopback. A native local client may omit `Origin`; an Origin-bearing request must exactly match `StreamableHTTPServerOptions::allowed_origins`. This restriction is an executable deployment boundary: the current API does not provide the TLS, authentication, proxy-trust, or authorization controls required for public listeners.

The endpoint defaults to `/mcp`. POST requests use strict JSON and must accept both `application/json` and `text/event-stream`, although this buffered implementation returns JSON only. Modern requests are stateless and validate the protocol version and mirrored MCP headers on every request. Legacy `initialize` creates an MCP session; subsequent POST requests carry its `Mcp-Session-Id`, and DELETE removes it. Session count and idle lifetime are configurable. Sessions are associated with MCP identifiers rather than TCP connections.

Notifications return HTTP 202 with no body. GET returns HTTP 405 because server-sent events are not implemented.

## Programming guide

### Initialize the modules

Initialize Runtime and register the MCP module. The MCP module depends on VariantUtils, Frontend, and HTTP; HTTP brings in Network through the normal module dependency graph.

```cpp
lupanic_if_failed(Luna::init());
lupanic_if_failed(Luna::add_modules({Luna::MCP::module_mcp()}));
lupanic_if_failed(Luna::init_modules());
```

### Create the Frontend service

Create a Frontend instance and register application functions. Handlers receive only the application argument object; they do not receive JSON-RPC or MCP envelope fields.

```cpp
using namespace Luna;

Ref<Frontend::IFrontend> frontend = Frontend::new_frontend();
Frontend::FunctionHandler add_handler =
    [](Frontend::IFrontend*, const Variant& arguments) -> R<Variant>
{
    const Variant& a = arguments.find("a");
    const Variant& b = arguments.find("b");
    if(a.type() != VariantType::number || b.type() != VariantType::number)
    {
        return set_error(E_BAD_ARGUMENTS, "a and b must be numbers");
    }
    Variant result(VariantType::object);
    result["sum"] = a.fnum() + b.fnum();
    return result;
};
lupanic_if_failed(frontend->set_resource_function(
    "/math/add", move(add_handler)));
```

### Create the MCP server

Fill the required implementation identity and any cache or presentation metadata, then create a server. The server retains the Frontend instance.

```cpp
MCP::ServerDesc server_desc;
server_desc.name = "luna-math";
server_desc.version = "1.0.0";
server_desc.title = "Luna Math Tools";
server_desc.instructions = "Use these tools for exact arithmetic.";
server_desc.discovery_ttl_ms = 60000;
server_desc.tools_ttl_ms = 60000;

R<Ref<MCP::IMCPServer>> server_result =
    MCP::new_server(frontend, server_desc);
lupanic_if_failed(server_result);
Ref<MCP::IMCPServer> server = server_result.get();
```

### Export a tool

Create the input schema as a Variant object. Registering the export checks that the mapped Frontend resource currently exists and is a function.

```cpp
Variant properties(VariantType::object);
properties["a"]["type"] = "number";
properties["b"]["type"] = "number";

Variant required(VariantType::array);
required.push_back("a");
required.push_back("b");

MCP::ToolDesc tool;
tool.name = "add";
tool.frontend_url = "/math/add";
tool.title = "Add numbers";
tool.description = "Adds two numbers and returns their sum.";
tool.input_schema = Variant(VariantType::object);
tool.input_schema["type"] = "object";
tool.input_schema["properties"] = move(properties);
tool.input_schema["required"] = move(required);

lupanic_if_failed(server->set_tool(move(tool)));
```

Tool metadata is independent from the Frontend registry. If the application removes or replaces an exported Frontend resource, it must keep the MCP export registry consistent. `tools/call` detects a missing mapped function and reports an internal server error.

### Process messages in another transport

Create one processor for each ordered protocol conversation. Use `process_message` when another component already owns JSON decoding, or `process_json` when it can supply one complete JSON frame. A notification returns `has_response == false` or an empty output string.

```cpp
R<Ref<MCP::IMCPMessageProcessor>> processor_result =
    server->new_message_processor(MCP::ProtocolVersion::v2026_07_28);
lupanic_if_failed(processor_result);
Ref<MCP::IMCPMessageProcessor> processor = processor_result.get();

R<String> output = processor->process_json(request_bytes, request_size);
if(output.valid() && !output.get().empty())
{
    // Send output.get() as one response message using this transport.
}
```

Transport adapters must preserve message boundaries and decide which fixed processor owns each conversation. For a legacy client, pass `initialize`, `notifications/initialized`, and subsequent requests to the same legacy processor in order. The MCP core processor neither reads transport headers nor associates authorization state or external request IDs.

### Run the standard IO transport

After all tools are registered, run the synchronous standard IO loop. Do not write unrelated text to standard output while it is active.

```cpp
MCP::StdioServerOptions options;
options.max_message_size = 16 * 1024 * 1024;
lupanic_if_failed(MCP::run_stdio_server(server, options));
```

The call returns when standard input reaches EOF or when an IO/framing error occurs.

### Run the local Streamable HTTP transport

Create a loopback address and the HTTP adapter after registering all tools. Port zero asks the platform to choose an available port.

```cpp
#include <Luna/MCP/StreamableHTTP.hpp>

Network::SocketAddress address = {};
address.family = Network::AddressFamily::ipv4;
address.ipv4.address = {127, 0, 0, 1};
address.ipv4.port = 0;

MCP::StreamableHTTPServerOptions http_options;
http_options.endpoint = "/mcp";
http_options.allowed_origins.push_back("http://localhost:3000");

R<Ref<HTTP::IServer>> http_result =
    MCP::new_streamable_http_server(server, address, http_options);
lupanic_if_failed(http_result);
Ref<HTTP::IServer> http_server = http_result.get();

while(!http_server->is_closed())
{
    lupanic_if_failed(http_server->poll(U32_MAX));
}
```

The polling thread also executes Frontend handlers synchronously. Keep handlers bounded and do not access the MCP service concurrently from another thread. Call `HTTP::IServer::wake` from another thread only when the owner poll loop must be interrupted.

Close the modules and Runtime only after the selected transport returns and all transport objects are released.

```cpp
Luna::close();
```

## Standalone interoperability test server

`MCPTestServer` is a standalone MCP server used to verify the LunaSDK MCP module against clients outside LunaSDK. It implements the addition example from [[MCP]] over either standard IO or local Streamable HTTP, so a test can cover process startup, transport framing, strict JSON, protocol selection, discovery or initialization, tool registration, Frontend dispatch, and result serialization together.

The server accepts modern MCP revision `2026-07-28` and legacy revision `2025-06-18`. Standard IO selects a fixed facility from its first request. HTTP handles modern requests without sessions and creates independent sessions for legacy clients.

### Concepts

#### Addition tool

The server exports exactly one tool:

| Property | Value |
| --- | --- |
| MCP name | `add` |
| Frontend URL | `/math/add` |
| Input | Object containing numeric `a` and `b` properties |
| Structured output | Object containing numeric `sum` |
| Side effects | None |

The input and output descriptors use JSON Schema objects. Both schemas reject additional properties. The tool is annotated as read-only, non-destructive, idempotent, and closed-world.

#### Process standard IO

The client launches `MCPTestServer` and owns its standard input, standard output, and lifetime. Each input request and output response is one compact JSON object followed by a newline. Standard output is reserved for protocol frames; startup and failure diagnostics are written to standard error.

#### Local Streamable HTTP

The HTTP mode listens only on IPv4 loopback. The client connects to `/mcp`; the server reports its actual endpoint on standard error. HTTP responses are buffered JSON, and legacy MCP session state survives across TCP connections.

### Programming guide

#### Build the server

From the repository root, build the dedicated target:

```shell
dotnet run --project LunaBuild.csproj -- build --target MCPTestServer
```

The executable is placed under the platform-specific LunaBuild output directory:

```text
build/LunaBuild/<platform>/<architecture>/<configuration>/bin/MCPTestServer
```

On Windows, the filename has the `.exe` suffix.

#### Select a transport

No arguments, or `--stdio`, runs the standard IO transport:

```shell
dotnet run --project LunaBuild.csproj -- run --target MCPTestServer -- --stdio
```

`--http <port>` runs local Streamable HTTP. Port `0` selects an available port:

```shell
dotnet run --project LunaBuild.csproj -- run --target MCPTestServer -- --http 0
```

The server prints a line such as `MCPTestServer listening at http://127.0.0.1:57424/mcp` to standard error and then polls until the process is stopped.

#### Connect a modern MCP client

Configure the client to launch the executable as a standard IO server and to select protocol revision `2026-07-28`. A client that defaults to the pre-2026 `initialize` handshake must explicitly enable modern version negotiation.

The official TypeScript MCP SDK v2 can be used as an independent verifier:

```shell
npm install @modelcontextprotocol/client@2.0.0
```

```javascript
import { Client } from '@modelcontextprotocol/client';
import { StdioClientTransport } from '@modelcontextprotocol/client/stdio';

const client = new Client(
    { name: 'lunasdk-verifier', version: '1.0.0' },
    { versionNegotiation: { mode: { pin: '2026-07-28' } } });
const transport = new StdioClientTransport({
    command: '/absolute/path/to/MCPTestServer'
});

try
{
    await client.connect(transport);
    const tools = await client.listTools();
    const result = await client.callTool({
        name: 'add',
        arguments: { a: 12.5, b: 7.25 }
    });
    console.log(client.getProtocolEra(), tools.tools, result);
}
finally
{
    await client.close();
}
```

The expected protocol era is `modern`; the tool list contains only `add`; and the call returns `structuredContent.sum` equal to `19.75` with `isError` equal to `false`.

#### Connect a legacy MCP client

Use the same transport without modern version negotiation and restrict the verifier to the supported legacy revision:

```javascript
const client = new Client(
    { name: 'lunasdk-legacy-verifier', version: '1.0.0' },
    { supportedProtocolVersions: ['2025-06-18'] });
const transport = new StdioClientTransport({
    command: '/absolute/path/to/MCPTestServer'
});

try
{
    await client.connect(transport);
    const tools = await client.listTools();
    const result = await client.callTool({
        name: 'add',
        arguments: { a: 12.5, b: 7.25 }
    });
    console.log(client.getProtocolEra(), tools.tools, result);
}
finally
{
    await client.close();
}
```

The expected protocol era is `legacy`, with the same `add` tool and sum result.

#### Codex standard IO compatibility

Codex CLI `0.147.0-alpha.6.5`, tested on August 14, 2026, opens configured standard IO MCP servers with `initialize` and requests protocol revision `2025-06-18`. The test server selects its legacy facility and can be configured using its absolute executable path:

```toml
[mcp_servers.lunasdk_add]
command = "/absolute/path/to/MCPTestServer"
required = true
```

In the verified call, Codex discovered `add`, invoked it once with `a` equal to `12.5` and `b` equal to `7.25`, received text and structured content containing `19.75`, and returned `19.75` to the user. A future Codex release that supports modern standard IO negotiation can use the same executable and select the modern facility instead.

#### Connect Codex over Streamable HTTP

Start the HTTP server on a fixed local port, then add the reported URL to Codex configuration. The server uses no authentication because it cannot bind a non-loopback address.

```toml
[mcp_servers.lunasdk_add_http]
url = "http://127.0.0.1:43129/mcp"
required = true
```

Codex CLI `0.148.0-alpha.9`, tested on August 20, 2026, initialized this endpoint with legacy revision `2025-06-18`, discovered `add`, and completed a real tool call. The verification prompt asked Codex to calculate `31 + 11`; Codex logged `lunasdk_add_http/add` as completed and returned `42`.

The same check can be run without persisting a server entry by supplying a one-process configuration override:

```shell
codex exec --ephemeral --ignore-user-config \
  -c 'mcp_servers.lunasdk_add_http.url="http://127.0.0.1:43129/mcp"' \
  -c 'mcp_servers.lunasdk_add_http.required=true' \
  'Use the lunasdk_add_http add tool to calculate 31 + 11.'
```

This override still uses the normal Codex authentication state, but it does not add the test server to the user's persistent `config.toml`.

#### Send a modern HTTP request directly

Modern HTTP requests repeat the protocol metadata and mirrored method headers on every POST. For example, after starting the server on port `43129`:

```shell
curl --request POST http://127.0.0.1:43129/mcp \
  --header 'Content-Type: application/json' \
  --header 'Accept: application/json, text/event-stream' \
  --header 'MCP-Protocol-Version: 2026-07-28' \
  --header 'Mcp-Method: tools/call' \
  --header 'Mcp-Name: add' \
  --data '{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"add","arguments":{"a":19,"b":23},"_meta":{"io.modelcontextprotocol/protocolVersion":"2026-07-28","io.modelcontextprotocol/clientCapabilities":{}}}}'
```

The response contains `structuredContent.sum` equal to `42` and `isError` equal to `false`.

### Modern protocol message example

The following call is sent by a modern client after discovery:

```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "tools/call",
  "params": {
    "name": "add",
    "arguments": {
      "a": 12.5,
      "b": 7.25
    },
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {},
      "io.modelcontextprotocol/clientInfo": {
        "name": "example-client",
        "version": "1.0.0"
      }
    }
  }
}
```

The structured portion of the result is:

```json
{
  "sum": 19.75
}
```
