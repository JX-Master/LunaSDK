/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HID.cpp
* @author JXMaster
* @date 2019/7/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include <Luna/Runtime/Module.hpp>

#include "HID.hpp"

namespace Luna
{
	namespace HID
	{
		static RV hid_init()
		{
			return platform_init();
		}
		static void hid_close()
		{
			platform_close();
		}
		LUNA_STATIC_REGISTER_MODULE(HID, "", hid_init, hid_close);
	}
}