/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext_x86.S
* @author JXMaster
* @date 2026/2/19
*/
#include "FiberContextBase.h"
#include "FiberContext_x86.h"

.text
.global LUNA_ASM_SYMBOL(luna_fiber_swap)
.align 4
LUNA_ASM_SYMBOL(luna_fiber_swap):
    movl    4(%esp), %eax
    movl    %ebx, LUNA_REG_EBX(%eax)
    movl    %ebp, LUNA_REG_EBP(%eax)
    movl    %esi, LUNA_REG_ESI(%eax)
    movl    %edi, LUNA_REG_EDI(%eax)
    movl    (%esp), %ecx
    movl    %ecx, LUNA_REG_EIP(%eax)
    lea     4(%esp), %ecx
    movl    %ecx, LUNA_REG_ESP(%eax)
    movl    8(%esp), %ecx
    movl    LUNA_REG_EBX(%ecx), %ebx
    movl    LUNA_REG_EBP(%ecx), %ebp
    movl    LUNA_REG_ESI(%ecx), %esi
    movl    LUNA_REG_EDI(%ecx), %edi
    movl    LUNA_REG_ESP(%ecx), %esp
    movl    LUNA_REG_EIP(%ecx), %ecx
    jmp     *%ecx