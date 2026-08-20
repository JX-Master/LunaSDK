```c++
#include <Luna/Network/Network.hpp>
```

The `Network` module provides non-blocking TCP and UDP socket APIs for IPv4 and IPv6 on Windows, macOS, and Linux. Add and initialize the module before using any socket API:

```c++
init();
lupanic_if_failed(add_modules({Luna::Network::module_network()}));
lupanic_if_failed(init_modules());
```

## Socket model

Every socket created by the module is non-blocking for its complete lifetime. The module does not provide a blocking mode or blocking convenience operations. An operation that cannot make progress immediately returns `BasicError::not_ready`; wait for the corresponding native socket readiness event before trying it again.

TCP sockets do not implement `IStream`. `receive` and `send` are available in the `connected` and `peer_closed` states, perform at most one native socket operation, and return `BasicError::bad_calling_time` in other states. A successful `send` may transfer fewer bytes than requested. The caller is responsible for retaining and resending the remaining suffix. A successful non-zero-size `receive` that reports zero bytes indicates an orderly shutdown of the peer's sending direction.

Socket objects are not thread-safe. Synchronize all access externally or assign each socket to one event-loop thread.

All socket types implement `ISocket::close`. `close` releases the native socket immediately, is idempotent, and is also called automatically when the object is destroyed. The Network module does not expose TCP half-close.

## Addresses

`SocketAddress` stores either an IPv4 or IPv6 endpoint. Set `family` first, then fill the corresponding union member.

```c++
using namespace Luna;
using namespace Luna::Network;

SocketAddress address = {};
address.family = AddressFamily::ipv4;
address.ipv4.address = {127, 0, 0, 1};
address.ipv4.port = 8080;
```

For IPv6, use `SocketAddressIPv6`. `flow_info` and `scope_id` are usually `0`; set `scope_id` when addressing scoped IPv6 addresses such as link-local addresses.

```c++
SocketAddress address = {};
address.family = AddressFamily::ipv6;
address.ipv6.address = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
address.ipv6.port = 8080;
address.ipv6.flow_info = 0;
address.ipv6.scope_id = 0;
```

`IPV4_ADDRESS_ANY` and `IPV6_ADDRESS_ANY` can be used when binding to all local interfaces.

## Address lookup

Use `getaddrinfo` to resolve a host name, numeric address, service name, or port string. The `hints` argument can restrict the returned address family, socket type, and protocol.

```c++
Vector<AddressInfo> addresses;
AddressInfo hints = {};
hints.family = AddressFamily::ipv6;
hints.socktype = SocketType::stream;
hints.protocol = Protocol::tcp;
lupanic_if_failed(getaddrinfo("::1", "8080", &hints, addresses));
```

Set `AddressInfoFlag::passive` when the result will be used for `bind`, and pass `nullptr` as `node` to request wildcard local addresses.

```c++
Vector<AddressInfo> bind_addresses;
AddressInfo hints = {};
hints.flags = AddressInfoFlag::passive;
hints.family = AddressFamily::ipv4;
hints.socktype = SocketType::dgram;
hints.protocol = Protocol::udp;
lupanic_if_failed(getaddrinfo(nullptr, "0", &hints, bind_addresses));
```

## TCP connection states

`ITCPSocket::get_status` returns the current `TCPConnectionState` without blocking:

- `not_connected`: the socket has not started a connection and is not listening.
- `connecting`: a non-blocking connection attempt is still in progress.
- `connected`: the socket is connected to a peer.
- `listening`: the socket is accepting incoming connections.
- `peer_closed`: an orderly shutdown of the peer's sending direction has been observed. The socket may still be writable.
- `error`: the connection or its status query failed.
- `closed`: the native socket was closed locally.

`get_status` itself does not return an error. When the state is `error`, call `get_error` to retrieve the translated error code. `get_error` returns `ErrCode(0)` if no connection error has been cached, and `close` does not erase a cached error.

## TCP servers

Create a listener with `new_tcp_socket`, bind it, and call `listen`. A successful `listen` changes the socket state to `listening`.

```c++
lulet(listener, new_tcp_socket(AddressFamily::ipv4));
lupanic_if_failed(listener->bind(address));
lupanic_if_failed(listener->listen(I32_MAX));
luassert(listener->get_status() == TCPConnectionState::listening);
```

Call `accept` when the native listener reports read readiness. If no connection is queued, `accept` returns `BasicError::not_ready`. Every accepted socket is already non-blocking and starts in the `connected` state.

