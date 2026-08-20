/*!
* This file is a portion of LunaSDK.
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
#include "Network.generated.hpp"

#ifndef LUNA_NETWORK_API
#define LUNA_NETWORK_API
#endif

namespace Luna
{
    namespace Network
    {
        //! @addtogroup Network Network
        //! Network module provides uniform socket-based APIs to access platform's network features.
        //! @{
        
        //! Converts one unsigned integer from host byte order to network byte order.
        //! @param[in] hostlong The integer to convert.
        //! @return Returns the converted integer.
        //! @remark There are two kinds of byte orders: big endian and little endian.
        //! For a multi-byte value (like a 32-bit integer), the byte order determines which byte is stored in the 
        //! memory firstly, thus get transmitted through the network firstly. For example, for a 32-bit
        //! value 0x0A0B0C0D, the memory layout (from lower address to higher) is 0x0A, 0x0B, 0x0C, 0x0D in big endian, 
        //! and 0x0D, 0x0C, 0x0B, 0x0A in little endian.
        //! 
        //! The network byte order is always big-endian, so a 32-bit value 0x0A0B0C0D will be sent as: 0x0A, 0x0B, 0x0C, 0x0D.
        //! The host byte order is CPU-dependent. When the network byte order and host byte order is not the same, @ref hton
        //! and @ref ntoh can be used to convert between host byte order and network byte order.
        LUNA_NETWORK_API u32 hton(u32 hostlong);
        //! Converts one unsigned short integer from host byte order to network byte order. 
        //! See remarks of @ref hton for details.
        //! @param[in] hostshort The integer to convert.
        //! @return Return the converted integer.
        LUNA_NETWORK_API u16 hton(u16 hostshort);
        //! Converts one unsigned integer from network byte order to host byte order.
        //! See remarks of @ref hton for details.
        //! @param[in] netlong The integer to convert.
        //! @return Returns the converted integer.
        LUNA_NETWORK_API u32 ntoh(u32 netlong);
        //! Converts one unsigned short integer from network byte order to host byte order.
        //! See remarks of @ref hton for details.
        //! @param[in] netshort The integer to convert.
        //! @return Returns the converted integer.
        LUNA_NETWORK_API u16 ntoh(u16 netshort);

        //! Specifies one IPv4 address.
        struct IPv4Address
        {
            //! The address bytes.
            u8 bytes[4];
        };

        //! Specifies one IPv6 address.
        struct IPv6Address
        {
            //! The address bytes.
            u8 bytes[16];
        };

        //! A special IPv4 address that does not specify any particular address.
        constexpr IPv4Address IPV4_ADDRESS_ANY = { 0, 0, 0, 0 };
        //! A special IPv6 address that does not specify any particular address.
        constexpr IPv6Address IPV6_ADDRESS_ANY = {};

        //! The address to use when opening a socket using IPv4 address.
        struct SocketAddressIPv4
        {
            //! The IPv4 address.
            IPv4Address address;
            //! The port number of the address in host byte order.
            u16 port;
        };

        //! The address to use when opening a socket using IPv6 address.
        struct SocketAddressIPv6
        {
            //! The IPv6 address.
            IPv6Address address;
            //! The port number of the address in host byte order.
            u16 port;
            //! The IPv6 flow information. Usually `0`.
            u32 flow_info;
            //! The IPv6 scope ID. Usually `0`, unless the address is scoped such as link-local addresses.
            u32 scope_id;
        };

        //! Specifies address family.
        enum class AddressFamily : u32
        {
            //! Maps to `AF_UNSPEC`. The address family is unspecified.
            unspecified = 0,
            //! Maps to `AF_INET`. The Internet Protocol version 4 (IPv4) address family.
            ipv4,
            //! Maps to `AF_INET6`. The Internet Protocol version 6 (IPv6) address family.
            ipv6
        };
        
        //! Specifies address to use when opening a socket.
        struct SocketAddress
        {
            //! The address family to use.
            AddressFamily family;
            union
            {
                //! The address to use if `family` is @ref AddressFamily::ipv4.
                SocketAddressIPv4 ipv4;
                //! The address to use if `family` is @ref AddressFamily::ipv6.
                SocketAddressIPv6 ipv6;

            };
        };

        //! @interface ISocket
        //! Represents one socket, which is a network communication endpoint.
        //! @details Each socket is associated with a socket address, which consists of an IP address and a port number.
        struct [[Luna::interface("{36233BD3-54A0-4E67-B01E-C79E8115F548}")]] ISocket : virtual Interface
        {
            //! Gets the native handle of this socket.
            //! @details On Windows platforms, the returned handle can be reinterpreted to `SOCKET` type.
            //! On POSIX platforms, the returned handle can be reinterpreted to `int`, which is the file
            //! descriptor of the socket.
            //! @return Returns the native handle of this socket.
            virtual opaque_t get_native_handle() = 0;

            //! Gets the local address assigned to this socket.
            //! @param[out] address Returns the local address.
            virtual RV get_local_address(SocketAddress& address) = 0;

            //! Binds one address to this socket.
            //! @param[in] address The address to bind.
            virtual RV bind(const SocketAddress& address) = 0;
        };

        //! @interface ITCPSocket
        //! Represents one TCP socket.
        //! @details TCP sockets provide reliable byte-stream communication.
        struct [[Luna::interface("{FE548F0C-F3E6-49EE-B729-36B0B7C6CE2E}")]] ITCPSocket : virtual ISocket, virtual IStream
        {
            //! Gets the remote address connected to this socket.
            //! @param[out] address Returns the remote address.
            virtual RV get_remote_address(SocketAddress& address) = 0;

            //! Starts listening for incoming connections.
            //! @param[in] len The maximum number of connections that can be queued to be accepted.
            virtual RV listen(i32 len) = 0;

            //! Connects to the specified host.
            //! @param[in] address The target address to connect.
            virtual RV connect(const SocketAddress& address) = 0;

            //! Accepts incoming connection attempt on this socket.
            //! @param[out] address The assigned address for the accepted connection.
            //! @return Returns the socket that represents the accepted connection.
            virtual R<Ref<ITCPSocket>> accept(SocketAddress& address) = 0;
        };

        //! @interface IUDPSocket
        //! Represents one UDP socket.
        //! @details UDP sockets provide connectionless datagram communication.
        struct [[Luna::interface("{560F8D2B-F29F-481E-B7DC-226F16336972}")]] IUDPSocket : virtual ISocket
        {
            //! Sends one datagram to the specified address.
            //! @param[in] buffer The buffer that holds data to send.
            //! @param[in] size The size, in bytes, to send from the buffer.
            //! @param[in] address The destination address.
            //! @param[out] sent_bytes If not `nullptr`, returns the actual number of bytes sent.
            virtual RV send_to(const void* buffer, usize size, const SocketAddress& address, usize* sent_bytes = nullptr) = 0;

            //! Receives one datagram and optionally reports the source address.
            //! @param[in] buffer The buffer to accept received data.
            //! @param[in] size The size, in bytes, of `buffer`.
            //! @param[out] address If not `nullptr`, returns the source address of the datagram.
            //! @param[out] received_bytes If not `nullptr`, returns the actual number of bytes received.
            virtual RV receive_from(void* buffer, usize size, SocketAddress* address = nullptr, usize* received_bytes = nullptr) = 0;
        };

        //! Specifies the socket type.
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
        };

        //! Specifies the transmission protocol used by the socket.
        enum class Protocol : u32
        {
            //! The network protocol is unspecified. The system chooses the most suitable protocol based 
            //! on `af` and `type` parameters.
            unspecified = 0,
            //! Use Transmission Control Protocol (TCP). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6 
            //! and the `type` parameter is @ref SocketType::stream.
            tcp,
            //! Use User Datagram Protocol (UDP). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6
            //! and the `type` parameter is @ref SocketType::dgram.
            udp,
        };

        //! Creates one new TCP socket.
        //! @param[in] af The address family for the new socket.
        //! @return Returns the created socket.
        LUNA_NETWORK_API R<Ref<ITCPSocket>> new_tcp_socket(AddressFamily af);

        //! Creates one new UDP socket.
        //! @param[in] af The address family for the new socket.
        //! @return Returns the created socket.
        LUNA_NETWORK_API R<Ref<IUDPSocket>> new_udp_socket(AddressFamily af);

        //! Specifies flag attributes of one address.
        enum class AddressInfoFlag : u8
        {
            none = 0,
            //! If set, this address is used for @ref ISocket::bind. If unset, this address is used for @ref ITCPSocket::connect.
            passive = 0x01,
        };

        //! The address information fetched from @ref getaddrinfo.
        struct AddressInfo
        {
            //! The flag attributes.
            AddressInfoFlag flags;
            //! The address family.
            AddressFamily family;
            //! The address socket type.
            SocketType socktype;
            //! The used protocol for the address.
            Protocol protocol;
            //! The canon name of the address.
            Name canonname;
            //! The address data.
            SocketAddress addr;
        };
        
        //! Gets address from host domain name, or gets port number from service name.
        //! @param[in] node The host domain name or address string.
        //! @param[in] service The service decimal port number or service name (like "ftp", "http", etc.).
        //! @param[in] hints Hints to the type of the information expected to get from this function. This may be `nullptr`.
        //! @param[out] result The vector to accept query result. Results will be pushed to the back of this vector. Existing elements will not be modified.
        LUNA_NETWORK_API RV getaddrinfo(const c8* node, const c8* service, const AddressInfo* hints, Vector<AddressInfo>& result);

        //! @}
    }
    //! @addtogroup Network
    //! @{
    //! @defgroup NetworkResultCodes Network Result Codes
    //! @}
    namespace Network
    {
        //! @addtogroup NetworkResultCodes
        //! @{
        
        //! The Network error category identifier.
        inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK);

        //! The socket is not connected.
        inline constexpr ResultCode E_NOT_CONNECTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -1);
        //! The socket is already connected.
        inline constexpr ResultCode E_ALREADY_CONNECTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -2);
        //! The network subsystem has failed.
        inline constexpr ResultCode E_NETWORK_DOWN = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -3);
        //! The address family is not supported.
        inline constexpr ResultCode E_ADDRESS_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -4);
        //! The address is already in use.
        inline constexpr ResultCode E_ADDRESS_IN_USE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -5);
        //! The address is not available.
        inline constexpr ResultCode E_ADDRESS_NOT_AVAILABLE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -6);
        //! The network connection was reset.
        inline constexpr ResultCode E_NETWORK_RESET = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -7);
        //! The connection attempt was refused.
        inline constexpr ResultCode E_CONNECTION_REFUSED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -8);
        //! The connection was aborted.
        inline constexpr ResultCode E_CONNECTION_ABORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -9);
        //! The connection was reset by its peer.
        inline constexpr ResultCode E_CONNECTION_RESET = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -10);
        //! The network is unreachable.
        inline constexpr ResultCode E_NETWORK_UNREACHABLE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -11);
        //! The host is unreachable.
        inline constexpr ResultCode E_HOST_UNREACHABLE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -12);
        //! The protocol is not supported.
        inline constexpr ResultCode E_PROTOCOL_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -13);
        //! The host was not found.
        inline constexpr ResultCode E_HOST_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -14);
        //! The network service was not found.
        inline constexpr ResultCode E_SERVICE_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::NETWORK, -15);

        //! @}
    }

    struct Module;
    LUNA_NETWORK_API Module* module_network();
}
