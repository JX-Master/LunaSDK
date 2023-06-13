/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.cpp
* @author JXMaster
* @date 2019/12/26
 */
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT

#include "Signal.hpp"
namespace Luna
{
	LUNA_RUNTIME_API Ref<ISignal> new_signal(bool manual_reset)
	{
		return new_object<Signal>(manual_reset);
	}
}