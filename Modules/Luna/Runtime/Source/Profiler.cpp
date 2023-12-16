/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Profiler.cpp
* @author JXMaster
* @date 2023/11/2
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Profiler.hpp"
#include "../Event.hpp"
#include "../ReadWriteLock.hpp"
#include "../Time.hpp"
#include "../Thread.hpp"
#include "OS.hpp"
#include "../Vector.hpp"

namespace Luna
{
    Event<on_profiler_event_t, OSAllocator> g_profiler_callbacks;
    opaque_t g_profiler_callbacks_lock;
    opaque_t g_profiler_thread_context_tls;
    bool g_profiler_inited = false;

    struct ProfilerEventEntry
    {
        u64 id;
        u64 timestamp;
        void* data = nullptr;
        usize data_size = 0;
        void(*dtor)(void*) = nullptr;
    };
    struct ProfilerDataBuffer
    {
        void* data;
        usize size;
        usize capacity;
        ProfilerDataBuffer(usize capacity) :
            capacity(capacity),
            size(0)
        {
            data = OS::memalloc(capacity);
        }
        ~ProfilerDataBuffer()
        {
            OS::memfree(data);
        }
    };
    struct ProfilerThreadContext
    {
        Vector<ProfilerEventEntry, OSAllocator> m_events;
        Vector<ProfilerDataBuffer, OSAllocator> m_event_data_buffers;
        ProfilerEventEntry m_next_entry;

        bool m_thread_locked_callbacks = false;

