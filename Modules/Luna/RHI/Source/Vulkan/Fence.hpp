/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Fence.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct Fence : IFence
		{
			lustruct("RHI::Fence", "{DCC665F4-475F-4EAA-8837-17362D44BAD9}");
			luiimpl();

			Ref<Device> m_device;
			VkSemaphore m_semaphore = VK_NULL_HANDLE;
			Name m_name;

			RV init();
			~Fence();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
		};
	}
}