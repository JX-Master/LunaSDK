/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "DescriptorSet.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		RV DescriptorSet::init(const DescriptorSetDesc& desc)
		{
			lutry
			{
				m_layout = (DescriptorSetLayout*)desc.layout->get_object();
				VkDescriptorSetAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				alloc_info.descriptorPool = m_device->m_desc_pool;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts = &m_layout->m_layout;
				/*VkDescriptorSetVariableDescriptorCountAllocateInfo variable_info{};
				if (test_flags(m_layout->m_desc.flags, DescriptorSetLayoutFlag::variable_descriptors))
				{
					variable_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
					u32 counts = desc.num_variable_descriptors;
					variable_info.pDescriptorCounts = &counts;
					variable_info.descriptorSetCount = 1;
					alloc_info.pNext = &variable_info;
				}*/
				luexp(encode_vk_result(m_device->m_funcs.vkAllocateDescriptorSets(m_device->m_device, &alloc_info, &m_desc_set)));
			}
			lucatchret;
			return ok;
		}
		DescriptorSet::~DescriptorSet()
		{
			if (m_desc_set != VK_NULL_HANDLE)
			{
				MutexGuard guard(m_device->m_mtx);
				m_device->m_funcs.vkFreeDescriptorSets(m_device->m_device, m_device->m_desc_pool, 1, &m_desc_set);
				m_desc_set = VK_NULL_HANDLE;
			}
		}
		void DescriptorSet::update_descriptors(Span<const DescriptorSetWrite> writes, Span<const DescriptorSetCopy> copies)
		{
			
		}
	}
}