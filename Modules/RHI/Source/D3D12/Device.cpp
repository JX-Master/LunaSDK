/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.cpp
* @author JXMaster
* @date 2019/7/17
*/
#include "Device.hpp"

#ifdef LUNA_RHI_D3D12

#include "ResourceHeap.hpp"
#include "PipelineState.hpp"
#include "CommandQueue.hpp"
#include "Resource.hpp"
#include "DescriptorSet.hpp"
#include "ShaderInputLayout.hpp"
#include "DescriptorSetLayout.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "QueryHeap.hpp"
#include "CommandBuffer.hpp"

namespace Luna
{
	namespace RHI
	{
		RV ShaderSourceDescriptorHeap::init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
		{
			m_type = desc.Type;
			m_heap_size = desc.NumDescriptors;
			m_mutex = new_mutex();
			if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap))))
			{
				return BasicError::bad_platform_call();
			}
			m_cpu_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
			m_gpu_handle = m_heap->GetGPUDescriptorHandleForHeapStart();
			m_descriptor_size = device->GetDescriptorHandleIncrementSize(m_type);
			FreeRange range;
			range.offset = 0;
			range.size = m_heap_size;
			m_free_ranges.push_back(range);
			return ok;
		}

		void ShaderSourceDescriptorHeap::internal_free_descs(u32 offset, u32 count)
		{
			// Insert before this iterator. May be `end`.
			auto after = m_free_ranges.begin();
			while (after != m_free_ranges.end())
			{
				if (after->offset >= (offset + count)) break;
				++after;
			}
			if(after != m_free_ranges.begin())
			{
				// Merge to before.
				auto before = after;
				--before;
				if ((before->offset + before->size) == offset)
				{
					before->size += count;
					if (after != m_free_ranges.end() && (before->offset + before->size == after->offset))
					{
						// Merge before to after.
						before->size += after->size;
						m_free_ranges.erase(after);
					}
					return;
				}
			}
			if ((after != m_free_ranges.end()) && ((offset + count) == after->offset))
			{
				// Merge to after.
				after->offset = offset;
				after->size += count;
				return;
			}
			// Cannot merge, insert a new node.
			FreeRange range;
			range.offset = offset;
			range.size = count;
			m_free_ranges.insert(after, range);
		}

		u32 ShaderSourceDescriptorHeap::allocate_descs(u32 count)
		{
			luassert(count);
			MutexGuard guard(m_mutex);
			for (auto iter = m_free_ranges.begin(); iter != m_free_ranges.end(); ++iter)
			{
				if (iter->size >= count)
				{
					u32 ret = iter->offset;
					if (iter->size > count)
					{
						iter->size -= count;
						iter->offset += count;
					}
					else
					{
						m_free_ranges.erase(iter);
					}
					return ret;
				}
			}
			lupanic_msg("Out of descriptors.");
			return U32_MAX;
		}
		void ShaderSourceDescriptorHeap::free_descs(u32 offset, u32 count)
		{
			MutexGuard guard(m_mutex);
			internal_free_descs(offset, count);
		}

		void RenderTargetDescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
		{
			m_device = device;
			m_type = type;
			m_mutex = new_mutex();
			m_descriptor_size = device->GetDescriptorHandleIncrementSize(type);
		}
		R<ComPtr<ID3D12DescriptorHeap>> RenderTargetDescriptorHeap::allocate_view()
		{
			MutexGuard guard(m_mutex);
			if (m_free_views.empty())
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc;
				desc.Type = m_type;
				desc.NodeMask = 0;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				ComPtr<ID3D12DescriptorHeap> ret;
				if (FAILED(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ret))))
				{
					return BasicError::bad_platform_call();
				}
				return ret;
			}
			else
			{
				ComPtr<ID3D12DescriptorHeap> ret = m_free_views.front();
				m_free_views.pop_front();
				return ret;
			}
		}
		void RenderTargetDescriptorHeap::free_view(ID3D12DescriptorHeap* view)
		{
			MutexGuard guard(m_mutex);
			ComPtr<ID3D12DescriptorHeap> v{ view };
			m_free_views.push_back(move(v));
		}
		Device::~Device()
		{
#ifdef LUNA_DEBUG_LEVEL_DEBUG
			ID3D12DebugDevice* pDebugDevice = nullptr;
			HRESULT hr = m_device->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(&pDebugDevice));
			if (SUCCEEDED(hr))
			{
				hr = pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
			}
			if (pDebugDevice != nullptr)
			{
				pDebugDevice->Release();
			}
