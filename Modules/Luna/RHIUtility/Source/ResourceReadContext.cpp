/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResourceReadContext.cpp
* @author JXMaster
* @date 2025/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "ResourceReadContext.hpp"
#include <Luna/RHI/Device.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        using namespace RHI;
        RV ResourceReadContext::map_data_if_not()
        {
            lutry
            {
                if(!m_readback_buffer_mapped)
                {
                    luexp(m_readback_buffer->map(0, USIZE_MAX, &m_readback_buffer_mapped));
                }
            }
            lucatchret;
            return ok;
        }
        void ResourceReadContext::set_readback_buffer_debug_name(RHI::IBuffer* buffer)
        {
            if(!m_resource_name) return;
            String buf;
            strprintf(buf, "UploadBuffer - %s", m_resource_name.c_str());
            buffer->set_name(buf.c_str());
        }
        void ResourceReadContext::reset()
        {
            m_ops.clear();
            m_readback_buffer_required_size = 0;
            m_buffer_barriers.clear();
            m_texture_barriers.clear();
            if(m_readback_buffer_mapped)
            {
                m_readback_buffer->unmap(0, 0);
                m_readback_buffer_mapped = nullptr;
            }
        }
        usize ResourceReadContext::read_buffer(RHI::IBuffer* buffer, u64 offset, usize size)
        {
            usize ret = m_ops.size();
            ResourceReadOp op;
            op.buffer = buffer;
            op.buffer_params.offset = offset;
            op.buffer_params.size = size;
            op.readback_buffer_offset = m_readback_buffer_required_size;
            m_ops.push_back(move(op));
            m_readback_buffer_required_size += size;
            m_buffer_barriers.emplace_back(buffer, BufferStateFlag::automatic, BufferStateFlag::copy_source);
            return ret;
        }
        usize ResourceReadContext::read_texture(RHI::ITexture* texture, RHI::SubresourceIndex subresource, 
            u32 x, u32 y, u32 z, 
            u32 width, u32 height, u32 depth)
        {
            usize ret = m_ops.size();
            ResourceReadOp op;
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
            usize offset = align_upper(m_readback_buffer_required_size, (usize)alignment);
            op.readback_buffer_offset = offset;
            m_ops.push_back(move(op));
            m_readback_buffer_required_size = offset + (usize)size;
            m_texture_barriers.emplace_back(texture, subresource, TextureStateFlag::automatic, TextureStateFlag::copy_source);
            return ret;
        }
        RV ResourceReadContext::commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait)
        {
            lutry
            {
                usize readback_buffer_size = 0;
                if(m_readback_buffer)
                {
                    readback_buffer_size = m_readback_buffer->get_desc().size;
                }
                if(m_readback_buffer_required_size > readback_buffer_size)
                {
                    luset(m_readback_buffer, m_device->new_buffer(MemoryType::readback, BufferDesc(BufferUsageFlag::copy_dest, m_readback_buffer_required_size)));
                    set_readback_buffer_debug_name(m_readback_buffer);
                }
                // Use GPU to copy data.
                command_buffer->begin_copy_pass();
                command_buffer->resource_barrier(m_buffer_barriers.cspan(), m_texture_barriers.cspan());
                for(auto& op : m_ops)
                {
                    if(op.buffer)
                    {
                        command_buffer->copy_buffer(m_readback_buffer, op.readback_buffer_offset, op.buffer, op.buffer_params.offset, op.buffer_params.size);
                    }
                    else if(op.texture)
                    {
                        command_buffer->copy_texture_to_buffer(m_readback_buffer, op.readback_buffer_offset, (u32)op.texture_params.row_pitch, (u32)op.texture_params.slice_pitch,
                            op.texture, op.texture_params.subresource, op.texture_params.x, op.texture_params.y, op.texture_params.z, op.texture_params.width, op.texture_params.height, op.texture_params.depth);
                    }
                }
                command_buffer->end_copy_pass();
                if(submit_and_wait)
                {
                    // Submit copy command to GPU and wait for completion.
                    luexp(command_buffer->submit({}, {}, true));
                    command_buffer->wait();
                    luexp(command_buffer->reset());
                    luexp(map_data_if_not());
                }
            }
            lucatchret;
            return ok;
        }
        R<const void*> ResourceReadContext::get_buffer_data(usize handle)
        {
            const void* ret = nullptr;
            lutry
            {
                luexp(map_data_if_not());
                ret = (const void*)((usize)m_readback_buffer_mapped + m_ops[handle].readback_buffer_offset);
            }
            lucatchret;
            return ret;
        }
        R<const void*> ResourceReadContext::get_texture_data(usize handle, u32& out_row_pitch, u32& out_slice_pitch)
        {
            const void* ret = nullptr;
            lutry
            {
                luexp(map_data_if_not());
                ret = (const void*)((usize)m_readback_buffer_mapped + m_ops[handle].readback_buffer_offset);
                out_row_pitch = m_ops[handle].texture_params.row_pitch;
                out_slice_pitch = m_ops[handle].texture_params.slice_pitch;
            }
            lucatchret;
            return ret;
        }
        LUNA_RHI_UTILITY_API Ref<IResourceReadContext> new_resource_read_context(RHI::IDevice* device)
        {
            auto r = new_object<ResourceReadContext>();
            r->m_device = device;
            return r;
        }
    }
}