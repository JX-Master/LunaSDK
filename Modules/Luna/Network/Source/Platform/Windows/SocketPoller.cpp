/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SocketPoller.cpp
* @author JXMaster
* @date 2026/8/14
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "Socket.hpp"

namespace Luna
{
    namespace Network
    {
        namespace
        {
            constexpr SocketEventFlag INTEREST_FLAGS =
                SocketEventFlag::readable | SocketEventFlag::writable;
            constexpr SocketEventFlag OUTPUT_FLAGS =
                SocketEventFlag::error | SocketEventFlag::hang_up;

            ErrCode translate_poller_error(int error)
            {
                switch(error)
                {
                case WSANOTINITIALISED: return BasicError::bad_calling_time();
                case WSAEACCES: return BasicError::access_denied();
                case WSAEFAULT: return BasicError::bad_memory_address();
                case WSAEINTR: return BasicError::interrupted();
                case WSAEINVAL: return BasicError::bad_arguments();
                case WSAEMFILE: return BasicError::out_of_resource();
                case WSAENETDOWN: return NetworkError::network_down();
                case WSAENOBUFS: return BasicError::insufficient_system_buffer();
                case WSAENOTSOCK: return BasicError::bad_calling_time();
                case WSAEWOULDBLOCK: return BasicError::not_ready();
                default: return BasicError::bad_platform_call();
                }
            }

            bool validate_interests(SocketEventFlag interests)
            {
                u8 value = (u8)interests;
                return (value & ~(u8)INTEREST_FLAGS) == 0;
            }

            socket_poll_token_t make_token(u32 index, u32 generation)
            {
                return socket_poll_token_t((u64)index | ((u64)generation << 32));
            }

            u32 get_token_index(socket_poll_token_t token)
            {
                return (u32)token.value;
            }

            u32 get_token_generation(socket_poll_token_t token)
            {
                return (u32)(token.value >> 32);
            }

            SocketPollRegistration* find_registration(
                Vector<SocketPollRegistration>& registrations,
                socket_poll_token_t token)
            {
                if(!token) return nullptr;
                u32 index = get_token_index(token);
                if((usize)index >= registrations.size()) return nullptr;
                SocketPollRegistration& registration = registrations[index];
                if(!registration.active || registration.generation != get_token_generation(token))
                {
                    return nullptr;
                }
                return &registration;
            }

            SOCKET get_native_socket(ISocket* socket)
            {
                return (SOCKET)socket->get_native_handle();
            }

            RV configure_wake_socket(SOCKET socket)
            {
                u_long non_blocking = 1;
                if(::ioctlsocket(socket, FIONBIO, &non_blocking) == SOCKET_ERROR)
                {
                    return translate_poller_error(WSAGetLastError());
                }
                return ok;
            }

            void drain_wake_socket(SOCKET socket)
            {
                c8 buffer[64];
                for(;;)
                {
                    int result = ::recv(socket, buffer, (int)sizeof(buffer), 0);
                    if(result > 0) continue;
                    if(result == SOCKET_ERROR && WSAGetLastError() == WSAEINTR) continue;
                    break;
                }
            }

            void append_poll_event(
                SocketPoller* poller,
                Span<SocketPollEvent> events,
                usize& event_count,
                socket_poll_token_t token,
                SocketEventFlag native_events)
            {
                SocketPollRegistration* registration = find_registration(poller->m_registrations, token);
                if(!registration || registration->interests == SocketEventFlag::none) return;

                SocketEventFlag allowed_events = registration->interests | OUTPUT_FLAGS;
                SocketEventFlag reported_events = native_events & allowed_events;
                if(reported_events == SocketEventFlag::none) return;

                if(event_count == events.size()) return;
                events[event_count].token = token;
                events[event_count].events = reported_events;
                events[event_count].user_data = registration->user_data;
                ++event_count;
            }
        }

        SocketPoller::~SocketPoller()
        {
            if(m_wake_receiver != INVALID_SOCKET)
            {
                ::closesocket(m_wake_receiver);
                m_wake_receiver = INVALID_SOCKET;
            }
            if(m_wake_sender != INVALID_SOCKET)
            {
                ::closesocket(m_wake_sender);
                m_wake_sender = INVALID_SOCKET;
            }
        }

