/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StackAllocator.cpp
* @author JXMaster
* @date 2024/8/22
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../StackAllocator.hpp"
#include "../Fiber.hpp"
#include "../UniquePtr.hpp"
#include "../SpinLock.hpp"

namespace Luna
{
    opaque_t g_stack_allocator_fls;

    // Allocators for all threads.
    struct StackAllocatorFLSContext
    {
        byte_t* data;
        byte_t* cursor;

        static constexpr usize STACK_ALLOC_SIZE = 4_mb;

        StackAllocatorFLSContext()
        {
            data = (byte_t*)memalloc(STACK_ALLOC_SIZE);
            cursor = data;
        }
        ~StackAllocatorFLSContext()
        {
            memfree(data);
        }

        opaque_t begin_scope()
        {
            return cursor;
        }
        void* allocate(usize size, usize alignment)
        {
            alignment = alignment < MAX_ALIGN ? MAX_ALIGN : alignment;
            size = align_upper(size, alignment);
            usize addr = align_upper((usize)cursor, alignment);
            if((addr + size) > ((usize)data + STACK_ALLOC_SIZE))
            {
                // stack overflow
                return nullptr;
            }
            cursor = (byte_t*)(addr + size);
            return (byte_t*)addr;
        }
        void end_scope(opaque_t handle)
        {
            lucheck_msg((usize)handle <= (usize)cursor, "Try to close one scope that is already closed.");
            cursor = (byte_t*)handle;
        }
    };
    
    Vector<UniquePtr<StackAllocatorFLSContext>> g_stack_allocator_ctxs;
    SpinLock g_stack_allocator_ctxs_lock;

    RV stack_allocator_init()
    {
        auto r = fls_alloc([](void* ptr)
        {
            StackAllocatorFLSContext* ctx = (StackAllocatorFLSContext*)ptr;
            LockGuard guard(g_stack_allocator_ctxs_lock);
            for(auto iter = g_stack_allocator_ctxs.begin(); iter != g_stack_allocator_ctxs.end(); ++iter)
            {
                if(ctx == iter->get())
                {
                    g_stack_allocator_ctxs.erase(iter);
                    break;
                }
            }
        });
        if(failed(r)) return r.errcode();
        g_stack_allocator_fls = r.get();
        return ok;
    }
    void stack_allocator_close()
    {
        {
            LockGuard guard(g_stack_allocator_ctxs_lock);
            g_stack_allocator_ctxs.clear();
            g_stack_allocator_ctxs.shrink_to_fit();
        }
        fls_free(g_stack_allocator_fls);
    }
    StackAllocatorFLSContext* get_stack_allocator_ctx()
    {
        StackAllocatorFLSContext* ctx = (StackAllocatorFLSContext*)fls_get(g_stack_allocator_fls);
        if(!ctx)
        {
            LockGuard guard(g_stack_allocator_ctxs_lock);
            g_stack_allocator_ctxs.emplace_back(memnew<StackAllocatorFLSContext>());
            ctx = g_stack_allocator_ctxs.back().get();
            fls_set(g_stack_allocator_fls, ctx);
        }
        return ctx;
    }
    LUNA_RUNTIME_API opaque_t begin_stack_alloc_scope()
    {
        return get_stack_allocator_ctx()->begin_scope();
    }
    LUNA_RUNTIME_API void* stack_alloc(usize size, usize alignment)
    {
        if(!size) return 0;
        return get_stack_allocator_ctx()->allocate(size, alignment);
    }
    LUNA_RUNTIME_API void end_stack_alloc_scope(opaque_t handle)
    {
        get_stack_allocator_ctx()->end_scope(handle);
    }
}
