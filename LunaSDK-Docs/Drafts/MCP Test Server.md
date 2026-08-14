# MCP Test Server

`MCPTestServer` is a standalone standard IO MCP server used to verify the LunaSDK MCP module against clients outside LunaSDK. It implements the addition example from [[MCP Server]] as a real child process, so a test covers process startup, newline-delimited standard IO framing, strict JSON, protocol selection, discovery or initialization, tool registration, Frontend dispatch, and result serialization together.

The server accepts modern MCP revision `2026-07-28` and legacy revision `2025-06-18`. Its first request selects the facility for that process connection.

## Concepts

### Addition tool

The server exports exactly one tool:

| Property | Value |
| --- | --- |
| MCP name | `add` |
| Frontend URL | `/math/add` |
| Input | Object containing numeric `a` and `b` properties |
| Structured output | Object containing numeric `sum` |
| Side effects | None |

The input and output descriptors use JSON Schema objects. Both schemas reject additional properties. The tool is annotated as read-only, non-destructive, idempotent, and closed-world.

### Process standard IO

The client launches `MCPTestServer` and owns its standard input, standard output, and lifetime. Each input request and output response is one compact JSON object followed by a newline. Standard output is reserved for protocol frames; startup and failure diagnostics are written to standard error.

## Programming guide

### Build the server

From the repository root, build the dedicated target:

```shell
dotnet run --project LunaBuild.csproj -- build --target MCPTestServer
```

The executable is placed under the platform-specific LunaBuild output directory:

```text
build/LunaBuild/<platform>/<architecture>/<configuration>/bin/MCPTestServer
```

On Windows, the filename has the `.exe` suffix.

### Connect a modern MCP client

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

### Connect a legacy MCP client

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

### Codex compatibility

Codex CLI `0.147.0-alpha.6.5`, tested on August 14, 2026, opens configured standard IO MCP servers with `initialize` and requests protocol revision `2025-06-18`. The test server selects its legacy facility and can be configured using its absolute executable path:

```toml
[mcp_servers.lunasdk_add]
command = "/absolute/path/to/MCPTestServer"
required = true
```

In the verified call, Codex discovered `add`, invoked it once with `a` equal to `12.5` and `b` equal to `7.25`, received text and structured content containing `19.75`, and returned `19.75` to the user. A future Codex release that supports modern standard IO negotiation can use the same executable and select the modern facility instead.

## Direct protocol example

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