        RV SocketPoller::init()
        {
            m_wake_receiver = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(m_wake_receiver == INVALID_SOCKET)
            {
                return translate_poller_error(WSAGetLastError());
            }
            m_wake_sender = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(m_wake_sender == INVALID_SOCKET)
            {
                return translate_poller_error(WSAGetLastError());
            }
            sockaddr_in receiver_address = {};
            receiver_address.sin_family = AF_INET;
            receiver_address.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
            receiver_address.sin_port = 0;
            if(::bind(
                m_wake_receiver,
                (const sockaddr*)&receiver_address,
                (int)sizeof(receiver_address)) == SOCKET_ERROR)
            {
                return translate_poller_error(WSAGetLastError());
            }
            int address_size = sizeof(receiver_address);
            if(::getsockname(
                m_wake_receiver,
                (sockaddr*)&receiver_address,
                &address_size) == SOCKET_ERROR)
            {
                return translate_poller_error(WSAGetLastError());
            }
            if(::connect(
                m_wake_sender,
                (const sockaddr*)&receiver_address,
                (int)sizeof(receiver_address)) == SOCKET_ERROR)
            {
                return translate_poller_error(WSAGetLastError());
            }

            sockaddr_in sender_address = {};
            address_size = sizeof(sender_address);
            if(::getsockname(
                m_wake_sender,
                (sockaddr*)&sender_address,
                &address_size) == SOCKET_ERROR)
            {
                return translate_poller_error(WSAGetLastError());
            }
            if(::connect(
                m_wake_receiver,
                (const sockaddr*)&sender_address,
                (int)sizeof(sender_address)) == SOCKET_ERROR)
            {
                return translate_poller_error(WSAGetLastError());
            }
            RV result = configure_wake_socket(m_wake_receiver);
            if(failed(result)) return result;
            result = configure_wake_socket(m_wake_sender);
            if(failed(result)) return result;
            return ok;
        }

        R<socket_poll_token_t> SocketPoller::add(
            ISocket* socket,
            SocketEventFlag interests,
            opaque_t user_data)
        {
            if(!socket || !validate_interests(interests)) return BasicError::bad_arguments();
            object_t socket_object = socket->get_object();
            for(const SocketPollRegistration& registration : m_registrations)
            {
                if(registration.active && registration.socket.object() == socket_object)
                {
                    return BasicError::already_exists();
                }
            }
            if(get_native_socket(socket) == INVALID_SOCKET) return BasicError::bad_calling_time();

            u32 index;
            if(!m_free_slots.empty())
            {
                index = m_free_slots.back();
                m_free_slots.pop_back();
            }
            else
            {
                if(m_registrations.size() >= (usize)U32_MAX) return BasicError::out_of_resource();
                index = (u32)m_registrations.size();
                m_registrations.emplace_back();
            }

            SocketPollRegistration& registration = m_registrations[index];
            registration.socket = socket;
            registration.interests = interests;
            registration.user_data = user_data;
            registration.active = true;
            return make_token(index, registration.generation);
        }

        RV SocketPoller::modify(socket_poll_token_t token, SocketEventFlag interests)
        {
            if(!validate_interests(interests)) return BasicError::bad_arguments();
            SocketPollRegistration* registration = find_registration(m_registrations, token);
            if(!registration) return BasicError::not_found();
            if(get_native_socket(registration->socket.get()) == INVALID_SOCKET)
            {
                return BasicError::bad_calling_time();
            }
            registration->interests = interests;
            return ok;
        }

        RV SocketPoller::remove(socket_poll_token_t token)
        {
            SocketPollRegistration* registration = find_registration(m_registrations, token);
            if(!registration) return BasicError::not_found();

            u32 index = get_token_index(token);
            registration->socket.reset();
            registration->interests = SocketEventFlag::none;
            registration->user_data = nullptr;
            registration->active = false;
            ++registration->generation;
            if(!registration->generation) ++registration->generation;
            m_free_slots.push_back(index);
            return ok;
        }

