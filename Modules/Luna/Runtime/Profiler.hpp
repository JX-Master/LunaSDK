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

    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeProfiler Debugging
    //! @}

    //! @addtogroup RuntimeProfiler
    //! @{
    
    //! A emitted profiler event.
    struct ProfilerEvent
    {
        //! The time that this 
        u64 timestamp;
        //! The event ID.
        u64 id;
        //! The thread that submits this event.
        IThread* thread;
        //! The user-defined event data.
        const void* data;
    };

    //! Allocates one temporary buffer that can be used to store event data for the next profiler event.
    //! @param[in] size The size to allocate in bytes.
    //! @param[in] alignment The alignment requirement of the allocated memory in bytes. This can be `0`, indicating
    //! that no alignment requirement is specified.
    //! @param[in] dtor The function that will be called when the memory is going to be freed. This can be `nullptr`,
    //! indicating that no operation is performed before freeing memory.
    //! @return Returns the allocated memory.
    //! @par Valid Usage
    //! * If `alignment` is not `0`, `alignment` must be power of 2 (like 4, 8, 16, etc.).
    LUNA_RUNTIME_API void* allocate_profiler_event_data(usize size, usize alignment, void(*dtor)(void*) = nullptr);

    //! One helper function that calls the destructor of the specified type on the pointer.
    //! @param[in] data The pointer to the object to be destructed.
    template <typename _Ty>
    inline void profiler_event_data_dtor(void* data)
    {
        ((_Ty*)data)->~_Ty();
    }

    //! Allocates one temporary object that can be used to store event data for the next profiler event.
    //! @details This function uses the specified type to provide size, alignment and destructor for the memory.
    //! @return Returns the allocated object. The returned object is not initialized, the user should call
    //! `new (ptr) _Ty(...)` to initialize the object manually.
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
    //! @param[in] handler The callback function object to register.
    //! @return Returns one handle that can be used to unregister the callback function.
    LUNA_RUNTIME_API usize register_profiler_callback(const Function<on_profiler_event_t>& handler);
    //! Unregisters one profiler callback function.
    //! @param[in] handler_id The handler that returned by @ref register_profiler_callback for the callback function 
    //! to unregister. 
    LUNA_RUNTIME_API void unregister_profiler_callback(usize handler_id);

    namespace ProfilerEventId
    {
        //! The memory allocation event ID.
        constexpr u64 MEMORY_ALLOCATE = strhash64("MEMORY_ALLOCATE");
        //! The memory deallocation event ID.
        constexpr u64 MEMORY_DEALLOCATE = strhash64("MEMORY_DEALLOCATE");
        //! The set memory name event ID.
        constexpr u64 SET_MEMORY_NAME = strhash64("SET_MEMORY_NAME");
        //! The set memory type event ID.
        constexpr u64 SET_MEMORY_TYPE = strhash64("SET_MEMORY_TYPE");
        //! The set memory domain event ID.
        constexpr u64 SET_MEMORY_DOMAIN = strhash64("SET_MEMORY_DOMAIN");
    }
    namespace ProfilerEventData
    {
        //! The memory allocation event data.
        struct MemoryAllocate
        {
            //! The memory pointer.
            void* ptr;
            //! The size of the memory.
            usize size;
        };
        //!  @brief The memory deallocation event data.
        struct MemoryDeallocate
        {
            //! The memory pointer.
            void* ptr;
        };
        //! The set memory name event data.
        struct SetMemoryName
        {
            //! The memory pointer.
            void* ptr;
            //! The name of the memory to set.
            //! The string buffer is allocated along with this structure, and can be
            //! referred directly by referring this property. The string buffer is valid 
            //! so long as this structure is valid.
            const c8 name[1];
        };
        //! The set memory type event data.
        struct SetMemoryType
        {
            //! The memory pointer.
            void* ptr;
            //! The type of the memory to set.
            //! The name of the memory to set.
            //! The string buffer is allocated along with this structure, and can be
            //! referred directly by referring this property. The string buffer is valid 
            //! so long as this structure is valid.
            const c8 type[1];
        };
        //! The set memory domain event data.
        struct SetMemoryDomain
        {
            //! The memory pointer.
            void* ptr;
            //! The domain of the memory to set.
            //! The name of the memory to set.
            //! The string buffer is allocated along with this structure, and can be
            //! referred directly by referring this property. The string buffer is valid 
            //! so long as this structure is valid.
            const c8 domain[1];
        };
    }

#ifdef LUNA_MEMORY_PROFILER_ENABLED
	//! Emits one @ref PROFILER_EVENT_ID_MEMORY_ALLOCATE profiler event.
	//! @param[in] ptr The pointer that represents the memory.
	//! This pointer is used only for identifing the memory block, it may not be the real memory address of the memory block, but must be unique in the application domain.
	//! @param[in] size The size of the memory block, in bytes.
	//! @remark Memory allocations through `memalloc` call this internally when memory profiling is enabled, thus the user does not
	//! need to call this again.
	LUNA_RUNTIME_API void memory_profiler_allocate(void* ptr, usize size);

	//! Emits one @ref PROFILER_EVENT_ID_MEMORY_DEALLOCATE profiler event.
	//! @param[in] ptr The registered memory pointer.
	//! @remark Memory deallocations through `memfree` call this internally when memory profiling is enabled, thus the user does not
	//! need to call this again.
	LUNA_RUNTIME_API void memory_profiler_deallocate(void* ptr);

	//! Sets a debug name for the memory block, for example, the name of the resource file this memory block is allocated for. 
	//! This function emits one @ref PROFILER_EVENT_ID_SET_MEMORY_NAME profiler event.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] name The debug name for the memory block.
    //! @param[in] str_size The size of the name, not including the null terminator. If this is `USIZE_MAX`, the size is determined by the system
    //! using @ref strlen.
	LUNA_RUNTIME_API void memory_profiler_set_memory_name(void* ptr, const c8* name, usize str_size = USIZE_MAX);

	//! Sets the type of the object this memory block.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] domain The type name for the memory block.
    //! @param[in] str_size The size of the name, not including the null terminator. If this is `USIZE_MAX`, the size is determined by the system
    //! using @ref strlen.
	LUNA_RUNTIME_API void memory_profiler_set_memory_type(void* ptr, const c8* type, usize str_size = USIZE_MAX);

	//! Sets the memory domain. 
    //! @details The memory domain is usually the heap or pool that allocates this memory block. 
	//! This function emits one @ref PROFILER_EVENT_ID_SET_MEMORY_DOMAIN profiler event.
	//! @param[in] ptr The memory block pointer.
	//! @param[in] domain The domain name for the memory block.
    //! @param[in] str_size The size of the name, not including the null terminator. If this is `USIZE_MAX`, the size is determined by the system
    //! using @ref strlen.
	LUNA_RUNTIME_API void memory_profiler_set_memory_domain(void* ptr, const c8* domain, usize str_size = USIZE_MAX);
#endif

    //! @}
}