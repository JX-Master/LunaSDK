## Status
Approved.

## Last updated
2026/8/14

## Background
ADR-0008 changed Network sockets to permanently non-blocking APIs. Non-blocking operations report `BasicError::not_ready`, but the module currently provides no platform-independent way to wait until one of several sockets can make progress. A consumer must otherwise busy-retry, periodically sleep, or use `ISocket::get_native_handle` with platform-specific polling code.

The planned HTTP service layer needs to manage listeners and multiple client connections without dedicating a blocked thread to each socket. The same readiness facility will also be useful to other Network consumers, so implementing it privately in HTTP would duplicate the platform abstraction.

Runtime's `IWaitable` represents one synchronization object with blocking `wait` and non-blocking `try_wait`. It does not represent read and write interests, return multiple ready objects, support a timeout, or provide the error and hang-up hints required by sockets. Making sockets implement `IWaitable` would therefore lose essential readiness information and conflate synchronization objects with network endpoints.

## Decision
Network provides a socket-specific readiness poller in a separate public `SocketPoller.hpp` header. The poller uses level-triggered readiness and does not perform socket I/O on behalf of the caller. A consumer reacts to an event by calling `accept`, `receive`, `send`, or `get_status` until the operation reports `BasicError::not_ready` or its immediate work quota is exhausted.

The public API uses these concepts:

```cpp
enum class SocketEventFlag : u8
{
    none = 0x00,
    readable = 0x01,
    writable = 0x02,
    error = 0x04,
    hang_up = 0x08,
};

struct socket_poll_token_t
{
    u64 value;
};

struct SocketPollEvent
{
    socket_poll_token_t token;
    SocketEventFlag events;
    opaque_t user_data;
};

struct ISocketPoller : virtual Interface
{
    virtual R<socket_poll_token_t> add(
        ISocket* socket,
        SocketEventFlag interests,
        opaque_t user_data = nullptr) = 0;

    virtual RV modify(
        socket_poll_token_t token,
        SocketEventFlag interests) = 0;

    virtual RV remove(socket_poll_token_t token) = 0;

    virtual R<usize> poll(
        Span<SocketPollEvent> events,
        u32 timeout_ms = U32_MAX) = 0;

    virtual void wake() = 0;
};
```

`readable` and `writable` are registration interests. `error` and `hang_up` are output-only hints and are reported whenever the platform provides them for an active native registration. Passing output-only or unknown flags to `add` or `modify` returns `BasicError::bad_arguments`. `SocketEventFlag::none` creates or changes a registration to a dormant state without changing its token.

`readable` does not guarantee that bytes will be returned. It can also indicate a queued TCP connection, an orderly peer shutdown, or a pending socket error. `writable` does not prove that a non-blocking TCP connection succeeded. A connecting TCP socket must call `get_status` after a readiness notification and use `get_error` if the state becomes `error`. A connected socket must use the result of its next transfer operation as authoritative.

Each successful `add` returns an opaque, non-zero token composed internally from a slot index and generation. Removing and reusing a slot changes its generation, so a delayed native event or stale caller token cannot refer to a newer registration. `modify` and `remove` return `BasicError::not_found` for invalid, removed, or stale tokens. Events referring to stale native tokens are silently discarded.

A socket may be registered at most once in one poller. A duplicate `add` returns `BasicError::already_exists`. The poller retains one strong reference to every registered socket, but the application must still remove the registration before calling `ISocket::close`; retaining the object does not keep an explicitly closed native handle valid. Destroying the poller releases all retained references and platform resources.

`poll` waits until readiness, timeout, interruption, or an explicit wake. `timeout_ms == 0` performs a non-blocking query, and `U32_MAX` waits indefinitely. The output span must contain at least one element. The call writes no more than the span capacity and returns the number written. Multiple native notifications for the same token in one call are coalesced into one `SocketPollEvent`. Because readiness is level-triggered, events omitted due to output capacity remain observable while their conditions remain true.

The poller is not thread-safe. Its registration methods and `poll` are owned by one reactor thread. `wake` is the only thread-safe method and may be called by another thread to make a blocked `poll` return. Wake notifications are coalesced and are not exposed as `SocketPollEvent` entries. Cross-thread users enqueue commands in an application-owned synchronized queue, call `wake`, and let the reactor thread apply those commands.

The first platform backends are:

- Linux: `epoll` without `EPOLLET`.
- macOS and iOS: `kqueue` without `EV_CLEAR` for socket filters.
- Windows: `WSAPoll`.
- The POSIX backend uses an internal non-blocking pipe for cross-thread wakeups. The Windows backend uses an internal connected loopback UDP pair.

Windows remains readiness-oriented to match the public non-blocking `receive` and `send` APIs. Network does not emulate readiness through IOCP. A future completion-oriented API, if required by measured Windows server workloads, will expose overlapped buffer ownership and completion semantics explicitly rather than hiding them behind this poller.

The poller does not provide callbacks, timers, task queues, fairness quotas, or worker-thread dispatch. These policies belong to an event loop or protocol service above Network. In particular, consumers should subscribe to `writable` only while connecting or while unsent output exists; level-triggered connected sockets are normally writable and would otherwise cause a busy loop.

## Impact
HTTP and other Network consumers can wait for many sockets through one platform-independent API. They no longer need to access native handles for ordinary readiness handling, and the non-blocking socket contract has a complete scheduling mechanism.

The API deliberately exposes readiness rather than completion. Callers must still maintain receive buffers, preserve unsent suffixes, drain accepts and receives, update write interests, interpret TCP connection state, and remove sockets before closing them.

The strong-reference registry and generation checks add small per-registration memory and bookkeeping costs. `WSAPoll` scans all registered descriptors on Windows, so its scaling characteristics are weaker than epoll or kqueue. This is acceptable for the initial HTTP and local-tool workloads and does not change the public API if a more appropriate readiness backend becomes available.

Only `wake` is safe across threads. This keeps socket and poller mutation deterministic on one reactor thread, but applications must provide their own command queue when other threads need to request network work.

## Alternatives considered
### Implement polling privately in HTTP
This would make HTTP functional sooner but force every later Network consumer to repeat the same platform code. It would also make socket scheduling behavior part of the HTTP implementation rather than the socket abstraction.

### Make sockets implement `IWaitable`
`IWaitable` cannot express read versus write readiness or return multiple socket events. Waiting on one socket at a time would also prevent a scalable reactor.

### Expose callbacks from Network
Callbacks would require Network to own dispatch, reentrancy, thread affinity, and handler lifetime policies. Returning plain events keeps Network independent of the application event-loop model.

### Use edge-triggered polling
Edge triggering can reduce repeated notifications but makes missed drains and interest changes much easier to implement incorrectly. Level triggering has a portable, predictable contract and is adequate until profiling demonstrates a need for an opt-in edge mode.

### Use IOCP as the Windows backend
IOCP reports completion of submitted overlapped operations rather than readiness for immediate `receive` or `send`. Hiding that model behind readiness would require internal buffer ownership and speculative I/O that conflict with the public socket contract.

### Store raw socket pointers without retaining them
This would avoid one reference per registration but would allow socket destruction to leave native events associated with dangling object pointers. Generational tokens avoid returning object pointers, and strong references keep each registered object alive until removal.

## Version history
* **2026/8/14** Proposed and approved.
* **2026/8/14** Implemented the public poller, platform backends, tests, and Socket Polling documentation.
