/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.cpp
* @author JXMaster
* @date 2022/6/2
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "../../../Network.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Interface.hpp>
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
    namespace Network
    {
        struct Socket : ISocket
        {
            lustruct("Network::Socket", "{35d804cf-4249-491f-a3e0-c95944ad5339}");
            luiimpl();

            AddressFamily m_af;
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
            case EALREADY: return BasicError::not_ready();
            case ECONNREFUSED: return NetworkError::connection_refused();
            case EINPROGRESS: return BasicError::in_progress();
            case EISCONN: return NetworkError::already_connected();
            case ENETUNREACH: return NetworkError::network_unreachable();
            case EPROTOTYPE: return NetworkError::protocol_not_supported();
            case ETIMEDOUT: return BasicError::timeout();
            default: return BasicError::bad_platform_call();
            }
        }

        RV Socket::read(void* buffer, usize size, usize* read_bytes)
        {
            isize read_sz = ::read(m_socket, buffer, size);
            if(read_sz == -1)
            {
                if (read_bytes) *read_bytes = 0;
                return translate_error(errno);
            }
            if (read_bytes) *read_bytes = (usize)read_sz;
            return ok;
        }

        RV Socket::write(const void* buffer, usize size, usize* write_bytes)
        {
            isize write_sz = ::write(m_socket, buffer, size);
            if(write_sz == -1)
            {
                if (write_bytes) *write_bytes = 0;
                return translate_error(errno);
            }
            if (write_bytes) *write_bytes = (usize)write_sz;
            return ok;
        }
        RV Socket::bind(const SocketAddress& address)
        {
            if(address.family == AddressFamily::ipv4)
            {
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = address.ipv4.port;
                memcpy(&addr.sin_addr.s_addr, &address.ipv4.address, 4);
                auto r = ::bind(m_socket, (sockaddr*)&addr, sizeof(addr));
                if (r == -1)
                {
                    return translate_error(errno);
                }
                return ok;
            }
            return NetworkError::address_not_supported();
        }
        RV Socket::listen(i32 len)
        {
            int r = ::listen(m_socket, len);
            if (r == -1)
            {
                return translate_error(errno);
            }
            return ok;
        }
        RV Socket::connect(const SocketAddress& address)
        {
            if(address.family == AddressFamily::ipv4)
            {
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = address.ipv4.port;
                memcpy(&addr.sin_addr.s_addr, &address.ipv4.address, 4);
                int r = ::connect(m_socket, (sockaddr*)&addr, sizeof(addr));
                if (r == -1)
                {
                    return translate_error(errno);
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
                socklen_t size = sizeof(addr);
                auto r = ::accept(m_socket, (sockaddr*)&addr, &size);
                if (r == -1)
                {
                    return translate_error(errno);
                }
                address.ipv4.port = addr.sin_port;
                memcpy(&address.ipv4.address, &addr.sin_addr.s_addr, 4);
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
            return ok;
        }
        void platform_close()
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
        LUNA_NETWORK_API R<Ref<ISocket>> new_socket(AddressFamily af, SocketType type)
        {
            int iaf = 0;
            int itype = 0;
            switch (af)
            {
            case AddressFamily::ipv4:
                iaf = AF_INET;
                break;
            case AddressFamily::ipv6:
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
            s->m_af = af;
            s->m_socket = r;
            return Ref<ISocket>(s);
        }
        LUNA_NETWORK_API RV getaddrinfo(const c8* node, const c8* service, const AddressInfo* hints, Vector<AddressInfo>& out_result)
        {
            struct addrinfo d_hints;
            memzero(&d_hints, sizeof(d_hints));
            if(hints)
            {
                switch(hints->family)
                {
                    case AddressFamily::unspecified: d_hints.ai_family = AF_UNSPEC; break;
                    case AddressFamily::ipv4: d_hints.ai_family = AF_INET; break;
                    case AddressFamily::ipv6: d_hints.ai_family = AF_INET6; break;
                    default: return NetworkError::address_not_supported();
                }
                switch(hints->socktype)
                {
                    case SocketType::unspecified: d_hints.ai_socktype = 0; break;
                    case SocketType::stream: d_hints.ai_socktype = SOCK_STREAM; break;
                    case SocketType::dgram: d_hints.ai_socktype = SOCK_DGRAM; break;
                    case SocketType::raw: d_hints.ai_socktype = SOCK_RAW; break;
                    case SocketType::rdm: d_hints.ai_socktype = SOCK_RDM; break;
                    default: return NetworkError::protocol_not_supported();
                }
                switch(hints->protocol)
                {
                    case Protocol::unspecified: d_hints.ai_protocol = 0; break;
                    case Protocol::icmp: d_hints.ai_protocol = IPPROTO_ICMP; break;
                    case Protocol::igmp: d_hints.ai_protocol = IPPROTO_IGMP; break;
                    case Protocol::tcp: d_hints.ai_protocol = IPPROTO_TCP; break;
                    case Protocol::udp: d_hints.ai_protocol = IPPROTO_UDP; break;
                    case Protocol::icmpv6: d_hints.ai_protocol = IPPROTO_ICMPV6; break;
                    default: return NetworkError::protocol_not_supported();
                }
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
            struct addrinfo* result = nullptr;
            auto err = ::getaddrinfo(node, service, &d_hints, &result);
            if(err)
            {
                return translate_error(err);
            }
            for(auto i = result; i; i = i->ai_next)
            {
                AddressInfo r;
                switch(i->ai_family)
                {
                    case AF_UNSPEC: r.family = AddressFamily::unspecified; break;
                    case AF_INET: r.family = AddressFamily::ipv4; break;
                    case AF_INET6: r.family = AddressFamily::ipv6; break;
                    default: continue;
                }
                switch(i->ai_socktype)
                {
                    case 0: r.socktype = SocketType::unspecified; break;
                    case SOCK_STREAM: r.socktype = SocketType::stream; break;
                    case SOCK_DGRAM: r.socktype = SocketType::dgram; break;
                    case SOCK_RAW: r.socktype = SocketType::raw; break;
                    case SOCK_RDM: r.socktype = SocketType::rdm; break;
                    default: continue;
                }
                switch(i->ai_protocol)
                {
                    case 0: r.protocol = Protocol::unspecified; break;
                    case IPPROTO_ICMP: r.protocol = Protocol::icmp; break;
                    case IPPROTO_IGMP: r.protocol = Protocol::igmp; break;
                    case IPPROTO_TCP: r.protocol = Protocol::tcp; break;
                    case IPPROTO_UDP: r.protocol = Protocol::udp; break;
                    case IPPROTO_ICMPV6: r.protocol = Protocol::icmpv6; break;
                }
                if(i->ai_canonname)
                {
                    r.canonname = Name(i->ai_canonname);
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
                    memcpy(&r.addr.ipv4.address, &addr->sin_addr.s_addr, 4);
                }
                else continue;
                out_result.push_back(r);
            }
            if(result)
            {
                freeaddrinfo(result);
            }
            return ok;
        }
    }
}
