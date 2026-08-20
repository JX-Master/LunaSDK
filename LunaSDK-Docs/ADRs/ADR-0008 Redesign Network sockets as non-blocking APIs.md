## Status
Approved.

## Last updated
2026/8/14

## Background
The Network module currently models a TCP socket as both `ISocket` and `IStream`. `IStream` promises blocking reads and complete blocking writes, so the platform implementations create blocking native sockets, suspend in `accept`, `connect`, and `receive`, and repeat `send` until the requested byte range has been written. UDP operations are also performed on blocking native sockets.

This model does not fit an event-driven HTTP server. A blocking socket would require one thread per active connection or an additional asynchronous socket abstraction that duplicates the existing TCP and UDP APIs. Keeping both blocking and non-blocking modes on one object would also make the behavior of familiar operations depend on mutable mode and would retain the misleading implication that a TCP socket is an ordinary `IStream`.

A repository-wide reference scan found no Network consumers in application or feature modules. Only `NetworkTest` uses the socket APIs, and the Network manual documents them. The APIs can therefore be redesigned before the HTTP module depends on them without changing the behavior of an existing repository feature. The redesign is nevertheless a source and ABI breaking change for external SDK consumers.

## Decision
All TCP and UDP sockets created by the Network module use the platform's non-blocking mode for their complete lifetime. The public API does not expose a blocking mode or a mode switch, and Network does not provide blocking convenience operations. Higher-level code that needs to wait for progress must use a socket readiness facility and repeat the relevant non-blocking operation.

`ITCPSocket` no longer derives from `IStream`. Its byte transfer operations are named `receive` and `send` instead of `read` and `write` so their names do not suggest the blocking `IStream` contract. Each call performs at most one native receive or send operation:

```cpp
virtual RV receive(
    void* buffer,
    usize size,
    usize* out_received_bytes = nullptr) = 0;

virtual RV send(
    const void* buffer,
    usize size,
    usize* out_sent_bytes = nullptr) = 0;
```

Non-zero-size `receive` and `send` calls are valid in the `connected` and `peer_closed` states and return `BasicError::bad_calling_time` in other states. They return `BasicError::not_ready` when the operation would block. A successful `send` may report fewer bytes than requested, and the caller retains responsibility for the unsent suffix. A successful non-zero-size `receive` that reports zero bytes indicates an orderly peer shutdown. Zero-size operations succeed with zero transferred bytes and do not probe connection closure.

UDP `send_to` and `receive_from` remain single-datagram operations but use the same non-blocking `BasicError::not_ready` behavior. A successfully accepted TCP socket is placed in non-blocking mode before it is returned to the caller.

Starting a TCP connection and observing its completion are separated by state rather than by a `finish_connect` operation. `connect` returns `ok` when the connection either completes immediately or is successfully started asynchronously. It returns an error only when the attempt cannot be started. Calling `connect` in an invalid state returns an appropriate calling-time or already-connected error.

`ITCPSocket` exposes a non-blocking status query:

```cpp
enum class TCPConnectionState : u8
{
    not_connected,
    connecting,
    connected,
    listening,
    peer_closed,
    error,
    closed,
};

virtual TCPConnectionState get_status() = 0;
virtual ErrCode get_error() = 0;
```

A newly created or merely bound TCP socket is `not_connected`. A successfully initiated asynchronous connection is `connecting`; `get_status` changes it to `connected` after the platform confirms completion. A socket becomes `listening` after `listen` succeeds, and a socket returned by `accept` starts as `connected`.

`peer_closed` means an orderly shutdown of the peer's sending direction has been observed. It is distinct from `closed` because TCP can remain writable after the peer stops sending. `error` means that the connection or the non-blocking platform status query has failed. `closed` means the local native socket has been released and no further network operation is permitted. `BasicError::not_ready` from a transfer operation does not change connection state.

`get_status` always returns a `TCPConnectionState`; querying object state is not itself a fallible API operation. The socket stores the concrete translated error separately, and `get_error` returns that cached `ErrCode`, or `ErrCode(0)` if the socket has not encountered an error. If refreshing a connecting socket's platform state fails, `get_status` records the error and returns `TCPConnectionState::error`.

If `connect` fails before an attempt can be started, it returns that error, stores the same error, and changes the state to `error`. Fatal connection errors observed by `receive`, `send`, or connection completion are cached in the same way. Calling `close` after an error transitions the public state to `closed` and releases the native platform resources, but retains the cached error so diagnostics may still retrieve it with `get_error`.

