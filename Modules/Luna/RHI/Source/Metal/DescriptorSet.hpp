/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2022/7/17
*/
#pragma once
#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "TextureView.hpp"

namespace Luna
{
    namespace RHI
    {
        struct DescriptorSet : IDescriptorSet
        {
            lustruct("RHI::DescriptorSet", "{ff931d6c-44db-459b-b36b-786a7848613a}");
            luiimpl();

            Ref<Device> m_device;
            Ref<DescriptorSetLayout> m_layout;
            NSPtr<MTL::Buffer> m_buffer;
            RV init(const DescriptorSetDesc& desc);

            HashMap<u32, Ref<Sampler>> m_samplers;

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const Name& name) override  { set_object_name(m_buffer.get(), name); }
            virtual RV update_descriptors(Span<const WriteDescriptorSet> writes) override;
        };
    }
}