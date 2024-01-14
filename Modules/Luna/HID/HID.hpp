/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HID.hpp
* @author JXMaster
* @date 2022/4/1
* @brief
* The Human Interface Devices (HID) module provides an abstraction layer on the platfrom's native Human Interface Devices, including mouses, keyboards,
* controllers, joysticks, sensors, cameras and so on. Most HIDs are used to provide inputs for the application, while some of them may also receive 
* feedbacks from the application like taptics, flashlights, etc. The HID module is designed to work closely to the OS layer and does not provide 
* high-level abstractions like custom actions, key-mappings and so on.
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif
namespace Luna
{
	namespace HID
	{
		//! Describes one HID device.
		struct DeviceDesc
		{
			Vector<Guid> supported_iids;
			void* userdata;
			RV(*on_request_device)(void* userdata, const Guid& iid, object_t* out_device_object);
			void (*on_unregister)(void* userdata);
		};

		//! Registers one HID device to the system.
		//! @param[in] device_name The name of the device.
		//! @param[in] desc The device descriptor. The descriptor will be copied into the system.
		//! @return `BasicError::already_exists()` if one device with the same name already exists in the system.
		LUNA_HID_API RV register_device(const Name& device_name, DeviceDesc& desc);

		//! Unregisters one HID device from the system. The `on_unregister` callback of the device will be called.
		//! @param[in] device_name The name of the device to unregister. If the device is not found, this call does nothing.
		//! @remark If the client code does not call `unregister_device` for the device it registers, they will be unregistered
		//! when the HID system closes, and `on_unregister` callback of the device will be called.
		LUNA_HID_API void unregister_device(const Name& device_name);

		//! Gets the device interface.
		//! @param[in] iid The device interface ID to fetch.
		//! @param[out] out_device_object If this call succeeds, returns the device object.
		//! @return Returns `BasicError::not_found()` if no device supports the specified interface.
		//! @remark The system searches supportted devices by their `supported_iids` specified when registering. If multiple devices support
		//! the specified interface, the later registered device will be used.
		//! 
		//! `on_request_device` callback of the chosen device will be called to fetch the device object of the device. The device object's reference
		//! count will increase by one if this call succeeds.
		LUNA_HID_API RV get_device_by_interface(const Guid& iid, object_t* out_device_object);

		//! Template wrapper for `get_device_by_interface`.
		template <typename _Ty>
		inline R<Ref<_Ty>> get_device()
		{
			object_t obj;
			RV r = get_device_by_interface(_Ty::__guid, &obj);
			if (succeeded(r))
			{
				Ref<_Ty> ret;
				ret.attach(obj);
				return ret;
			}
			return r.errcode();
		}
	}
	struct Module;
	LUNA_HID_API Module* module_hid();
}