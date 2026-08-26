```c++
#include <Luna/Runtime/Log.hpp>
```

LunaSDK contains a log system that can be used for logging and debugging purposes.

## Log entries

A log entry consists of a `LogVerbosity`, a UTF-8 tag, and a UTF-8 message. Tags identify the module or subsystem that emitted the entry and let handlers apply their own filtering policy. The available verbosity levels, from most severe to most detailed, are `fatal_error`, `error`, `warning`, `info`, `debug`, and `verbose`.

Use `log` to format and submit a message to all registered global handlers:

```c++
log(LogVerbosity::info, "Asset", "Loaded asset %s.", asset_name);
```

The convenience functions `log_verbose`, `log_debug`, `log_info`, `log_warning`, and `log_error` select the corresponding verbosity. Every formatting function also has a `logv` form that accepts `VarList`. Use `log_unformatted` when the tag and message are already available as explicitly sized character ranges.

There are also overloads that take a `LogHandler&` as the first argument. Those overloads send the entry directly to one handler instead of dispatching it to the global handler list.

## Log handler

`LogHandler` is a function object with the following callback arguments:

```c++
void handle_log(LogVerbosity verbosity,
    const c8* tag, usize tag_length,
    const c8* message, usize message_length);
```

The tag and message are read-only ranges described by their explicit lengths. A handler can display, persist, or filter the entry. Register a handler with `register_log_handler`; the returned identifier is passed to `unregister_log_handler` when the handler is no longer needed.

## Built-in log handlers

The log system includes two built-in log handlers: the platform log handler and the file log handler.

### Platform log handler

The platform log handler outputs log messages to the platform's default logging device. On desktop platforms this is normally the process console or standard output. The handler is disabled by default; call `set_log_to_platform_enabled` to enable or disable it and `set_log_to_platform_verbosity` to configure its verbosity.

Keep the platform log handler disabled when standard output is reserved for a framed protocol such as MCP stdio, because log messages written to standard output corrupt the protocol stream. Use the file log handler or a custom handler that writes diagnostics to standard error instead. See [[Standard IO]] for the raw standard-stream APIs.

### File log handler

The file log handler writes messages to a configurable file. It is disabled by default; use `set_log_to_file_enabled` to enable it, `set_log_to_file_verbosity` to set its verbosity threshold, and `set_log_file` to select the destination. If no file is selected, it uses `./Log.txt` in the current working directory.

File output is buffered. Call `flush_log_to_file` when buffered messages must be written immediately. Runtime also flushes this buffer during normal shutdown.
