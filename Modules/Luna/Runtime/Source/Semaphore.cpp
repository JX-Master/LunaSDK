/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.cpp
* @author JXMaster
* @date 2019/12/26
 */
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT

#include "Semaphore.hpp"
namespace Luna
{
	LUNA_RUNTIME_API Ref<ISemaphore> new_semaphore(i32 initial_count, i32 max_count)
	{
		return new_object<Semaphore>(initial_count, max_count);
	}
}