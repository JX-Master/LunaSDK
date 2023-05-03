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
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			Ref<DescriptorSetLayout> m_layout;

			VkDescriptorSet m_desc_set = VK_NULL_HANDLE;

			HashMap<u32, VkImageView> m_image_views;
			HashMap<u32, VkSampler> m_samplers;

			RV init(const DescriptorSetDesc& desc);
			~DescriptorSet();

			void set_image_view(u32 slot, VkImageView image_view);
			void set_sampler(u32 slot, VkSampler sampler);

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual void update_descriptors(Span<const DescriptorSetWrite> writes, Span<const DescriptorSetCopy> copies) override;
		};
	}
}