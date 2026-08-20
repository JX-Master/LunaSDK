/*!
* This file is a portion of LunaSDK.
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
#include <string.h>
#include "Socket.hpp"
#include "Network.meta.generated.hpp"

namespace Luna
{
    namespace Network
    {
        inline ResultCode translate_error(int err)
        {
            switch (err)
            {
            case EACCES: return E_ACCESS_DENIED;
            case EAFNOSUPPORT: return Network::E_ADDRESS_NOT_SUPPORTED;
            case EINVAL: return E_BAD_ARGUMENTS;
            case EMFILE: return E_OUT_OF_RESOURCE;
            case ENFILE: return E_OUT_OF_RESOURCE;
            case ENOBUFS: return E_INSUFFICIENT_SYSTEM_BUFFER;
            case ENETDOWN: return Network::E_NETWORK_DOWN;
            case EPROTONOSUPPORT: return Network::E_PROTOCOL_NOT_SUPPORTED;
            case EINTR: return E_INTERRUPTED;
            case ENOTCONN: return Network::E_NOT_CONNECTED;
            case EDESTADDRREQ: return Network::E_NOT_CONNECTED;
            case EADDRINUSE: return Network::E_ADDRESS_IN_USE;
            case EADDRNOTAVAIL: return Network::E_ADDRESS_NOT_AVAILABLE;
            case EOPNOTSUPP: return E_NOT_SUPPORTED;
            case EALREADY: return E_NOT_READY;
            case ECONNREFUSED: return Network::E_CONNECTION_REFUSED;
            case ECONNABORTED: return Network::E_CONNECTION_ABORTED;
            case ECONNRESET: return Network::E_CONNECTION_RESET;
            case EINPROGRESS: return E_IN_PROGRESS;
            case EISCONN: return Network::E_ALREADY_CONNECTED;
            case ENETRESET: return Network::E_NETWORK_RESET;
            case ENETUNREACH: return Network::E_NETWORK_UNREACHABLE;
            case EHOSTUNREACH: return Network::E_HOST_UNREACHABLE;
            case EPROTOTYPE: return Network::E_PROTOCOL_NOT_SUPPORTED;
            case ETIMEDOUT: return E_TIMEOUT;
            case EPIPE: return Network::E_CONNECTION_RESET;
            default: return E_BAD_PLATFORM_CALL;
            }
        }

        inline ResultCode translate_addrinfo_error(int err)
        {
            switch (err)
            {
            case EAI_AGAIN: return E_NOT_READY;
            case EAI_BADFLAGS: return E_BAD_ARGUMENTS;
            case EAI_FAIL: return E_BAD_PLATFORM_CALL;
            case EAI_FAMILY: return Network::E_ADDRESS_NOT_SUPPORTED;
            case EAI_MEMORY: return E_OUT_OF_MEMORY;
            case EAI_NONAME: return Network::E_HOST_NOT_FOUND;
            case EAI_SERVICE: return Network::E_SERVICE_NOT_FOUND;
            case EAI_SOCKTYPE: return Network::E_PROTOCOL_NOT_SUPPORTED;
#ifdef EAI_SYSTEM
            case EAI_SYSTEM: return translate_error(errno);
#endif
            default: return E_BAD_PLATFORM_CALL;
            }
        }

        inline bool encode_af(AddressFamily af, int& out, bool allow_unspecified)
        {
            switch (af)
            {
            case AddressFamily::unspecified:
                if(!allow_unspecified) return false;
                out = AF_UNSPEC;
                return true;
            case AddressFamily::ipv4:
                out = AF_INET;
                return true;
            case AddressFamily::ipv6:
                out = AF_INET6;
                return true;
            default:
                return false;
            }
        }

        inline AddressFamily decode_af(int af)
        {
            switch (af)
            {
            case AF_INET: return AddressFamily::ipv4;
            case AF_INET6: return AddressFamily::ipv6;
            default: return AddressFamily::unspecified;
            }
        }

        inline bool encode_type(SocketType type, int& out, bool allow_unspecified)
        {
            switch (type)
            {
            case SocketType::unspecified:
                if(!allow_unspecified) return false;
                out = 0;
                return true;
            case SocketType::stream:
                out = SOCK_STREAM;
                return true;
            case SocketType::dgram:
                out = SOCK_DGRAM;
                return true;
            default:
                return false;
            }
        }

        inline SocketType decode_type(int type)
        {
            switch(type)
            {
            case SOCK_STREAM: return SocketType::stream;
            case SOCK_DGRAM: return SocketType::dgram;
            default: return SocketType::unspecified;
            }
        }

        inline bool encode_protocol(Protocol protocol, int& out)
        {
            switch(protocol)
            {
            case Protocol::unspecified:
                out = 0;
                return true;
            case Protocol::tcp:
                out = IPPROTO_TCP;
                return true;
            case Protocol::udp:
                out = IPPROTO_UDP;
                return true;
            default:
                return false;
            }
        }

        inline Protocol decode_protocol(int protocol)
        {
            switch (protocol)
            {
            case IPPROTO_TCP: return Protocol::tcp;
            case IPPROTO_UDP: return Protocol::udp;
            default: return Protocol::unspecified;
            }
        }

        inline R<socklen_t> encode_socket_address(sockaddr_storage& out, const SocketAddress& address)
        {
            memzero(&out, sizeof(out));
            switch(address.family)
            {
            case AddressFamily::ipv4:
            {
                sockaddr_in* addr = (sockaddr_in*)&out;
                addr->sin_family = AF_INET;
                addr->sin_port = hton(address.ipv4.port);
                memcpy(&addr->sin_addr.s_addr, &address.ipv4.address.bytes, 4);
                return (socklen_t)sizeof(sockaddr_in);
            }
            case AddressFamily::ipv6:
            {
                sockaddr_in6* addr = (sockaddr_in6*)&out;
                addr->sin6_family = AF_INET6;
                addr->sin6_port = hton(address.ipv6.port);
                addr->sin6_flowinfo = address.ipv6.flow_info;
                addr->sin6_scope_id = address.ipv6.scope_id;
                memcpy(&addr->sin6_addr, &address.ipv6.address.bytes, 16);
                return (socklen_t)sizeof(sockaddr_in6);
            }
            default:
                return Network::E_ADDRESS_NOT_SUPPORTED;
            }
        }

        inline bool decode_socket_address(SocketAddress& out, const sockaddr* address)
        {
            memzero(&out, sizeof(out));
            switch(address->sa_family)
            {
            case AF_INET:
            {
                const sockaddr_in* addr = (const sockaddr_in*)address;
                out.family = AddressFamily::ipv4;
                out.ipv4.port = ntoh(addr->sin_port);
                memcpy(&out.ipv4.address.bytes, &addr->sin_addr.s_addr, 4);
                return true;
            }
            case AF_INET6:
            {
                const sockaddr_in6* addr = (const sockaddr_in6*)address;
                out.family = AddressFamily::ipv6;
                out.ipv6.port = ntoh(addr->sin6_port);
                out.ipv6.flow_info = addr->sin6_flowinfo;
                out.ipv6.scope_id = addr->sin6_scope_id;
                memcpy(&out.ipv6.address.bytes, &addr->sin6_addr, 16);
                return true;
            }
            default:
                return false;
            }
        }

        inline int send_flags()
        {
#ifdef MSG_NOSIGNAL
            return MSG_NOSIGNAL;
#else
            return 0;
#endif
        }

        RV SocketBase::get_local_address(SocketAddress& address)
        {
            sockaddr_storage addr;
            socklen_t size = sizeof(addr);
            int r = ::getsockname(m_socket, (sockaddr*)&addr, &size);
            if(r == -1)
            {
                return translate_error(errno);
            }
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(Network::E_ADDRESS_NOT_SUPPORTED);
        }

        RV TCPSocket::get_remote_address(SocketAddress& address)
        {
            sockaddr_storage addr;
            socklen_t size = sizeof(addr);
            int r = ::getpeername(m_socket, (sockaddr*)&addr, &size);
            if(r == -1)
            {
                return translate_error(errno);
            }
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(Network::E_ADDRESS_NOT_SUPPORTED);
        }

        RV TCPSocket::read(void* buffer, usize size, usize* read_bytes)
        {
            usize read_size = size > (usize)ISIZE_MAX ? (usize)ISIZE_MAX : size;
            isize read_sz = ::recv(m_socket, buffer, read_size, 0);
            if(read_sz == -1)
            {
                if (read_bytes) *read_bytes = 0;
                return translate_error(errno);
            }
            if (read_bytes) *read_bytes = (usize)read_sz;
            return ok;
        }

        RV TCPSocket::write(const void* buffer, usize size, usize* write_bytes)
        {
            usize total_written = 0;
            while(total_written < size)
            {
                usize chunk = size - total_written;
                if(chunk > (usize)ISIZE_MAX) chunk = (usize)ISIZE_MAX;
                isize write_sz = ::send(m_socket, (const u8*)buffer + total_written, chunk, send_flags());
                if(write_sz == -1)
                {
                    if (write_bytes) *write_bytes = total_written;
                    return translate_error(errno);
                }
                if(write_sz == 0)
                {
                    if (write_bytes) *write_bytes = total_written;
                    return E_NO_DATA;
                }
                total_written += (usize)write_sz;
                if(m_type != SocketType::stream)
                {
                    break;
                }
            }
            if (write_bytes) *write_bytes = total_written;
            return ok;
        }
        RV SocketBase::bind(const SocketAddress& address)
        {
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid()) return addr_size.errcode();
            auto r = ::bind(m_socket, (sockaddr*)&addr, addr_size.get());
            if (r == -1)
            {
                return translate_error(errno);
            }
            return ok;
        }
        RV TCPSocket::listen(i32 len)
        {
            if (len == I32_MAX)
            {
                len = SOMAXCONN;
            }
            int r = ::listen(m_socket, len);
            if (r == -1)
            {
                return translate_error(errno);
            }
            return ok;
        }
        RV TCPSocket::connect(const SocketAddress& address)
        {
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid()) return addr_size.errcode();
            int r = ::connect(m_socket, (sockaddr*)&addr, addr_size.get());
            if (r == -1)
            {
                return translate_error(errno);
            }
            return ok;
        }
        R<Ref<ITCPSocket>> TCPSocket::accept(SocketAddress& address)
        {
            sockaddr_storage addr;
            socklen_t size = sizeof(addr);
            auto r = ::accept(m_socket, (sockaddr*)&addr, &size);
            if (r == -1)
            {
                return translate_error(errno);
            }
            if(!decode_socket_address(address, (sockaddr*)&addr))
            {
                ::close(r);
                return Network::E_ADDRESS_NOT_SUPPORTED;
            }
            Ref<TCPSocket> s = new_object<TCPSocket>();
            s->m_af = address.family;
            s->m_type = m_type;
            s->m_socket = r;
            return Ref<ITCPSocket>(s);
        }
        RV UDPSocket::send_to(const void* buffer, usize size, const SocketAddress& address, usize* sent_bytes)
        {
            if(size > (usize)ISIZE_MAX)
            {
                if(sent_bytes) *sent_bytes = 0;
                return E_DATA_TOO_BIG;
            }
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                if(sent_bytes) *sent_bytes = 0;
                return addr_size.errcode();
            }
            isize sent = ::sendto(m_socket, buffer, size, send_flags(), (sockaddr*)&addr, addr_size.get());
            if(sent == -1)
            {
                if(sent_bytes) *sent_bytes = 0;
                return translate_error(errno);
            }
            if(sent_bytes) *sent_bytes = (usize)sent;
            return ok;
        }
        RV UDPSocket::receive_from(void* buffer, usize size, SocketAddress* address, usize* received_bytes)
        {
            usize read_size = size > (usize)ISIZE_MAX ? (usize)ISIZE_MAX : size;
            sockaddr_storage addr;
            socklen_t addr_size = sizeof(addr);
            isize received = ::recvfrom(m_socket, buffer, read_size, 0, (sockaddr*)&addr, &addr_size);
            if(received == -1)
            {
                if(received_bytes) *received_bytes = 0;
                return translate_error(errno);
            }
            if(address && !decode_socket_address(*address, (sockaddr*)&addr))
            {
                if(received_bytes) *received_bytes = 0;
                return Network::E_ADDRESS_NOT_SUPPORTED;
            }
            if(received_bytes) *received_bytes = (usize)received;
            return ok;
        }
        RV platform_init()
        {
            Meta::register_Network_types();
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
        inline R<int> new_native_socket(AddressFamily af, SocketType type, Protocol protocol)
        {
            int iaf;
            if(!encode_af(af, iaf, false)) return Network::E_ADDRESS_NOT_SUPPORTED;
            int itype;
            if(!encode_type(type, itype, false)) return E_BAD_ARGUMENTS;
            int iprotocol;
            if(!encode_protocol(protocol, iprotocol)) return Network::E_PROTOCOL_NOT_SUPPORTED;
            int r = ::socket(iaf, itype, iprotocol);
            if(r == -1)
            {
                return translate_error(errno);
            }
#ifdef SO_NOSIGPIPE
            int no_sigpipe = 1;
            ::setsockopt(r, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe));
#endif
            return r;
        }

        LUNA_NETWORK_API R<Ref<ITCPSocket>> new_tcp_socket(AddressFamily af)
        {
            auto native_socket = new_native_socket(af, SocketType::stream, Protocol::tcp);
            if(!native_socket.valid()) return native_socket.errcode();
            Ref<TCPSocket> s = new_object<TCPSocket>();
            s->m_af = af;
            s->m_type = SocketType::stream;
            s->m_socket = native_socket.get();
            return Ref<ITCPSocket>(s);
        }

        LUNA_NETWORK_API R<Ref<IUDPSocket>> new_udp_socket(AddressFamily af)
        {
            auto native_socket = new_native_socket(af, SocketType::dgram, Protocol::udp);
            if(!native_socket.valid()) return native_socket.errcode();
            Ref<UDPSocket> s = new_object<UDPSocket>();
            s->m_af = af;
            s->m_type = SocketType::dgram;
            s->m_socket = native_socket.get();
            return Ref<IUDPSocket>(s);
        }
        LUNA_NETWORK_API RV getaddrinfo(const c8* node, const c8* service, const AddressInfo* hints, Vector<AddressInfo>& out_result)
        {
            struct addrinfo d_hints;
            memzero(&d_hints, sizeof(d_hints));
            if(hints)
            {
                if(!encode_af(hints->family, d_hints.ai_family, true)) return Network::E_ADDRESS_NOT_SUPPORTED;
                if(!encode_type(hints->socktype, d_hints.ai_socktype, true)) return E_BAD_ARGUMENTS;
                if(!encode_protocol(hints->protocol, d_hints.ai_protocol)) return Network::E_PROTOCOL_NOT_SUPPORTED;
                if(test_flags(hints->flags, AddressInfoFlag::passive))
                {
                    d_hints.ai_flags |= AI_PASSIVE;
                }
            }
            else
            {
                d_hints.ai_family = AF_UNSPEC;
            }
            if(node)
            {
                d_hints.ai_flags |= AI_CANONNAME;
            }
            struct addrinfo* result = nullptr;
            auto err = ::getaddrinfo(node, service, &d_hints, &result);
            if(err)
            {
                return translate_addrinfo_error(err);
            }
            for(auto i = result; i; i = i->ai_next)
            {
                AddressInfo r = {};
                r.family = decode_af(i->ai_family);
                if(r.family == AddressFamily::unspecified) continue;
                r.socktype = decode_type(i->ai_socktype);
                if(r.socktype == SocketType::unspecified && i->ai_socktype != 0) continue;
                r.protocol = decode_protocol(i->ai_protocol);
                if(i->ai_canonname)
                {
                    r.canonname = Name(i->ai_canonname);
                }
                r.flags = AddressInfoFlag::none;
                if(i->ai_flags & AI_PASSIVE)
                {
                    set_flags(r.flags, AddressInfoFlag::passive);
                }
                if(i->ai_addr && decode_socket_address(r.addr, i->ai_addr))
                {
                    out_result.push_back(r);
                }
            }
            if(result)
            {
                freeaddrinfo(result);
            }
            return ok;
        }
    }
}