        R<usize> SocketPoller::poll(Span<SocketPollEvent> events, u32 timeout_ms)
        {
            if(events.empty()) return BasicError::bad_arguments();
            m_poll_descriptors.clear();
            m_poll_tokens.clear();
            m_poll_descriptors.reserve(m_registrations.size() + 1);
            m_poll_tokens.reserve(m_registrations.size() + 1);

            WSAPOLLFD wake_descriptor = {};
            wake_descriptor.fd = m_wake_receiver;
            wake_descriptor.events = POLLRDNORM;
            m_poll_descriptors.push_back(wake_descriptor);
            m_poll_tokens.push_back(NULL_SOCKET_POLL_TOKEN);

            for(u32 index = 0; (usize)index < m_registrations.size(); ++index)
            {
                const SocketPollRegistration& registration = m_registrations[index];
                if(!registration.active || registration.interests == SocketEventFlag::none) continue;

                WSAPOLLFD descriptor = {};
                descriptor.fd = get_native_socket(registration.socket.get());
                if(test_flags(registration.interests, SocketEventFlag::readable))
                {
                    descriptor.events |= POLLRDNORM;
                }
                if(test_flags(registration.interests, SocketEventFlag::writable))
                {
                    descriptor.events |= POLLWRNORM;
                }
                m_poll_descriptors.push_back(descriptor);
                m_poll_tokens.push_back(make_token(index, registration.generation));
            }

            if(m_poll_descriptors.size() > (usize)U32_MAX) return BasicError::out_of_resource();
            int result;
            if(timeout_ms == U32_MAX)
            {
                result = ::WSAPoll(
                    m_poll_descriptors.data(),
                    (ULONG)m_poll_descriptors.size(),
                    -1);
            }
            else
            {
                u32 remaining_timeout = timeout_ms;
                for(;;)
                {
                    int timeout = remaining_timeout > (u32)I32_MAX ?
                        I32_MAX : (int)remaining_timeout;
                    result = ::WSAPoll(
                        m_poll_descriptors.data(),
                        (ULONG)m_poll_descriptors.size(),
                        timeout);
                    if(result != 0 || remaining_timeout <= (u32)I32_MAX) break;
                    remaining_timeout -= (u32)I32_MAX;
                }
            }
            if(result == SOCKET_ERROR) return translate_poller_error(WSAGetLastError());

            usize event_count = 0;
            for(usize i = 0; i < m_poll_descriptors.size(); ++i)
            {
                const WSAPOLLFD& descriptor = m_poll_descriptors[i];
                if(!descriptor.revents) continue;
                if(i == 0)
                {
                    drain_wake_socket(m_wake_receiver);
                    continue;
                }

                SocketEventFlag reported_events = SocketEventFlag::none;
                if(descriptor.revents & (POLLRDNORM | POLLRDBAND))
                {
                    reported_events |= SocketEventFlag::readable;
                }
                if(descriptor.revents & (POLLWRNORM | POLLWRBAND))
                {
                    reported_events |= SocketEventFlag::writable;
                }
                if(descriptor.revents & (POLLERR | POLLNVAL))
                {
                    reported_events |= SocketEventFlag::error;
                }
                if(descriptor.revents & POLLHUP)
                {
                    reported_events |= SocketEventFlag::hang_up;
                }
                append_poll_event(
                    this,
                    events,
                    event_count,
                    m_poll_tokens[i],
                    reported_events);
            }
            return event_count;
        }

        void SocketPoller::wake()
        {
            if(m_wake_sender == INVALID_SOCKET) return;
            c8 value = 1;
            for(;;)
            {
                int result = ::send(m_wake_sender, &value, 1, 0);
                if(result == 1) return;
                if(result == SOCKET_ERROR && WSAGetLastError() == WSAEINTR) continue;
                return;
            }
        }

        LUNA_NETWORK_API R<Ref<ISocketPoller>> new_socket_poller()
        {
            Ref<SocketPoller> poller = new_object<SocketPoller>();
            RV result = poller->init();
            if(failed(result)) return result.errcode();
            return Ref<ISocketPoller>(poller);
        }
    }
}
