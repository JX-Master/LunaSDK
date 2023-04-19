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
		inline void encode_descriptor_set_binding(VkDescriptorSetLayoutBinding& dest, const DescriptorSetLayoutBinding& src)
		{
			dest.binding = src.binding_slot;
			dest.descriptorCount = src.num_descs;
			switch (src.type)
			{
			case DescriptorType::texture_srv:
				dest.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
			case DescriptorType::texture_uav:
				dest.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
			case DescriptorType::buffer_srv:
			case DescriptorType::buffer_uav:
				dest.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
			case DescriptorType::sampler:
				dest.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER; break;
			}
			if (src.shader_visibility_flags == ShaderVisibilityFlag::all)
			{
				dest.stageFlags = VK_SHADER_STAGE_ALL;
			}
			else
			{
				dest.stageFlags = 0;
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::vertex))
				{
					dest.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::hull))
				{
					dest.stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::domain))
				{
					dest.stageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::geometry))
				{
					dest.stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::pixel))
				{
					dest.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				}
				if (test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::compute))
				{
					dest.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
				}
			}
			dest.pImmutableSamplers = nullptr;
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
				VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags{};
				if (test_flags(desc.flags, DescriptorSetLayoutFlag::variable_descriptors) && info.bindingCount)
				{
					if (!m_device->m_descriptor_binding_variable_descriptor_count_supported)
					{
						return set_error(BasicError::not_supported(), "variable descriptors is not supported on this device.");
					}
					binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
					binding_flags.bindingCount = info.bindingCount;
					auto flags = (VkDescriptorBindingFlags*)alloca(sizeof(VkDescriptorBindingFlags) * info.bindingCount);
					memzero(flags, sizeof(VkDescriptorBindingFlags) * info.bindingCount);
					flags[info.bindingCount - 1] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
					info.pNext = &binding_flags;
				}
				luexp(encode_vk_result(vkCreateDescriptorSetLayout(m_device->m_device, &info, nullptr, &m_layout)));
			}
			lucatchret;
			return ok;
		}
		DescriptorSetLayout::~DescriptorSetLayout()
		{
			if (m_layout != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorSetLayout(m_device->m_device, m_layout, nullptr);
				m_layout = VK_NULL_HANDLE;
			}
		}
	}
}