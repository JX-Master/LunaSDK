/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.hpp
* @author JXMaster
* @date 2020/4/1
*/
#pragma once
#include "Base.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
	LUNA_RUNTIME_API void debug_printf(const c8* fmt, ...);

	LUNA_RUNTIME_API void debug_vprintf(const c8* fmt, VarList args);
}