/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.cpp
* @author JXMaster
* @date 2022/6/2
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "../../../Network.hpp"
#include <Runtime/Module.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

namespace Luna
{
    namespace Net
    {
        struct Socket
        {
            lutype("Net::Socket", "{35d804cf-4249-491f-a3e0-c95944ad5339}");

            int m_socket;

            Socket() :
                m_socket(-1) {}
            ~Socket()
            {
                if(m_socket != -1)
                {
                    ::close(m_socket);
                    m_socket = -1;
                }
            }
            static RV read(object_t self, void* buffer, usize size, usize* read_bytes);
            static RV write(object_t self, const void* buffer, usize size, usize* write_bytes);
            static RV bind_ipv4(object_t self, const SocketAddressIPv4& address);
            static RV listen(object_t self, i32 len);
            static RV connect_ipv4(object_t self, const SocketAddressIPv4& address);
            static R<Ref<ISocket>> accept_ipv4(object_t self, SocketAddressIPv4& address);
            static void reg()
            {
                register_boxed_type<Socket>();
                impl_interface_for_type<Socket, IStream>({ read, write });
                impl_interface_for_type<Socket, ISocket>({ read, write, bind_ipv4, listen, connect_ipv4, accept_ipv4 });
            }
        };

        inline ErrCode translate_error(int err)
        {
            switch (err)
            {
            case EACCES: return BasicError::access_denied();
            case EAFNOSUPPORT: return NetworkError::address_not_supported();
            case EINVAL: return BasicError::bad_arguments();
            case EMFILE: return BasicError::out_of_resource();
            case ENFILE: return BasicError::out_of_resource();
            case ENOBUFS: return BasicError::insufficient_system_buffer();
            case EPROTONOSUPPORT: return NetworkError::protocol_not_supported();
            case EINTR: return BasicError::interrupted();
            case EDESTADDRREQ: return NetworkError::not_connected();
            case EADDRINUSE: return NetworkError::address_in_use();
            case EADDRNOTAVAIL: return NetworkError::address_not_available();
            case EOPNOTSUPP: return BasicError::not_supported();
            case EALREADY: return BasicError::busy();
            case ECONNREFUSED: return NetworkError::connection_refused();
            case EINPROGRESS: return BasicError::in_progress();
            case EISCONN: return NetworkError::already_connected();
            case ENETUNREACH: return NetworkError::network_unreachable();
            case EPROTOTYPE: return NetworkError::protocol_not_supported();
            case ETIMEDOUT: return BasicError::timeout();
            default: return BasicError::bad_system_call();
            }
        }

        RV Socket::read(object_t self, void* buffer, usize size, usize* read_bytes)
		{
			Socket* o = (Socket*)self;
            isize read_sz = ::read(o->m_socket, buffer, size);
            if(read_sz == -1)
            {
                if (read_bytes) *read_bytes = 0;
                return translate_error(errno);
            }
            if (read_bytes) *read_bytes = (usize)read_sz;
            return ok;
		}

        RV Socket::write(object_t self, const void* buffer, usize size, usize* write_bytes)
        {
            Socket* o = (Socket*)self;
            isize write_sz = ::write(o->m_socket, buffer, size);
            if(write_sz == -1)
            {
                if (write_bytes) *write_bytes = 0;
                return translate_error(errno);
            }
            if (write_bytes) *write_bytes = (usize)write_sz;
            return ok;
        }
        RV Socket::bind_ipv4(object_t self, const SocketAddressIPv4& address)
		{
			Socket* o = (Socket*)self;
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = address.port;
			memcpy(&addr.sin_addr.s_addr, &address.address, 4);
			auto r = ::bind(o->m_socket, (sockaddr*)&addr, sizeof(addr));
			if (r == -1)
			{
				return translate_error(errno);
			}
			return ok;
		}
        RV Socket::listen(object_t self, i32 len)
		{
			Socket* o = (Socket*)self;
			int r = ::listen(o->m_socket, len);
			if (r == -1)
			{
				return translate_error(errno);
			}
			return ok;
		}
        RV Socket::connect_ipv4(object_t self, const SocketAddressIPv4& address)
		{
			Socket* o = (Socket*)self;
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = address.port;
			memcpy(&addr.sin_addr.s_addr, &address.address, 4);
			int r = ::connect(o->m_socket, (sockaddr*)&addr, sizeof(addr));
			if (r == -1)
			{
				return translate_error(errno);
			}
			return ok;
		}
        R<Ref<ISocket>> Socket::accept_ipv4(object_t self, SocketAddressIPv4& address)
		{
			Socket* o = (Socket*)self;
			sockaddr_in addr;
			socklen_t size = sizeof(addr);
			auto r = ::accept(o->m_socket, (sockaddr*)&addr, &size);
			if (r == -1)
			{
				return translate_error(errno);
			}
			address.port = addr.sin_port;
			memcpy(&address.address, & addr.sin_addr.s_addr, 4);
			Ref<Socket> s = new_object<Socket>();
			s->m_socket = r;
			return Ref<ISocket>(s);
		}
        RV init()
		{
			register_interface<ISocket>();
			Socket::reg();
			return ok;
		}

		void close()
		{
			
		}
        LUNA_NETWORK_API u32 hton(u32 hostlong)
		{
			return htonl(hostlong);
		}
		LUNA_NETWORK_API u16 hton(u16 hostshort)
		{
			return htons(hostshort);
		}
		LUNA_NETWORK_API u32 ntoh(u32 netlong)
		{
			return ntohl(netlong);
		}
		LUNA_NETWORK_API u16 ntoh(u16 netshort)
		{
			return ntohs(netshort);
		}
        LUNA_NETWORK_API R<Ref<ISocket>> socket(SocketAddressFamily af, SocketType type)
        {
            int iaf = 0;
			int itype = 0;
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
            int r = ::socket(iaf, itype, 0);
            if(r == -1)
            {
                return translate_error(errno);
            }
            Ref<Socket> s = new_object<Socket>();
			s->m_socket = r;
			return Ref<ISocket>(s);
        }

        StaticRegisterModule network_module("Network", "", init, close);
    }
}