```c++
SocketAddress client_address = {};
auto accepted = listener->accept(client_address);
if(!accepted.valid() && accepted.errcode() == BasicError::not_ready())
{
    // Wait for listener readiness before trying again.
}
```

## TCP clients

`connect` returns `ok` when the connection either completes immediately or is successfully started asynchronously. It does not return `BasicError::in_progress`. Use `get_status` to distinguish the two successful outcomes.

```c++
lulet(socket, new_tcp_socket(AddressFamily::ipv4));
lupanic_if_failed(socket->connect(address));

switch(socket->get_status())
{
case TCPConnectionState::connected:
    // The socket is ready for transfers.
    break;
case TCPConnectionState::connecting:
    // Wait for write or error readiness, then call get_status again.
    break;
case TCPConnectionState::error:
    log_error("Network", "Connect failed: %s", explain(socket->get_error()));
    break;
default:
    break;
}
```

After a readiness notification for a connecting socket, call `get_status` again. It is the authoritative connection-completion query and changes the state to either `connected` or `error`.

## TCP data transfer

Call `send` only when data is pending. Preserve the unsent suffix after a partial operation, and retry only after the socket becomes writable if `BasicError::not_ready` is returned.

```c++
usize offset = 0;
while(offset < message.size())
{
    usize sent = 0;
    RV result = socket->send(message.data() + offset, message.size() - offset, &sent);
    if(failed(result))
    {
        if(result.errcode() == BasicError::not_ready())
        {
            // Stop this loop iteration and wait for write readiness.
            break;
        }
        // Handle the fatal error and stop sending on this socket.
        break;
    }
    offset += sent;
}
```

Call `receive` when the socket becomes readable. `BasicError::not_ready` means no bytes are currently available. A successful non-zero-size receive that returns zero bytes changes the socket state to `peer_closed`.

```c++
c8 buffer[4096];
usize received = 0;
RV result = socket->receive(buffer, sizeof(buffer), &received);
if(succeeded(result) && received == 0)
{
    luassert(socket->get_status() == TCPConnectionState::peer_closed);
}
```

Zero-size `receive` and `send` calls succeed with zero transferred bytes and do not probe the connection.

## UDP sockets

Create UDP sockets with `new_udp_socket`. `send_to` and `receive_from` each perform one non-blocking datagram operation. They return `BasicError::not_ready` if the datagram cannot be sent immediately or no datagram is available.

```c++
lulet(socket, new_udp_socket(AddressFamily::ipv6));
lupanic_if_failed(socket->bind(address));

const c8 message[] = {'u', 'd', 'p'};
usize sent = 0;
RV result = socket->send_to(message, sizeof(message), address, &sent);
```

Use `receive_from` to receive one datagram and optionally get the sender address.

```c++
c8 buffer[1024];
SocketAddress sender = {};
usize received = 0;
RV result = socket->receive_from(buffer, sizeof(buffer), &sender, &received);
```

## Native handles and lifetime

Use `ISocketPoller` for ordinary cross-platform readiness handling. See [[Socket Polling]] for the poller concepts, registration lifecycle, and event-loop examples.

`get_native_handle` remains available for integration with platform facilities that are not represented by the Network module. On Windows, reinterpret it as `SOCKET`; on POSIX platforms, reinterpret it as an `int` file descriptor. The handle is owned by the socket object and becomes invalid immediately after `close`.

```c++
socket->close();
socket->close(); // Safe: close is idempotent.
```

## Error handling

Network operations return LunaSDK `RV` or `R<T>` values. Platform socket errors are translated to `NetworkError` where possible, such as `not_connected`, `address_not_supported`, `address_in_use`, `connection_refused`, and `host_not_found`.

`BasicError::not_ready` is a normal non-blocking result and should normally cause the operation to wait for a readiness notification rather than close the socket. `BasicError::interrupted` may also be retried. Fatal TCP connection failures change the connection state to `error` and are available from `get_error`.

```c++
auto socket_result = new_tcp_socket(AddressFamily::unspecified);
if(!socket_result.valid())
{
    luassert(socket_result.errcode() == NetworkError::address_not_supported());
}
```

Some invalid operation combinations can be reported as generic `BasicError` values by the host platform, such as `bad_arguments` or `not_supported`. Code should check for failure before using output values unless it specifically needs one exact error code documented by the API.
