/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CopyResourceData.cpp
* @author JXMaster
* @date 2023/6/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../Utility.hpp"
#include "../Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct CopyBufferPlacementInfo
		{
			u64 offset;
			u64 row_pitch;
			u64 slice_pitch;
			Format pixel_format;
		};
        LUNA_RHI_API RV copy_resource_data(ICommandBuffer* command_buffer, Span<const CopyResourceData> copies)
        {
            auto dev = command_buffer->get_device();
			// Allocate one upload and one readback heap.
			u64 upload_buffer_size = 0;
			u64 readback_buffer_size = 0;
			Vector<CopyBufferPlacementInfo> placements;
            Vector<BufferBarrier> buffer_barriers;
            Vector<TextureBarrier> texture_barriers;
			placements.reserve(copies.size());
			for (auto& i : copies)
			{
				if (i.op == ResourceDataCopyOp::read_buffer)
				{
					u64 offset = readback_buffer_size;
					placements.push_back({ offset, 0, 0, Format::unknown });
					readback_buffer_size += i.read_buffer_desc.copy_size;
					buffer_barriers.emplace_back(i.read_buffer_desc.src, BufferStateFlag::automatic, BufferStateFlag::copy_source);
				}
				else if (i.op == ResourceDataCopyOp::write_buffer)
				{
					u64 offset = upload_buffer_size;
					placements.push_back({ offset, 0, 0, Format::unknown });
					upload_buffer_size += i.write_buffer_desc.copy_size;
					buffer_barriers.emplace_back(i.write_buffer_desc.dst, BufferStateFlag::automatic, BufferStateFlag::copy_dest);
				}
				else if (i.op == ResourceDataCopyOp::read_texture)
				{
					u64 size, alignment, row_pitch, slice_pitch;
					auto desc = i.read_texture_desc.src->get_desc();
					dev->get_texture_data_placement_info(i.read_texture_desc.copy_width, i.read_texture_desc.copy_height, i.read_texture_desc.copy_depth,
						desc.format, &size, &alignment, &row_pitch, &slice_pitch);
					u64 offset = align_upper(readback_buffer_size, alignment);
					placements.push_back({ offset, row_pitch, slice_pitch, desc.format });
					readback_buffer_size = offset + size;
					texture_barriers.emplace_back(i.read_texture_desc.src, i.read_texture_desc.src_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_source);
				}
				else if (i.op == ResourceDataCopyOp::write_texture)
				{
					u64 size, alignment, row_pitch, slice_pitch;
					auto desc = i.write_texture_desc.dst->get_desc();
					dev->get_texture_data_placement_info(i.write_texture_desc.copy_width, i.write_texture_desc.copy_height, i.write_texture_desc.copy_depth,
						desc.format, &size, &alignment, &row_pitch, &slice_pitch);
					u64 offset = align_upper(upload_buffer_size, alignment);
					placements.push_back({ offset, row_pitch, slice_pitch, desc.format });
					upload_buffer_size = offset + size;
					texture_barriers.emplace_back(i.write_texture_desc.dst, i.write_texture_desc.dst_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_dest);
				}
			}
			lutry
			{
				Ref<IBuffer> upload_buffer;
				Ref<IBuffer> readback_buffer;
				void* upload_data = nullptr;
				void* readback_data = nullptr;
				if (upload_buffer_size)
				{
					luset(upload_buffer, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, upload_buffer_size)));
					luexp(upload_buffer->map(0, 0, &upload_data));
					// Fill upload data.
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
                        auto& placement = placements[i];
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
					upload_buffer->unmap(0, USIZE_MAX);
				}
				if (readback_buffer_size)
				{
					luset(readback_buffer, dev->new_buffer(MemoryType::readback, BufferDesc(BufferUsageFlag::copy_dest, readback_buffer_size)));
				}
				// Use GPU to copy data.
				command_buffer->begin_copy_pass();
                command_buffer->resource_barrier({buffer_barriers.data(), buffer_barriers.size()}, {texture_barriers.data(), texture_barriers.size()});
				for (usize i = 0; i < copies.size(); ++i)
				{
					auto& copy = copies[i];
                    auto& placement = placements[i];
					if (copy.op == ResourceDataCopyOp::read_buffer)
					{
                        auto& desc = copy.read_buffer_desc;
                        command_buffer->copy_buffer(readback_buffer, placement.offset, desc.src, desc.src_offset, desc.copy_size);
					}
					else if (copy.op == ResourceDataCopyOp::write_buffer)
					{
                        auto& desc = copy.write_buffer_desc;
                        command_buffer->copy_buffer(desc.dst, desc.dst_offset, upload_buffer, placement.offset, desc.copy_size);
					}
					else if (copy.op == ResourceDataCopyOp::read_texture)
					{
                        auto& desc = copy.read_texture_desc;
                        
                        command_buffer->copy_texture_to_buffer(readback_buffer, placement.offset, (u32)placement.row_pitch, (u32)placement.slice_pitch,
                            desc.src, desc.src_subresource, desc.src_x, desc.src_y, desc.src_z, desc.copy_width, desc.copy_height, desc.copy_depth);
					}
					else if (copy.op == ResourceDataCopyOp::write_texture)
					{
                        auto& desc = copy.write_texture_desc;
                        command_buffer->copy_buffer_to_texture(desc.dst, desc.dst_subresource, desc.dst_x, desc.dst_y, desc.dst_z, 
                            upload_buffer, placement.offset, (u32)placement.row_pitch, (u32)placement.slice_pitch, desc.copy_width, desc.copy_height, desc.copy_depth);
					}
				}
				command_buffer->end_copy_pass();
				// Submit copy command to GPU and wait for completion.
                luexp(command_buffer->submit({}, {}, true));
                command_buffer->wait();
                luexp(command_buffer->reset());
				// Read data for read calls.
				if (readback_buffer)
				{
					luexp(readback_buffer->map(0, USIZE_MAX, &readback_data));
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
                        auto& placement = placements[i];
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
					readback_buffer->unmap(0, 0);
				}
			}
			lucatchret;
			return ok;
        }
    }
}
