/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceReadContext.hpp
* @author JXMaster
* @date 2025/9/4
*/
#pragma once
#include "../ResourceReadContext.hpp"

namespace Luna
{
    namespace RHIUtility
    {
        struct ResourceReadContext : IResourceReadContext
        {
            lustruct("RHIUtility::ResourceReadContext", "6d0910c8-d0ad-4d38-8daf-3d92ba4a4d7b");
            luiimpl();

            struct BufferReadParams
            {
                u64 offset;
                usize size;
            };

            struct TextureReadParams
            {
                RHI::SubresourceIndex subresource;
                u32 x;
                u32 y;
                u32 z;
                u32 width;
                u32 height;
                u32 depth;
                u64 row_pitch;
                u64 slice_pitch;
            };

            struct ResourceReadOp
            {
                Ref<RHI::IBuffer> buffer;
                Ref<RHI::ITexture> texture;
                usize readback_buffer_offset;
                union
                {
                    BufferReadParams buffer_params;
                    TextureReadParams texture_params;
                };
            };

            Ref<RHI::IDevice> m_device;
            Name m_resource_name;
            Ref<RHI::IBuffer> m_readback_buffer;
            void* m_readback_buffer_mapped = nullptr;
            usize m_readback_buffer_required_size = 0;
            Vector<ResourceReadOp> m_ops;
            Vector<RHI::BufferBarrier> m_buffer_barriers;
            Vector<RHI::TextureBarrier> m_texture_barriers;

            void set_readback_buffer_debug_name(RHI::IBuffer* buffer);

            RV map_data_if_not();

            virtual RHI::IDevice* get_device() override
            {
                return m_device.get();
            }
            virtual void set_name(const c8* name) override
            {
                m_resource_name = name;
            }
            virtual void reset() override;
            virtual usize read_buffer(RHI::IBuffer* buffer, u64 offset, usize size) override;
            virtual usize read_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
                u32 x, u32 y, u32 z, 
                u32 width, u32 height, u32 depth) override;
            virtual RV commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait) override;
            virtual R<const void*> get_buffer_data(usize handle) override;
            virtual R<const void*> get_texture_data(usize handle, u32& out_row_pitch, u32& out_slice_pitch) override;
        };
    }
}