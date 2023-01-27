/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.hpp
* @author JXMaster
* @date 2022/6/1
*/
#pragma once
#include <Runtime/Stream.hpp>
#include <Runtime/Ref.hpp>

#ifndef LUNA_NETWORK_API
#define LUNA_NETWORK_API
#endif

namespace Luna
{
	namespace Net
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
			IPv4Address address = IPV4_ADDRESS_ANY;
			u16 port;
		};

		struct ISocket : virtual IStream
		{
			luiid("{36233BD3-54A0-4E67-B01E-C79E8115F548}");

			//! Binds one IPv4 address to this socket.
			//! The socket must be created with `SocketAddressFamily::ipv4`, or this call fails.
			virtual RV bind(const SocketAddressIPv4& address) = 0;

			//! Starts listening for incoming connections.
			virtual RV listen(i32 len) = 0;

			//! Connects to the specified host.
			virtual RV connect(const SocketAddressIPv4& address) = 0;

			//! Permits incoming connection attempt on this socket.
			virtual R<Ref<ISocket>> accept(SocketAddressIPv4& address) = 0;
		};

		enum class SocketAddressFamily : u32
		{
			//! Maps to AF_INET. The Internet Protocol version 4 (IPv4) address family.
			ipv4,
			//! Maps to AF_INET6. The Internet Protocol version 6 (IPv6) address family.
			ipv6,
		};

		enum class SocketType : u32
		{
			//! Provides sequenced, reliable, two-way, connection-based byte streams.
			//! If ipv4 or ipv6 is choosed, TCP will be used.
			stream,
			//! Supports datagrams (connectionless, unreliable messages of a fixed 
			//! maximum length).
			//! If ipv4 or ipv6 is choosed, UDP will be used.
			dgram,
		};

		LUNA_NETWORK_API R<Ref<ISocket>> socket(SocketAddressFamily af, SocketType type);
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
	}
}
