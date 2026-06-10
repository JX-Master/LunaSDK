/*!
* This file is a portion of LunaSDK.
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

#include <Luna/Runtime/StackAllocator.hpp>
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
            case WSANOTINITIALISED: return BasicError::bad_calling_time();
            case WSAENETDOWN: return NetworkError::network_down();
            case WSAENOBUFS: return BasicError::insufficient_system_buffer();
            case WSAENOTCONN: return NetworkError::not_connected();
            case WSAEINTR: return BasicError::interrupted();
            case WSAEINPROGRESS: return BasicError::in_progress();
            case WSAENETRESET: return NetworkError::network_reset();
            case WSAEMSGSIZE: return BasicError::data_too_big();
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

        inline R<int> encode_socket_address(sockaddr_storage& out, const SocketAddress& address)
        {
            memzero(&out, sizeof(out));
            switch(address.family)
            {
            case AddressFamily::ipv4:
            {
                sockaddr_in* addr = (sockaddr_in*)&out;
                addr->sin_family = AF_INET;
                addr->sin_port = hton(address.ipv4.port);
                memcpy(&addr->sin_addr.S_un.S_un_b.s_b1, &address.ipv4.address.bytes, 4);
                return (int)sizeof(sockaddr_in);
            }
            case AddressFamily::ipv6:
            {
                sockaddr_in6* addr = (sockaddr_in6*)&out;
                addr->sin6_family = AF_INET6;
                addr->sin6_port = hton(address.ipv6.port);
                addr->sin6_flowinfo = address.ipv6.flow_info;
                addr->sin6_scope_id = address.ipv6.scope_id;
                memcpy(&addr->sin6_addr, &address.ipv6.address.bytes, 16);
                return (int)sizeof(sockaddr_in6);
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
                memcpy(&out.ipv4.address.bytes, &addr->sin_addr.S_un.S_un_b.s_b1, 4);
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

        RV SocketBase::get_local_address(SocketAddress& address)
        {
            sockaddr_storage addr;
            int size = sizeof(addr);
            int r = ::getsockname(m_socket, (sockaddr*)&addr, &size);
            if(r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(NetworkError::address_not_supported());
        }

        RV TCPSocket::get_remote_address(SocketAddress& address)
        {
            sockaddr_storage addr;
            int size = sizeof(addr);
            int r = ::getpeername(m_socket, (sockaddr*)&addr, &size);
            if(r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(NetworkError::address_not_supported());
        }

        RV TCPSocket::read(void* buffer, usize size, usize* read_bytes)
        {
            usize read_size = size > (usize)I32_MAX ? (usize)I32_MAX : size;
            int r = ::recv(m_socket, (char*)buffer, (int)read_size, 0);
            if (r == SOCKET_ERROR)
            {
                if(read_bytes) *read_bytes = 0;
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if (read_bytes) *read_bytes = r;
            return ok;
        }
        RV TCPSocket::write(const void* buffer, usize size, usize* write_bytes)
        {
            usize total_written = 0;
            while(total_written < size)
            {
                usize chunk = size - total_written;
                if(chunk > (usize)I32_MAX) chunk = (usize)I32_MAX;
                int r = ::send(m_socket, (const char*)buffer + total_written, (int)chunk, 0);
                if (r == SOCKET_ERROR)
                {
                    if(write_bytes) *write_bytes = total_written;
                    int err = WSAGetLastError();
                    return translate_error(err);
                }
                if(r == 0)
                {
                    if(write_bytes) *write_bytes = total_written;
                    return BasicError::no_data();
                }
                total_written += (usize)r;
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
            if (r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                return translate_error(err);
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
            if (r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            return ok;
        }
        RV TCPSocket::connect(const SocketAddress& address)
        {
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid()) return addr_size.errcode();
            int r = ::connect(m_socket, (sockaddr*)&addr, addr_size.get());
            if (r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            return ok;
        }
        R<Ref<ITCPSocket>> TCPSocket::accept(SocketAddress& address)
        {
            sockaddr_storage addr;
            int size = sizeof(addr);
            SOCKET r = ::accept(m_socket, (sockaddr*)&addr, &size);
            if (r == INVALID_SOCKET)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if(!decode_socket_address(address, (sockaddr*)&addr))
            {
                closesocket(r);
                return NetworkError::address_not_supported();
            }
            Ref<TCPSocket> s = new_object<TCPSocket>();
            s->m_af = address.family;
            s->m_type = m_type;
            s->m_socket = r;
            return Ref<ITCPSocket>(s);
        }
        RV UDPSocket::send_to(const void* buffer, usize size, const SocketAddress& address, usize* sent_bytes)
        {
            if(size > (usize)I32_MAX)
            {
                if(sent_bytes) *sent_bytes = 0;
                return BasicError::data_too_big();
            }
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                if(sent_bytes) *sent_bytes = 0;
                return addr_size.errcode();
            }
            int r = ::sendto(m_socket, (const char*)buffer, (int)size, 0, (sockaddr*)&addr, addr_size.get());
            if(r == SOCKET_ERROR)
            {
                if(sent_bytes) *sent_bytes = 0;
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if(sent_bytes) *sent_bytes = (usize)r;
            return ok;
        }
        RV UDPSocket::receive_from(void* buffer, usize size, SocketAddress* address, usize* received_bytes)
        {
            usize read_size = size > (usize)I32_MAX ? (usize)I32_MAX : size;
            sockaddr_storage addr;
            int addr_size = sizeof(addr);
            int r = ::recvfrom(m_socket, (char*)buffer, (int)read_size, 0, (sockaddr*)&addr, &addr_size);
            if(r == SOCKET_ERROR)
            {
                if(received_bytes) *received_bytes = 0;
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if(address && !decode_socket_address(*address, (sockaddr*)&addr))
            {
                if(received_bytes) *received_bytes = 0;
                return NetworkError::address_not_supported();
            }
            if(received_bytes) *received_bytes = (usize)r;
            return ok;
        }
        RV platform_init()
        {
            Meta::register_Network_types();
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
                default: return SocketType::unspecified;
            }
        }

        inline int encode_protocol(Protocol protocol)
        {
            switch(protocol)
            {
                case Protocol::unspecified: return 0;
                case Protocol::tcp: return IPPROTO_TCP;
                case Protocol::udp: return IPPROTO_UDP;
                default: lupanic(); return 0;
            }
        }

        inline Protocol decode_protocol(int protocol)
        {
            switch (protocol)
            {
            case 0: return Protocol::unspecified;
            case IPPROTO_TCP: return Protocol::tcp;
            case IPPROTO_UDP: return Protocol::udp;
            default: return Protocol::unspecified;
            }
        }

        inline R<SOCKET> new_native_socket(AddressFamily af, SocketType type, Protocol protocol)
        {
            if(af == AddressFamily::unspecified) return NetworkError::address_not_supported();
            if(type == SocketType::unspecified) return BasicError::bad_arguments();
            int iaf = encode_af(af);
            int itype = encode_type(type);
            int iprotocol = encode_protocol(protocol);
            SOCKET r = ::socket(iaf, itype, iprotocol);
            if (r == INVALID_SOCKET)
            {
                int err = WSAGetLastError();
                return translate_error(err);
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
            StackAllocator salloc;
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
            if(node)
            {
                d_hints.ai_flags |= AI_CANONNAME;
            }
            wchar_t* wnode = nullptr;
            wchar_t* wservice = nullptr;
            if(node)
            {
                usize node_len = utf8_to_utf16_len(node);
                wnode = (wchar_t*)salloc.allocate(sizeof(wchar_t) * (node_len + 1));
                utf8_to_utf16((c16*)wnode, node_len + 1, node);
            }
            if(service)
            {
                usize service_len = utf8_to_utf16_len(service);
                wservice = (wchar_t*)salloc.allocate(sizeof(wchar_t) * (service_len + 1));
                utf8_to_utf16((c16*)wservice, service_len + 1, service);
            }
            ADDRINFOW* result = nullptr;
            auto err = GetAddrInfoW(wnode, wservice, &d_hints, &result);
            if(err)
            {
                return translate_error(err);
            }
            Vector<c8> buffer;
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
                if(i->ai_addr && decode_socket_address(r.addr, i->ai_addr))
                {
                    out_result.push_back(r);
                }
            }
            if(result)
            {
                FreeAddrInfoW(result);
            }
            return ok;
        }
    }
}