#endif
		}

		RV Device::init(ID3D12Device* dev)
		{
			m_device = dev;
			HRESULT hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_feature_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
			if (FAILED(hr)) return BasicError::bad_platform_call();
			m_architecture.NodeIndex = 0;
			hr = m_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &m_architecture, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE));
			if (FAILED(hr)) return BasicError::bad_platform_call();
			D3D12_COMMAND_QUEUE_DESC desc;
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 0;
			if (FAILED(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_internal_copy_queue))))
			{
				return BasicError::bad_platform_call();
			}
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc;
				desc.NodeMask = 0;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				auto r = m_cbv_srv_uav_heap.init(m_device.Get(), desc);
				if (failed(r))
				{
					return r.errcode();
				}
				desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
				r = m_sampler_heap.init(m_device.Get(), desc);
				if (failed(r))
				{
					return r.errcode();
				}
				m_rtv_heap.init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				m_dsv_heap.init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			}
			return ok;
		}
		usize Device::get_constant_buffer_data_alignment()
		{
			return 256;
		}
		void Device::get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch)
		{
			u64 bpp = bits_per_pixel(format);
			u64 row = align_upper(bpp * width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			if(row_pitch) *row_pitch = row;
			u64 slice = row * height;
			if(slice_pitch) *slice_pitch = slice;
			u64 sz = align_upper(slice * depth, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
			if(size) *size = sz;
			if(alignment) *alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
		}
		u64 Device::get_resource_size(const ResourceDesc& desc, u64* out_alignment)
		{
			D3D12_RESOURCE_DESC res_desc = encode_resource_desc(validate_resource_desc(desc));
			D3D12_RESOURCE_ALLOCATION_INFO info = m_device->GetResourceAllocationInfo(0, 1, &res_desc);
			if (out_alignment) *out_alignment = info.Alignment;
			return info.SizeInBytes;
		}
		R<Ref<IResource>> Device::new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<Resource> res = new_object<Resource>();
			res->m_device = this;
			RV r = res->init_as_committed(desc, optimized_clear_value);
			if (!r.valid())
			{
				return r.errcode();
			}
			return res;
		}
		R<Ref<IResourceHeap>> Device::new_resource_heap(const ResourceHeapDesc& desc)
		{
			Ref<ResourceHeap> heap = new_object<ResourceHeap>();
			heap->m_device = this;
			RV r = heap->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return heap;
		}
		R<Ref<IShaderInputLayout>> Device::new_shader_input_layout(const ShaderInputLayoutDesc& desc)
		{
			Ref<ShaderInputLayout> slayout = new_object<ShaderInputLayout>();
			slayout->m_device = this;
			lutry
			{
				luexp(slayout->init(desc));
			}
			lucatchret;
			return slayout;
		}
		R<Ref<IPipelineState>> Device::new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc)
		{
			Ref<PipelineState> s = new_object<PipelineState>(this);
			if (!s->init_graphic(desc))
			{
				return BasicError::failure();
			}
			return s;
		}
		R<Ref<IPipelineState>> Device::new_compute_pipeline_state(const ComputePipelineStateDesc& desc)
		{
			Ref<PipelineState> s = new_object<PipelineState>(this);
			if (!s->init_compute(desc))
			{
				return BasicError::failure();
			}
			return s;
		}
		R<Ref<IDescriptorSetLayout>> Device::new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
		{
			Ref<DescriptorSetLayout> ret = new_object<DescriptorSetLayout>();
			ret->m_device = this;
			ret->init(desc);
			return ret;
		}
		R<Ref<IDescriptorSet>> Device::new_descriptor_set(DescriptorSetDesc& desc)
		{
			Ref<DescriptorSet> ds = new_object<DescriptorSet>();
			ds->m_device = this;
			RV r = ds->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return ds;
		}
		R<Ref<ICommandQueue>> Device::new_command_queue(CommandQueueType type)
		{
			Ref<CommandQueue> q = new_object<CommandQueue>(this);
			RV r = q->init(type);
			if (!r.valid())
			{
				return r.errcode();
			}
			return q;
		}
		R<Ref<IRenderTargetView>> Device::new_render_target_view(IResource* res, const RenderTargetViewDesc* desc)
		{
			lucheck_msg(res, "\"res\" was nullptr");
			Ref<RenderTargetView> view = new_object<RenderTargetView>();
			view->m_device = this;
			RV r = view->init(res, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IDepthStencilView>> Device::new_depth_stencil_view(IResource* res, const DepthStencilViewDesc* desc)
		{
			lucheck_msg(res, "\"res\" was nullptr");
			Ref<DepthStencilView> view = new_object<DepthStencilView>();
			view->m_device = this;
			RV r = view->init(res, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IQueryHeap>> Device::new_query_heap(const QueryHeapDesc& desc)
		{
			Ref<QueryHeap> heap = new_object<QueryHeap>();
			heap->m_device = this;
			RV r = heap->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return heap;
		}
		struct CopyBufferPlacementInfo
		{
			u64 offset;
			u64 row_pitch;
			u64 depth_pitch;
			Format pixel_format;
		};
		RV Device::copy_resource(Span<const ResourceCopyDesc> copies)
		{
			ResourceStateTrackingSystem tracking_system;
			// Allocate one upload and one readback heap.
			u64 upload_buffer_size = 0;
			u64 readback_buffer_size = 0;
			Vector<CopyBufferPlacementInfo> placements;
			placements.reserve(copies.size());
			for (auto& i : copies)
			{
				if (i.op == ResourceCopyOp::read_buffer)
				{
					u64 offset = readback_buffer_size;
					placements.push_back({ offset, 0, 0, Format::unknown });
					readback_buffer_size += i.read_buffer.size;
					tracking_system.pack_barrier(ResourceBarrierDesc::as_transition(i.resource, ResourceState::copy_source));
				}
				else if (i.op == ResourceCopyOp::write_buffer)
				{
					u64 offset = upload_buffer_size;
					placements.push_back({ offset, 0, 0, Format::unknown });
					upload_buffer_size += i.write_buffer.size;
					tracking_system.pack_barrier(ResourceBarrierDesc::as_transition(i.resource, ResourceState::copy_dest));
				}
				else if (i.op == ResourceCopyOp::read_texture)
				{
					u64 size, alignment, row_pitch, depth_pitch;
					auto desc = i.resource->get_desc();
					get_texture_data_placement_info(i.read_texture.read_box.width, i.read_texture.read_box.height, i.read_texture.read_box.depth,
						desc.pixel_format, &size, &alignment, &row_pitch, &depth_pitch);
					u64 offset = align_upper(readback_buffer_size, alignment);
					placements.push_back({ offset, row_pitch, depth_pitch, desc.pixel_format });
					readback_buffer_size = offset + size;
					tracking_system.pack_barrier(ResourceBarrierDesc::as_transition(i.resource, ResourceState::copy_source));
				}
				else if (i.op == ResourceCopyOp::write_texture)
				{
					u64 size, alignment, row_pitch, depth_pitch;
					auto desc = i.resource->get_desc();
					get_texture_data_placement_info(i.write_texture.write_box.width, i.write_texture.write_box.height, i.write_texture.write_box.depth,
						desc.pixel_format, &size, &alignment, &row_pitch, &depth_pitch);
					u64 offset = align_upper(upload_buffer_size, alignment);
					placements.push_back({ offset, row_pitch, depth_pitch, desc.pixel_format });
					upload_buffer_size = offset + size;
					tracking_system.pack_barrier(ResourceBarrierDesc::as_transition(i.resource, ResourceState::copy_dest));
				}
			}
			lutry
			{
				Ref<IResource> upload_buffer;
				Ref<IResource> readback_buffer;
				byte_t* upload_data = nullptr;
				byte_t* readback_data = nullptr;
				if (upload_buffer_size)
				{
					luset(upload_buffer, new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::none, upload_buffer_size), nullptr));
					luexp(upload_buffer->map_subresource(0, 0, 0, (void**)&upload_data));
					// Fill upload data.
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
						if (copy.op == ResourceCopyOp::write_buffer)
						{
							memcpy(upload_data + (usize)placements[i].offset, copy.write_buffer.src, copy.write_buffer.size);
						}
						else if (copy.op == ResourceCopyOp::write_texture)
						{
							usize copy_size_per_row = bits_per_pixel(placements[i].pixel_format) * copy.write_texture.write_box.width / 8;
							memcpy_bitmap3d(upload_data + (usize)placements[i].offset, copy.write_texture.src,
								copy_size_per_row, copy.write_texture.write_box.height, copy.write_texture.write_box.depth,
								(usize)placements[i].row_pitch, copy.write_texture.src_row_pitch, (usize)placements[i].depth_pitch, copy.write_texture.src_depth_pitch);
						}
					}
					upload_buffer->unmap_subresource(0, 0, USIZE_MAX);
				}
				if (readback_buffer_size)
				{
					luset(readback_buffer, new_resource(ResourceDesc::buffer(ResourceHeapType::readback, ResourceUsageFlag::none, readback_buffer_size), nullptr));
				}
				// Use GPU to copy data.
				ResourceCopyContext context;
				m_copy_contexts_lock.lock();
				if (m_copy_contexts.empty())
				{
					m_copy_contexts_lock.unlock();
					luexp(context.init(m_device.Get()));
				}
				else
				{
					context = move(m_copy_contexts.back());
					m_copy_contexts.pop_back();
					m_copy_contexts_lock.unlock();
				}
				auto barriers = tracking_system.m_barriers;
				tracking_system.resolve();
				barriers.insert(barriers.end(), tracking_system.m_barriers.begin(), tracking_system.m_barriers.end());
				if (!barriers.empty())
				{
					context.m_li->ResourceBarrier((UINT)barriers.size(), barriers.data());
				}
				tracking_system.apply(CommandQueueType::copy);
				for (usize i = 0; i < copies.size(); ++i)
				{
					auto& copy = copies[i];
					if (copy.op == ResourceCopyOp::read_buffer)
					{
						context.m_li->CopyBufferRegion(((Resource*)readback_buffer.object())->m_res.Get(), placements[i].offset,
							((Resource*)copy.resource->get_object())->m_res.Get(), copy.read_buffer.src_offset, copy.read_buffer.size);
					}
					else if (copy.op == ResourceCopyOp::write_buffer)
					{
						context.m_li->CopyBufferRegion(((Resource*)copy.resource->get_object())->m_res.Get(), copy.write_buffer.dest_offset,
							((Resource*)upload_buffer.object())->m_res.Get(), placements[i].offset, copy.write_buffer.size);
					}
					else if (copy.op == ResourceCopyOp::read_texture)
					{
						D3D12_TEXTURE_COPY_LOCATION dest, src;
						dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
						dest.pResource = ((Resource*)readback_buffer.object())->m_res.Get();
						dest.PlacedFootprint.Offset = placements[i].offset;
						dest.PlacedFootprint.Footprint.Width = copy.read_texture.read_box.width;
						dest.PlacedFootprint.Footprint.Height = copy.read_texture.read_box.height;
						dest.PlacedFootprint.Footprint.Depth = copy.read_texture.read_box.depth;
						dest.PlacedFootprint.Footprint.Format = encode_pixel_format(placements[i].pixel_format);
						dest.PlacedFootprint.Footprint.RowPitch = placements[i].row_pitch;
						src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						src.pResource = ((Resource*)copy.resource->get_object())->m_res.Get();
						src.SubresourceIndex = copy.read_texture.src_subresource;
						D3D12_BOX src_box;
						src_box.left = copy.read_texture.read_box.offset_x;
						src_box.right = copy.read_texture.read_box.offset_x + copy.read_texture.read_box.width;
						src_box.top = copy.read_texture.read_box.offset_y;
						src_box.bottom = copy.read_texture.read_box.offset_y + copy.read_texture.read_box.height;
						src_box.front = copy.read_texture.read_box.offset_z;
						src_box.back = copy.read_texture.read_box.offset_z + copy.read_texture.read_box.depth;
						context.m_li->CopyTextureRegion(&dest, 0, 0, 0, &src, &src_box);
					}
					else if (copy.op == ResourceCopyOp::write_texture)
					{
						D3D12_TEXTURE_COPY_LOCATION dest, src;
						dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						dest.pResource = ((Resource*)copy.resource->get_object())->m_res.Get();
						dest.SubresourceIndex = copy.write_texture.dest_subresource;
						src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
						src.pResource = ((Resource*)upload_buffer.object())->m_res.Get();
						src.PlacedFootprint.Offset = placements[i].offset;
						src.PlacedFootprint.Footprint.Width = copy.write_texture.write_box.width;
						src.PlacedFootprint.Footprint.Height = copy.write_texture.write_box.height;
						src.PlacedFootprint.Footprint.Depth = copy.write_texture.write_box.depth;
						src.PlacedFootprint.Footprint.Format = encode_pixel_format(placements[i].pixel_format);
						src.PlacedFootprint.Footprint.RowPitch = placements[i].row_pitch;
						D3D12_BOX src_box;
						src_box.left = 0;
						src_box.right = copy.write_texture.write_box.width;
						src_box.top = 0;
						src_box.bottom = copy.write_texture.write_box.height;
						src_box.front = 0;
						src_box.back = copy.write_texture.write_box.depth;
						context.m_li->CopyTextureRegion(&dest,
							copy.write_texture.write_box.offset_x, copy.write_texture.write_box.offset_y, copy.write_texture.write_box.offset_z,
							&src, &src_box);
					}
				}
				// Submit copy command to GPU and wait for completion.
				HRESULT hr = context.m_li->Close();
				if (FAILED(hr)) return encode_d3d12_error(hr);
				ID3D12CommandList* list = context.m_li.Get();
				m_internal_copy_queue->ExecuteCommandLists(1, &list);
				++context.m_event_value;
				hr = context.m_fence->SetEventOnCompletion(context.m_event_value, context.m_event);
				if (FAILED(hr)) return encode_d3d12_error(hr);
				hr = m_internal_copy_queue->Signal(context.m_fence.Get(), context.m_event_value);
				if (FAILED(hr)) return encode_d3d12_error(hr);
				DWORD res = ::WaitForSingleObject(context.m_event, INFINITE);
				if (res != WAIT_OBJECT_0)
				{
					return BasicError::bad_platform_call();
				}
				BOOL b = ::ResetEvent(context.m_event);
				if (!b)
				{
					return BasicError::bad_platform_call();
				}
				hr = context.m_ca->Reset();
				if (FAILED(hr)) return encode_d3d12_error(hr);
				hr = context.m_li->Reset(context.m_ca.Get(), NULL);
				if (FAILED(hr)) return encode_d3d12_error(hr);
				// Give back context.
				m_copy_contexts_lock.lock();
				m_copy_contexts.push_back(move(context));
				m_copy_contexts_lock.unlock();
				// Read data for read calls.
				if (readback_buffer)
				{
					luexp(readback_buffer->map_subresource(0, 0, USIZE_MAX, (void**)&readback_data));
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
						if (copy.op == ResourceCopyOp::read_buffer)
						{
							memcpy(copy.read_buffer.dest, readback_data + (usize)placements[i].offset, copy.read_buffer.size);
						}
						else if (copy.op == ResourceCopyOp::read_texture)
						{
							usize copy_size_per_row = bits_per_pixel(placements[i].pixel_format) * copy.read_texture.read_box.width / 8;
							memcpy_bitmap3d(copy.read_texture.dest, readback_data + (usize)placements[i].offset,
								copy_size_per_row, copy.read_texture.read_box.height, copy.read_texture.read_box.depth,
								copy.read_texture.dest_row_pitch, (usize)placements[i].row_pitch, copy.read_texture.dest_depth_pitch, (usize)placements[i].depth_pitch);
						}
					}
					readback_buffer->unmap_subresource(0, 0, 0);
				}
			}
			lucatchret;
			return ok;
		}
	}
}

#endif