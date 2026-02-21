/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext_arm.S
* @author JXMaster
* @date 2026/2/19
*/
#include "FiberContextBase.h"
#include "FiberContext_arm.h"

.text
.global LUNA_ASM_SYMBOL(luna_fiber_swap)
.align 4
.type LUNA_ASM_SYMBOL(luna_fiber_swap), %function
LUNA_ASM_SYMBOL(luna_fiber_swap):
    str r12, [r0, #LUNA_REG_r12]
    str r4, [r0, #LUNA_REG_r4]
    str r5, [r0, #LUNA_REG_r5]
    str r6, [r0, #LUNA_REG_r6]
    str r7, [r0, #LUNA_REG_r7]
    str r8, [r0, #LUNA_REG_r8]
    str r9, [r0, #LUNA_REG_r9]
    str r10, [r0, #LUNA_REG_r10]
    str r11, [r0, #LUNA_REG_r11]
    str sp, [r0, #LUNA_REG_SP]
    str lr, [r0, #LUNA_REG_LR]
    mov r3, r1
    ldr r12, [r3, #LUNA_REG_r12]
    ldr r4, [r3, #LUNA_REG_r4]
    ldr r5, [r3, #LUNA_REG_r5]
    ldr r6, [r3, #LUNA_REG_r6]
    ldr r7, [r3, #LUNA_REG_r7]
    ldr r8, [r3, #LUNA_REG_r8]
    ldr r9, [r3, #LUNA_REG_r9]
    ldr r10, [r3, #LUNA_REG_r10]
    ldr r11, [r3, #LUNA_REG_r11]
    ldr r0, [r3, #LUNA_REG_r0]
    ldr r1, [r3, #LUNA_REG_r1]
    ldr sp, [r3, #LUNA_REG_SP]
    ldr lr, [r3, #LUNA_REG_LR]
    mov pc, lr