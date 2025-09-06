/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceWriteContext.cpp
* @author JXMaster
* @date 2025/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "ResourceWriteContext.hpp"
#include <Luna/RHI/Device.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        using namespace RHI;
        void ResourceWriteContext::set_upload_buffer_debug_name(RHI::IBuffer* buffer)
        {
            if(!m_resource_name) return;
            String buf;
            strprintf(buf, "UploadBuffer - %s", m_resource_name.c_str());
            buffer->set_name(buf.c_str());
        }
        void ResourceWriteContext::reset()
        {
            m_ops.clear();
            m_last_batch_upload_buffer_required_size = m_upload_buffer_required_size;
            m_upload_buffer_required_size = 0;
            m_buffer_barriers.clear();
            m_texture_barriers.clear();
            if(m_upload_buffer_mapped)
            {
                m_upload_buffer->unmap(0, 0); // Write ops are canceled, so data can be discarded safely.
                m_upload_buffer_mapped = nullptr;
            }
        }
        RV ResourceWriteContext::reserve_upload_buffer()
        {
            lutry
            {
                u64 upload_buffer_size = 0;
                if(m_upload_buffer)
                {
                    upload_buffer_size = m_upload_buffer->get_desc().size;
                }
                if(m_last_batch_upload_buffer_required_size && upload_buffer_size < m_last_batch_upload_buffer_required_size)
                {
                    luset(m_upload_buffer, m_device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, m_last_batch_upload_buffer_required_size)));
                    set_upload_buffer_debug_name(m_upload_buffer);
                    luexp(m_upload_buffer->map(0, m_last_batch_upload_buffer_required_size, &m_upload_buffer_mapped));
                }
            }
            lucatchret;
            return ok;
        }
        R<void*> ResourceWriteContext::write_buffer(RHI::IBuffer* buffer, u64 offset, usize size)
        {
            void* ret = nullptr;
            lutry
            {
                luexp(reserve_upload_buffer());
                usize upload_buffer_size = 0;
                if(m_upload_buffer)
                {
                    upload_buffer_size = (usize)m_upload_buffer->get_desc().size;
                }
                ResourceWriteOp op;
                op.buffer = buffer;
                op.buffer_params.offset = offset;
                op.buffer_params.size = size;
                if(m_upload_buffer_required_size + size > upload_buffer_size)
                {
                    // Allocate dedicated buffer for this.
                    luset(op.upload_buffer, m_device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, size)));
                    set_upload_buffer_debug_name(op.upload_buffer);
                    op.upload_buffer_offset = 0;
                    luexp(op.upload_buffer->map(0, 0, &ret));
                }
                else
                {
                    // Allocate in the buffer.
                    op.upload_buffer = m_upload_buffer;
                    op.upload_buffer_offset = m_upload_buffer_required_size;
                    ret = (void*)((usize)m_upload_buffer_mapped + m_upload_buffer_required_size);
                }
                m_ops.push_back(move(op));
                m_upload_buffer_required_size += size;
                m_buffer_barriers.emplace_back(buffer, BufferStateFlag::automatic, BufferStateFlag::copy_dest);
            }
            lucatchret;
            return ret;
        }
        R<void*> ResourceWriteContext::write_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
                u32 x, u32 y, u32 z,
                u32 width, u32 height, u32 depth,
                u32& out_row_pitch, u32& out_slice_pitch)
        {
            void* ret = nullptr;
            lutry
            {
                luexp(reserve_upload_buffer());
                usize upload_buffer_size = 0;
                if(m_upload_buffer)
                {
                    upload_buffer_size = (usize)m_upload_buffer->get_desc().size;
                }
                ResourceWriteOp op;
                op.texture = texture;
                op.texture_params.subresource = subresource;
                op.texture_params.x = x;
                op.texture_params.y = y;
                op.texture_params.z = z;
                op.texture_params.width = width;
                op.texture_params.height = height;
                op.texture_params.depth = depth;
                auto texture_desc = texture->get_desc();
                u64 size, alignment, row_pitch, slice_pitch;
                m_device->get_texture_data_placement_info(width, height, depth,
                    texture_desc.format, &size, &alignment, &row_pitch, &slice_pitch);
                op.texture_params.row_pitch = row_pitch;
                op.texture_params.slice_pitch = slice_pitch;
                usize offset = align_upper(m_upload_buffer_required_size, (usize)alignment);
                if(offset + size > upload_buffer_size)
                {
                    // Allocate dedicated buffer for this.
                    luset(op.upload_buffer, m_device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, size)));
                    set_upload_buffer_debug_name(op.upload_buffer);
                    op.upload_buffer_offset = 0;
                    luexp(op.upload_buffer->map(0, 0, &ret));
                }
                else
                {
                    // Allocate in the buffer.
                    op.upload_buffer = m_upload_buffer;
                    op.upload_buffer_offset = offset;
                    ret = (void*)((usize)m_upload_buffer_mapped + offset);
                }
                m_ops.push_back(move(op));
                m_upload_buffer_required_size = offset + (usize)size;
                out_row_pitch = row_pitch;
                out_slice_pitch = slice_pitch;
                m_texture_barriers.emplace_back(texture, subresource, TextureStateFlag::automatic, TextureStateFlag::copy_dest);
            }
            lucatchret;
            return ret;
        }
        RV ResourceWriteContext::commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait)
        {
            lutry
            {
                // Unmap upload heap data.
                if(m_upload_buffer_mapped)
                {
                    m_upload_buffer->unmap(0, USIZE_MAX);
                    m_upload_buffer_mapped = nullptr;
                }
                for(auto& op : m_ops)
                {
                    if(op.upload_buffer != m_upload_buffer)
                    {
                        op.upload_buffer->unmap(0, USIZE_MAX);
                    }
                }
                // Use GPU to copy data.
                command_buffer->begin_copy_pass();
                command_buffer->resource_barrier(m_buffer_barriers.cspan(), m_texture_barriers.cspan());
                for (auto& copy : m_ops)
                {
                    if (copy.buffer)
                    {
                        command_buffer->copy_buffer(copy.buffer, copy.buffer_params.offset, copy.upload_buffer, copy.upload_buffer_offset, copy.buffer_params.size);
                    }
                    else if (copy.texture)
                    {
                        command_buffer->copy_buffer_to_texture(copy.texture, copy.texture_params.subresource, copy.texture_params.x, copy.texture_params.y, copy.texture_params.z, 
                            copy.upload_buffer, copy.upload_buffer_offset, (u32)copy.texture_params.row_pitch, (u32)copy.texture_params.slice_pitch, 
                            copy.texture_params.width, copy.texture_params.height, copy.texture_params.depth);
                    }
                }
                command_buffer->end_copy_pass();
                if(submit_and_wait)
                {
                    // Submit copy command to GPU and wait for completion.
                    luexp(command_buffer->submit({}, {}, true));
                    command_buffer->wait();
                    luexp(command_buffer->reset());
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_RHI_UTILITY_API Ref<IResourceWriteContext> new_resource_write_context(RHI::IDevice* device)
        {
            auto r = new_object<ResourceWriteContext>();
            r->m_device = device;
            return r;
        }
    }
}