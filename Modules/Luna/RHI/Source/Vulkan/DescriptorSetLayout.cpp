/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSetLayout.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		inline void encode_descriptor_set_binding(VkDescriptorSetLayoutBinding& dst, const DescriptorSetLayoutBinding& src)
		{
			dst.binding = src.binding_slot;
			dst.descriptorCount = src.num_descs;
			dst.descriptorType = encode_descriptor_type(src.type);
			if (src.shader_visibility_flags == ShaderVisibilityFlag::all)
			{
				dst.stageFlags = VK_SHADER_STAGE_ALL;
			}
			else
			{
				dst.stageFlags = 0;
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::vertex))
				{
					dst.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::pixel))
				{
					dst.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::compute))
				{
					dst.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
				}
			}
			dst.pImmutableSamplers = nullptr;
		}
		RV DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				VkDescriptorSetLayoutCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				VkDescriptorSetLayoutBinding* bindings = nullptr;
				if (!desc.bindings.empty())
				{
					bindings = (VkDescriptorSetLayoutBinding*)alloca(sizeof(VkDescriptorSetLayoutBinding) * desc.bindings.size());
					for (usize i = 0; i < desc.bindings.size(); ++i)
					{
						encode_descriptor_set_binding(bindings[i], desc.bindings[i]);
					}
					info.pBindings = bindings;
					info.bindingCount = (u32)desc.bindings.size();
				}
				else
				{
					info.pBindings = nullptr;
					info.bindingCount = 0;
				}
				//VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags{};
				if (test_flags(desc.flags, DescriptorSetLayoutFlag::variable_descriptors) && info.bindingCount)
				{
					return set_error(BasicError::not_supported(), "variable descriptors is not supported on this device.");
					/*binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
					binding_flags.bindingCount = info.bindingCount;
					auto flags = (VkDescriptorBindingFlags*)alloca(sizeof(VkDescriptorBindingFlags) * info.bindingCount);
					memzero(flags, sizeof(VkDescriptorBindingFlags) * info.bindingCount);
					flags[info.bindingCount - 1] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
					info.pNext = &binding_flags;*/
				}
				luexp(encode_vk_result(m_device->m_funcs.vkCreateDescriptorSetLayout(m_device->m_device, &info, nullptr, &m_layout)));
			}
			lucatchret;
			return ok;
		}
		DescriptorSetLayout::~DescriptorSetLayout()
		{
			if (m_layout != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyDescriptorSetLayout(m_device->m_device, m_layout, nullptr);
				m_layout = VK_NULL_HANDLE;
			}
		}
	}
}