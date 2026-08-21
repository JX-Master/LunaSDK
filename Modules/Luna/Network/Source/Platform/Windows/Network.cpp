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
        inline ResultCode translate_error(int err)
        {
            switch (err)
            {
            case WSANOTINITIALISED: return E_BAD_CALLING_TIME;
            case WSAENETDOWN: return Network::E_NETWORK_DOWN;
            case WSAENOBUFS: return E_INSUFFICIENT_SYSTEM_BUFFER;
            case WSAENOTCONN: return Network::E_NOT_CONNECTED;
            case WSAENOTSOCK: return E_BAD_CALLING_TIME;
            case WSAEINTR: return E_INTERRUPTED;
            case WSAEINPROGRESS: return E_IN_PROGRESS;
            case WSAEWOULDBLOCK: return E_NOT_READY;
            case WSAENETRESET: return Network::E_NETWORK_RESET;
            case WSAEMSGSIZE: return E_DATA_TOO_BIG;
            case WSAEINVAL: return E_BAD_ARGUMENTS;
            case WSAECONNABORTED: return Network::E_CONNECTION_ABORTED;
            case WSAETIMEDOUT: return E_TIMEOUT;
            case WSAECONNRESET: return Network::E_CONNECTION_RESET;
            case WSAEADDRINUSE: return Network::E_ADDRESS_IN_USE;
            case WSAEADDRNOTAVAIL: return Network::E_ADDRESS_NOT_AVAILABLE;
            case WSAEISCONN: return Network::E_ALREADY_CONNECTED;
            case WSAEMFILE: return E_OUT_OF_RESOURCE;
            case WSAEOPNOTSUPP: return E_NOT_SUPPORTED;
            case WSAEALREADY: return Network::E_ALREADY_CONNECTED;
            case WSAEAFNOSUPPORT: return Network::E_ADDRESS_NOT_SUPPORTED;
            case WSAECONNREFUSED: return Network::E_CONNECTION_REFUSED;
            case WSAENETUNREACH: return Network::E_NETWORK_UNREACHABLE;
            case WSAEHOSTUNREACH: return Network::E_HOST_UNREACHABLE;
            case WSASYSNOTREADY: return E_BAD_CALLING_TIME;
            case WSAVERNOTSUPPORTED: return E_NOT_SUPPORTED;
            case WSAEPROCLIM: return E_OUT_OF_RESOURCE;
            case WSAEPROTOTYPE: return Network::E_PROTOCOL_NOT_SUPPORTED;
            case WSAESOCKTNOSUPPORT: return E_BAD_ARGUMENTS;
            case WSAEPROTONOSUPPORT: return Network::E_PROTOCOL_NOT_SUPPORTED;
            case WSATRY_AGAIN: return E_NOT_READY;
            case WSANO_RECOVERY: return E_BAD_ARGUMENTS;
            case WSA_NOT_ENOUGH_MEMORY: return E_OUT_OF_MEMORY;
            case WSAHOST_NOT_FOUND: return Network::E_HOST_NOT_FOUND;
            case WSATYPE_NOT_FOUND: return Network::E_SERVICE_NOT_FOUND;

            default: return E_BAD_PLATFORM_CALL;
            }
        }

        inline bool is_connection_error(int err)
        {
            switch(err)
            {
            case WSAENETDOWN:
            case WSAENOTCONN:
            case WSAENETRESET:
            case WSAECONNABORTED:
            case WSAETIMEDOUT:
            case WSAECONNRESET:
            case WSAENETUNREACH:
            case WSAEHOSTUNREACH:
                return true;
            default:
                return false;
            }
        }

        inline RV configure_native_socket(SOCKET socket)
        {
            u_long non_blocking = 1;
            if(::ioctlsocket(socket, FIONBIO, &non_blocking) == SOCKET_ERROR)
            {
                return translate_error(WSAGetLastError());
            }
            return ok;
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

        void SocketBase::close()
        {
            if(m_socket != INVALID_SOCKET)
            {
                SOCKET socket = m_socket;
                m_socket = INVALID_SOCKET;
                ::closesocket(socket);
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
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(Network::E_ADDRESS_NOT_SUPPORTED);
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
            return decode_socket_address(address, (sockaddr*)&addr) ? ok : RV(Network::E_ADDRESS_NOT_SUPPORTED);
        }

        TCPConnectionState TCPSocket::get_status()
        {
            if(m_socket == INVALID_SOCKET) return TCPConnectionState::closed;
            if(m_status != TCPConnectionState::connecting) return m_status;

            WSAPOLLFD descriptor = {};
            descriptor.fd = m_socket;
            descriptor.events = POLLWRNORM;
            int result = ::WSAPoll(&descriptor, 1, 0);
            if(result == 0) return TCPConnectionState::connecting;
            if(result == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if(err == WSAEINTR) return TCPConnectionState::connecting;
                m_error = translate_error(err);
                m_status = TCPConnectionState::error;
                return m_status;
            }

            int socket_error = 0;
            int error_size = sizeof(socket_error);
            if(::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&socket_error, &error_size) == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if(err == WSAEINTR) return TCPConnectionState::connecting;
                m_error = translate_error(err);
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
            if(m_socket == INVALID_SOCKET) return E_BAD_CALLING_TIME;
            if(m_status != TCPConnectionState::connected && m_status != TCPConnectionState::peer_closed)
            {
                return E_BAD_CALLING_TIME;
            }
            usize read_size = size > (usize)I32_MAX ? (usize)I32_MAX : size;
            int received = ::recv(m_socket, (char*)buffer, (int)read_size, 0);
            if(received == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                ResultCode error = translate_error(err);
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
            if(m_socket == INVALID_SOCKET) return E_BAD_CALLING_TIME;
            if(m_status != TCPConnectionState::connected && m_status != TCPConnectionState::peer_closed)
            {
                return E_BAD_CALLING_TIME;
            }
            usize write_size = size > (usize)I32_MAX ? (usize)I32_MAX : size;
            int sent = ::send(m_socket, (const char*)buffer, (int)write_size, 0);
            if(sent == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                ResultCode error = translate_error(err);
                if(is_connection_error(err))
                {
                    m_error = error;
                    m_status = TCPConnectionState::error;
                }
                return error;
            }
            if(sent == 0) return E_NO_DATA;
            if(out_sent_bytes) *out_sent_bytes = (usize)sent;
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
            if(m_socket == INVALID_SOCKET) return E_BAD_CALLING_TIME;
            if(m_status != TCPConnectionState::not_connected) return E_BAD_CALLING_TIME;
            if (len == I32_MAX)
            {
                len = SOMAXCONN;
            }
            int r = ::listen(m_socket, len);
            if (r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                m_error = translate_error(err);
                m_status = TCPConnectionState::error;
                return m_error;
            }
            m_status = TCPConnectionState::listening;
            return ok;
        }
        RV TCPSocket::connect(const SocketAddress& address)
        {
            if(m_socket == INVALID_SOCKET) return E_BAD_CALLING_TIME;
            if(m_status == TCPConnectionState::connected || m_status == TCPConnectionState::peer_closed)
            {
                return Network::E_ALREADY_CONNECTED;
            }
            if(m_status != TCPConnectionState::not_connected) return E_BAD_CALLING_TIME;
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                m_error = addr_size.errcode();
                m_status = TCPConnectionState::error;
                return m_error;
            }
            int r = ::connect(m_socket, (sockaddr*)&addr, addr_size.get());
            if (r == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if(err == WSAEWOULDBLOCK || err == WSAEINPROGRESS || err == WSAEALREADY)
                {
                    m_status = TCPConnectionState::connecting;
                    return ok;
                }
                if(err == WSAEISCONN)
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
            if(m_socket == INVALID_SOCKET) return E_BAD_CALLING_TIME;
            if(m_status != TCPConnectionState::listening) return E_BAD_CALLING_TIME;
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
                return Network::E_ADDRESS_NOT_SUPPORTED;
            }
            RV configure_result = configure_native_socket(r);
            if(failed(configure_result))
            {
                closesocket(r);
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
            if(size > (usize)I32_MAX)
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                return E_DATA_TOO_BIG;
            }
            sockaddr_storage addr;
            auto addr_size = encode_socket_address(addr, address);
            if(!addr_size.valid())
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                return addr_size.errcode();
            }
            int r = ::sendto(m_socket, (const char*)buffer, (int)size, 0, (sockaddr*)&addr, addr_size.get());
            if(r == SOCKET_ERROR)
            {
                if(out_sent_bytes) *out_sent_bytes = 0;
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if(out_sent_bytes) *out_sent_bytes = (usize)r;
            return ok;
        }
        RV UDPSocket::receive_from(void* buffer, usize size, SocketAddress* address, usize* out_received_bytes)
        {
            usize read_size = size > (usize)I32_MAX ? (usize)I32_MAX : size;
            sockaddr_storage addr;
            int addr_size = sizeof(addr);
            int r = ::recvfrom(m_socket, (char*)buffer, (int)read_size, 0, (sockaddr*)&addr, &addr_size);
            if(r == SOCKET_ERROR)
            {
                if(out_received_bytes) *out_received_bytes = 0;
                int err = WSAGetLastError();
                return translate_error(err);
            }
            if(address && !decode_socket_address(*address, (sockaddr*)&addr))
            {
                if(out_received_bytes) *out_received_bytes = 0;
                return Network::E_ADDRESS_NOT_SUPPORTED;
            }
            if(out_received_bytes) *out_received_bytes = (usize)r;
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
            if(af == AddressFamily::unspecified) return Network::E_ADDRESS_NOT_SUPPORTED;
            if(type == SocketType::unspecified) return E_BAD_ARGUMENTS;
            int iaf = encode_af(af);
            int itype = encode_type(type);
            int iprotocol = encode_protocol(protocol);
            SOCKET r = ::socket(iaf, itype, iprotocol);
            if (r == INVALID_SOCKET)
            {
                int err = WSAGetLastError();
                return translate_error(err);
            }
            RV configure_result = configure_native_socket(r);
            if(failed(configure_result))
            {
                closesocket(r);
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
