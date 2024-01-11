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
		struct HIDModule : public Module
		{
			virtual const c8* get_name() override { return "HID"; }
			virtual RV on_init() override
			{
				return platform_init();
			}
			virtual void on_close() override
			{
				platform_close();
			}
		};
	}
	LUNA_HID_API Module* module_hid()
	{
		static HID::HIDModule m;
		return &m;
	}
}