/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CopyResourceData.cpp
* @author JXMaster
* @date 2025/9/2
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "CopyResourceData.hpp"
#include <Luna/RHI/Device.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        void ResourceDataCopyContext::init(RHI::IDevice* device)
        {
            m_device = device;
        }

        void ResourceDataCopyContext::reset()
        {
            m_ops.clear();
            m_upload_buffer_required_size = 0;
            m_readback_buffer_required_size = 0;
            m_placements.clear();
            m_buffer_barriers.clear();
            m_texture_barriers.clear();
        }

        RV ResourceDataCopyContext::commit(RHI::ICommandBuffer* command_buffer, bool submit_and_wait)
        {
            lutry
            {
                using namespace RHI;
                m_placements.reserve(m_ops.size());
                for (auto& i : m_ops)
                {
                    if (i.op == ResourceDataCopyOp::read_buffer)
                    {
                        u64 offset = m_readback_buffer_required_size;
                        m_placements.push_back({ offset, 0, 0, Format::unknown });
                        m_readback_buffer_required_size += i.read_buffer_desc.copy_size;
                        m_buffer_barriers.emplace_back(i.read_buffer_desc.src, BufferStateFlag::automatic, BufferStateFlag::copy_source);
                    }
                    else if (i.op == ResourceDataCopyOp::write_buffer)
                    {
                        u64 offset = m_upload_buffer_required_size;
                        m_placements.push_back({ offset, 0, 0, Format::unknown });
                        m_upload_buffer_required_size += i.write_buffer_desc.copy_size;
                        m_buffer_barriers.emplace_back(i.write_buffer_desc.dst, BufferStateFlag::automatic, BufferStateFlag::copy_dest);
                    }
                    else if (i.op == ResourceDataCopyOp::read_texture)
                    {
                        u64 size, alignment, row_pitch, slice_pitch;
                        auto desc = i.read_texture_desc.src->get_desc();
                        m_device->get_texture_data_placement_info(i.read_texture_desc.copy_width, i.read_texture_desc.copy_height, i.read_texture_desc.copy_depth,
                            desc.format, &size, &alignment, &row_pitch, &slice_pitch);
                        u64 offset = align_upper(m_readback_buffer_required_size, alignment);
                        m_placements.push_back({ offset, row_pitch, slice_pitch, desc.format });
                        m_readback_buffer_required_size = offset + size;
                        m_texture_barriers.emplace_back(i.read_texture_desc.src, i.read_texture_desc.src_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_source);
                    }
                    else if (i.op == ResourceDataCopyOp::write_texture)
                    {
                        u64 size, alignment, row_pitch, slice_pitch;
                        auto desc = i.write_texture_desc.dst->get_desc();
                        m_device->get_texture_data_placement_info(i.write_texture_desc.copy_width, i.write_texture_desc.copy_height, i.write_texture_desc.copy_depth,
                            desc.format, &size, &alignment, &row_pitch, &slice_pitch);
                        u64 offset = align_upper(m_upload_buffer_required_size, alignment);
                        m_placements.push_back({ offset, row_pitch, slice_pitch, desc.format });
                        m_upload_buffer_required_size = offset + size;
                        m_texture_barriers.emplace_back(i.write_texture_desc.dst, i.write_texture_desc.dst_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_dest);
                    }
                }
                usize upload_buffer_size = 0;
                usize readback_buffer_size = 0;
                void* upload_data = nullptr;
                if(m_upload_buffer)
                {
                    upload_buffer_size = m_upload_buffer->get_desc().size;
                }
                if(m_readback_buffer)
                {
                    readback_buffer_size = m_readback_buffer->get_desc().size;
                }
                if(m_upload_buffer_required_size > upload_buffer_size)
                {
                    luset(m_upload_buffer, m_device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, m_upload_buffer_required_size)));
                }
                if(m_readback_buffer_required_size > readback_buffer_size)
                {
                    luset(m_readback_buffer, m_device->new_buffer(MemoryType::readback, BufferDesc(BufferUsageFlag::copy_dest, m_readback_buffer_required_size)));
                }
                if(upload_buffer_size)
                {
                    luexp(m_upload_buffer->map(0, 0, &upload_data));
                    // Fill upload data.
                    for (usize i = 0; i < m_ops.size(); ++i)
                    {
                        auto& copy = m_ops[i];
                        auto& placement = m_placements[i];
                        if (copy.op == ResourceDataCopyOp::write_buffer)
                        {
                            memcpy((u8*)upload_data + (usize)placement.offset, copy.write_buffer_desc.src, copy.write_buffer_desc.copy_size);
                        }
                        else if (copy.op == ResourceDataCopyOp::write_texture)
                        {
                            usize copy_size_per_row = bits_per_pixel(placement.pixel_format) * copy.write_texture_desc.copy_width / 8;
                            memcpy_bitmap3d((u8*)upload_data + (usize)placement.offset, copy.write_texture_desc.src,
                                copy_size_per_row, copy.write_texture_desc.copy_height, copy.write_texture_desc.copy_depth,
                                (usize)placement.row_pitch, copy.write_texture_desc.src_row_pitch, (usize)placement.slice_pitch, copy.write_texture_desc.src_slice_pitch);
                        }
                    }
                    m_upload_buffer->unmap(0, USIZE_MAX);
                }
                // Use GPU to copy data.
                command_buffer->begin_copy_pass();
                command_buffer->resource_barrier(m_buffer_barriers.cspan(), m_texture_barriers.cspan());
                for (usize i = 0; i < m_ops.size(); ++i)
                {
                    auto& copy = m_ops[i];
                    auto& placement = m_placements[i];
                    if (copy.op == ResourceDataCopyOp::read_buffer)
                    {
                        auto& desc = copy.read_buffer_desc;
                        command_buffer->copy_buffer(m_readback_buffer, placement.offset, desc.src, desc.src_offset, desc.copy_size);
                    }
                    else if (copy.op == ResourceDataCopyOp::write_buffer)
                    {
                        auto& desc = copy.write_buffer_desc;
                        command_buffer->copy_buffer(desc.dst, desc.dst_offset, m_upload_buffer, placement.offset, desc.copy_size);
                    }
                    else if (copy.op == ResourceDataCopyOp::read_texture)
                    {
                        auto& desc = copy.read_texture_desc;
                        
                        command_buffer->copy_texture_to_buffer(m_readback_buffer, placement.offset, (u32)placement.row_pitch, (u32)placement.slice_pitch,
                            desc.src, desc.src_subresource, desc.src_x, desc.src_y, desc.src_z, desc.copy_width, desc.copy_height, desc.copy_depth);
                    }
                    else if (copy.op == ResourceDataCopyOp::write_texture)
                    {
                        auto& desc = copy.write_texture_desc;
                        command_buffer->copy_buffer_to_texture(desc.dst, desc.dst_subresource, desc.dst_x, desc.dst_y, desc.dst_z, 
                            m_upload_buffer, placement.offset, (u32)placement.row_pitch, (u32)placement.slice_pitch, desc.copy_width, desc.copy_height, desc.copy_depth);
                    }
                }
                command_buffer->end_copy_pass();
                if(submit_and_wait)
                {
                    // Submit copy command to GPU and wait for completion.
                    luexp(command_buffer->submit({}, {}, true));
                    command_buffer->wait();
                    luexp(command_buffer->reset());
                    // Read data for read calls.
                    luexp(copy_read_back_data());
                }
            }
            lucatchret;
            return ok;
        }
        RV ResourceDataCopyContext::copy_read_back_data()
        {
            lutry
            {
                if (m_readback_buffer_required_size)
                {
                    void* readback_data = nullptr;
                    luexp(m_readback_buffer->map(0, USIZE_MAX, &readback_data));
                    for (usize i = 0; i < m_ops.size(); ++i)
                    {
                        auto& copy = m_ops[i];
                        auto& placement = m_placements[i];
                        if (copy.op == ResourceDataCopyOp::read_buffer)
                        {
                            memcpy(copy.read_buffer_desc.dst, (u8*)readback_data + (usize)placement.offset, copy.read_buffer_desc.copy_size);
                        }
                        else if (copy.op == ResourceDataCopyOp::read_texture)
                        {
                            usize copy_size_per_row = bits_per_pixel(placement.pixel_format) * copy.read_texture_desc.copy_width / 8;
                            memcpy_bitmap3d(copy.read_texture_desc.dst, (u8*)readback_data + (usize)placement.offset,
                                copy_size_per_row, copy.read_texture_desc.copy_height, copy.read_texture_desc.copy_depth,
                                copy.read_texture_desc.dst_row_pitch, (usize)placement.row_pitch, copy.read_texture_desc.dst_slice_pitch, (usize)placement.slice_pitch);
                        }
                    }
                    m_readback_buffer->unmap(0, 0);
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_RHI_UTILITY_API Ref<IResourceDataCopyContext> new_resource_data_copy_context(RHI::IDevice* device)
        {
            Ref<ResourceDataCopyContext> ctx = new_object<ResourceDataCopyContext>();
            ctx->init(device);
            return ctx;
        }
    }
}