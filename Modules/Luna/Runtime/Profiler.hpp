/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Profiler.hpp
* @author JXMaster
* @date 2023/11/2
*/
#pragma once
#include "Functional.hpp"
#include "Name.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

#if (defined(LUNA_ENABLE_MEMORY_PROFILER) || (LUNA_DEBUG_LEVEL >= LUNA_DEBUG_LEVEL_PROFILE))
#define LUNA_MEMORY_PROFILER_ENABLED
#endif

namespace Luna
{
    struct IThread;
    struct ProfilerEvent
    {
        u64 timestamp;
        u64 id;
        IThread* thread;
        const void* data;
    };

    //! Allocates one temporary buffer that can be used to store event data for the next profiler event.
    LUNA_RUNTIME_API void* allocate_profiler_event_data(usize size, usize alignment, void(*dtor)(void*) = nullptr);

    template <typename _Ty>
    inline void profiler_event_data_dtor(void* data)
    {
        ((_Ty*)data)->~_Ty();
    }

    template <typename _Ty>
    inline _Ty* allocate_profiler_event_data()
    {
        return (_Ty*)allocate_profiler_event_data(sizeof(_Ty), alignof(_Ty), profiler_event_data_dtor<_Ty>);
    }

    //! Submits one profiler event.
    //! @param[in] event_id The ID of the event to set.
    LUNA_RUNTIME_API void submit_profiler_event(u64 event_id);

    using on_profiler_event_t = void(const ProfilerEvent& event);

    //! Registers one profiler callback function.
    LUNA_RUNTIME_API usize register_profiler_callback(const Function<on_profiler_event_t>& handler);
    //! Unregisters one profiler callback function.
    LUNA_RUNTIME_API void unregister_profiler_callback(usize handler_id);

    namespace ProfilerEventId
    {
        constexpr u64 MEMORY_ALLOCATE = strhash64("MEMORY_ALLOCATE");
        constexpr u64 MEMORY_REALLOCATE = strhash64("MEMORY_REALLOCATE");
        constexpr u64 MEMORY_DEALLOCATE = strhash64("MEMORY_DEALLOCATE");
        constexpr u64 SET_MEMORY_NAME = strhash64("SET_MEMORY_NAME");
        constexpr u64 SET_MEMORY_TYPE = strhash64("SET_MEMORY_TYPE");
        constexpr u64 SET_MEMORY_DOMAIN = strhash64("SET_MEMORY_DOMAIN");
    }
    namespace ProfilerEventData
    {
        struct MemoryAllocate
        {
            void* ptr;
            usize size;
        };
        struct MemoryReallocate
        {
            void* ptr;
            usize size;
            void* new_ptr;
            usize new_size;
        };
        struct MemoryDeallocate
        {
            void* ptr;
            usize size;
        };
        struct SetMemoryName
        {
            void* ptr;
            Name name;
        };
        struct SetMemoryType
        {
            void* ptr;
            Name type;
        };
        struct SetMemoryDomain
        {
            void* ptr;
            Name domain;
        };
    }

#ifdef LUNA_MEMORY_PROFILER_ENABLED
	//! @brief Emits one @ref PROFILER_EVENT_ID_MEMORY_ALLOCATE profiler event.
	//! @param[in] ptr The pointer that represents the memory.
	//! This pointer is used only for identifing the memory block, it may not be the real memory address of the memory block, but must be unique in the application domain.
	//! @param[in] size The size of the memory block, in bytes.
	//! @remark Memory allocations through `memalloc` call this internally when memory profiling is enabled, thus the user does not
	//! need to call this again.
	LUNA_RUNTIME_API void memory_profiler_allocate(void* ptr, usize size);

	//! @brief Emits one @ref PROFILER_EVENT_ID_MEMORY_REALLOCATE profiler event.
	//! @param[in] ptr The old memory pointer.
	//! @param[in] new_ptr The new memory pointer to register.
	//! @param[in] new_size The new memory size.
	//! @remark Memory reallocations through `memrealloc` call this internally when memory profiling is enabled, thus the user does not
	//! need to call this again.
	LUNA_RUNTIME_API void memory_profiler_reallocate(void* ptr, usize size, void* new_ptr, usize new_size);

	//! @brief Emits one @ref PROFILER_EVENT_ID_MEMORY_DEALLOCATE profiler event.
	//! @param[in] ptr The registered memory pointer.
	//! @remark Memory deallocations through `memfree` call this internally when memory profiling is enabled, thus the user does not
	//! need to call this again.
	LUNA_RUNTIME_API void memory_profiler_deallocate(void* ptr, usize size);

    LUNA_RUNTIME_API void memory_profiler_set_memory_name_static(void* ptr, const c8* name);

	//! @brief Sets a debug name for the memory block, for example, the name of the resource file this memory block is allocated for. 
	//! This function emits one @ref PROFILER_EVENT_ID_SET_MEMORY_NAME profiler event.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] name The debug name for the memory block.
	LUNA_RUNTIME_API void memory_profiler_set_memory_name(void* ptr, const Name& name);

    LUNA_RUNTIME_API void memory_profiler_set_memory_type_static(void* ptr, const Name& type);

	//! @brief Sets the type of the object this memory block.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] domain The type name for the memory block.
	LUNA_RUNTIME_API void memory_profiler_set_memory_type(void* ptr, const Name& type);

	//! @brief Sets the memory domain. The memory domain is usually the heap or pool that allocates this memory block. 
	//! This function emits one @ref PROFILER_EVENT_ID_SET_MEMORY_DOMAIN profiler event.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] domain The domain name for the memory block.
	LUNA_RUNTIME_API void memory_profiler_set_memory_domain(void* ptr, const Name& domain);
#endif
}