```c++
#include <Luna/Network/SocketPoller.hpp>
```

Socket polling lets one reactor thread wait efficiently for readiness on multiple non-blocking Network sockets. It completes the scheduling side of the non-blocking socket API: socket operations report `E_NOT_READY`, and a socket poller tells the reactor when an operation may make progress again.

The poller does not receive, send, accept, dispatch callbacks, or own an application event loop. Protocol implementations such as HTTP retain control of buffers, connection state, work quotas, and error handling.

## Concepts

### Readiness interests

A registration subscribes to a combination of two interests:

- `SocketEventFlag::readable`: a listener may accept, a TCP or UDP socket may receive, or a TCP peer shutdown or error may be observable.
- `SocketEventFlag::writable`: a socket may send, or a non-blocking TCP connection attempt may have completed.

The poller is level-triggered. A condition can be reported by every `poll` call until the application consumes it or removes the corresponding interest.

`SocketEventFlag::error` and `SocketEventFlag::hang_up` are output-only hints. They cannot be registration interests and are not authoritative error results. Call the relevant socket operation to observe the final result. In particular, call `ITCPSocket::get_status` after a connecting socket reports `writable`, `error`, or `hang_up`.

### Registrations and tokens

`ISocketPoller::add` returns a `socket_poll_token_t` that identifies one registration. A token remains valid until `remove` succeeds. Tokens use an internal generation so a removed token cannot accidentally refer to a later registration that reused the same registry slot.

A socket can be registered only once in one poller. The poller retains a strong reference to every registered socket, but explicit `ISocket::close` still invalidates its native handle. Always remove a socket before closing it.

`SocketEventFlag::none` creates or modifies a dormant registration. A dormant registration retains its token, socket reference, and user data but produces no events.

### Event data

Every `SocketPollEvent` contains the registration token, reported flags, and the `user_data` value supplied to `add`. User data is opaque and is not retained or released by Network.

Multiple native notifications for the same registration are combined into one output event in each `poll` call. If the event buffer is too small, the call returns only the events that fit. Level-triggered conditions remain observable by later calls while they are still true.

### Threading and wakeups

A poller is owned by one reactor thread. `add`, `modify`, `remove`, and `poll` are not thread-safe and must not be called concurrently.

`wake` is the only thread-safe poller operation. Another thread can enqueue a command in an application-owned synchronized queue and call `wake`; the blocked reactor returns from `poll`, drains the command queue, and applies registrations on its own thread. Wake notifications may be coalesced and do not appear in the event buffer.

## Programming guide

### Creating a poller

Initialize the Network module, then create a platform poller:

```c++
lulet(poller, new_socket_poller());
```

The platform implementation uses epoll on Linux, kqueue on macOS and iOS, and WSAPoll on Windows.

### Registering a listener

Create, bind, and listen before registering the TCP listener for read readiness:

```c++
lulet(listener, new_tcp_socket(AddressFamily::ipv4));
lupanic_if_failed(listener->bind(bind_address));
lupanic_if_failed(listener->listen(I32_MAX));

lulet(listener_token, poller->add(
    listener.get(),
    SocketEventFlag::readable,
    listener_context));
```

When the listener becomes readable, call `accept` repeatedly until it returns `E_NOT_READY`, subject to the reactor's fairness quota. Register every accepted socket that should remain active.

### Starting a TCP connection

Register a connecting socket for write readiness after `connect` successfully starts it:

```c++
lupanic_if_failed(socket->connect(remote_address));
lulet(token, poller->add(
    socket.get(),
    SocketEventFlag::writable,
    connection_context));
```

After a readiness event, call `get_status`. A `writable` event can represent either successful connection or failure:

```c++
switch(socket->get_status())
{
case TCPConnectionState::connected:
    lupanic_if_failed(poller->modify(token, SocketEventFlag::readable));
    break;
case TCPConnectionState::error:
    handle_connection_error(socket->get_error());
    break;
default:
    break;
}
```

### Managing read and write interests

Keep `readable` enabled while the protocol accepts inbound data. On a read event, call `receive` until it returns `E_NOT_READY`, reaches the connection's work quota, or reports zero received bytes.

Enable `writable` only when a connection is still connecting or has unsent output. Connected TCP sockets are normally writable, so leaving the interest enabled without pending output causes the reactor to spin.

```c++
SocketEventFlag interests = SocketEventFlag::readable;
if(!pending_output.empty()) interests |= SocketEventFlag::writable;
lupanic_if_failed(poller->modify(token, interests));
```

After the output queue becomes empty, remove the write interest again.

### Polling events

Pass a non-empty event buffer to `poll`. A timeout of `0` performs a non-blocking query; `U32_MAX` waits indefinitely.

```c++
SocketPollEvent events[64];
lulet(event_count, poller->poll(
    Span<SocketPollEvent>(events, 64),
    U32_MAX));

for(usize i = 0; i < event_count; ++i)
{
    const SocketPollEvent& event = events[i];
    dispatch_socket_event(event.token, event.events, event.user_data);
}
```

`poll` may return `E_INTERRUPTED`; the reactor can process its other work and retry. A return value of zero means the timeout expired or `wake` interrupted the wait without a socket event.

### Removing and closing sockets

Unregister a socket before closing it. Removed or stale tokens return `E_NOT_FOUND` from later `modify` and `remove` calls.

```c++
lupanic_if_failed(poller->remove(token));
socket->close();
```

Destroying the poller releases its registered socket references and platform polling resources. Sockets that remain externally referenced stay open; releasing the last reference destroys and closes the socket normally.

### Waking a reactor from another thread

The sending thread owns synchronization for its command queue:

```c++
{
    MutexGuard guard(command_mutex);
    commands.push_back(command);
}
poller->wake();
```

The reactor resumes from `poll`, drains the command queue, and performs any `add`, `modify`, or `remove` operations itself.
