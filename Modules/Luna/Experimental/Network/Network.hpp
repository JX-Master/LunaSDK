/*!
* This file is a portion of Luna SDK.
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
            u8 bytes[16];
        };

        //! A special IPv4 address that does not specify any particular address.
        constexpr IPv4Address IPV4_ADDRESS_ANY = { 0, 0, 0, 0 };

        //! The address to use when opening a socket using IPv4 address.
        struct SocketAddressIPv4
        {
            //! The IPv4 address.
            IPv4Address address;
            //! The port number of the address in host byte order.
            u16 port;
        };

        //! Specifies address family.
        enum class AddressFamily : u32
        {
            //! Maps to `AF_UNSPEC`. The address family is unspecified.
            unspecified = 0,
            //! Maps to `AF_INET`. The Internet Protocol version 4 (IPv4) address family.
            ipv4,
            //! Maps to `AF_INET6`. The Internet Protocol version 6 (IPv6) address family.
            ipv6,
            //! Maps to `AF_BTH` or `AF_BLUETOOTH`. The Bluetooth address family.
            bluetooth
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

            };
        };

        //! @interface ISocket
        //! Represents one socket, which is a network communication endpoint.
        //! @details Each socket is associated with a socket address, which consists of an IP address and a port number. 
        //! Sockets allow for real-time, bi-directional communication between a client and a server, and are used in various protocols like TCP/IP and UDP.
        struct ISocket : virtual IStream
        {
            luiid("{36233BD3-54A0-4E67-B01E-C79E8115F548}");

            //! Gets the native handle of this socket.
            //! @details On Windows platforms, the returned handle can be reinterpreted to `SOCKET` type.
            //! On POSIX platforms, the returned handle can be reinterpreted to `int`, which is the file
            //! descriptor of the socket.
            //! @return Returns the native handle of this socket.
            virtual opaque_t get_native_handle() = 0;

            //! Binds one address to this socket, so that it can be used to listen connections from that address. 
            //! @param[in] address The address to bind.
            //! @par Valid 
            virtual RV bind(const SocketAddress& address) = 0;

            //! Starts listening for incoming connections.
            //! @param[in] len The maximum number of connections that can be queued to be accepted.
            virtual RV listen(i32 len) = 0;

            //! Connects to the specified host.
            //! @param[in] address The target address to connect.
            virtual RV connect(const SocketAddress& address) = 0;

            //! Accepts incoming connection attempt on this socket.
            //! @param[out] address The assigned address for the accepted connection.
            //! @return Returns the socket that represents the accepted connection.
            virtual R<Ref<ISocket>> accept(SocketAddress& address) = 0;
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
            //! Maps to `SOCK_RAW`
            //! Provides raw network protocol access.
            raw,
            //! Maps to `SOCK_RDM`
            //! Provides a reliable datagram layer that does not guarantee ordering.
            rdm,
        };

        //! Specifies the transmission protocol used by the socket.
        enum class Protocol : u32
        {
            //! The network protocol is unspecified. The system chooses the most suitable protocol based 
            //! on `af` and `type` parameters.
            unspecified = 0,
            //! The Internet Control Message Protocol (ICMP). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::unspecified, 
            //! @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6 and the `type` parameter is @ref SocketType::raw or @ref SocketType::unspecified.
            icmp,
            //! The Internet Group Management Protocol (IGMP).
            //! This is a possible value when the `af` parameter is @ref AddressFamily::unspecified, 
            //! @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6 and the `type` parameter is SocketType::raw or @ref SocketType::unspecified.
            igmp,
            //! The Bluetooth Radio Frequency Communications (Bluetooth RFCOMM) protocol. 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::bluetooth
            //! and the `type` parameter is @ref SocketType::stream.
            rfcomm,
            //! Use Transmission Control Protocol (TCP). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6 
            //! and the `type` parameter is @ref SocketType::stream.
            tcp,
            //! Use User Datagram Protocol (UDP). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6
            //! and the `type` parameter is @ref SocketType::dgram.
            udp,
            //! The Internet Control Message Protocol Version 6 (ICMPv6). 
            //! This is a possible value when the `af` parameter is @ref AddressFamily::unspecified, 
            //! @ref AddressFamily::ipv4 or @ref AddressFamily::ipv6 and the `type` parameter is @ref SocketType::raw or @ref SocketType::unspecified.
            icmpv6,
        };

        //! Creates one new socket.
        //! @param[in] af The address family for the new socket.
        //! @param[in] type The socket type.
        //! @param[in] protocol The transmission protocol used by the socket.
        //! @return Returns the created socket.
        LUNA_NETWORK_API R<Ref<ISocket>> new_socket(AddressFamily af, SocketType type, Protocol protocol);

        //! Specifies flag attributes of one address.
        enum class AddressInfoFlag : u8
        {
            none = 0,
            //! If set, this address is used for @ref ISocket::bind`. If unset, this address is used for @ref ISocket::connect.
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
    //! @defgroup NetworkError Network Errors
    //! @}
    namespace NetworkError
    {
        //! @addtogroup NetworkError
        //! @{
        
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

        //! The specified host cannot be found.
        LUNA_NETWORK_API ErrCode host_not_found();

        //! The service is not supported on the target host with specified socket type.
        LUNA_NETWORK_API ErrCode service_not_found();

        //! @}
    }

    struct Module;
    LUNA_NETWORK_API Module* module_network();
}
