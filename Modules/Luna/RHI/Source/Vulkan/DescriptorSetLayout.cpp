/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSetLayout.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "VulkanDescriptorSetLayout.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/StackAllocator.hpp>

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
            StackAllocator salloc;
            lutry
            {
                const bool has_variable_descriptors = test_flags(desc.flags, DescriptorSetLayoutFlag::variable_descriptors);
                if (has_variable_descriptors && desc.bindings.empty())
                {
                    return set_error(E_BAD_ARGUMENTS, "A variable descriptor set layout must contain at least one binding.");
                }
                if (has_variable_descriptors && !m_device->m_supports_variable_descriptor_count)
                {
                    return set_error(E_NOT_SUPPORTED, "Variable descriptor counts are not supported by this Vulkan device.");
                }
                m_flags = desc.flags;
                VkDescriptorSetLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                VkDescriptorSetLayoutBinding* bindings = nullptr;
                if (!desc.bindings.empty())
                {
                    bindings = (VkDescriptorSetLayoutBinding*)salloc.allocate(sizeof(VkDescriptorSetLayoutBinding) * desc.bindings.size());
                    for (usize i = 0; i < desc.bindings.size(); ++i)
                    {
                        encode_descriptor_set_binding(bindings[i], desc.bindings[i]);
                    }
                    // Vulkan requires the variable descriptor binding to have the largest
                    // binding number. Sort the encoded bindings so the last flag below always
                    // refers to that binding, independently of caller-provided array order.
                    sort(bindings, bindings + desc.bindings.size(),
                        [](const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs)
                        {
                            return lhs.binding < rhs.binding;
                        });
                    info.pBindings = bindings;
                    info.bindingCount = (u32)desc.bindings.size();
                }
                else
                {
                    info.pBindings = nullptr;
                    info.bindingCount = 0;
                }
                VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags{};
                if (has_variable_descriptors)
                {
                    binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
                    binding_flags.bindingCount = info.bindingCount;
                    auto flags = (VkDescriptorBindingFlags*)salloc.allocate(sizeof(VkDescriptorBindingFlags) * info.bindingCount);
                    memzero(flags, sizeof(VkDescriptorBindingFlags) * info.bindingCount);
                    flags[info.bindingCount - 1] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
                    binding_flags.pBindingFlags = flags;
                    info.pNext = &binding_flags;
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
