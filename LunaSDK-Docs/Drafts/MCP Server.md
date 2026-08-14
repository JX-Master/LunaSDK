# MCP Server

The MCP module exposes selected Frontend functions as tools through Model Context Protocol (MCP) revision `2026-07-28`. It supplies the protocol shell that Frontend deliberately omits: JSON-RPC validation, per-request MCP metadata and version checks, server discovery, tool metadata, tool invocation responses, strict JSON serialization, and newline-delimited standard IO framing.

The first implementation is a synchronous tools server. It implements `server/discover`, `tools/list`, and `tools/call`. It does not implement legacy `initialize` negotiation, prompts, resources, subscriptions, cancellation, progress, Multi Round-Trip Requests, Streamable HTTP, or protocol extensions. See [[ADR-0007 Implement MCP 2026-07-28 tools server]] for the architectural decision and scope.

## Concepts

### MCP server

An MCP server owns the protocol-facing state for one service. It retains one `Frontend::IFrontend` instance, publishes selected Frontend functions in an MCP tool registry, and processes one MCP message at a time.

The MCP server and its retained Frontend object are not thread-safe. An application that shares either object between threads must synchronize all access to both objects.

### Server descriptor

`ServerDesc` describes the server implementation returned in result metadata and controls discovery and tool-list cache hints. Its required `name` and `version` fields identify the implementation. Optional display fields and instructions improve client presentation and model guidance.

`discovery_ttl_ms` and `tools_ttl_ms` control the `ttlMs` value of `server/discover` and `tools/list`. `cache_scope` is `private_cache` by default so that a response is not reused across authorization contexts unless the application explicitly declares it public.

### Tool export

`ToolDesc` associates one MCP tool name and descriptor with one Frontend function URL. Tool registration is explicit: a Frontend function is never exposed merely because it exists.

The MCP tool name must be between 1 and 128 ASCII characters and may contain letters, digits, `_`, `-`, and `.`. `input_schema` must be a JSON object whose root `type` is `object`. Optional output schema, annotations, icons, and metadata must have the types required by the MCP schema. Metadata keys follow the MCP `_meta` key syntax. All descriptor data must be representable as strict JSON.

The MCP module validates the shared request envelope and requires `tools/call` arguments to be an object. Multi Round-Trip Request fields are rejected because that feature is outside the first implementation. The Frontend handler must validate tool-specific values against the published input schema, ensure successful output conforms to any published output schema, and enforce authorization, security, and business rules. The MCP module does not contain a partial JSON Schema evaluator.

### Message processor

`IMCPServer::process_message` accepts one already-decoded `Variant` message and returns either one response object or no response for a notification. `IMCPServer::process_json` performs the same operation on one complete JSON byte range using strict JSON decoding and encoding. Invalid JSON produces a JSON-RPC parse-error response rather than a LunaSDK error.

The processor accepts one JSON-RPC object per call. JSON arrays are not treated as batches in MCP revision `2026-07-28` and are rejected as invalid requests. The request `_meta` object must contain both `io.modelcontextprotocol/protocolVersion` and `io.modelcontextprotocol/clientCapabilities`. Only protocol version `2026-07-28` is supported.

Frontend handler errors are tool results with `isError: true`, which lets a model inspect and potentially correct the call. An unknown tool, malformed request, unsupported method, missing Frontend function, or other protocol/server failure is a JSON-RPC error response.

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

Transport adapters must preserve the one-message boundary. The MCP core neither reads transport headers nor associates connections, authorization state, or external request IDs.

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
