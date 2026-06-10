```c++
#include <Luna/Network/Network.hpp>
```

The `Network` module provides TCP and UDP socket APIs for IPv4 and IPv6 on Windows, macOS, and Linux. Add and initialize the module before using any socket API:

```c++
init();
lupanic_if_failed(add_modules({Luna::Network::module_network()}));
lupanic_if_failed(init_modules());
```

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

## TCP sockets

Create TCP sockets with `new_tcp_socket`. `ITCPSocket` derives from both `ISocket` and `IStream`, so connected TCP sockets can be used with stream-based helpers.

```c++
lulet(server_socket, new_tcp_socket(AddressFamily::ipv4));
lupanic_if_failed(server_socket->bind(address));
lupanic_if_failed(server_socket->listen(I32_MAX));

SocketAddress client_address = {};
lulet(client_socket, server_socket->accept(client_address));
```

Use `connect` for client sockets, then `read` and `write` through the `IStream` interface.

```c++
lulet(socket, new_tcp_socket(AddressFamily::ipv4));
lupanic_if_failed(socket->connect(address));

const c8 message[] = {'p', 'i', 'n', 'g'};
usize written = 0;
lupanic_if_failed(socket->write(message, sizeof(message), &written));
```

## UDP sockets

Create UDP sockets with `new_udp_socket`. `IUDPSocket` derives from `ISocket`, but not from `IStream`, because UDP is message-oriented rather than stream-oriented.

```c++
lulet(socket, new_udp_socket(AddressFamily::ipv6));
lupanic_if_failed(socket->bind(address));

const c8 message[] = {'u', 'd', 'p'};
usize sent = 0;
lupanic_if_failed(socket->send_to(message, sizeof(message), address, &sent));
```

Use `receive_from` to receive one datagram and optionally get the sender address.

```c++
c8 buffer[1024];
SocketAddress sender = {};
usize received = 0;
lupanic_if_failed(socket->receive_from(buffer, sizeof(buffer), &sender, &received));
```

## Error handling

Network operations return LunaSDK `RV` or `R<T>` values. Platform socket errors are translated to `NetworkError` where possible, such as `not_connected`, `address_not_supported`, `address_in_use`, `connection_refused`, and `host_not_found`.

```c++
auto socket_result = new_tcp_socket(AddressFamily::unspecified);
if(!socket_result.valid())
{
    luassert(socket_result.errcode() == NetworkError::address_not_supported());
}
```

Some invalid operation combinations can be reported as generic `BasicError` values by the host platform, such as `bad_arguments` or `not_supported`. Code should check for failure before using output values unless it specifically needs one exact error code documented by the API.
