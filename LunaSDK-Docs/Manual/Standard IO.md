```cpp
#include <Luna/Runtime/StdIO.hpp>
```

The Runtime standard IO feature exposes the process standard input, standard output, and standard error as blocking raw-byte operations. It is intended for command-line programs, redirected files, pipes, and framed transports such as MCP stdio.

Unlike text-oriented console APIs, standard IO does not decode UTF-8, detect C string terminators, or add and remove newlines. Every byte in the requested range, including `0`, `\r`, and `\n`, is data. Runtime also does not change the Windows console code page or mode.

## Concepts

### Standard streams

A process has three standard streams:

1. **Standard input** supplies bytes to the process.
2. **Standard output** carries the process's normal result or protocol data.
3. **Standard error** carries diagnostics independently from standard output.

These streams are process-global platform resources rather than LunaSDK objects. Runtime does not own or close them and does not expose an `IStream` for them. Each operation uses the standard handle that is current when the function is called, so redirection performed after `Luna::init` is observed by subsequent calls.

### Raw byte operations

Each standard IO function performs one native blocking I/O operation. A successful read or write may transfer fewer bytes than requested. The caller must inspect `read_bytes` or `write_bytes` and repeat the operation when an exact byte count is required.

End of input is represented by a successful read of zero bytes. A zero-size operation also succeeds with zero transferred bytes without accessing the buffer. Writing to a pipe whose reader has closed fails with `E_BAD_PIPE`; on POSIX platforms Runtime prevents that operation's `SIGPIPE` from terminating the process.

Calls are not serialized. Concurrent calls are allowed, but the ordering of bytes from concurrent writers is unspecified. Applications must synchronize replacement of standard handles against active I/O calls.

## Programming guide

### Initialization and shutdown

Initialize Runtime before using standard IO and close Runtime during normal application shutdown:

```cpp
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/StdIO.hpp>

int main()
{
    if(Luna::failed(Luna::init())) return 1;

    // Use standard IO here.

    Luna::close();
    return 0;
}
```

There are no standard IO objects to retain or release. Runtime does not close the host process's standard handles during shutdown.

### Reading standard input

Call `read_standard_input` with a writable byte buffer. Always use the returned byte count; the function does not append a null terminator.

```cpp
Luna::u8 buffer[4096];
Luna::usize read_bytes = 0;
Luna::RV result = Luna::read_standard_input(buffer, sizeof(buffer), &read_bytes);
if (!result.valid())
{
    // Handle result.errcode().
}
else if (read_bytes == 0)
{
    // End of input.
}
else
{
    // Consume exactly read_bytes bytes.
}
```

### Writing standard output and standard error

Use `write_standard_output` for normal results or protocol frames and `write_standard_error` for diagnostics. Neither function stops at an embedded null byte.

```cpp
const Luna::u8 response[] = {'O', 'K', '\n', 0};
Luna::usize written_bytes = 0;
Luna::RV result = Luna::write_standard_output(
    response, sizeof(response), &written_bytes);
```

If the complete buffer must be written, repeat the operation for the remaining suffix after every successful short write. Treat a successful zero-byte write of a non-empty range as no forward progress rather than retrying indefinitely.

### Using standard IO for a framed protocol

Reserve standard output exclusively for protocol bytes. Do not enable the built-in platform log handler in a process that uses standard output as an MCP or another framed transport, because platform log messages may corrupt the frame stream. Either keep that handler disabled, log to a file, or emit deliberately formatted diagnostics through `write_standard_error`.

The transport shell remains responsible for buffering short reads, parsing and assembling protocol frames, and repeating short writes. Standard IO only transfers bytes and does not parse JSON-RPC, MCP messages, headers, JSON, or text lines.

## Platform notes

On POSIX platforms, the functions operate on file descriptors `0`, `1`, and `2`. On Windows, they fetch `STD_INPUT_HANDLE`, `STD_OUTPUT_HANDLE`, or `STD_ERROR_HANDLE` for every call and use the current handle directly.

When a Windows standard handle refers to an interactive console, the console's existing mode and code page continue to control its text behavior. The raw standard IO feature does not impose UTF-8 or provide terminal text conversion. Redirected files and pipes are transferred as byte streams.
