/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.cpp
* @author JXMaster
* @date 2019/8/10
* @brief D3D12 implementation of Resource Object
*/
#include "Resource.hpp"

#ifdef LUNA_RHI_D3D12

#include "../../RHI.hpp"

namespace Luna
{
	namespace RHI
	{
		inline u32 calc_mip_levels(u32 width, u32 height, u32 depth)
		{
			return 1 + (u32)floorf(log2f((f32)max(width, max(height, depth))));
		}

		void Resource::set_desc(const ResourceDesc& desc)
		{
			m_desc = desc;
			if (m_desc.type == ResourceType::buffer)
			{
				m_desc.pixel_format = Format::unknown;
				m_desc.height = 1;
				m_desc.depth_or_array_size = 1;
				m_desc.mip_levels = 1;
				m_desc.sample_count = 1;
				m_desc.sample_quality = 0;
			}
			else if (m_desc.type == ResourceType::texture_1d)
			{
				m_desc.height = 1;
				m_desc.sample_count = 1;
				m_desc.sample_quality = 0;
			}
			else if (m_desc.type == ResourceType::texture_3d)
			{
				m_desc.sample_count = 1;
				m_desc.sample_quality = 0;
			}
			if (!m_desc.mip_levels)
			{
				if (m_desc.type != ResourceType::texture_3d)
				{
					m_desc.mip_levels = calc_mip_levels((u32)desc.width_or_buffer_size, desc.height, 1);
				}
				else
				{
					m_desc.mip_levels = calc_mip_levels((u32)desc.width_or_buffer_size, desc.height, desc.depth_or_array_size);
				}
			}
		}

