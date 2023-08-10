/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2023/7/17
*/
#pragma once
#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "TextureView.hpp"

namespace Luna
{
    namespace RHI
    {
        struct DescriptorSetBinding
        {
            // The resource being used in this binding.
            Array<MTL::Resource*> m_resources;
            MTL::ResourceUsage m_usages = 0;
            // Used if this is a descriptor set for render pipeline.
            MTL::RenderStages m_render_stages = 0;
        };
        struct DescriptorSet : IDescriptorSet
        {
            lustruct("RHI::DescriptorSet", "{ff931d6c-44db-459b-b36b-786a7848613a}");
            luiimpl();

            Ref<Device> m_device;
            Ref<DescriptorSetLayout> m_layout;
            NSPtr<MTL::Buffer> m_buffer;
            
            Array<DescriptorSetBinding> m_bindings;
            
            RV init(const DescriptorSetDesc& desc);

            HashMap<u32, NSPtr<MTL::SamplerState>> m_samplers;

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const Name& name) override  { set_object_name(m_buffer.get(), name); }
            virtual RV update_descriptors(Span<const WriteDescriptorSet> writes) override;
        };
    }
}
