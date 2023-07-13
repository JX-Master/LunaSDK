/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PipelineLayout.cpp
* @author JXMaster
* @date 2023/4/23
*/
#include "PipelineLayout.hpp"
#include "DescriptorSetLayout.hpp"
namespace Luna
{
	namespace RHI
	{
		RV PipelineLayout::init(const PipelineLayoutDesc& desc)
		{
			lutry
			{
				VkPipelineLayoutCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				create_info.flags = 0;
				if (desc.descriptor_set_layouts.empty())
				{
					create_info.pSetLayouts = nullptr;
					create_info.setLayoutCount = 0;
				}
				else
				{
					VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)alloca(sizeof(VkDescriptorSetLayout*) * desc.descriptor_set_layouts.size());
					for (usize i = 0; i < desc.descriptor_set_layouts.size(); ++i)
					{
						DescriptorSetLayout* desc_layout = (DescriptorSetLayout*)desc.descriptor_set_layouts[i]->get_object();
						layouts[i] = desc_layout->m_layout;
					}
					create_info.pSetLayouts = layouts;
					create_info.setLayoutCount = (u32)desc.descriptor_set_layouts.size();
				}
				create_info.pushConstantRangeCount = 0;
				create_info.pPushConstantRanges = nullptr;
				luexp(encode_vk_result(m_device->m_funcs.vkCreatePipelineLayout(m_device->m_device, &create_info, nullptr, &m_pipeline_layout)));
			}
			lucatchret;
			return ok;
		}
		PipelineLayout::~PipelineLayout()
		{
			if (m_pipeline_layout != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyPipelineLayout(m_device->m_device, m_pipeline_layout, nullptr);
				m_pipeline_layout = VK_NULL_HANDLE;
			}
		}
	}
}