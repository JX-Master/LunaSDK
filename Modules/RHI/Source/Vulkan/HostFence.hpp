/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HostFence.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct HostFence : IHostFence
		{
			lustruct("RHI::HostFence", "{DCC665F4-475F-4EAA-8837-17362D44BAD9}");
			luiimpl();

			Ref<Device> m_device;
			VkFence m_fence = VK_NULL_HANDLE;
			Name m_name;

			RV init();
			~HostFence();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual void wait() override;
			virtual bool try_wait() override;
			virtual void reset() override;
		};
	}
}