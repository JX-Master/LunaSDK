/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.cpp
* @author JXMaster
* @date 2022/8/3
*/
#include "DescriptorSetLayout.hpp"
namespace Luna
{
    namespace RHI
    {
        inline D3D12_DESCRIPTOR_RANGE_TYPE encode_descriptor_range_type(DescriptorType type)
        {
            switch (type)
            {
            case DescriptorType::read_buffer_view:
            case DescriptorType::read_texture_view:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case DescriptorType::read_write_buffer_view:
            case DescriptorType::read_write_texture_view:
                return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorType::uniform_buffer_view:
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case DescriptorType::sampler:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            default:
                lupanic();
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            }
        }
        inline D3D12_SHADER_VISIBILITY encode_shader_visibility(ShaderVisibilityFlag shader_visibility)
        {
            switch (shader_visibility)
            {
            case ShaderVisibilityFlag::pixel:
                return D3D12_SHADER_VISIBILITY_PIXEL;
                break;
            case ShaderVisibilityFlag::vertex:
                return D3D12_SHADER_VISIBILITY_VERTEX;
                break;
            default:
                return D3D12_SHADER_VISIBILITY_ALL;
            }
        }
        void DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
        {
            Array<DescriptorSetLayoutBinding> desc_bindings(desc.bindings.data(), desc.bindings.size());
            // Sort bindings by their binding slot.
            sort(desc_bindings.begin(), desc_bindings.end(),
                [](const DescriptorSetLayoutBinding& lhs, const DescriptorSetLayoutBinding& rhs) {return lhs.binding_slot < rhs.binding_slot; });
            m_bindings.assign(desc.bindings.size());
            for(usize i = 0; i < desc.bindings.size(); ++i)
            {
                m_bindings[i].binding_slot = desc_bindings[i].binding_slot;
                m_bindings[i].num_descs = desc_bindings[i].num_descs;
            }
            // Handle variable bindings.
            bool variable_binding_enabled = false;
            if (test_flags(desc.flags, DescriptorSetLayoutFlag::variable_descriptors))
            {
                variable_binding_enabled = true;
                m_bindings.back().num_descs = U32_MAX;
            }
            // Resolve bindings to D3D12 descriptor heaps.
            for (usize i = 0; i < m_bindings.size(); ++i)
            {
                auto& binding = m_bindings[i];
                switch (desc_bindings[i].type)
                {
                case DescriptorType::read_buffer_view:
                case DescriptorType::read_texture_view:
                case DescriptorType::read_write_buffer_view:
                case DescriptorType::read_write_texture_view:
                case DescriptorType::uniform_buffer_view:
                    binding.target_heap = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; break;
                case DescriptorType::sampler:
                    binding.target_heap = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER; break;
                default:
                    lupanic();
                    break;
                }
                HeapInfo* heap = get_heap_by_type(binding.target_heap);
                binding.offset_in_heap = heap->m_size;
                if(variable_binding_enabled && binding.num_descs == U32_MAX)
                {
                    heap->m_variable = true;
                }
                else
                {
                    heap->m_size += binding.num_descs;
                }
            }
            // Resolve bindings to D3D12 root parameters (descriptor table type).
            // The system merges continuous bindings with the same type and shader visibility to the same parameter.
            for (usize i = 0; i < m_bindings.size(); ++i)
            {
                auto& binding = m_bindings[i];
                auto& desc = desc_bindings[i];
                binding.root_parameter_index = get_root_parameter_index(desc.type, desc.shader_visibility_flags);
                auto& root_parameter = m_root_parameters[binding.root_parameter_index];
                // Check whether we can merge to the last range.
                if (!root_parameter.m_ranges.empty() && binding.num_descs != U32_MAX)
                {
                    D3D12_DESCRIPTOR_RANGE& last_range = root_parameter.m_ranges.back();
                    if (last_range.RangeType == encode_descriptor_range_type(desc.type) &&
                        (last_range.BaseShaderRegister + last_range.NumDescriptors == binding.binding_slot) &&
                        (last_range.OffsetInDescriptorsFromTableStart + last_range.NumDescriptors == binding.offset_in_heap)
                        )
                    {
                        binding.range_index = (u32)root_parameter.m_ranges.size() - 1;
                        last_range.NumDescriptors += binding.num_descs;
                        continue;
                    }
                }
                // Append a new range.
                D3D12_DESCRIPTOR_RANGE range;
                range.BaseShaderRegister = binding.binding_slot;
                range.NumDescriptors = binding.num_descs == U32_MAX ? UINT_MAX : binding.num_descs;
                range.OffsetInDescriptorsFromTableStart = binding.offset_in_heap;
                range.RangeType = encode_descriptor_range_type(desc.type);
                range.RegisterSpace = 0;
                binding.range_index = (u32)root_parameter.m_ranges.size();
                root_parameter.m_ranges.push_back(range);
            }
            m_root_parameters.shrink_to_fit();
        }
        DescriptorSetLayout::HeapInfo* DescriptorSetLayout::get_heap_by_type(D3D12_DESCRIPTOR_HEAP_TYPE heap)
        {
            HeapInfo* target = nullptr;
            if (heap == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            {
                return &m_view_heap;
            }
            else if (heap == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
            {
                return &m_sampler_heap;
            }
            lupanic();
            return nullptr;
        }
        inline bool root_parameter_type_compatible(DescriptorType desc_type, D3D12_DESCRIPTOR_HEAP_TYPE root_type)
        {
            if ((root_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) && (
                desc_type == DescriptorType::read_buffer_view ||
                desc_type == DescriptorType::read_texture_view ||
                desc_type == DescriptorType::read_write_buffer_view ||
                desc_type == DescriptorType::read_write_texture_view ||
                desc_type == DescriptorType::uniform_buffer_view)
                ) return true;
            if ((root_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) && (
                desc_type == DescriptorType::sampler
                )) return true;
            return false;
        }
        u32 DescriptorSetLayout::get_root_parameter_index(DescriptorType type, ShaderVisibilityFlag shader_visibility)
        {
            for (u32 i = 0; i < (u32)m_root_parameters.size(); ++i)
            {
                auto& root_param = m_root_parameters[i];
                if (root_param.m_shader_visibility == encode_shader_visibility(shader_visibility) &&
                    root_parameter_type_compatible(type, root_param.m_type))
                {
                    return i;
                }
            }
            RootParameterInfo info;
            switch (type)
            {
            case DescriptorType::read_buffer_view:
            case DescriptorType::read_texture_view:
            case DescriptorType::read_write_buffer_view:
            case DescriptorType::read_write_texture_view:
            case DescriptorType::uniform_buffer_view:
                info.m_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                break;
            case DescriptorType::sampler:
                info.m_type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                break;
            default:
                lupanic();
                break;
            }
            info.m_shader_visibility = encode_shader_visibility(shader_visibility);
            m_root_parameters.push_back(info);
            return (u32)m_root_parameters.size() - 1;
        }
    }
}