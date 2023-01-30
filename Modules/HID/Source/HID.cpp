/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HID.cpp
* @author JXMaster
* @date 2019/7/24
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../HID.hpp"
#include <Runtime/Vector.hpp>
#include <Runtime/Mutex.hpp>
#include <Runtime/Module.hpp>

#include "../Mouse.hpp"
#include "../Keyboard.hpp"
#include "../Controller.hpp"

namespace Luna
{
	namespace HID
	{
		//! Implemented by underlying platform to register platform-specific devices.
		RV register_platform_devices();
		//! Implemented by underlying platform to unregister platform-specific devices.
		void unregister_platform_devices();
		struct Device
		{
			Name name;
			DeviceDesc desc;
		};
		Vector<Device> g_devices;
		Ref<IMutex> g_mutex;
		LUNA_HID_API RV register_device(const Name& device_name, DeviceDesc& desc)
		{
			MutexGuard guard(g_mutex);
			for (auto& i : g_devices)
			{
				if (i.name == device_name)
				{
					return BasicError::already_exists();
				}
			}
			Device new_device;
			new_device.name = device_name;
			new_device.desc = desc;
			g_devices.push_back(new_device);
			return ok;
		}
		LUNA_HID_API void unregister_device(const Name& device_name)
		{
			MutexGuard guard(g_mutex);
			for (auto iter = g_devices.begin(); iter != g_devices.end(); ++iter)
			{
				if (iter->name == device_name)
				{
					if (iter->desc.on_unregister) iter->desc.on_unregister(iter->desc.userdata);
					g_devices.erase(iter);
					return;
				}
			}
		}
		LUNA_HID_API RV get_device_by_interface(const Guid& iid, object_t* out_device_object)
		{
			MutexGuard guard(g_mutex);
			for (auto iter = g_devices.rbegin(); iter != g_devices.rend(); ++iter)
			{
				for (auto& guid : iter->desc.supported_iids)
				{
					if (iid == guid)
					{
						return iter->desc.on_request_device(iter->desc.userdata, iid, out_device_object);
					}
				}
			}
			return BasicError::not_found();
		}
		static RV hid_init()
		{
			g_mutex = new_mutex();
			return register_platform_devices();
		}
		static void hid_close()
		{
			unregister_platform_devices();
			for (auto& device : g_devices)
			{
				if (device.desc.on_unregister) device.desc.on_unregister(device.desc.userdata);
			}
			g_devices.clear();
			g_devices.shrink_to_fit();
			g_mutex.reset();
		}
		StaticRegisterModule hid("HID", "", hid_init, hid_close);
	}
}