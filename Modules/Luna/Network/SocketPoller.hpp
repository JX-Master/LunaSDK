/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SocketPoller.hpp
* @author JXMaster
* @date 2026/8/14
*/
#pragma once
#include "Network.hpp"
#include <Luna/Runtime/Span.hpp>
#include "SocketPoller.generated.hpp"

namespace Luna
{
    namespace Network
    {
        //! @addtogroup Network Network
        //! @{

        //! Specifies socket readiness interests and events.
        enum class SocketEventFlag : u8
        {
            //! No readiness interest or event.
            none = 0x00,
            //! The socket may accept a connection, receive bytes, or observe an orderly peer shutdown.
            readable = 0x01,
            //! The socket may send bytes, or a non-blocking connection attempt may have completed.
            writable = 0x02,
            //! The platform reported a pending socket error.
            //! @details This is an output-only flag and cannot be used as a registration interest.
            error = 0x04,
            //! The platform reported that the peer or native endpoint has hung up.
            //! @details This is an output-only hint and cannot be used as a registration interest.
            hang_up = 0x08,
        };

        //! Identifies one socket registration in one socket poller.
        struct socket_poll_token_t
        {
            //! The opaque token value. A value of `0` is invalid.
            u64 value;

            //! Constructs an invalid token.
            constexpr socket_poll_token_t() : value(0) {}
            //! Constructs one token from its opaque value.
            explicit constexpr socket_poll_token_t(u64 value) : value(value) {}
            //! Tests whether this token is valid.
            explicit constexpr operator bool() const { return value != 0; }
            //! Tests two tokens for equality.
            constexpr bool operator==(const socket_poll_token_t& rhs) const { return value == rhs.value; }
            //! Tests two tokens for inequality.
            constexpr bool operator!=(const socket_poll_token_t& rhs) const { return value != rhs.value; }
        };

        //! The invalid socket poll token.
        constexpr socket_poll_token_t NULL_SOCKET_POLL_TOKEN;

        //! Describes readiness reported for one registered socket.
        struct SocketPollEvent
        {
            //! The token returned when the socket was registered.
            socket_poll_token_t token;
            //! The readiness flags reported for the socket.
            SocketEventFlag events;
            //! The user data supplied when the socket was registered.
            opaque_t user_data;
        };

        //! @interface ISocketPoller
        //! Waits for readiness on multiple non-blocking sockets.
        //! @details The poller uses level-triggered readiness. The poller retains one strong reference to
        //! every registered socket. Except for @ref wake, this interface is not thread-safe and should be
        //! owned by one reactor thread.
        struct [[Luna::interface("{41631B58-0BC5-48B0-A025-9612A27F6AA3}")]] ISocketPoller : virtual Interface
        {
            //! Registers one socket with this poller.
            //! @param[in] socket The socket to register.
            //! @param[in] interests A combination of @ref SocketEventFlag::readable and
            //! @ref SocketEventFlag::writable. @ref SocketEventFlag::none creates a dormant registration.
            //! @param[in] user_data Opaque application data copied to every event for this registration.
            //! @return Returns a non-zero token for the registration. Returns @ref E_ALREADY_EXISTS
            //! if this socket is already registered with this poller.
            //! @remark The socket must be removed from the poller before @ref ISocket::close is called.
            virtual R<socket_poll_token_t> add(
                ISocket* socket,
                SocketEventFlag interests,
                opaque_t user_data = nullptr) = 0;

            //! Changes the readiness interests of one registration.
            //! @param[in] token The token returned by @ref add.
            //! @param[in] interests A combination of @ref SocketEventFlag::readable and
            //! @ref SocketEventFlag::writable. @ref SocketEventFlag::none makes the registration dormant.
            //! @return Returns @ref E_NOT_FOUND if `token` is invalid, removed, or stale.
            virtual RV modify(socket_poll_token_t token, SocketEventFlag interests) = 0;

            //! Removes one socket registration.
            //! @param[in] token The token returned by @ref add.
            //! @return Returns @ref E_NOT_FOUND if `token` is invalid, removed, or stale.
            //! @details This operation releases the strong socket reference held by the poller.
            virtual RV remove(socket_poll_token_t token) = 0;

            //! Waits for registered sockets to become ready.
            //! @param[out] events The buffer that receives readiness events. This span must not be empty.
            //! @param[in] timeout_ms The maximum wait time in milliseconds. `0` performs a non-blocking
            //! query and `U32_MAX` waits indefinitely.
            //! @return Returns the number of events written to `events`. Multiple native events for one
            //! registration are coalesced into one output event.
            //! @details Calling @ref wake makes a blocked call return without adding a wake event to `events`.
            virtual R<usize> poll(Span<SocketPollEvent> events, u32 timeout_ms = U32_MAX) = 0;

            //! Wakes one thread blocked in @ref poll.
            //! @details This is the only thread-safe method on this interface. Multiple pending wake calls
            //! may be coalesced. The poller object must remain alive for the duration of this call.
            virtual void wake() = 0;
        };

        //! Creates one socket readiness poller for the current platform.
        //! @return Returns the created poller.
        LUNA_NETWORK_API R<Ref<ISocketPoller>> new_socket_poller();

        //! @}
    }
}
