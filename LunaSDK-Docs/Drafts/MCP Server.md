# MCP Server

The MCP module exposes selected Frontend functions as tools through Model Context Protocol (MCP) revisions `2026-07-28` and `2025-06-18`. It supplies the protocol shell that Frontend deliberately omits: JSON-RPC validation, protocol selection and lifecycle, version and capability checks, server discovery, tool metadata, tool invocation responses, strict JSON serialization, and newline-delimited standard IO framing.

The implementation is a synchronous tools server with two protocol facilities over one shared Frontend and tool registry. The modern `2026-07-28` facility implements `server/discover`, `tools/list`, and `tools/call`. The legacy `2025-06-18` facility implements `initialize`, `notifications/initialized`, `ping`, `tools/list`, and `tools/call`. Prompts, resources, subscriptions, cancellation, progress, Multi Round-Trip Requests, Streamable HTTP, and protocol extensions are outside the current scope. See [[ADR-0007 Implement MCP 2026-07-28 tools server]] for the architectural decision and scope.

## Concepts

### MCP server

An MCP server owns the protocol-facing state for one client connection. It retains one `Frontend::IFrontend` instance, publishes selected Frontend functions in an MCP tool registry, and processes one MCP message at a time.

The first valid JSON-RPC request selects the protocol facility for the lifetime of the server object. An `initialize` request selects legacy `2025-06-18`; every other request selects modern `2026-07-28`. Notifications received before the first request do not select a facility. A transport must create a separate MCP server object for each independent client connection so that protocol selection and legacy initialization state are not shared between clients.

The MCP server and its retained Frontend object are not thread-safe. An application that shares either object between threads must synchronize all access to both objects.

### Server descriptor

`ServerDesc` describes the server implementation returned during discovery or initialization and controls modern discovery and tool-list cache hints. Its required `name` and `version` fields identify the implementation. Optional display fields and instructions improve client presentation and model guidance.

`discovery_ttl_ms` and `tools_ttl_ms` control the `ttlMs` value of `server/discover` and `tools/list`. `cache_scope` is `private_cache` by default so that a response is not reused across authorization contexts unless the application explicitly declares it public.

### Tool export

`ToolDesc` associates one MCP tool name and descriptor with one Frontend function URL. Tool registration is explicit: a Frontend function is never exposed merely because it exists.

The MCP tool name must be between 1 and 128 ASCII characters and may contain letters, digits, `_`, `-`, and `.`. `input_schema` must be a JSON object whose root `type` is `object`. Optional output schema, annotations, icons, and metadata must have the types required by the MCP schema. Metadata keys follow the MCP `_meta` key syntax. All descriptor data must be representable as strict JSON.

The MCP module validates the shared request envelope and requires `tools/call` arguments to be an object. Multi Round-Trip Request fields are rejected because that feature is outside the implementation. The Frontend handler must validate tool-specific values against the published input schema, ensure successful output conforms to any published output schema, and enforce authorization, security, and business rules. The MCP module does not contain a partial JSON Schema evaluator.

Tool descriptors are projected into the schema of the selected protocol. The modern definition includes supported presentation fields such as icons. The `2025-06-18` definition omits fields that did not exist in that revision while retaining title, description, input and output schemas, annotations, and custom metadata.

### Message processor

`IMCPServer::process_message` accepts one already-decoded `Variant` message and returns either one response object or no response for a notification. `IMCPServer::process_json` performs the same operation on one complete JSON byte range using strict JSON decoding and encoding. Invalid JSON produces a JSON-RPC parse-error response rather than a LunaSDK error.

The processor accepts one JSON-RPC object per call. JSON arrays are not treated as batches and are rejected as invalid requests.

Modern requests must contain both `io.modelcontextprotocol/protocolVersion` and `io.modelcontextprotocol/clientCapabilities` in their `_meta` object. Each request is validated independently against `2026-07-28`.

Legacy clients begin with `initialize`. The server responds with protocol version `2025-06-18`, the tools capability, implementation information, and optional instructions. Tool operations are rejected until the client sends `notifications/initialized`. The `ping` request is accepted during and after initialization. Repeated initialization and attempts to switch protocol facilities are rejected.

Frontend handler errors are tool results with `isError: true`, which lets a model inspect and potentially correct the call. An unknown tool, malformed request, unsupported method, missing Frontend function, or other protocol/server failure is a JSON-RPC error response. Modern results include modern result metadata and cache fields where applicable; legacy results omit those fields.

### Standard IO runner

`run_stdio_server` continuously reads raw bytes from standard input, divides them at newline characters, processes each complete frame, and writes each response followed by one newline to standard output. It handles short reads and short writes and exits successfully when standard input reaches EOF after a complete frame.

Every input message must be terminated by a newline and must not exceed `StdioServerOptions::max_message_size`. Embedded newlines are not allowed in one message. A process using this transport must reserve standard output exclusively for MCP frames; diagnostics should go to standard error or another sink.

## Programming guide

### Initialize the modules

Initialize Runtime and register the MCP module. The MCP module depends on VariantUtils and Frontend and registers their required runtime metadata through the normal module dependency graph.

```cpp
Luna::init();
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
        return set_error(BasicError::bad_arguments(), "a and b must be numbers");
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

Use `process_message` when another component already owns JSON decoding, or `process_json` when it can supply one complete JSON frame. A notification returns `has_response == false` or an empty output string.

```cpp
R<String> output = server->process_json(request_bytes, request_size);
if(output.valid() && !output.get().empty())
{
    // Send output.get() as one response message using the selected transport.
}
```

Transport adapters must preserve the one-message boundary and create one MCP server object per independent connection. The MCP core neither reads transport headers nor associates authorization state or external request IDs.

The adapter does not need to select a protocol version explicitly. The first valid JSON-RPC request performs selection. For a legacy client, pass `initialize`, `notifications/initialized`, and subsequent requests to the same server object in order.

### Run the standard IO transport

After all tools are registered, run the synchronous standard IO loop. Do not write unrelated text to standard output while it is active.

```cpp
MCP::StdioServerOptions options;
options.max_message_size = 16 * 1024 * 1024;
lupanic_if_failed(MCP::run_stdio_server(server, options));
```

The call returns when standard input reaches EOF or when an IO/framing error occurs. Close the modules and Runtime only after the runner returns.

```cpp
Luna::close();
```

## Standalone interoperability test server

The `MCPTestServer` executable provides a process-level example and interoperability target for the standard IO transport. It exports the `add` tool used throughout this guide without writing diagnostics to standard output. See [[MCP Test Server]] for its tool contract, build instructions, and third-party client verification procedure.
