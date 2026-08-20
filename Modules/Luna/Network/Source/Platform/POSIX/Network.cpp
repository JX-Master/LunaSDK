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
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include "Socket.hpp"
#include "Network.meta.generated.hpp"

namespace Luna
{
    namespace Network
    {
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
            case ENETDOWN: return NetworkError::network_down();
            case EPROTONOSUPPORT: return NetworkError::protocol_not_supported();
            case EINTR: return BasicError::interrupted();
            case ENOTCONN: return NetworkError::not_connected();
            case EDESTADDRREQ: return NetworkError::not_connected();
            case EADDRINUSE: return NetworkError::address_in_use();
            case EADDRNOTAVAIL: return NetworkError::address_not_available();
            case EOPNOTSUPP: return BasicError::not_supported();
            case EALREADY: return BasicError::not_ready();
#if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK: return BasicError::not_ready();
#endif
            case EAGAIN: return BasicError::not_ready();
            case EBADF: return BasicError::bad_calling_time();
            case ENOTSOCK: return BasicError::bad_calling_time();
            case ECONNREFUSED: return NetworkError::connection_refused();
            case ECONNABORTED: return NetworkError::connection_aborted();
            case ECONNRESET: return NetworkError::connection_reset();
            case EINPROGRESS: return BasicError::in_progress();
            case EISCONN: return NetworkError::already_connected();
            case ENETRESET: return NetworkError::network_reset();
            case ENETUNREACH: return NetworkError::network_unreachable();
            case EHOSTUNREACH: return NetworkError::host_unreachable();
            case EPROTOTYPE: return NetworkError::protocol_not_supported();
            case ETIMEDOUT: return BasicError::timeout();
            case EPIPE: return NetworkError::connection_reset();
            default: return BasicError::bad_platform_call();
            }
        }

        inline ErrCode translate_addrinfo_error(int err)
        {
            switch (err)
            {
            case EAI_AGAIN: return BasicError::not_ready();
            case EAI_BADFLAGS: return BasicError::bad_arguments();
            case EAI_FAIL: return BasicError::bad_platform_call();
            case EAI_FAMILY: return NetworkError::address_not_supported();
            case EAI_MEMORY: return BasicError::out_of_memory();
            case EAI_NONAME: return NetworkError::host_not_found();
            case EAI_SERVICE: return NetworkError::service_not_found();
            case EAI_SOCKTYPE: return NetworkError::protocol_not_supported();
#ifdef EAI_SYSTEM
            case EAI_SYSTEM: return translate_error(errno);
#endif
            default: return BasicError::bad_platform_call();
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
                return NetworkError::address_not_supported();
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

        inline bool is_connection_error(int err)
        {
            switch(err)
            {
            case ENETDOWN:
            case ENOTCONN:
            case ECONNABORTED:
            case ECONNRESET:
            case ENETRESET:
            case ENETUNREACH:
            case EHOSTUNREACH:
            case ETIMEDOUT:
            case EPIPE:
                return true;
            default:
                return false;
            }
        }

        inline RV configure_native_socket(int socket)
        {
            int flags = ::fcntl(socket, F_GETFL, 0);
            if(flags == -1) return translate_error(errno);
            if(::fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
            {
                return translate_error(errno);
            }
#ifdef SO_NOSIGPIPE
            int no_sigpipe = 1;
            if(::setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe)) == -1)
            {
                return translate_error(errno);
            }
#endif
            return ok;
        }

        void SocketBase::close()
        {
            if(m_socket != -1)
            {
                int socket = m_socket;
                m_socket = -1;
                ::close(socket);
            }
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
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(NetworkError::address_not_supported());
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
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(NetworkError::address_not_supported());
        }

        TCPConnectionState TCPSocket::get_status()
        {
            if(m_socket == -1) return TCPConnectionState::closed;
            if(m_status != TCPConnectionState::connecting) return m_status;

            pollfd descriptor = {};
            descriptor.fd = m_socket;
            descriptor.events = POLLOUT;
            int result = ::poll(&descriptor, 1, 0);
            if(result == 0) return TCPConnectionState::connecting;
            if(result == -1)
            {
                if(errno == EINTR) return TCPConnectionState::connecting;
                m_error = translate_error(errno);
                m_status = TCPConnectionState::error;
                return m_status;
            }

            int socket_error = 0;
            socklen_t error_size = sizeof(socket_error);
            if(::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &socket_error, &error_size) == -1)
            {
                if(errno == EINTR) return TCPConnectionState::connecting;
                m_error = translate_error(errno);
                m_status = TCPConnectionState::error;
                return m_status;
            }
            if(socket_error)
            {
                m_error = translate_error(socket_error);
                m_status = TCPConnectionState::error;
                return m_status;
            }
            m_status = TCPConnectionState::connected;
            return m_status;
        }

        RV TCPSocket::receive(void* buffer, usize size, usize* out_received_bytes)
        {
            if(out_received_bytes) *out_received_bytes = 0;
            if(!size) return ok;
            if(m_socket == -1) return BasicError::bad_calling_time();
            if(m_status != TCPConnectionState::connected && m_status != TCPConnectionState::peer_closed)
            {
                return BasicError::bad_calling_time();
            }
            usize read_size = size > (usize)ISIZE_MAX ? (usize)ISIZE_MAX : size;
            isize received = ::recv(m_socket, buffer, read_size, 0);
            if(received == -1)
            {
                int err = errno;
                ErrCode error = translate_error(err);
                if(is_connection_error(err))
                {
                    m_error = error;
                    m_status = TCPConnectionState::error;
                }
                return error;
            }
            if(out_received_bytes) *out_received_bytes = (usize)received;
            if(received == 0)
            {
                m_status = TCPConnectionState::peer_closed;
            }
            return ok;
        }

        RV TCPSocket::send(const void* buffer, usize size, usize* out_sent_bytes)
        {
            if(out_sent_bytes) *out_sent_bytes = 0;
            if(!size) return ok;
            if(m_socket == -1) return BasicError::bad_calling_time();
            if(m_status != TCPConnectionState::connected && m_status != TCPConnectionState::peer_closed)
            {
                return BasicError::bad_calling_time();
            }
            usize write_size = size > (usize)ISIZE_MAX ? (usize)ISIZE_MAX : size;
            isize sent = ::send(m_socket, buffer, write_size, send_flags());
            if(sent == -1)
            {
                int err = errno;
                ErrCode error = translate_error(err);
                if(is_connection_error(err))
                {
                    m_error = error;
                    m_status = TCPConnectionState::error;
                }
                return error;
            }
            if(sent == 0) return BasicError::no_data();
            if(out_sent_bytes) *out_sent_bytes = (usize)sent;
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
            if(m_socket == -1) return BasicError::bad_calling_time();
            if(m_status != TCPConnectionState::not_connected) return BasicError::bad_calling_time();
            if (len == I32_MAX)
            {
                len = SOMAXCONN;
            }
            int r = ::listen(m_socket, len);
            if (r == -1)
            {
                m_error = translate_error(errno);
                m_status = TCPConnectionState::error;
                return m_error;
            }
            m_status = TCPConnectionState::listening;
            return ok;
        }
        RV TCPSocket::connect(const SocketAddress& address)
        {
            if(m_socket == -1) return BasicError::bad_calling_time();
            if(m_status == TCPConnectionState::connected || m_status == TCPConnectionState::peer_closed)
            {
                return NetworkError::already_connected();
            }
            if(m_status != TCPConnectionState::not_connected) return BasicError::bad_calling_time();
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                m_error = addr_size.errcode();
                m_status = TCPConnectionState::error;
                return m_error;
            }
            int r = ::connect(m_socket, (sockaddr*)&addr, addr_size.get());
            if (r == -1)
            {
                int err = errno;
                if(err == EINPROGRESS || err == EALREADY)
                {
                    m_status = TCPConnectionState::connecting;
                    return ok;
                }
                if(err == EISCONN)
                {
                    m_status = TCPConnectionState::connected;
                    return ok;
                }
                m_error = translate_error(err);
                m_status = TCPConnectionState::error;
                return m_error;
            }
            m_status = TCPConnectionState::connected;
            return ok;
        }
        R<Ref<ITCPSocket>> TCPSocket::accept(SocketAddress& address)
        {
            if(m_socket == -1) return BasicError::bad_calling_time();
            if(m_status != TCPConnectionState::listening) return BasicError::bad_calling_time();
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
                return NetworkError::address_not_supported();
            }
            RV configure_result = configure_native_socket(r);
            if(failed(configure_result))
            {
                ::close(r);
                return configure_result.errcode();
            }
            Ref<TCPSocket> s = new_object<TCPSocket>();
            s->m_af = address.family;
            s->m_type = m_type;
            s->m_socket = r;
            s->m_status = TCPConnectionState::connected;
            return Ref<ITCPSocket>(s);
        }
        RV UDPSocket::send_to(const void* buffer, usize size, const SocketAddress& address, usize* out_sent_bytes)
        {
            if(size > (usize)ISIZE_MAX)
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                return BasicError::data_too_big();
            }
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                return addr_size.errcode();
            }
            isize sent = ::sendto(m_socket, buffer, size, send_flags(), (sockaddr*)&addr, addr_size.get());
            if(sent == -1)
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                return translate_error(errno);
            }
            if(out_sent_bytes) *out_sent_bytes = (usize)sent;
            return ok;
        }
        RV UDPSocket::receive_from(void* buffer, usize size, SocketAddress* address, usize* out_received_bytes)
        {
            usize read_size = size > (usize)ISIZE_MAX ? (usize)ISIZE_MAX : size;
            sockaddr_storage addr;
            socklen_t addr_size = sizeof(addr);
            isize received = ::recvfrom(m_socket, buffer, read_size, 0, (sockaddr*)&addr, &addr_size);
            if(received == -1)
            {
                if(out_received_bytes) *out_received_bytes = 0;
                return translate_error(errno);
            }
            if(address && !decode_socket_address(*address, (sockaddr*)&addr))
            {
                if(out_received_bytes) *out_received_bytes = 0;
                return NetworkError::address_not_supported();
            }
            if(out_received_bytes) *out_received_bytes = (usize)received;
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
            if(!encode_af(af, iaf, false)) return NetworkError::address_not_supported();
            int itype;
            if(!encode_type(type, itype, false)) return BasicError::bad_arguments();
            int iprotocol;
            if(!encode_protocol(protocol, iprotocol)) return NetworkError::protocol_not_supported();
            int r = ::socket(iaf, itype, iprotocol);
            if(r == -1)
            {
                return translate_error(errno);
            }
            RV configure_result = configure_native_socket(r);
            if(failed(configure_result))
            {
                ::close(r);
                return configure_result.errcode();
            }
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
                if(!encode_af(hints->family, d_hints.ai_family, true)) return NetworkError::address_not_supported();
                if(!encode_type(hints->socktype, d_hints.ai_socktype, true)) return BasicError::bad_arguments();
                if(!encode_protocol(hints->protocol, d_hints.ai_protocol)) return NetworkError::protocol_not_supported();
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
