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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

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
                case EACCES: return BasicError::access_denied();
                case EAGAIN: return BasicError::not_ready();
                case EBADF: return BasicError::bad_calling_time();
                case EEXIST: return BasicError::already_exists();
                case EINTR: return BasicError::interrupted();
                case EINVAL: return BasicError::bad_arguments();
                case EMFILE: return BasicError::out_of_resource();
                case ENFILE: return BasicError::out_of_resource();
                case ENOENT: return BasicError::not_found();
                case ENOMEM: return BasicError::out_of_memory();
                case ENOSPC: return BasicError::out_of_resource();
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

            int get_native_socket(ISocket* socket)
            {
                return (int)(isize)(usize)socket->get_native_handle();
            }

            RV configure_pipe_end(int fd)
            {
                int flags = ::fcntl(fd, F_GETFL, 0);
                if(flags == -1) return translate_poller_error(errno);
                if(::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
                {
                    return translate_poller_error(errno);
                }
                flags = ::fcntl(fd, F_GETFD, 0);
                if(flags == -1) return translate_poller_error(errno);
                if(::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
                {
                    return translate_poller_error(errno);
                }
                return ok;
            }

            void drain_wake_pipe(int fd)
            {
                u8 buffer[64];
                for(;;)
                {
                    isize read_size = ::read(fd, buffer, sizeof(buffer));
                    if(read_size > 0) continue;
                    if(read_size == -1 && errno == EINTR) continue;
                    break;
                }
            }

#if defined(LUNA_PLATFORM_LINUX)
            u32 encode_native_events(SocketEventFlag interests)
            {
                u32 events = 0;
#ifdef EPOLLRDHUP
                events |= EPOLLRDHUP;
#endif
                if(test_flags(interests, SocketEventFlag::readable)) events |= EPOLLIN | EPOLLPRI;
                if(test_flags(interests, SocketEventFlag::writable)) events |= EPOLLOUT;
                return events;
            }

            int wait_for_epoll_events(
                int poller,
                epoll_event* events,
                int event_capacity,
                u32 timeout_ms)
            {
                if(timeout_ms == U32_MAX)
                {
                    return ::epoll_wait(poller, events, event_capacity, -1);
                }
                u32 remaining_timeout = timeout_ms;
                for(;;)
                {
                    int timeout = remaining_timeout > (u32)I32_MAX ?
                        I32_MAX : (int)remaining_timeout;
                    int result = ::epoll_wait(poller, events, event_capacity, timeout);
                    if(result != 0 || remaining_timeout <= (u32)I32_MAX) return result;
                    remaining_timeout -= (u32)I32_MAX;
                }
            }

            RV update_native_registration(
                SocketPoller* poller,
                int fd,
                socket_poll_token_t token,
                SocketEventFlag old_interests,
                SocketEventFlag new_interests)
            {
                bool was_active = old_interests != SocketEventFlag::none;
                bool is_active = new_interests != SocketEventFlag::none;
                if(!was_active && !is_active) return ok;

                if(was_active && !is_active)
                {
                    if(::epoll_ctl(poller->m_poller, EPOLL_CTL_DEL, fd, nullptr) == -1)
                    {
                        int error = errno;
                        if(error != ENOENT && error != EBADF) return translate_poller_error(error);
                    }
                    return ok;
                }

                epoll_event event = {};
                event.events = encode_native_events(new_interests);
                event.data.u64 = token.value;
                int operation = was_active ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
                if(::epoll_ctl(poller->m_poller, operation, fd, &event) == -1)
                {
                    return translate_poller_error(errno);
                }
                return ok;
            }
#elif defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            RV apply_kqueue_filter(
                SocketPoller* poller,
                int fd,
                i16 filter,
                bool add,
                socket_poll_token_t token)
            {
                struct kevent change = {};
                EV_SET(
                    &change,
                    (uintptr_t)fd,
                    filter,
                    add ? (EV_ADD | EV_ENABLE) : EV_DELETE,
                    0,
                    0,
                    (void*)(usize)token.value);
                if(::kevent(poller->m_poller, &change, 1, nullptr, 0, nullptr) == -1)
                {
                    int error = errno;
                    if(!add && (error == ENOENT || error == EBADF)) return ok;
                    return translate_poller_error(error);
                }
                return ok;
            }

            RV update_native_registration(
                SocketPoller* poller,
                int fd,
                socket_poll_token_t token,
                SocketEventFlag old_interests,
                SocketEventFlag new_interests)
            {
                bool old_readable = test_flags(old_interests, SocketEventFlag::readable);
                bool new_readable = test_flags(new_interests, SocketEventFlag::readable);
                bool old_writable = test_flags(old_interests, SocketEventFlag::writable);
                bool new_writable = test_flags(new_interests, SocketEventFlag::writable);
                bool changed_readable = old_readable != new_readable;

                if(changed_readable)
                {
                    RV result = apply_kqueue_filter(poller, fd, EVFILT_READ, new_readable, token);
                    if(failed(result)) return result;
                }
                if(old_writable != new_writable)
                {
                    RV result = apply_kqueue_filter(poller, fd, EVFILT_WRITE, new_writable, token);
                    if(failed(result))
                    {
                        if(changed_readable)
                        {
                            RV rollback_result = apply_kqueue_filter(
                                poller,
                                fd,
                                EVFILT_READ,
                                old_readable,
                                token);
                            (void)rollback_result;
                        }
                        return result;
                    }
                }
                return ok;
            }
#else
#error Unsupported POSIX platform for Network socket poller.
#endif

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

                for(usize i = 0; i < event_count; ++i)
                {
                    if(events[i].token == token)
                    {
                        events[i].events |= reported_events;
                        return;
                    }
                }
                if(event_count == events.size()) return;
                events[event_count].token = token;
                events[event_count].events = reported_events;
                events[event_count].user_data = registration->user_data;
                ++event_count;
            }
        }

        SocketPoller::~SocketPoller()
        {
            if(m_poller != -1)
            {
                ::close(m_poller);
                m_poller = -1;
            }
            if(m_wake_read != -1)
            {
                ::close(m_wake_read);
                m_wake_read = -1;
            }
            if(m_wake_write != -1)
            {
                ::close(m_wake_write);
                m_wake_write = -1;
            }
        }

        RV SocketPoller::init()
        {
            int pipe_fds[2];
            if(::pipe(pipe_fds) == -1) return translate_poller_error(errno);
            m_wake_read = pipe_fds[0];
            m_wake_write = pipe_fds[1];
            RV result = configure_pipe_end(m_wake_read);
            if(failed(result)) return result;
            result = configure_pipe_end(m_wake_write);
            if(failed(result)) return result;

#if defined(LUNA_PLATFORM_LINUX)
            m_poller = ::epoll_create1(EPOLL_CLOEXEC);
            if(m_poller == -1) return translate_poller_error(errno);
            epoll_event wake_event = {};
            wake_event.events = EPOLLIN;
            wake_event.data.u64 = 0;
            if(::epoll_ctl(m_poller, EPOLL_CTL_ADD, m_wake_read, &wake_event) == -1)
            {
                return translate_poller_error(errno);
            }
#elif defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            m_poller = ::kqueue();
            if(m_poller == -1) return translate_poller_error(errno);
            int flags = ::fcntl(m_poller, F_GETFD, 0);
            if(flags == -1) return translate_poller_error(errno);
            if(::fcntl(m_poller, F_SETFD, flags | FD_CLOEXEC) == -1)
            {
                return translate_poller_error(errno);
            }
            struct kevent wake_event = {};
            EV_SET(&wake_event, (uintptr_t)m_wake_read, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
            if(::kevent(m_poller, &wake_event, 1, nullptr, 0, nullptr) == -1)
            {
                return translate_poller_error(errno);
            }
#endif
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

            int fd = get_native_socket(socket);
            if(fd < 0) return BasicError::bad_calling_time();

            u32 index;
            bool reused_slot = !m_free_slots.empty();
            if(reused_slot)
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
            socket_poll_token_t token = make_token(index, registration.generation);
            RV result = update_native_registration(
                this,
                fd,
                token,
                SocketEventFlag::none,
                interests);
            if(failed(result))
            {
                if(reused_slot) m_free_slots.push_back(index);
                else m_registrations.pop_back();
                return result.errcode();
            }

            registration.socket = socket;
            registration.interests = interests;
            registration.user_data = user_data;
            registration.active = true;
            return token;
        }

        RV SocketPoller::modify(socket_poll_token_t token, SocketEventFlag interests)
        {
            if(!validate_interests(interests)) return BasicError::bad_arguments();
            SocketPollRegistration* registration = find_registration(m_registrations, token);
            if(!registration) return BasicError::not_found();
            if(registration->interests == interests) return ok;

            int fd = get_native_socket(registration->socket.get());
            if(fd < 0) return BasicError::bad_calling_time();
            RV result = update_native_registration(
                this,
                fd,
                token,
                registration->interests,
                interests);
            if(failed(result)) return result;
            registration->interests = interests;
            return ok;
        }

        RV SocketPoller::remove(socket_poll_token_t token)
        {
            SocketPollRegistration* registration = find_registration(m_registrations, token);
            if(!registration) return BasicError::not_found();

            int fd = get_native_socket(registration->socket.get());
            if(fd >= 0)
            {
                RV result = update_native_registration(
                    this,
                    fd,
                    token,
                    registration->interests,
                    SocketEventFlag::none);
                if(failed(result)) return result;
            }

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
            usize native_capacity = events.size() < (usize)I32_MAX ? events.size() + 1 : (usize)I32_MAX;
            if(m_native_events.size() < native_capacity) m_native_events.resize(native_capacity);
            usize event_count = 0;

#if defined(LUNA_PLATFORM_LINUX)
            int result = wait_for_epoll_events(
                m_poller,
                m_native_events.data(),
                (int)native_capacity,
                timeout_ms);
            if(result == -1) return translate_poller_error(errno);
            for(int i = 0; i < result; ++i)
            {
                const epoll_event& native_event = m_native_events[i];
                if(native_event.data.u64 == 0)
                {
                    drain_wake_pipe(m_wake_read);
                    continue;
                }
                SocketEventFlag reported_events = SocketEventFlag::none;
                if(native_event.events & (EPOLLIN | EPOLLPRI)) reported_events |= SocketEventFlag::readable;
                if(native_event.events & EPOLLOUT) reported_events |= SocketEventFlag::writable;
                if(native_event.events & EPOLLERR) reported_events |= SocketEventFlag::error;
                u32 hang_up_events = EPOLLHUP;
#ifdef EPOLLRDHUP
                hang_up_events |= EPOLLRDHUP;
#endif
                if(native_event.events & hang_up_events) reported_events |= SocketEventFlag::hang_up;
                append_poll_event(
                    this,
                    events,
                    event_count,
                    socket_poll_token_t(native_event.data.u64),
                    reported_events);
            }
#elif defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            timespec timeout = {};
            timespec* timeout_ptr = nullptr;
            if(timeout_ms != U32_MAX)
            {
                timeout.tv_sec = timeout_ms / 1000;
                timeout.tv_nsec = (long)(timeout_ms % 1000) * 1000000L;
                timeout_ptr = &timeout;
            }
            int result = ::kevent(
                m_poller,
                nullptr,
                0,
                m_native_events.data(),
                (int)native_capacity,
                timeout_ptr);
            if(result == -1) return translate_poller_error(errno);
            for(int i = 0; i < result; ++i)
            {
                const struct kevent& native_event = m_native_events[i];
                if(native_event.udata == nullptr && native_event.ident == (uintptr_t)m_wake_read)
                {
                    drain_wake_pipe(m_wake_read);
                    continue;
                }
                SocketEventFlag reported_events = SocketEventFlag::none;
                if(native_event.filter == EVFILT_READ) reported_events |= SocketEventFlag::readable;
                if(native_event.filter == EVFILT_WRITE) reported_events |= SocketEventFlag::writable;
                if(native_event.flags & EV_ERROR) reported_events |= SocketEventFlag::error;
                if(native_event.flags & EV_EOF) reported_events |= SocketEventFlag::hang_up;
                append_poll_event(
                    this,
                    events,
                    event_count,
                    socket_poll_token_t((u64)(usize)native_event.udata),
                    reported_events);
            }
#endif
            return event_count;
        }

        void SocketPoller::wake()
        {
            if(m_wake_write == -1) return;
            u8 value = 1;
            for(;;)
            {
                isize result = ::write(m_wake_write, &value, 1);
                if(result == 1) return;
                if(result == -1 && errno == EINTR) continue;
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
