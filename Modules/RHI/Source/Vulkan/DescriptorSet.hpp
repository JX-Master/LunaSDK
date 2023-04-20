/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DescriptorSet : IDescriptorSet
		{
			lustruct("RHI::DescriptorSet", "{E215C4E0-69C5-4D04-8514-A980E1C6C74B}");

			Ref<Device> m_device;
			Name m_name;
			Ref<DescriptorSetLayout> m_layout;

			VkDescriptorSet m_desc_set = VK_NULL_HANDLE;

			RV init(const DescriptorSetDesc& desc);
			~DescriptorSet();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
		};
	}
}