/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.hpp
* @author JXMaster
* @date 2021/5/10
*/
#pragma once
#include "../Memory.hpp"

namespace Luna
{
#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
	void memory_check_init();
	void memory_check_close();
#endif
}