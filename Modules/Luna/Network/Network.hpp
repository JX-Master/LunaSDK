/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.hpp
* @author JXMaster
* @date 2022/6/1
*/
#pragma once
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_NETWORK_API
#define LUNA_NETWORK_API
#endif

namespace Luna
{
	namespace Network
	{
		//! Converts the unsigned integer hostlong from host byte order to network byte order.
		LUNA_NETWORK_API u32 hton(u32 hostlong);
		//! Converts the unsigned short integer hostshort from host byte order to network byte order.
		LUNA_NETWORK_API u16 hton(u16 hostshort);
		//! Converts the unsigned integer netlong from network byte order to host byte order.
		LUNA_NETWORK_API u32 ntoh(u32 netlong);
		//! Converts the unsigned short integer netshort from network byte order to host byte order.
		LUNA_NETWORK_API u16 ntoh(u16 netshort);

		struct IPv4Address
		{
			u8 bytes[4];
		};

		struct IPv6Address
		{
			u8 bytes[16];
		};

		constexpr IPv4Address IPV4_ADDRESS_ANY = { 0, 0, 0, 0 };

		struct SocketAddressIPv4
		{
			IPv4Address address;
			//! The port number of the address in host byte order.
			u16 port;
		};

		enum class AddressFamily : u32
		{
			//! Maps to `AF_UNSPEC`. The address family is unspecified.
			unspecified = 0,
			//! Maps to `AF_INET`. The Internet Protocol version 4 (IPv4) address family.
			ipv4,
			//! Maps to `AF_INET6`. The Internet Protocol version 6 (IPv6) address family.
			ipv6,
			//! Maps to `AF_BTH` or `AF_BLUETOOTH`. The Bluetooth address family.
			bluetooth
		};

		struct SocketAddress
		{
			AddressFamily family;
			union
			{
				SocketAddressIPv4 ipv4;

			};
		};

		struct ISocket : virtual IStream
		{
			luiid("{36233BD3-54A0-4E67-B01E-C79E8115F548}");

			//! Gets the native handle of this socket.
			//! On Windows platforms, the returned handle can be reinterpreted to `SOCKET` type.
			//! On POSIX platforms, the returned handle can be reinterpreted to `int`, which is the file
			//! descriptor of the socket.
			virtual opaque_t get_native_handle() = 0;

			//! Binds one address to this socket.
			virtual RV bind(const SocketAddress& address) = 0;

			//! Starts listening for incoming connections.
			virtual RV listen(i32 len) = 0;

			//! Connects to the specified host.
			virtual RV connect(const SocketAddress& address) = 0;

			//! Permits incoming connection attempt on this socket.
			virtual R<Ref<ISocket>> accept(SocketAddress& address) = 0;
		};

		enum class SocketType : u32
		{
			//! The socket type is unspecified.
			unspecified = 0,
			//! Maps to `SOCK_STREAM`
			//! Provides sequenced, reliable, two-way, connection-based byte streams.  
			//! An out-of-band data transmission mechanism may be supported.
			stream,
			//! Maps to `SOCK_DGRAM`
			//! Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
			dgram,
			//! Maps to `SOCK_RAW`
			//! Provides raw network protocol access.
			raw,
			//! Maps to `SOCK_RDM`
			//! Provides a reliable datagram layer that does not guarantee ordering.
			rdm,
		};

