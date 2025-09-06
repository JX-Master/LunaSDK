/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceWriteContext.hpp
* @author JXMaster
* @date 2025/9/4
*/
#pragma once
#include "../ResourceWriteContext.hpp"
#include <Luna/RHI/Device.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        struct ResourceWriteContext : IResourceWriteContext
        {
            lustruct("RHIUtility::ResourceWriteContext", "f47deab1-ebc1-45be-a229-58824379e533");
            luiimpl();

            struct BufferWriteParams
            {
                u64 offset;
                usize size;
            };

            struct TextureWriteParams
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

            struct ResourceWriteOp
            {
                Ref<RHI::IBuffer> buffer;
                Ref<RHI::ITexture> texture;
                Ref<RHI::IBuffer> upload_buffer;
                usize upload_buffer_offset;
                union
                {
                    BufferWriteParams buffer_params;
                    TextureWriteParams texture_params;
                };
            };

            Ref<RHI::IDevice> m_device;
            Name m_resource_name;
            Ref<RHI::IBuffer> m_upload_buffer;
            void* m_upload_buffer_mapped = nullptr;
            usize m_upload_buffer_required_size = 0;
            usize m_last_batch_upload_buffer_required_size = 0;
            Vector<ResourceWriteOp> m_ops;
            Vector<RHI::BufferBarrier> m_buffer_barriers;
            Vector<RHI::TextureBarrier> m_texture_barriers;

            RV reserve_upload_buffer();

            void set_upload_buffer_debug_name(RHI::IBuffer* buffer);

            virtual RHI::IDevice* get_device() override
            {
                return m_device.get();
            }
            virtual void set_name(const c8* name) override
            {
                m_resource_name = name;
            }
            virtual void reset() override;
            virtual R<void*> write_buffer(RHI::IBuffer* buffer, u64 offset, usize size) override;
            virtual R<void*> write_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
                u32 x, u32 y, u32 z,
                u32 width, u32 height, u32 depth,
                u32& out_row_pitch, u32& out_slice_pitch) override;
            virtual RV commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait) override;
        };
    }
}