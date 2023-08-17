/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSetLayout.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "../../DescriptorSetLayout.hpp"
#include "Device.hpp"
namespace Luna
{
	namespace RHI
	{
		struct DescriptorSetLayout : IDescriptorSetLayout
		{
			lustruct("RHI::DescriptorSetLayout", "{A98BDEC9-14D2-4CD0-AC5F-666BB828A1F7}");
			luiimpl();

			Ref<Device> m_device;
			DescriptorSetLayoutDesc m_desc;
			VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
			Name m_name;

			RV init(const DescriptorSetLayoutDesc& desc);
			~DescriptorSetLayout();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
		};
	}
}