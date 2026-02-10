/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.cpp
* @author JXMaster
* @date 2026/2/10
*/
#include "../../OS.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <ucontext.h>

namespace Luna
{
    namespace OS
    {
        union FiberArgs
        {
            FiberContext* context;
            struct 
            {
                int a;
                int b;
            };
        };

        void fiber_entry(int a, int b)
        {
            FiberArgs args;
            args.a = a;
            args.b = b;
            args.context->entry_func(args.context->param);
        }

        RV new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, FiberContext& out_fiber)
        {
            int r = getcontext(&out_fiber.context);
            if(r) return BasicError::bad_platform_call();
            out_fiber.entry_func = entry_func;
            out_fiber.param = param;
            out_fiber.stack_size = stack_size;
            out_fiber.stack = memalloc(out_fiber.stack_size, 16);
            if(!out_fiber.stack) return BasicError::out_of_memory();
            out_fiber.context.uc_stack.ss_sp = out_fiber.stack;
            out_fiber.context.uc_stack.ss_size = out_fiber.stack_size;
            out_fiber.context.uc_link = nullptr;
            FiberArgs args {};
            args.context = &out_fiber;
            makecontext(&out_fiber.context, reinterpret_cast<void (*)()>(fiber_entry), 2, args.a, args.b);
            return ok;
        }

        void delete_fiber(FiberContext& fiber)
        {
            if(fiber.stack)
            {
                memfree(fiber.stack, 16);
                fiber.stack = nullptr;
                fiber.stack_size = 0;
            }
        }

        RV convert_thread_to_fiber(FiberContext& out_fiber)
        {
            int r = getcontext(&out_fiber.context);
            if(r) return BasicError::bad_platform_call();
            out_fiber.entry_func = nullptr;
            out_fiber.param = nullptr;
            out_fiber.stack_size = 0;
            out_fiber.stack = nullptr;
            return ok;
        }

        RV convert_fiber_to_thread()
        {
            return ok;
        }

        void switch_to_fiber(FiberContext& fiber)
        {
            setcontext(&fiber.context);
        }
    }
}
#pragma GCC diagnostic pop