/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Fence.hpp
* @author JXMaster
* @date 2023/5/16
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
	namespace RHI
	{
		struct Fence : IFence
		{
			lustruct("RHI::Fence", "{2FE83681-A765-41D6-BBBB-C64F1A08C78F}");
			luiimpl();

			Ref<Device> m_device;
			ComPtr<ID3D12Fence> m_fence;
			u64 m_wait_value = 0;

			RV init();
			virtual IDevice* get_device() override { return m_device; }
			virtual void set_name(const c8* name) override { set_object_name(m_fence.Get(), name); }
		};
	}
}