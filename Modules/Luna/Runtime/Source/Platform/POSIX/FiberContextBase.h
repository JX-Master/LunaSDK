/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberContextBase.h
* @author JXMaster
* @date 2026/2/19
*/
#pragma once
#if defined(__APPLE__)
#define LUNA_ASM_SYMBOL(x) _##x
#else
#define LUNA_ASM_SYMBOL(x) x
#endif