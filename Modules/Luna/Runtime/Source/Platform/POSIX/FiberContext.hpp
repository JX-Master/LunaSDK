/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext.hpp
* @author JXMaster
* @date 2026/2/19
*/
#pragma once
#include "../../../Base.hpp"

namespace Luna
{
    struct FiberContext
    {
    #if defined(LUNA_PLATFORM_ARM64)
        // parameter registers.
        usize r0, r1;
        // special purpose registers
        usize r16, r17, r18;
        // callee-saved registers
        usize r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        // callee-saved ARM Neon registers (64bits).
        usize v8, v9, v10, v11, v12, v13, v14, v15;
        // SP: stack pointer.
        // LR: link register (r30).
        usize SP, LR;
    #elif defined(LUNA_PLATFORM_ARM32)
        // parameter registers
        usize r0, r1;
        // special purpose registers
        usize r12;
        // callee-saved registers
        usize r4, r5, r6, r7, r8, r9, r10, r11;
        // arm neon registers (32bits).
        usize v8, v9, v10, v11, v12, v13, v14, v15;
        // SP: stack pointer.
        // LR: link register (r30).
        usize SP, LR;
    #elif defined(LUNA_PLATFORM_X86_64)
        // callee-saved registers
        usize RBX, RBP, R12, R13, R14, R15;
        // parameter registers
        usize RDI, RSI;
        // stack and instruction registers
        usize RSP, RIP;
    #elif defined(LUNA_PLATFORM_X86)
        // callee-saved registers
        usize EBX, EBP, ESI, EDI;
        // stack and instruction registers
        usize ESP, EIP;
    #else
    #error "Unsupported architecture"
    #endif
    };

#if defined(LUNA_PLATFORM_ARM64)
#include "FiberContext_arm64.h"
static_assert(offsetof(FiberContext, r0) == LUNA_REG_r0,
              "Bad register offset");
static_assert(offsetof(FiberContext, r1) == LUNA_REG_r1,
              "Bad register offset");
static_assert(offsetof(FiberContext, r16) == LUNA_REG_r16,
              "Bad register offset");
static_assert(offsetof(FiberContext, r17) == LUNA_REG_r17,
              "Bad register offset");
static_assert(offsetof(FiberContext, r18) == LUNA_REG_r18,
              "Bad register offset");
static_assert(offsetof(FiberContext, r19) == LUNA_REG_r19,
              "Bad register offset");
static_assert(offsetof(FiberContext, r20) == LUNA_REG_r20,
              "Bad register offset");
static_assert(offsetof(FiberContext, r21) == LUNA_REG_r21,
              "Bad register offset");
static_assert(offsetof(FiberContext, r22) == LUNA_REG_r22,
              "Bad register offset");
static_assert(offsetof(FiberContext, r23) == LUNA_REG_r23,
              "Bad register offset");
static_assert(offsetof(FiberContext, r24) == LUNA_REG_r24,
              "Bad register offset");
static_assert(offsetof(FiberContext, r25) == LUNA_REG_r25,
              "Bad register offset");
static_assert(offsetof(FiberContext, r26) == LUNA_REG_r26,
              "Bad register offset");
static_assert(offsetof(FiberContext, r27) == LUNA_REG_r27,
              "Bad register offset");
static_assert(offsetof(FiberContext, r28) == LUNA_REG_r28,
              "Bad register offset");
static_assert(offsetof(FiberContext, r29) == LUNA_REG_r29,
              "Bad register offset");
static_assert(offsetof(FiberContext, v8) == LUNA_REG_v8,
              "Bad register offset");
static_assert(offsetof(FiberContext, v9) == LUNA_REG_v9,
              "Bad register offset");
static_assert(offsetof(FiberContext, v10) == LUNA_REG_v10,
              "Bad register offset");
static_assert(offsetof(FiberContext, v11) == LUNA_REG_v11,
              "Bad register offset");
static_assert(offsetof(FiberContext, v12) == LUNA_REG_v12,
              "Bad register offset");
static_assert(offsetof(FiberContext, v13) == LUNA_REG_v13,
              "Bad register offset");
static_assert(offsetof(FiberContext, v14) == LUNA_REG_v14,
              "Bad register offset");
static_assert(offsetof(FiberContext, v15) == LUNA_REG_v15,
              "Bad register offset");
static_assert(offsetof(FiberContext, SP) == LUNA_REG_SP,
              "Bad register offset");
static_assert(offsetof(FiberContext, LR) == LUNA_REG_LR,
              "Bad register offset");
#elif defined(LUNA_PLATFORM_ARM32)
#include "FiberContext_arm.h"
static_assert(offsetof(FiberContext, r0) == LUNA_REG_r0,
              "Bad register offset");
static_assert(offsetof(FiberContext, r1) == LUNA_REG_r1,
              "Bad register offset");
static_assert(offsetof(FiberContext, r12) == LUNA_REG_r12,
              "Bad register offset");
static_assert(offsetof(FiberContext, r4) == LUNA_REG_r4,
              "Bad register offset");
static_assert(offsetof(FiberContext, r5) == LUNA_REG_r5,
              "Bad register offset");
static_assert(offsetof(FiberContext, r6) == LUNA_REG_r6,
              "Bad register offset");
static_assert(offsetof(FiberContext, r7) == LUNA_REG_r7,
              "Bad register offset");
static_assert(offsetof(FiberContext, r8) == LUNA_REG_r8,
              "Bad register offset");
static_assert(offsetof(FiberContext, r9) == LUNA_REG_r9,
              "Bad register offset");
static_assert(offsetof(FiberContext, r10) == LUNA_REG_r10,
              "Bad register offset");
static_assert(offsetof(FiberContext, r11) == LUNA_REG_r11,
              "Bad register offset");
static_assert(offsetof(FiberContext, v8) == LUNA_REG_v8,
              "Bad register offset");
static_assert(offsetof(FiberContext, v9) == LUNA_REG_v9,
              "Bad register offset");
static_assert(offsetof(FiberContext, v10) == LUNA_REG_v10,
              "Bad register offset");
static_assert(offsetof(FiberContext, v11) == LUNA_REG_v11,
              "Bad register offset");
static_assert(offsetof(FiberContext, v12) == LUNA_REG_v12,
              "Bad register offset");
static_assert(offsetof(FiberContext, v13) == LUNA_REG_v13,
              "Bad register offset");
static_assert(offsetof(FiberContext, v14) == LUNA_REG_v14,
              "Bad register offset");
static_assert(offsetof(FiberContext, v15) == LUNA_REG_v15,
              "Bad register offset");
static_assert(offsetof(FiberContext, SP) == LUNA_REG_SP,
              "Bad register offset");
static_assert(offsetof(FiberContext, LR) == LUNA_REG_LR,
              "Bad register offset");
#elif defined(LUNA_PLATFORM_X86_64)
#include "FiberContext_x86_64.h"
static_assert(offsetof(FiberContext, RBX) == LUNA_REG_RBX,
              "Bad register offset");
static_assert(offsetof(FiberContext, RBP) == LUNA_REG_RBP,
              "Bad register offset");
static_assert(offsetof(FiberContext, R12) == LUNA_REG_R12,
              "Bad register offset");
static_assert(offsetof(FiberContext, R13) == LUNA_REG_R13,
              "Bad register offset");
static_assert(offsetof(FiberContext, R14) == LUNA_REG_R14,
              "Bad register offset");
static_assert(offsetof(FiberContext, R15) == LUNA_REG_R15,
              "Bad register offset");
static_assert(offsetof(FiberContext, RDI) == LUNA_REG_RDI,
              "Bad register offset");
static_assert(offsetof(FiberContext, RSI) == LUNA_REG_RSI,
              "Bad register offset");
static_assert(offsetof(FiberContext, RSP) == LUNA_REG_RSP,
              "Bad register offset");
static_assert(offsetof(FiberContext, RIP) == LUNA_REG_RIP,
              "Bad register offset");
#elif defined(LUNA_PLATFORM_X86)
#include "FiberContext_x86.h"
static_assert(offsetof(FiberContext, EBX) == LUNA_REG_EBX,
              "Bad register offset");
static_assert(offsetof(FiberContext, EBP) == LUNA_REG_EBP,
              "Bad register offset");
static_assert(offsetof(FiberContext, ESI) == LUNA_REG_ESI,
              "Bad register offset");
static_assert(offsetof(FiberContext, EDI) == LUNA_REG_EDI,
              "Bad register offset");
static_assert(offsetof(FiberContext, ESP) == LUNA_REG_ESP,
              "Bad register offset");
static_assert(offsetof(FiberContext, EIP) == LUNA_REG_EIP,
              "Bad register offset");
#else
#error "Unsupported architecture"
#endif
}

extern "C" void luna_fiber_swap(Luna::FiberContext* from, const Luna::FiberContext* to);
extern "C" void luna_fiber_set_target(Luna::FiberContext* ctx, void* stack, uint32_t stack_size, void (*target)(void*), void* arg);
extern "C" void luna_fiber_entry(void (*target)(void*), void* arg);