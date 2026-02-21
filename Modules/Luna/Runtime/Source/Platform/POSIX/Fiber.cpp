/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* POSIX fiber implementation using assembly context switch (marl-style).
*/
#include "../Fiber.hpp"
#include "../Thread.hpp"
#include "Errno.hpp"
#include "../Mutex.hpp"

namespace Luna
{
    namespace Platform
    {
        opaque_t g_fiber_tls; // Fiber*
        pthread_key_t g_fiber_fls_tls; // FLSContext*

        struct FLSRegistry
        {
            struct Slot
            {
                void (*m_dtor)(void*) = nullptr;
                bool m_allocated = false;
            };

            Mutex m_mutex;
            Vector<Slot, Platform::Allocator> m_slots;
            Vector<FLSContext*, Platform::Allocator> m_contexts;

            void init()
            {
                new_mutex(m_mutex);
            }

            void close()
            {
                lock_mutex(m_mutex);
                for(FLSContext* ctx : m_contexts)
                {
                    free_context_internal(ctx);
                }
                m_contexts.clear();
                m_contexts.shrink_to_fit();
                m_slots.clear();
                m_slots.shrink_to_fit();
                unlock_mutex(m_mutex);
                delete_mutex(m_mutex);
            }

            usize allocate_index(void (*dtor)(void*))
            {
                MutexGuard guard(m_mutex);
                for(usize i = 0; i < m_slots.size(); ++i)
                {
                    auto& item = m_slots[i];
                    if(!item.m_allocated)
                    {
                        item.m_allocated = true;
                        item.m_dtor = dtor;
                        return i;
                    }
                }
                usize i = m_slots.size();
                m_slots.resize(m_slots.size() + 16);
                auto& item = m_slots[i];
                item.m_allocated = true;
                item.m_dtor = dtor;
                return i;
            }

            void free_index(usize index)
            {
                MutexGuard guard(m_mutex);
                if(index >= m_slots.size()) return;
                auto& item = m_slots[index];
                if(!item.m_allocated) return;
                // Delete all pointers.
                for(FLSContext* ctx : m_contexts)
                {
                    void* data = ctx->set(index, nullptr);
                    if(data && item.m_dtor)
                    {
                        item.m_dtor(data);
                    }
                }
                item.m_allocated = false;
            }

            bool is_index_valid(usize index)
            {
                MutexGuard guard(m_mutex);
                if(index >= m_slots.size()) return false;
                return m_slots[index].m_allocated;
            }

            FLSContext* allocate_context()
            {
                FLSContext* ctx = memnew<FLSContext>();
                MutexGuard guard(m_mutex);
                m_contexts.push_back(ctx);
                return ctx;
            }

            void free_context_internal(FLSContext* ctx)
            {
                for(usize i = 0; i < m_slots.size(); ++i)
                {
                    auto& item = m_slots[i];
                    if(item.m_allocated)
                    {
                        void* data = ctx->set(i, nullptr);
                        if(data && item.m_dtor)
                        {
                            item.m_dtor(data);
                        }
                    }
                }
                memdelete(ctx);
            }

            void free_context(FLSContext* ctx)
            {
                MutexGuard guard(m_mutex);
                for(auto iter = m_contexts.begin(); iter != m_contexts.end(); ++iter)
                {
                    if(*iter == ctx)
                    {
                        free_context_internal(ctx); // prevent double free.
                        m_contexts.swap_erase(iter);
                        break;
                    }
                }
            }
        };

        FLSRegistry g_fls_registry;

        void fiber_fls_free(void* data)
        {
            FLSContext* fls = (FLSContext*)data;
            g_fls_registry.free_context(fls);
        }

        Result fiber_init()
        {
            int r = pthread_key_create(&g_fiber_fls_tls, fiber_fls_free);
            if(r)
            {
                return encode_errno(r);
            }
            auto res = tls_alloc(g_fiber_tls);
            if(res != Result::success)
            {
                pthread_key_delete(g_fiber_fls_tls);
                return res;
            }
            g_fls_registry.init();
            return Result::success;
        }

        void fiber_close()
        {
            g_fls_registry.close();
            tls_free(g_fiber_tls);
            pthread_key_delete(g_fiber_fls_tls);
        }

        Result new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, Fiber& out_fiber)
        {
            out_fiber.entry_func = entry_func;
            out_fiber.param = param;
            out_fiber.stack_size = stack_size;
            out_fiber.stack = memalloc(out_fiber.stack_size, 16);
            if(!out_fiber.stack) return Result::out_of_memory;
            out_fiber.fls = nullptr;
            memzero(&out_fiber.context);
            luna_fiber_set_target(
                &out_fiber.context,
                out_fiber.stack,
                (u32)out_fiber.stack_size,
                entry_func,
                param);
            return Result::success;
        }

        void delete_fiber(Fiber& fiber)
        {
            if(fiber.stack)
            {
                memfree(fiber.stack, 16);
                fiber.stack = nullptr;
                fiber.stack_size = 0;
            }
            if(fiber.fls)
            {
                g_fls_registry.free_context(fiber.fls);
                fiber.fls = nullptr;
            }
        }

        Result convert_thread_to_fiber(Fiber& out_fiber)
        {
            memzero(&out_fiber.context);
            out_fiber.entry_func = nullptr;
            out_fiber.param = nullptr;
            out_fiber.stack_size = 0;
            out_fiber.stack = nullptr;
            out_fiber.fls = nullptr;
            tls_set(g_fiber_tls, &out_fiber);
            return Result::success;
        }

        Result convert_fiber_to_thread()
        {
            tls_set(g_fiber_tls, nullptr);
            return Result::success;
        }

        void switch_to_fiber(Fiber& fiber)
        {
            Fiber* cur_fiber = (Fiber*)tls_get(g_fiber_tls);
            tls_set(g_fiber_tls, &fiber);
            // Switch FLS.
            cur_fiber->fls = (FLSContext*)pthread_getspecific(g_fiber_fls_tls);
            pthread_setspecific(g_fiber_fls_tls, fiber.fls);
            fiber.fls = nullptr;
            // Swap context.
            luna_fiber_swap(
                &cur_fiber->context,
                &fiber.context);
        }

        Result fls_alloc(void(*destructor)(void* ptr), opaque_t& out_handle)
        {
            const usize index = g_fls_registry.allocate_index(destructor);
            out_handle = (opaque_t)index;
            return Result::success;
        }
        void fls_free(opaque_t handle)
        {
            const usize index = (usize)handle;
            g_fls_registry.free_index(index);
        }
        void fls_set(opaque_t handle, void* ptr)
        {
            const usize index = (usize)handle;
            lucheck(g_fls_registry.is_index_valid(index));
            FLSContext* ctx = (FLSContext*)pthread_getspecific(g_fiber_fls_tls);
            if(!ctx)
            {
                ctx = g_fls_registry.allocate_context();
                pthread_setspecific(g_fiber_fls_tls, ctx);
            }
            ctx->set(index, ptr);
        }
        void* fls_get(opaque_t handle)
        {
            const usize index = (usize)handle;
            lucheck(g_fls_registry.is_index_valid(index));
            FLSContext* ctx = (FLSContext*)pthread_getspecific(g_fiber_fls_tls);
            if(!ctx) return nullptr;
            return ctx->get(index);
        }
    }
}