		enum class Protocol : u32
		{
			//! The network protocol is unspecified. The system chooses the most suitable protocol based 
			//! on `AddressFamily` and `SocketType` parameters.
			unspecified = 0,
			//! The Internet Control Message Protocol (ICMP). 
			//! This is a possible value when the `AddressFamily` parameter is `unspecified`, `ipv4` or `ipv6`
			//! and the `SocketType` parameter is `raw` or `unspecified`.
			icmp,
			//! The Internet Group Management Protocol (IGMP).
			//! This is a possible value when the `AddressFamily` parameter is `unspecified`, `ipv4`, or `ipv6` 
			//! and the `SocketType` parameter is `raw` or `unspecified`.
			igmp,
			//! The Bluetooth Radio Frequency Communications (Bluetooth RFCOMM) protocol. 
			//! This is a possible value when the `AddressFamily` parameter is `bluetooth` 
			//! and the `SocketType` parameter is `stream`.
			rfcomm,
			//! Use Transmission Control Protocol (TCP). 
			//! This is a possible value when the `AddressFamily` parameter is `ipv4` or `ipv6` 
			//! and the `SocketType` parameter is `stream`.
			tcp,
			//! Use User Datagram Protocol (UDP). 
			//! This is a possible value when the `AddressFamily` parameter is `ipv4` or `ipv6` 
			//! and the `SocketType` parameter is `dgram`.
			udp,
			//! The Internet Control Message Protocol Version 6 (ICMPv6). 
			//! This is a possible value when the `AddressFamily` parameter is `unspecified`, `ipv4` or `ipv6`
			//! and the `SocketType` parameter is `raw` or `unspecified`.
			icmpv6,
		};

		//! Creates a new socket object.
		LUNA_NETWORK_API R<Ref<ISocket>> new_socket(AddressFamily af, SocketType type, Protocol protocol);

		enum class AddressInfoFlag : u8
		{
			none = 0,
			//! If set, this address is used for `bind`. If unset, this address is used for `connect`.
			passive = 0x01,
		};

		struct AddressInfo
		{
			AddressInfoFlag flags;
			AddressFamily family;
			SocketType socktype;
			Protocol protocol;
			Name canonname;
			SocketAddress addr;
		};

		LUNA_NETWORK_API R<Vector<AddressInfo>> getaddrinfo(const c8* node, const c8* service, const AddressInfo* hints);
	}

	namespace NetworkError
	{
		LUNA_NETWORK_API errcat_t errtype();

		//! The socket is not connected.
		LUNA_NETWORK_API ErrCode not_connected();

		//! The socket is already connected.
		LUNA_NETWORK_API ErrCode already_connected();

		//! The network subsystem has failed.
		LUNA_NETWORK_API ErrCode network_down();

		//! The specified address family is not supported by the socket/protocol.
		LUNA_NETWORK_API ErrCode address_not_supported();

		//! The speciifed address is already bound to one existing socket.
		LUNA_NETWORK_API ErrCode address_in_use();

		//! The requested address is not available.
		LUNA_NETWORK_API ErrCode address_not_available();

		//! For a connection-oriented socket, this error indicates that the connection has been broken 
		//! due to keep-alive activity that detected a failure while the operation was in progress.
		//! For a datagram socket, this error indicates that the time to live has expired.
		LUNA_NETWORK_API ErrCode network_reset();

		//! The attempt to connect was forcefully rejected.
		LUNA_NETWORK_API ErrCode connection_refused();

		//! The virtual circuit was terminated due to a time-out or other failure. 
		//! The application should close the socket as it is no longer usable.
		LUNA_NETWORK_API ErrCode connection_aborted();

		//! The virtual circuit was reset by the remote side executing a hard or abortive close. 
		//! The application should close the socket as it is no longer usable. 
		//! On a UDP-datagram socket, this error would indicate that a previous send operation 
		//! resulted in an ICMP "Port Unreachable" message.
		LUNA_NETWORK_API ErrCode connection_reset();

		//! The network cannot be reached from this host at this time.
		LUNA_NETWORK_API ErrCode network_unreachable();

		//! A socket operation was attempted to an unreachable host.
		LUNA_NETWORK_API ErrCode host_unreachable();

		//! The specified protocol is not supported within this address family.
		LUNA_NETWORK_API ErrCode protocol_not_supported();

		//! The specified host cannot be found.
		LUNA_NETWORK_API ErrCode host_not_found();

		//! The service is not supported on the target host with specified socket type.
		LUNA_NETWORK_API ErrCode service_not_found();
	}

	struct Module;
	LUNA_NETWORK_API Module* module_network();
}