		RV Resource::init_as_committed(const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			set_desc(desc);
			D3D12_HEAP_PROPERTIES hp = encode_heap_properties(m_device.as<Device>(), m_desc.heap_type);
			D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
			D3D12_RESOURCE_DESC rd = encode_resource_desc(m_desc);

			D3D12_CLEAR_VALUE* pcv = nullptr;
			D3D12_CLEAR_VALUE cv;
			if (optimized_clear_value)
			{
				cv.Format = encode_pixel_format(optimized_clear_value->pixel_format);
				if (optimized_clear_value->type == ClearValueType::color)
				{
					cv.Color[0] = optimized_clear_value->color[0];
					cv.Color[1] = optimized_clear_value->color[1];
					cv.Color[2] = optimized_clear_value->color[2];
					cv.Color[3] = optimized_clear_value->color[3];
				}
				else
				{
					cv.DepthStencil.Depth = optimized_clear_value->depth_stencil.depth;
					cv.DepthStencil.Stencil = optimized_clear_value->depth_stencil.stencil;
				}
				pcv = &cv;
			}
			D3D12_RESOURCE_STATES s;
			if (desc.heap_type == ResourceHeapType::upload)
			{
				s = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (desc.heap_type == ResourceHeapType::readback)
			{
				s = D3D12_RESOURCE_STATE_COPY_DEST;
			}
			else
			{
				s = D3D12_RESOURCE_STATE_COMMON;
			}
			if (FAILED(m_device->m_device->CreateCommittedResource(&hp, flags, &rd, s,
				pcv, IID_PPV_ARGS(&m_res))))
			{
				return BasicError::bad_platform_call();
			}
			return post_init();
		}

		RV Resource::init_as_placed(ID3D12Heap* heap, UINT64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			set_desc(desc);
			D3D12_RESOURCE_DESC rd = encode_resource_desc(m_desc);
			D3D12_CLEAR_VALUE* pcv = nullptr;
			D3D12_CLEAR_VALUE cv;
			if (optimized_clear_value)
			{
				cv.Format = encode_pixel_format(optimized_clear_value->pixel_format);
				if (optimized_clear_value->type == ClearValueType::color)
				{
					cv.Color[0] = optimized_clear_value->color[0];
					cv.Color[1] = optimized_clear_value->color[1];
					cv.Color[2] = optimized_clear_value->color[2];
					cv.Color[3] = optimized_clear_value->color[3];
				}
				else
				{
					cv.DepthStencil.Depth = optimized_clear_value->depth_stencil.depth;
					cv.DepthStencil.Stencil = optimized_clear_value->depth_stencil.stencil;
				}
				pcv = &cv;
			}
			D3D12_RESOURCE_STATES s;
			if (desc.heap_type == ResourceHeapType::upload)
			{
				s = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (desc.heap_type == ResourceHeapType::readback)
			{
				s = D3D12_RESOURCE_STATE_COPY_DEST;
			}
			else
			{
				s = D3D12_RESOURCE_STATE_COMMON;
			}
			if (FAILED(m_device->m_device->CreatePlacedResource(heap, heap_offset, &rd, s, pcv, IID_PPV_ARGS(&m_res))))
			{
				return BasicError::bad_platform_call();
			}
			return post_init();
		}

		RV Resource::post_init()
		{
			if (m_desc.type != ResourceType::buffer && ((m_desc.flags & ResourceFlag::simultaneous_access) == ResourceFlag::none))
			{
				m_states.resize(count_subresources(), ResourceState::common);
			}
			// Allocate affiliate slots when needed. The real resource will be allocated on mapping.
			if ((m_desc.heap_type == ResourceHeapType::shared || m_desc.heap_type == ResourceHeapType::shared_upload) &&
				!m_device->m_architecture.UMA)
			{
				usize subresources = (m_desc.type == ResourceType::texture_3d) ?
					m_desc.mip_levels : m_desc.mip_levels * m_desc.depth_or_array_size;
				m_affiliate_resources.resize(subresources);

				for (usize i = 0; i < m_affiliate_resources.size(); ++i)
				{
					u32 mip_slice;
					u32 array_slice;
					calc_mip_array_slice((u32)i, m_desc.mip_levels, mip_slice, array_slice);
					auto& res = m_affiliate_resources[i];
					if (m_desc.type == ResourceType::buffer)
					{
						res.m_size = m_desc.width_or_buffer_size;
					}
					else if (m_desc.type == ResourceType::texture_1d)
					{
						res.m_width = max<u64>(m_desc.width_or_buffer_size >> mip_slice, 1);
						u32 row_pitch = (u32)align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
						res.m_size = row_pitch;
					}
					else if (m_desc.type == ResourceType::texture_2d)
					{
						res.m_width = max<u64>(m_desc.width_or_buffer_size >> mip_slice, 1);
						res.m_height = max<u32>(m_desc.height >> mip_slice, 1);
						u32 row_pitch = (u32)align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
						res.m_size = row_pitch * res.m_height;
					}
					else if (m_desc.type == ResourceType::texture_3d)
					{
						res.m_width = max<u64>(m_desc.width_or_buffer_size >> mip_slice, 1);
						res.m_height = max<u32>(m_desc.height >> mip_slice, 1);
						res.m_depth = max<u32>(m_desc.depth_or_array_size >> mip_slice, 1);
						u32 row_pitch = (u32)align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
						res.m_size = row_pitch * res.m_height * res.m_depth;
					}
					luassert(res.m_size != 0);
				}

				// Resources used to copy data between system/video memory.
				if (FAILED(m_device->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_copy_ca))))
				{
					return BasicError::bad_platform_call();
				}
				if (FAILED(m_device->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_copy_fence))))
				{
					return BasicError::bad_platform_call();
				}
				m_copy_event = ::CreateEventA(NULL, TRUE, FALSE, NULL);
				if (m_copy_event == NULL)
				{
					return BasicError::bad_platform_call();
				}
				m_copy_event_value = 0;
			}
			return ok;
		}

		RV Resource::map_subresource(u32 subresource, bool load_data, void** out_data)
		{
			lutsassert(this);
			if (m_affiliate_resources.empty())
			{
				D3D12_RANGE range;
				range.Begin = 0;
				range.End = 0;
				D3D12_RANGE* pRange = (load_data) ? nullptr : &range;
				if (FAILED(m_res->Map(subresource, pRange, out_data)))
				{
					return BasicError::bad_platform_call();
				}
				return ok;
			}
			else
			{
				auto& res = m_affiliate_resources[subresource];
				u32 after = atom_inc_u32(&res.m_ref_count);
				if (after == 1)
				{
					// Create mapped resources.
					D3D12_RESOURCE_DESC desc;
					desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
					desc.Alignment = 0;
					desc.Width = res.m_size;
					desc.Height = 1;
					desc.DepthOrArraySize = 1;
					desc.MipLevels = 1;
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
					desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
					desc.Flags = D3D12_RESOURCE_FLAG_NONE;
					D3D12_HEAP_PROPERTIES heap;
					heap.Type = D3D12_HEAP_TYPE_CUSTOM;
					if (m_desc.heap_type == ResourceHeapType::shared)
					{
						heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
					}
					else if (m_desc.heap_type == ResourceHeapType::shared_upload)
					{
						heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
					}
					else lupanic();
					heap.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
					heap.CreationNodeMask = 0;
					heap.VisibleNodeMask = 0;
					if (FAILED(m_device->m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
						D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&res.m_res))))
					{
						atom_dec_u32(&res.m_ref_count);
						return BasicError::bad_platform_call();
					}
					if (m_desc.heap_type == ResourceHeapType::shared && load_data)
					{
						// Read data back from video memory.
						ComPtr<ID3D12GraphicsCommandList> cmd;
						bool failed = FAILED(m_device->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copy_ca.Get(), NULL, IID_PPV_ARGS(&cmd)));
						if (!failed)
						{
							if (m_desc.type == ResourceType::buffer)
							{
								cmd->CopyBufferRegion(res.m_res.Get(), 0, m_res.Get(), 0, res.m_size);
							}
							else
							{
								D3D12_TEXTURE_COPY_LOCATION src, dst;
								dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
								dst.pResource = res.m_res.Get();
								dst.PlacedFootprint.Offset = 0;
								dst.PlacedFootprint.Footprint.Width = (UINT)res.m_width;
								dst.PlacedFootprint.Footprint.Height = (UINT)res.m_height;
								dst.PlacedFootprint.Footprint.Depth = (UINT)res.m_depth;
								dst.PlacedFootprint.Footprint.Format = encode_pixel_format(m_desc.pixel_format);
								dst.PlacedFootprint.Footprint.RowPitch = (UINT)align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
								src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
								src.pResource = m_res.Get();
								src.SubresourceIndex = subresource;
								D3D12_BOX box;
								box.left = 0;
								box.right = (UINT)res.m_width;
								box.top = 0;
								box.bottom = (UINT)res.m_height;
								box.front = 0;
								box.back = (UINT)res.m_depth;
								cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);
							}
							failed = FAILED(cmd->Close());
						}
						if (!failed)
						{
							m_device->m_internal_copy_queue->ExecuteCommandLists(1, (ID3D12CommandList**)cmd.GetAddressOf());
							// Accroding to D3D12 Docs, any resources being accessed on a Copy queue will decay to Common state
							// after it is used. So we modify the resource state here.
							if (!m_states.empty())
							{
								m_states[subresource] = ResourceState::common;
							}
							++m_copy_event_value;
							failed = FAILED(m_copy_fence->SetEventOnCompletion(m_copy_event_value, m_copy_event));
						}
						if (!failed)
						{
							failed = FAILED(m_device->m_internal_copy_queue->Signal(m_copy_fence.Get(), m_copy_event_value));
						}
						if (!failed)
						{
							::WaitForSingleObject(m_copy_event, INFINITE);
							failed = FAILED(m_copy_ca->Reset());
						}
						if (failed)
						{
							res.m_res = nullptr;
							atom_dec_u32(&res.m_ref_count);
							return BasicError::bad_platform_call();
						}
					}
					D3D12_RANGE range;
					range.Begin = 0;
					range.End = 0;
					D3D12_RANGE* pRange = (load_data) ? nullptr : &range;
					if (FAILED(res.m_res->Map(0, pRange, &res.m_mapped)))
					{
						res.m_mapped = nullptr;
						res.m_res = nullptr;
						atom_dec_u32(&res.m_ref_count);
						return BasicError::bad_platform_call();
					}
				}
				if (out_data) *out_data = res.m_mapped;
				return ok;
			}
		}
		void Resource::unmap_subresource(u32 subresource, bool store_data)
		{
			lutsassert(this);
			if (m_affiliate_resources.empty())
			{
				D3D12_RANGE range;
				range.Begin = 0;
				range.End = 0;
				D3D12_RANGE* pRange = (store_data) ? nullptr : &range;
				m_res->Unmap(subresource, pRange);
			}
			else
			{
				auto& res = m_affiliate_resources[subresource];
				u32 after = atom_dec_u32(&res.m_ref_count);
				{
					if (after == 0)
					{
						D3D12_RANGE range;
						range.Begin = 0;
						range.End = 0;
						D3D12_RANGE* pRange = (store_data) ? nullptr : &range;
						res.m_res->Unmap(subresource, pRange);
						if (store_data)
						{
							// Write the data back to the resource.
							ComPtr<ID3D12GraphicsCommandList> cmd;
							bool failed = FAILED(m_device->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copy_ca.Get(), NULL, IID_PPV_ARGS(&cmd)));
							if (!failed)
							{
								if (m_desc.type == ResourceType::buffer)
								{
									cmd->CopyBufferRegion(m_res.Get(), 0, res.m_res.Get(), 0, res.m_size);
								}
								else
								{
									D3D12_TEXTURE_COPY_LOCATION src, dst;
									dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
									dst.pResource = m_res.Get();
									dst.SubresourceIndex = subresource;
									src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
									src.pResource = res.m_res.Get();
									src.PlacedFootprint.Offset = 0;
									src.PlacedFootprint.Footprint.Width = (UINT)res.m_width;
									src.PlacedFootprint.Footprint.Height = (UINT)res.m_height;
									src.PlacedFootprint.Footprint.Depth = (UINT)res.m_depth;
									src.PlacedFootprint.Footprint.Format = encode_pixel_format(m_desc.pixel_format);
									src.PlacedFootprint.Footprint.RowPitch = (UINT)align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
									D3D12_BOX box;
									box.left = 0;
									box.right = (UINT)res.m_width;
									box.top = 0;
									box.bottom = (UINT)res.m_height;
									box.front = 0;
									box.back = (UINT)res.m_depth;
									cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);
								}
								failed = FAILED(cmd->Close());
							}
							if (!failed)
							{
								m_device->m_internal_copy_queue->ExecuteCommandLists(1, (ID3D12CommandList**)cmd.GetAddressOf());
								// Accroding to D3D12 Docs, any resources being accessed on a Copy queue will decay to Common state
								// after it is used. So we modify the resource state here.
								if (!m_states.empty())
								{
									m_states[subresource] = ResourceState::common;
								}
								++m_copy_event_value;
								failed = FAILED(m_copy_fence->SetEventOnCompletion(m_copy_event_value, m_copy_event));
							}
							if (!failed)
							{
								failed = FAILED(m_device->m_internal_copy_queue->Signal(m_copy_fence.Get(), m_copy_event_value));
							}
							if (!failed)
							{
								::WaitForSingleObject(m_copy_event, INFINITE);
								failed = FAILED(m_copy_ca->Reset());
								::ResetEvent(m_copy_event);
							}
						}
						// Release the affiliate resource.
						res.m_res = nullptr;
						res.m_mapped = nullptr;
					}
				}
			}
		}
		RV Resource::read_subresource(void* dest, u32 dest_row_pitch, u32 dest_depth_pitch, u32 subresource, const BoxU& read_box)
		{
			lutsassert(this);
			D3D12_BOX b;
			b.left = read_box.offset_x;
			b.right = read_box.offset_x + read_box.width;
			b.top = read_box.offset_y;
			b.bottom = read_box.offset_y + read_box.height;
			b.front = read_box.offset_z;
			b.back = read_box.offset_z + read_box.depth;
			if (m_affiliate_resources.empty())
			{
				return SUCCEEDED(m_res->ReadFromSubresource(dest, dest_row_pitch, dest_depth_pitch, subresource, &b)) ? ok : BasicError::failure();
			}
			else
			{
				auto& res = m_affiliate_resources[subresource];
				const usize offsetx = read_box.offset_x;
				const usize offsety = read_box.offset_y;
				const usize offsetz = read_box.offset_z;
				const usize copy_size_per_row = read_box.width * bits_per_pixel(m_desc.pixel_format) / 8;
				const usize src_row_pitch = align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
				const usize src_slice_pitch = src_row_pitch * res.m_height;
				const usize offset = src_slice_pitch * offsetz + src_row_pitch * offsety + bits_per_pixel(m_desc.pixel_format) * offsetx / 8;
				void* offsetted = (void*)((usize)res.m_mapped + offset);
				memcpy_bitmap3d(dest, offsetted, copy_size_per_row, read_box.height, read_box.depth,
					dest_row_pitch, src_row_pitch, dest_depth_pitch, src_slice_pitch);
				return ok;
			}
		}
		RV Resource::write_subresource(u32 subresource, const void* src, u32 src_row_pitch, u32 src_depth_pitch, const BoxU& write_box)
		{
			lutsassert(this);
			D3D12_BOX b;
			b.left = write_box.offset_x;
			b.right = write_box.offset_x + write_box.width;
			b.top = write_box.offset_y;
			b.bottom = write_box.offset_y + write_box.height;
			b.front = write_box.offset_z;
			b.back = write_box.offset_z + write_box.depth;
			if (m_affiliate_resources.empty())
			{
				return SUCCEEDED(m_res->WriteToSubresource(subresource, &b, src, src_row_pitch, src_depth_pitch)) ? ok : BasicError::failure();
			}
			else
			{
				auto& res = m_affiliate_resources[subresource];
				const usize offsetx = write_box.offset_x;
				const usize offsety = write_box.offset_y;
				const usize offsetz = write_box.offset_z;
				const usize copy_size_per_row = write_box.width * bits_per_pixel(m_desc.pixel_format) / 8;
				const usize dest_row_pitch = align_upper((u64)bits_per_pixel(m_desc.pixel_format) * res.m_width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
				const usize dest_slice_pitch = dest_row_pitch * res.m_height;
				const usize offset = dest_slice_pitch * offsetz + dest_row_pitch * offsety + bits_per_pixel(m_desc.pixel_format) * offsetx / 8;
				void* offsetted = (void*)((usize)res.m_mapped + offset);
				memcpy_bitmap3d(offsetted, src, copy_size_per_row, write_box.height, write_box.depth,
					dest_row_pitch, src_row_pitch, dest_slice_pitch, src_depth_pitch);
				return ok;
			}
		}
	}
}

#endif