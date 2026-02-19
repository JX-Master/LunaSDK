/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContext_arm64.S
* @author JXMaster
* @date 2026/2/19
*/
#include "FiberContextBase.h"
#include "FiberContext_arm64.h"

#if defined(__ARM_FEATURE_PAC_DEFAULT) && __ARM_FEATURE_PAC_DEFAULT
#define ENABLE_PAUTH 1
#if (__ARM_FEATURE_PAC_DEFAULT & (1 << 0))
#define PAUTH_SIGN_SP paciasp
#define PAUTH_AUTH_SP autiasp
#else
#define PAUTH_SIGN_SP pacibsp
#define PAUTH_AUTH_SP autibsp
#endif
#else
#define PAUTH_SIGN_SP
#define PAUTH_AUTH_SP
#endif

.text
.global LUNA_ASM_SYMBOL(luna_fiber_swap)
.align 4
LUNA_ASM_SYMBOL(luna_fiber_swap):
    PAUTH_SIGN_SP
    str x16, [x0, #LUNA_REG_r16]
    str x17, [x0, #LUNA_REG_r17]
    str x18, [x0, #LUNA_REG_r18]
    str x19, [x0, #LUNA_REG_r19]
    str x20, [x0, #LUNA_REG_r20]
    str x21, [x0, #LUNA_REG_r21]
    str x22, [x0, #LUNA_REG_r22]
    str x23, [x0, #LUNA_REG_r23]
    str x24, [x0, #LUNA_REG_r24]
    str x25, [x0, #LUNA_REG_r25]
    str x26, [x0, #LUNA_REG_r26]
    str x27, [x0, #LUNA_REG_r27]
    str x28, [x0, #LUNA_REG_r28]
    str x29, [x0, #LUNA_REG_r29]
    str d8,  [x0, #LUNA_REG_v8]
    str d9,  [x0, #LUNA_REG_v9]
    str d10, [x0, #LUNA_REG_v10]
    str d11, [x0, #LUNA_REG_v11]
    str d12, [x0, #LUNA_REG_v12]
    str d13, [x0, #LUNA_REG_v13]
    str d14, [x0, #LUNA_REG_v14]
    str d15, [x0, #LUNA_REG_v15]
    mov x2, sp
    str x2,  [x0, #LUNA_REG_SP]
    str x30, [x0, #LUNA_REG_LR]
    mov x7, x1
    ldr x16, [x7, #LUNA_REG_r16]
    ldr x17, [x7, #LUNA_REG_r17]
    ldr x18, [x7, #LUNA_REG_r18]
    ldr x19, [x7, #LUNA_REG_r19]
    ldr x20, [x7, #LUNA_REG_r20]
    ldr x21, [x7, #LUNA_REG_r21]
    ldr x22, [x7, #LUNA_REG_r22]
    ldr x23, [x7, #LUNA_REG_r23]
    ldr x24, [x7, #LUNA_REG_r24]
    ldr x25, [x7, #LUNA_REG_r25]
    ldr x26, [x7, #LUNA_REG_r26]
    ldr x27, [x7, #LUNA_REG_r27]
    ldr x28, [x7, #LUNA_REG_r28]
    ldr x29, [x7, #LUNA_REG_r29]
    ldr d8,  [x7, #LUNA_REG_v8]
    ldr d9,  [x7, #LUNA_REG_v9]
    ldr d10, [x7, #LUNA_REG_v10]
    ldr d11, [x7, #LUNA_REG_v11]
    ldr d12, [x7, #LUNA_REG_v12]
    ldr d13, [x7, #LUNA_REG_v13]
    ldr d14, [x7, #LUNA_REG_v14]
    ldr d15, [x7, #LUNA_REG_v15]
    ldr x0, [x7, #LUNA_REG_r0]
    ldr x1, [x7, #LUNA_REG_r1]
    ldr x30, [x7, #LUNA_REG_LR]
    ldr x2,  [x7, #LUNA_REG_SP]
    mov sp, x2
    PAUTH_AUTH_SP
    ret