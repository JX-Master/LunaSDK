/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.cpp
* @author JXMaster
* @date 2022/6/1
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "../../../Network.hpp"
#include <Luna/Runtime/Module.hpp>
#include <WinSock2.h>
#include <Luna/Runtime/Unicode.hpp>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace Luna
{
	namespace Network
	{
		struct Socket : ISocket
		{
			lustruct("Network::Socket", "{42EF7CB8-B292-4837-88A4-D2E8AC156BA2}");
			luiimpl();

			AddressFamily m_af;
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
			virtual opaque_t get_native_handle() override { return (opaque_t)m_socket; }
			virtual RV read(void* buffer, usize size, usize* read_bytes) override;
			virtual RV write(const void* buffer, usize size, usize* write_bytes) override;
			virtual RV bind(const SocketAddress& address) override;
			virtual RV listen(i32 len) override;
			virtual RV connect(const SocketAddress& address) override;
			virtual R<Ref<ISocket>> accept(SocketAddress& address) override;
		};
		inline ErrCode translate_error(int err)
		{
			switch (err)
			{
			case WSANOTINITIALISED: return BasicError::bad_calling_time();
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
			case WSASYSNOTREADY: return BasicError::bad_calling_time();
			case WSAVERNOTSUPPORTED: return BasicError::not_supported();
			case WSAEPROCLIM: return BasicError::out_of_resource();
			case WSAEPROTOTYPE: return NetworkError::protocol_not_supported();
			case WSAESOCKTNOSUPPORT: return BasicError::bad_arguments();
			case WSAEPROTONOSUPPORT: return NetworkError::protocol_not_supported();
			case WSATRY_AGAIN: return BasicError::not_ready();
			case WSANO_RECOVERY: return BasicError::bad_arguments();
			case WSA_NOT_ENOUGH_MEMORY: return BasicError::out_of_memory();
			case WSAHOST_NOT_FOUND: return NetworkError::host_not_found();
			case WSATYPE_NOT_FOUND: return NetworkError::service_not_found();

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
		RV Socket::bind(const SocketAddress& address)
		{
			if(address.family == AddressFamily::ipv4)
			{
				sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_port = hton(address.ipv4.port);
				memcpy(&addr.sin_addr.S_un.S_un_b.s_b1, &address.ipv4.address, 4);
				auto r = ::bind(m_socket, (sockaddr*)&addr, sizeof(addr));
				if (r == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					return translate_error(err);
				}
				return ok;
			}
			return NetworkError::address_not_supported();
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
		RV Socket::connect(const SocketAddress& address)
		{
			if(address.family == AddressFamily::ipv4)
			{
				sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_port = hton(address.ipv4.port);
				memcpy(&addr.sin_addr.S_un.S_un_b.s_b1, &address.ipv4.address, 4);
				int r = ::connect(m_socket, (sockaddr*)&addr, sizeof(addr));
				if (r == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					return translate_error(err);
				}
				return ok;
			}
			return NetworkError::address_not_supported();
		}
		R<Ref<ISocket>> Socket::accept(SocketAddress& address)
		{
			memzero(&address, sizeof(SocketAddress));
			if(m_af == AddressFamily::ipv4)
			{
				sockaddr_in addr;
				int size = sizeof(addr);
				SOCKET r = ::accept(m_socket, (sockaddr*)&addr, &size);
				if (r == INVALID_SOCKET)
				{
					int err = WSAGetLastError();
					return translate_error(err);
				}
				address.family = AddressFamily::ipv4;
				address.ipv4.port = ntoh(addr.sin_port);
				memcpy(&address.ipv4.address, &addr.sin_addr.S_un.S_un_b.s_b1, 4);
				Ref<Socket> s = new_object<Socket>();
				s->m_socket = r;
				return Ref<ISocket>(s);
			}
			return NetworkError::address_not_supported();
		}
		RV platform_init()
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

		void platform_close()
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

		inline int encode_af(AddressFamily af)
		{
			switch (af)
			{
			case AddressFamily::unspecified: return AF_UNSPEC;
			case AddressFamily::ipv4: return AF_INET;
			case AddressFamily::ipv6: return AF_INET6;
			case AddressFamily::bluetooth: return AF_BTH;
			default: lupanic(); return AF_UNSPEC;
			}
		}
		inline AddressFamily decode_af(int af)
		{
			switch(af)
			{
				case AF_UNSPEC: return AddressFamily::unspecified;
				case AF_INET: return AddressFamily::ipv4;
				case AF_INET6: return AddressFamily::ipv6;
				case AF_BTH: return AddressFamily::bluetooth;
				default: return AddressFamily::unspecified;
			}
		}

		inline int encode_type(SocketType type)
		{
			switch (type)
			{
			case SocketType::unspecified: return 0;
			case SocketType::stream: return SOCK_STREAM;
			case SocketType::dgram: return SOCK_DGRAM;
			case SocketType::raw: return SOCK_RAW;
			case SocketType::rdm: return SOCK_RDM;
			default: lupanic(); return 0;
			}
		}

		inline SocketType decode_type(int type)
		{
			switch(type)
			{
				case 0: return SocketType::unspecified;
				case SOCK_STREAM: return SocketType::stream;
				case SOCK_DGRAM: return SocketType::dgram;
				case SOCK_RAW: return SocketType::raw;
				case SOCK_RDM: return SocketType::rdm;
				default: return SocketType::unspecified;
			}
		}

		inline int encode_protocol(Protocol protocol)
		{
			switch(protocol)
			{
				case Protocol::unspecified: return 0;
				case Protocol::icmp: return IPPROTO_ICMP;
				case Protocol::igmp: return IPPROTO_IGMP;
				case Protocol::rfcomm: return 3; // BTHPROTO_RFCOMM
				case Protocol::tcp: return IPPROTO_TCP;
				case Protocol::udp: return IPPROTO_UDP;
				case Protocol::icmpv6: return IPPROTO_ICMPV6;
				default: lupanic(); return 0;
			}
		}

		inline Protocol decode_protocol(int protocol)
		{
			switch (protocol)
			{
			case 0: return Protocol::unspecified;
			case IPPROTO_ICMP: return Protocol::icmp;
			case IPPROTO_IGMP: return Protocol::igmp;
			case 3: return Protocol::rfcomm;
			case IPPROTO_TCP: return Protocol::tcp;
			case IPPROTO_UDP: return Protocol::udp;
			case IPPROTO_ICMPV6: return Protocol::icmpv6;
			default: return Protocol::unspecified;
			}
		}

		LUNA_NETWORK_API R<Ref<ISocket>> new_socket(AddressFamily af, SocketType type, Protocol protocol)
		{
			int iaf = encode_af(af);
			int itype = encode_type(type);
			int iprotocol = encode_protocol(protocol);
			SOCKET r = ::socket(iaf, itype, iprotocol);
			if (r == INVALID_SOCKET)
			{
				int err = WSAGetLastError();
				return translate_error(err);
			}
			Ref<Socket> s = new_object<Socket>();
			s->m_af = af;
			s->m_socket = r;
			return Ref<ISocket>(s);
		}

		LUNA_NETWORK_API R<Vector<AddressInfo>> getaddrinfo(const c8* node, const c8* service, const AddressInfo* hints)
		{
			ADDRINFOW d_hints;
			memzero(&d_hints, sizeof(ADDRINFOW));
			if(hints)
			{
				d_hints.ai_family = encode_af(hints->family);
				d_hints.ai_socktype = encode_type(hints->socktype);
				d_hints.ai_protocol = encode_protocol(hints->protocol);
				if(test_flags(hints->flags, AddressInfoFlag::passive))
				{
					d_hints.ai_flags |= AI_PASSIVE;
				}
			}
			else
			{
				d_hints.ai_family = AF_UNSPEC;
			}
			d_hints.ai_flags |= AI_CANONNAME;
			usize node_len = utf8_to_utf16_len(node);
			usize service_len = utf8_to_utf16_len(service);
			wchar_t* wnode = (wchar_t*)alloca(sizeof(wchar_t) * (node_len + 1));
			wchar_t* wservice = (wchar_t*)alloca(sizeof(wchar_t) * (service_len + 1));
			utf8_to_utf16((c16*)wnode, node_len + 1, node);
			utf8_to_utf16((c16*)wservice, service_len + 1, service);
			ADDRINFOW* result = nullptr;
			auto err = GetAddrInfoW(wnode, wservice, &d_hints, &result);
			if(err)
			{
				return translate_error(err);
			}
			Vector<AddressInfo> ret;
			Vector<c8> buffer;
			for(auto i = result; i; i = i->ai_next)
			{
				AddressInfo r;
				r.family = decode_af(i->ai_family);
				r.socktype = decode_type(i->ai_socktype);
				r.protocol = decode_protocol(i->ai_protocol);
				if(i->ai_canonname)
				{
					usize len = utf16_to_utf8_len((c16*)i->ai_canonname);
					buffer.resize(len + 1, 0);
					utf16_to_utf8(buffer.data(), buffer.size(), (c16*)i->ai_canonname);
					r.canonname = buffer.data();
				}
				r.flags = AddressInfoFlag::none;
				if(i->ai_flags & AI_PASSIVE)
				{
					set_flags(r.flags, AddressInfoFlag::passive);
				}
				if(i->ai_addr->sa_family == AF_INET)
				{
					sockaddr_in* addr = (sockaddr_in*)i->ai_addr;
					r.addr.family = AddressFamily::ipv4;
					r.addr.ipv4.port = ntoh(addr->sin_port);
					memcpy(&r.addr.ipv4.address, &addr->sin_addr.S_un.S_un_b.s_b1, 4);
				}
				else continue;
				ret.push_back(r);
			}
			if(result)
			{
				FreeAddrInfoW(result);
			}
			return ret;
		}
	}
}