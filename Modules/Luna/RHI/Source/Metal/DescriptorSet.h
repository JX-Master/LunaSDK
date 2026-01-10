/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.h
* @author JXMaster
* @date 2023/7/17
*/
#pragma once
#include "Device.h"
#include "DescriptorSetLayout.h"
#include "TextureView.h"

namespace Luna
{
    namespace RHI
    {
        // Used to track resources when binding this set to the heap.
        struct DescriptorSetBinding
        {
            // The resource being used in this binding.
            Array<NSWrapper<id<MTLResource>>> m_resources;
            MTLResourceUsage m_usages = 0;
            // Used if this is a descriptor set for render pipeline.
            MTLRenderStages m_render_stages = 0;
        };
        struct DescriptorSet : IDescriptorSet
        {
            lustruct("RHI::DescriptorSet", "{ff931d6c-44db-459b-b36b-786a7848613a}");
            luiimpl();

            Ref<Device> m_device;
            Ref<DescriptorSetLayout> m_layout;
            id<MTLBuffer> m_buffer;
            id<MTLArgumentEncoder> m_encoder; // Used only if the platform does not support metal 3.
            
            Array<DescriptorSetBinding> m_bindings;
            
            RV init(const DescriptorSetDesc& desc);

            HashMap<u32, id<MTLSamplerState>> m_samplers;

            usize calc_binding_index(u32 binding_slot) const;
            
            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override;
            virtual RV update_descriptors(Span<const WriteDescriptorSet> writes) override;
        };
    }
}