        void* allocate_data_buffer(usize size, usize alignment, void(*dtor)(void*));
        void merge_data_buffers();
        void dispatch_events();
    };
    void profiler_thread_context_dtor(void* data)
    {
        if(data) OS::memdelete((ProfilerThreadContext*)data);
    }
    void* ProfilerThreadContext::allocate_data_buffer(usize size, usize alignment, void(*dtor)(void*))
    {
        m_next_entry.data_size = size;
        m_next_entry.dtor = dtor;
        m_next_entry.data = nullptr;
        if(!m_event_data_buffers.empty())
        {
            // try allocate from existing buffer.
            auto& buffer = m_event_data_buffers.back();
            usize addr = align_upper((usize)buffer.data + buffer.size, alignment);
            if(addr + size <= (usize)buffer.data + buffer.capacity)
            {
                m_next_entry.data = (void*)addr;
                buffer.size = addr + size - (usize)buffer.data;
            }
        }
        if(!m_next_entry.data)
        {
            // allocate a new block.
            usize capacity = alignment > MAX_ALIGN ? size + alignment : size;
            m_event_data_buffers.emplace_back(capacity);
            auto& buffer = m_event_data_buffers.back();
            usize addr = align_upper((usize)buffer.data, alignment);
            buffer.size = addr + size - (usize)buffer.data;
            luassert(buffer.size <= buffer.capacity);
            m_next_entry.data = (void*)addr;
        }
        return m_next_entry.data;
    }
    void ProfilerThreadContext::merge_data_buffers()
    {
        if(m_event_data_buffers.size() > 1)
        {
            usize sz = 0;
            for(auto& buffer : m_event_data_buffers)
            {
                sz += buffer.capacity;
            }
            sz = align_upper(sz, 16);
            m_event_data_buffers.clear();
            m_event_data_buffers.emplace_back(sz);
        }
    }
    void ProfilerThreadContext::dispatch_events()
    {
        for(usize i = 0; i < m_events.size(); ++i)
        {
            ProfilerEvent dst;
            ProfilerEventEntry& src = m_events[i];
            dst.data = src.data;
            dst.id = src.id;
            dst.timestamp = src.timestamp;
            dst.thread = get_current_thread();
            OS::acquire_read_lock(g_profiler_callbacks_lock);
            g_profiler_callbacks(dst);
            OS::release_read_lock(g_profiler_callbacks_lock);
        }
        for(auto& e : m_events)
        {
            if(e.dtor) e.dtor(e.data);
        }
        m_events.clear();
        merge_data_buffers();
    }
    ProfilerThreadContext* get_profiler_thread_context()
    {
        ProfilerThreadContext* ctx = (ProfilerThreadContext*)OS::tls_get(g_profiler_thread_context_tls);
        if(!ctx)
        {
            ctx = OS::memnew<ProfilerThreadContext>();
            OS::tls_set(g_profiler_thread_context_tls, ctx);
        }
        return ctx;
    }
    void profiler_init()
    {
        g_profiler_callbacks_lock = OS::new_read_write_lock();
        g_profiler_thread_context_tls = OS::tls_alloc(profiler_thread_context_dtor);
        g_profiler_inited = true;
    }
    void profiler_close()
    {
        g_profiler_inited = false;
        g_profiler_callbacks.clear();
        OS::tls_free(g_profiler_thread_context_tls);
        OS::delete_read_write_lock(g_profiler_callbacks_lock);
    }
    LUNA_RUNTIME_API void* allocate_profiler_event_data(usize size, usize alignment, void(*dtor)(void*))
    {
        auto ctx = get_profiler_thread_context();
        return ctx->allocate_data_buffer(size, alignment, dtor);
    }
    LUNA_RUNTIME_API void submit_profiler_event(u64 event_id)
    {
        if(!g_profiler_inited) return;
        auto ctx = get_profiler_thread_context();
        ctx->m_next_entry.timestamp = OS::get_ticks();
        ctx->m_next_entry.id = event_id;
        ctx->m_events.push_back(ctx->m_next_entry);
        ctx->m_next_entry = ProfilerEventEntry();
        // This profiler event is issued in another profiler event or register/unregister profiler callback.
        if(ctx->m_events.size() == 1 && !ctx->m_thread_locked_callbacks)
        {
            ctx->dispatch_events();
        }
    }
    LUNA_RUNTIME_API usize register_profiler_callback(const Function<on_profiler_event_t>& handler)
    {
        auto ctx = get_profiler_thread_context();
        auto move_handler = handler;
        // Any events that occur in write scope will trigger dead lock, so we use m_thread_locked_callbacks to prevent dispatching events in write scope.
        OS::acquire_write_lock(g_profiler_callbacks_lock);
        ctx->m_thread_locked_callbacks = true;
        usize r = g_profiler_callbacks.add_handler(move(move_handler));
        ctx->m_thread_locked_callbacks = false;
        OS::release_write_lock(g_profiler_callbacks_lock);
        // Dispatch any events that occur during registration.
        if(!ctx->m_events.empty())
        {
            ctx->dispatch_events();
        }
        return r;
    }
    LUNA_RUNTIME_API void unregister_profiler_callback(usize handler_id)
    {
        auto ctx = get_profiler_thread_context();
        // Any events that occur in write scope will trigger dead lock, so we use m_thread_locked_callbacks to prevent dispatching events in write scope.
        OS::acquire_write_lock(g_profiler_callbacks_lock);
        ctx->m_thread_locked_callbacks = true;
        g_profiler_callbacks.remove_handler(handler_id);
        ctx->m_thread_locked_callbacks = false;
        OS::release_write_lock(g_profiler_callbacks_lock);
        // Dispatch any events that occur during unregistration.
        if(!ctx->m_events.empty())
        {
            ctx->dispatch_events();
        }
    }
#ifdef LUNA_MEMORY_PROFILER_ENABLED
	LUNA_RUNTIME_API void memory_profiler_allocate(void* ptr, usize size)
	{
		ProfilerEventData::MemoryAllocate* data = (ProfilerEventData::MemoryAllocate*)allocate_profiler_event_data(
                sizeof(ProfilerEventData::MemoryAllocate), 
                alignof(ProfilerEventData::MemoryAllocate));
		data->ptr = ptr;
		data->size = size;
		submit_profiler_event(ProfilerEventId::MEMORY_ALLOCATE);
	}
	LUNA_RUNTIME_API void memory_profiler_deallocate(void* ptr)
	{
		ProfilerEventData::MemoryDeallocate* data = (ProfilerEventData::MemoryDeallocate*)allocate_profiler_event_data(
            sizeof(ProfilerEventData::MemoryDeallocate),
            alignof(ProfilerEventData::MemoryDeallocate)
        );
		data->ptr = ptr;
		submit_profiler_event(ProfilerEventId::MEMORY_DEALLOCATE);
	}
	LUNA_RUNTIME_API void memory_profiler_set_memory_name(void* ptr, const c8* name, usize str_size)
	{
        if(str_size == USIZE_MAX) str_size = strlen(name);
        usize sz = sizeof(ProfilerEventData::SetMemoryName) + str_size; // One extra character is allocated in structure.
        ProfilerEventData::SetMemoryName* data = (ProfilerEventData::SetMemoryName*)allocate_profiler_event_data(sz, alignof(ProfilerEventData::SetMemoryName));
        data->ptr = ptr;
        c8* dst = const_cast<c8*>(data->name);
        memcpy(dst, name, str_size);
        dst[str_size] = 0;
		submit_profiler_event(ProfilerEventId::SET_MEMORY_NAME);
	}
	LUNA_RUNTIME_API void memory_profiler_set_memory_type(void* ptr, const c8* type, usize str_size)
	{
        if(str_size == USIZE_MAX) str_size = strlen(type);
        usize sz = sizeof(ProfilerEventData::SetMemoryType) + str_size; // One extra character is allocated in structure.
		ProfilerEventData::SetMemoryType* data = (ProfilerEventData::SetMemoryType*)allocate_profiler_event_data(sz, alignof(ProfilerEventData::SetMemoryType));
		data->ptr = ptr;
        c8* dst = const_cast<c8*>(data->type);
		memcpy(dst, type, str_size);
        dst[str_size] = 0;
		submit_profiler_event(ProfilerEventId::SET_MEMORY_TYPE);
	}
	LUNA_RUNTIME_API void memory_profiler_set_memory_domain(void* ptr, const c8* domain, usize str_size)
	{
        if(str_size == USIZE_MAX) str_size = strlen(domain);
        usize sz = sizeof(ProfilerEventData::SetMemoryDomain) + str_size; // One extra character is allocated in structure.
        ProfilerEventData::SetMemoryDomain* data = (ProfilerEventData::SetMemoryDomain*)allocate_profiler_event_data(sz, alignof(ProfilerEventData::SetMemoryDomain));
		data->ptr = ptr;
        c8* dst = const_cast<c8*>(data->domain);
		memcpy(dst, domain, str_size);
        dst[str_size] = 0;
		submit_profiler_event(ProfilerEventId::SET_MEMORY_DOMAIN);
	}
#endif
}