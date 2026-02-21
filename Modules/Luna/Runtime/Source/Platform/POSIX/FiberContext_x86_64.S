/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext_x86_64.S
* @author JXMaster
* @date 2026/2/19
*/
#include "FiberContextBase.h"
#include "FiberContext_x86_64.h"

.text
.global LUNA_ASM_SYMBOL(luna_fiber_swap)
.align 4
LUNA_ASM_SYMBOL(luna_fiber_swap):
    movq    %rbx, LUNA_REG_RBX(%rdi)
    movq    %rbp, LUNA_REG_RBP(%rdi)
    movq    %r12, LUNA_REG_R12(%rdi)
    movq    %r13, LUNA_REG_R13(%rdi)
    movq    %r14, LUNA_REG_R14(%rdi)
    movq    %r15, LUNA_REG_R15(%rdi)
    movq    (%rsp), %rcx
    movq    %rcx, LUNA_REG_RIP(%rdi)
    leaq    8(%rsp), %rcx
    movq    %rcx, LUNA_REG_RSP(%rdi)
    movq    %rsi, %r8
    movq    LUNA_REG_RBX(%r8), %rbx
    movq    LUNA_REG_RBP(%r8), %rbp
    movq    LUNA_REG_R12(%r8), %r12
    movq    LUNA_REG_R13(%r8), %r13
    movq    LUNA_REG_R14(%r8), %r14
    movq    LUNA_REG_R15(%r8), %r15
    movq    LUNA_REG_RDI(%r8), %rdi
    movq    LUNA_REG_RSI(%r8), %rsi
    movq    LUNA_REG_RSP(%r8), %rsp
    movq    LUNA_REG_RIP(%r8), %rcx
    jmp     *%rcx