/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Interface.hpp
* @author JXMaster
* @date 2022/5/27
*/
#pragma once
#include "../Interface.hpp"
#include "../HashMap.hpp"
#include "../UniquePtr.hpp"

namespace Luna
{
	struct InterfaceImplEntry
	{
		void* (*cast_to_interface)(object_t obj);
	};
}