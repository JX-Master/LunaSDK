/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.cpp
* @author JXMaster
* @date 2022/6/1
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "../../../Network.hpp"
#include <Runtime/Module.hpp>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

namespace Luna
{
	namespace Net
	{
		struct Socket : ISocket
		{
			lustruct("Net::Socket", "{42EF7CB8-B292-4837-88A4-D2E8AC156BA2}");
			luiimpl();
			SOCKET m_socket;

			Socket() :
				m_socket(INVALID_SOCKET) {}
			~Socket()
			{
				if (m_socket != INVALID_SOCKET)
				{
					closesocket(m_socket);
					m_socket = INVALID_SOCKET;
				}
			}
			RV read(void* buffer, usize size, usize* read_bytes);
			RV write(const void* buffer, usize size, usize* write_bytes);
			RV bind(const SocketAddressIPv4& address);
			RV listen(i32 len);
			RV connect(const SocketAddressIPv4& address);
			R<Ref<ISocket>> accept(SocketAddressIPv4& address);
		};
		inline ErrCode translate_error(int err)
		{
			switch (err)
			{
			case WSANOTINITIALISED: return BasicError::not_ready();
			case WSAENETDOWN: return NetworkError::network_down();
			case WSAENOBUFS: return BasicError::insufficient_system_buffer();
			case WSAENOTCONN: return NetworkError::not_connected();
			case WSAEINTR: return BasicError::interrupted();
			case WSAEINPROGRESS: return BasicError::in_progress();
			case WSAENETRESET: return NetworkError::network_reset();
			case WSAEMSGSIZE: return BasicError::data_too_long();
			case WSAEINVAL: return BasicError::bad_arguments();
			case WSAECONNABORTED: return NetworkError::connection_aborted();
			case WSAETIMEDOUT: return BasicError::timeout();
			case WSAECONNRESET: return NetworkError::connection_reset();
			case WSAEADDRINUSE: return NetworkError::address_in_use();
			case WSAEADDRNOTAVAIL: return NetworkError::address_not_available();
			case WSAEISCONN: return NetworkError::already_connected();
			case WSAEMFILE: return BasicError::out_of_resource();
			case WSAEOPNOTSUPP: return BasicError::not_supported();
			case WSAEALREADY: return NetworkError::already_connected();
			case WSAEAFNOSUPPORT: return NetworkError::address_not_supported();
			case WSAECONNREFUSED: return NetworkError::connection_refused();
			case WSAENETUNREACH: return NetworkError::network_unreachable();
			case WSAEHOSTUNREACH: return NetworkError::host_unreachable();
			case WSASYSNOTREADY: return BasicError::not_ready();
			case WSAVERNOTSUPPORTED: return BasicError::not_supported();
			case WSAEPROCLIM: return BasicError::out_of_resource();
			case WSAEPROTOTYPE: return NetworkError::protocol_not_supported();
			case WSAESOCKTNOSUPPORT: return BasicError::bad_arguments();
			case WSAEPROTONOSUPPORT: return NetworkError::protocol_not_supported();
			default: return BasicError::bad_platform_call();
			}
		}

		RV Socket::read(void* buffer, usize size, usize* read_bytes)
		{
			int r = ::recv(m_socket, (char*)buffer, (int)size, 0);
			if (r == SOCKET_ERROR)
			{
				if(read_bytes) *read_bytes = 0;
				int err = WSAGetLastError();
				return translate_error(err);
			}
			if (read_bytes) *read_bytes = r;
			return ok;
		}
		RV Socket::write(const void* buffer, usize size, usize* write_bytes)
		{
			int r = ::send(m_socket, (const char*)buffer, (int)size, 0);
			if (r == SOCKET_ERROR)
			{
				if(write_bytes) *write_bytes = 0;
				int err = WSAGetLastError();
				return translate_error(err);
			}
			if (write_bytes) *write_bytes = r;
			return ok;
		}
		RV Socket::bind(const SocketAddressIPv4& address)
		{
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = address.port;
			memcpy(&addr.sin_addr.S_un.S_un_b.s_b1, &address.address, 4);
			auto r = ::bind(m_socket, (sockaddr*)&addr, sizeof(addr));
			if (r == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			return ok;
		}
		RV Socket::listen(i32 len)
		{
			if (len == I32_MAX)
			{
				len = SOMAXCONN;
			}
			int r = ::listen(m_socket, len);
			if (r == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			return ok;
		}
		RV Socket::connect(const SocketAddressIPv4& address)
		{
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = address.port;
			memcpy(&addr.sin_addr.S_un.S_un_b.s_b1, &address.address, 4);
			int r = ::connect(m_socket, (sockaddr*)&addr, sizeof(addr));
			if (r == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			return ok;
		}
		R<Ref<ISocket>> Socket::accept(SocketAddressIPv4& address)
		{
			sockaddr_in addr;
			int size = sizeof(addr);
			SOCKET r = ::accept(m_socket, (sockaddr*)&addr, &size);
			if (r == INVALID_SOCKET)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			address.port = addr.sin_port;
			memcpy(&address.address, & addr.sin_addr.S_un.S_un_b.s_b1, 4);
			Ref<Socket> s = new_object<Socket>();
			s->m_socket = r;
			return s;
		}
		RV init()
		{
			register_boxed_type<Socket>();
			impl_interface_for_type<Socket, ISocket>();
			WORD sock_version = MAKEWORD(2, 2);
			WSADATA data;
			auto r = WSAStartup(sock_version, &data);
			if (r != 0)
			{
				return translate_error(r);
			}
			return ok;
		}

		void close()
		{
			WSACleanup();
		}

		LUNA_NETWORK_API u32 hton(u32 hostlong)
		{
			return ::htonl(hostlong);
		}
		LUNA_NETWORK_API u16 hton(u16 hostshort)
		{
			return ::htons(hostshort);
		}
		LUNA_NETWORK_API u32 ntoh(u32 netlong)
		{
			return ::ntohl(netlong);
		}
		LUNA_NETWORK_API u16 ntoh(u16 netshort)
		{
			return ::ntohs(netshort);
		}

		LUNA_NETWORK_API R<Ref<ISocket>> socket(SocketAddressFamily af, SocketType type)
		{
			int iaf = 0;
			int itype = 0;
			int iprotocol = 0;
			switch (af)
			{
			case SocketAddressFamily::ipv4:
				iaf = AF_INET;
				break;
			case SocketAddressFamily::ipv6:
				iaf = AF_INET6;
				break;
			default: lupanic(); break;
			}
			switch (type)
			{
			case SocketType::stream:
				itype = SOCK_STREAM;
				break;
			case SocketType::dgram:
				itype = SOCK_DGRAM;
				break;
			default: lupanic(); break;
			}
			if (iaf == AF_INET || iaf == AF_INET6)
			{
				if (itype == SOCK_STREAM) iprotocol = IPPROTO_TCP;
				else if (itype == SOCK_DGRAM) iprotocol = IPPROTO_UDP;
				else lupanic();
			}
			else lupanic();
			SOCKET r = ::socket(iaf, itype, iprotocol);
			if (r == INVALID_SOCKET)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			Ref<Socket> s = new_object<Socket>();
			s->m_socket = r;
			return s;
		}

		StaticRegisterModule network_module("Network", "", init, close);
	}
}