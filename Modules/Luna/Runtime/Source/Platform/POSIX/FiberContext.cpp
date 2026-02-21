/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext.cpp
* @author JXMaster
* @date 2026/2/19
*/
#include "FiberContext.hpp"
#include "../../../Assert.hpp"

using namespace Luna;

extern "C" void luna_fiber_set_target(Luna::FiberContext* ctx, void* stack, uint32_t stack_size, void (*target)(void*), void* arg)
{
#if defined(LUNA_PLATFORM_ARM64)
    u64* stack_top = (u64*)((uint8_t*)stack + stack_size);
    ctx->LR = (u64)&luna_fiber_entry;
    ctx->r0 = (u64)target;
    ctx->r1 = (u64)arg;
    ctx->SP = ((u64)stack_top) & ~(u64)15;
#elif defined(LUNA_PLATFORM_ARM32)
    usize* stack_top = (usize*)((uint8_t*)stack + stack_size);
    ctx->LR = (usize)&luna_fiber_entry;
    ctx->r0 = (usize)target;
    ctx->r1 = (usize)arg;
    ctx->SP = ((usize)stack_top) & ~(usize)15;
#elif defined(LUNA_PLATFORM_X86_64)
    usize* stack_top = (usize*)((uint8_t*)stack + stack_size);
    ctx->RIP = (usize)&luna_fiber_entry;
    ctx->RDI = (usize)target;
    ctx->RSI = (usize)arg;
    ctx->RSP = (usize)&stack_top[-3];
    stack_top[-2] = 0;
#elif defined(LUNA_PLATFORM_X86)
    usize* stack_top = (usize*)((uint8_t*)stack + stack_size);
    ctx->EIP = (usize)&luna_fiber_entry;
    ctx->ESP = (usize)&stack_top[-5];
    stack_top[-3] = (usize)arg;
    stack_top[-4] = (usize)target;
    stack_top[-5] = 0;
#else
#error "Unsupported architecture"
#endif

}
extern "C" void luna_fiber_entry(void (*target)(void*), void* arg)
{
    target(arg);
    lupanic_msg_always("Fiber entry function should not return.");
}