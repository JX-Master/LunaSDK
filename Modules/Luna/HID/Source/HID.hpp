/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HID.hpp
* @author JXMaster
* @date 2023/8/10
*/
#pragma once
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace HID
    {
        //! Implemented by underlying platform to register platform-specific devices.
		RV register_platform_devices();
		//! Implemented by underlying platform to unregister platform-specific devices.
		void unregister_platform_devices();
    }
}