While a connection is `connecting`, `get_status` performs only a non-blocking platform query. It reports `connecting` while completion is still pending, `connected` after successful completion, or `error` after failure. The socket poller may report that a connecting socket is writable or has an error, but callers must use `get_status` as the authoritative completion result and `get_error` to retrieve the failure reason.

The socket lifetime operation is named `close` rather than `shutdown` and is placed on `ISocket` so TCP listeners, TCP connections, and UDP sockets share the same lifetime operation:

```cpp
virtual void close() = 0;
```

`close` performs a full local close, is idempotent, and releases the native socket handle immediately. Socket destruction calls `close` automatically. Network does not expose TCP half-close in this API; a future feature that has a concrete half-close use case may introduce a separately named operation.

Socket objects remain non-thread-safe. An application must synchronize access or assign each socket to one reactor thread. The socket poller provides a thread-safe wake operation for cross-thread reactor commands rather than making individual sockets concurrently callable.

The implementation maps POSIX `EAGAIN` and `EWOULDBLOCK`, and Windows `WSAEWOULDBLOCK`, to `BasicError::not_ready`. Platform results that mean an asynchronous connection attempt has started are consumed by `connect` and exposed as the `connecting` state rather than returned to the caller as `BasicError::in_progress`.

Because the interface inheritance, virtual methods, and behavior change, the affected public socket interfaces receive newly generated GUIDs. `NetworkTest` is rewritten around non-blocking state transitions, short transfers, would-block results, orderly peer closure, explicit close, and both IPv4 and IPv6. The Network manual is updated before the redesigned API is considered complete.

## Impact
The Network module has one socket execution model that maps directly to native event-driven networking. HTTP and other network services can manage many connections without dedicating a blocked LunaSDK thread to every connection, and callers cannot accidentally switch a socket between incompatible blocking modes.

Callers must handle `BasicError::not_ready`, partial TCP sends, asynchronous connection state, and readiness-driven retries. Code that requires a complete write or a blocking operation must build that policy above Network rather than relying on the socket object. Removing `IStream` also prevents TCP sockets from being passed directly to helpers designed around blocking stream semantics.

Explicit `close` makes resource release deterministic and consistent with other LunaSDK lifetime APIs. Omitting half-close keeps the base socket API small, while the `peer_closed` state preserves the information needed to finish sending after receiving an orderly peer shutdown.

External source code using `ITCPSocket::read`, `ITCPSocket::write`, or conversion to `IStream` must migrate to `receive`, `send`, and readiness-driven repetition. Existing binaries compiled against the old interface GUIDs are not compatible with the redesigned module.

## Alternatives considered
### Preserve `IStream` and add a non-blocking socket capability
This would preserve compatibility but leave two overlapping I/O interfaces on every TCP socket. The same native handle would have to satisfy both blocking `IStream` behavior and non-blocking reactor behavior, adding state and implementation complexity for consumers that do not exist in the repository.

### Make blocking mode configurable
This was rejected because `receive`, `send`, `accept`, and `connect` would change behavior based on mutable object state. It would also allow event-loop code to block accidentally and make accepted-socket inheritance rules part of the public contract.

### Keep the names `read` and `write`
This was rejected because LunaSDK already uses those names for `IStream` operations that block and, for writes, normally attempt to transfer the complete range. `receive` and `send` communicate socket semantics and partial progress more clearly.

### Return `BasicError::in_progress` from `connect` and provide `finish_connect`
This was rejected because successfully starting a connection is a successful command. Connection progress belongs to socket state, and a separate finishing command would make callers coordinate two operation-specific APIs in addition to readiness events. `get_status` provides one authoritative state query before and after poller notification.

### Return `R<TCPConnectionState>` from `get_status`
This was rejected because reading the socket object's cached state is not itself a fallible operation. Conflating connection failure with failure of the status query would also prevent callers from observing a stable `error` state. `get_status` therefore returns the state directly, and `get_error` provides the separately cached failure reason.

### Retain the name `shutdown`
Native socket APIs commonly use shutdown to mean disabling only one direction of a TCP connection. The redesigned operation releases the complete socket and matches the `close` lifetime terminology used elsewhere in LunaSDK, so retaining `shutdown` would be both inconsistent and semantically misleading.

## Version history
* **2026/8/14** Proposed and approved.
* **2026/8/14** Made `get_status` infallible and separated the connection error code into `get_error`.
* **2026/8/14** Implemented the redesigned APIs for POSIX and Windows, and updated Network tests and documentation.
* **2026/8/14** Updated readiness references after ADR-0009 introduced the Network socket poller.
