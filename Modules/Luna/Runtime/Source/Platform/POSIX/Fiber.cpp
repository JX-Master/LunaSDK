/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.cpp
* @author JXMaster
* @date 2026/2/10
*/
#include "../Fiber.hpp"
#include "../Thread.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <ucontext.h>

namespace Luna
{
    namespace Platform
    {
        opaque_t g_fiber_params_tls;

        Result fiber_init()
        {
            return tls_alloc(nullptr, g_fiber_params_tls);
        }

        void fiber_close()
        {
            tls_free(g_fiber_params_tls);
        }

        void fiber_entry()
        {
            Fiber* ctx = (Fiber*)tls_get(g_fiber_params_tls);
            ctx->entry_func(ctx->param);
        }

        Result new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, Fiber& out_fiber)
        {
            memzero(&out_fiber.context);
            int r = getcontext(&out_fiber.context);
            if(r) return Result::bad_platform_call;
            out_fiber.entry_func = entry_func;
            out_fiber.param = param;
            out_fiber.stack_size = stack_size;
            out_fiber.stack = memalloc(out_fiber.stack_size, 16);
            if(!out_fiber.stack) return Result::out_of_memory;
            out_fiber.context.uc_stack.ss_sp = out_fiber.stack;
            out_fiber.context.uc_stack.ss_size = out_fiber.stack_size;
            out_fiber.context.uc_stack.ss_flags = 0;
            out_fiber.context.uc_link = nullptr;
            makecontext(&out_fiber.context, fiber_entry, 0);
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
        }

        Result convert_thread_to_fiber(Fiber& out_fiber)
        {
            memzero(&out_fiber.context);
            int r = getcontext(&out_fiber.context);
            if(r) return Result::bad_platform_call;
            out_fiber.entry_func = nullptr;
            out_fiber.param = nullptr;
            out_fiber.stack_size = 0;
            out_fiber.stack = nullptr;
            tls_set(g_fiber_params_tls, &out_fiber);
            return Result::success;
        }

        Result convert_fiber_to_thread()
        {
            tls_set(g_fiber_params_tls, nullptr);
            return Result::success;
        }

        void switch_to_fiber(Fiber& fiber)
        {
            Fiber* cur_fiber = (Fiber*)tls_get(g_fiber_params_tls);
            tls_set(g_fiber_params_tls, &fiber);
            swapcontext(&cur_fiber->context, &fiber.context);
        }
    }
}
#pragma GCC diagnostic pop