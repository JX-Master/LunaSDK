/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.cpp
* @author JXMaster
* @date 2022/8/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT

#include "ReadWriteLock.hpp"
namespace Luna
{
	LUNA_RUNTIME_API Ref<IReadWriteLock> new_read_write_lock()
	{
		return new_object<ReadWriteLock>();
	}
}