/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.cpp
* @author JXMaster
* @date 2020/3/11
*/
#include "CommandBuffer.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"

namespace Luna
{
	namespace RHI
	{
		void ResourceStateTrackingSystem::append_buffer(BufferResource* buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, ResourceBarrierFlag flags)
		{
			if ((before & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) && (after & D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				// Add UAV barrier.
				D3D12_RESOURCE_BARRIER uav;
				uav.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				uav.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				uav.UAV.pResource = buffer->m_res.Get();
				m_barriers.push_back(uav);
				return;
			}
			if (test_flags(flags, ResourceBarrierFlag::aliasing))
			{
				D3D12_RESOURCE_BARRIER aliasing;
				aliasing.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
				aliasing.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				aliasing.Aliasing.pResourceBefore = NULL;
				aliasing.Aliasing.pResourceAfter = buffer->m_res.Get();
				m_barriers.push_back(aliasing);
				return;
			}
			D3D12_RESOURCE_BARRIER t;
			t.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			t.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			t.Transition.StateBefore = before;
			t.Transition.StateAfter = after;
			// Skip unnecessary calls.
			if (t.Transition.StateBefore == t.Transition.StateAfter) return;
			// Buffers can be implicitly promoted to any state from COMMON.
			if (t.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON) return;
			t.Transition.Subresource = 0;
			t.Transition.pResource = buffer->m_res.Get();
			m_barriers.push_back(t);
		}
		inline bool is_texture_implicit_promotable(D3D12_RESOURCE_STATES state)
		{
			// Refs: https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions
			D3D12_RESOURCE_STATES implicit_promotable = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;
			return (state & ~implicit_promotable) == 0;
		}
		void ResourceStateTrackingSystem::append_texture(TextureResource* texture, u32 subresource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, ResourceBarrierFlag flags)
		{
			if ((before & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) && (after & D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				// Add UAV barrier.
				D3D12_RESOURCE_BARRIER uav;
				uav.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				uav.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				uav.UAV.pResource = texture->m_res.Get();
				m_barriers.push_back(uav);
				return;
			}
			if (test_flags(flags, ResourceBarrierFlag::aliasing))
			{
				D3D12_RESOURCE_BARRIER aliasing;
				aliasing.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
				aliasing.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				aliasing.Aliasing.pResourceBefore = NULL;
				aliasing.Aliasing.pResourceAfter = texture->m_res.Get();
				m_barriers.push_back(aliasing);
				return;
			}
			D3D12_RESOURCE_BARRIER t;
			t.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			t.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			t.Transition.StateBefore = before;
			t.Transition.StateAfter = after;
			// Skip unnecessary calls.
			if (t.Transition.StateBefore == t.Transition.StateAfter) return;
			// Textures 
			if (t.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON)
			{
				// simultaneous_access textures
				if (texture->m_states.empty()) return;
				// implicit_promotable states.
				if (is_texture_implicit_promotable(after)) return;
			}
			t.Transition.Subresource = subresource;
			t.Transition.pResource = texture->m_res.Get();
			m_barriers.push_back(t);
		}
		void ResourceStateTrackingSystem::pack_buffer_internal(BufferResource* buffer, const BufferBarrier& barrier, D3D12_RESOURCE_STATES recorded_before_state)
		{
			D3D12_RESOURCE_STATES after_state = encode_buffer_state(barrier.after);
			D3D12_RESOURCE_STATES before_state = (barrier.before == BufferStateFlag::automatic) ? recorded_before_state : encode_buffer_state(barrier.before);
			append_buffer(buffer, before_state, after_state, barrier.flags);
		}
		void ResourceStateTrackingSystem::pack_texture_internal(TextureResource* texture, u32 subresource, const TextureBarrier& barrier, D3D12_RESOURCE_STATES recorded_before_state)
		{
			D3D12_RESOURCE_STATES after_state = encode_texture_state(barrier.after);
			D3D12_RESOURCE_STATES before_state = (barrier.before == TextureStateFlag::automatic) ? recorded_before_state : encode_texture_state(barrier.before);
			append_texture(texture, subresource, before_state, after_state, barrier.flags);
		}
		void ResourceStateTrackingSystem::pack_buffer(const BufferBarrier& barrier)
		{
			BufferResource* res = cast_object<BufferResource>(barrier.buffer->get_object());
			auto iter = m_current_buffer_states.find(res);
			if (iter == m_current_buffer_states.end())
			{
				// This resource is used on the current buffer for the first time.
				pack_buffer_internal(res, barrier, D3D12_RESOURCE_STATE_COMMON);
				m_current_buffer_states.insert(make_pair(res, barrier.after));
			}
			else
			{
				pack_buffer_internal(res, barrier, encode_buffer_state(iter->second));
				iter->second = barrier.after;
			}
		}
		void ResourceStateTrackingSystem::pack_texture(const TextureBarrier& barrier)
		{
			TextureResource* res = cast_object<TextureResource>(barrier.texture->get_object());
			if (barrier.subresource == TEXTURE_BARRIER_ALL_SUBRESOURCES)
			{
				if (barrier.before == TextureStateFlag::automatic)
				{
					TextureBarrier sub_barrier = barrier;
					for (u32 array_slice = 0; array_slice < res->m_desc.array_size; ++array_slice)
					{
						for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
						{
							sub_barrier.subresource.array_slice = array_slice;
							sub_barrier.subresource.mip_slice = mip_slice;
							pack_texture(sub_barrier);
						}
					}
					return;
				}
				// The barrier's before state is not automatic, which can be determined now.
				pack_texture_internal(res, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barrier, D3D12_RESOURCE_STATE_COMMON);
				u32 num_subresources = res->count_subresources();
				for (u32 i = 0; i < num_subresources; ++i)
				{
					TextureKey key;
					key.m_res = res;
					key.m_subres = i;
					m_current_texture_states.insert_or_assign(key, barrier.after);
				}
			}
			else
			{
				TextureKey key;
				key.m_res = res;
				key.m_subres = calc_subresource_index(barrier.subresource.mip_slice, barrier.subresource.array_slice, res->m_desc.mip_levels);
				auto iter = m_current_texture_states.find(key);
				if (iter == m_current_texture_states.end())
				{
					// This resource is used on the current buffer for the first time.
					if (res->m_states.empty() || barrier.before != TextureStateFlag::automatic)
					{
						// simultaneous_access textures or the before state is specified, so that it can be
						// determined now.
						pack_texture_internal(res, key.m_subres, barrier, D3D12_RESOURCE_STATE_COMMON);
					}
					else
					{
						// defer the barrier to execution time.
						m_unresolved_texture_states.insert(make_pair(key, barrier));
					}
					m_current_texture_states.insert(make_pair(key, barrier.after));
				}
				else
				{
					pack_texture_internal(res, key.m_subres, barrier, encode_texture_state(iter->second));
					iter->second = barrier.after;
				}
			}
		}
		void ResourceStateTrackingSystem::resolve()
		{
			begin_new_barrier_batch();
			for (auto& i : m_unresolved_texture_states)
			{
				TextureResource* res = i.first.m_res;
				luassert(!res->m_states.empty());
				append_texture(res, i.first.m_subres, res->m_states[i.first.m_subres], encode_texture_state(i.second.after), i.second.flags);
			}
		}
		void ResourceStateTrackingSystem::apply(CommandQueueType type)
		{
			for (auto& i : m_current_texture_states)
			{
				if (i.first.m_res->m_states.empty())
				{
					continue;
				}
				// Any resources accessed by Copy queue can be implicitly decayed to common state.
				// Any read state that can be implicitly promoted from common state can be implicitly decayed to common state.
				if (type == CommandQueueType::copy/* || is_texture_implicit_promotable(encode_texture_state(i.second))*/)
				{
					i.first.m_res->m_states[i.first.m_subres] = D3D12_RESOURCE_STATE_COMMON;
				}
				else
				{
					i.first.m_res->m_states[i.first.m_subres] = encode_texture_state(i.second);
				}
			}
		}
		RV CommandBuffer::init()
		{
			HRESULT hr;
			auto& queue = m_device->m_command_queues[m_queue];
			hr = m_device->m_device->CreateCommandAllocator(encode_command_queue_type(queue->m_desc.type), IID_PPV_ARGS(&m_ca));
			if (FAILED(hr)) return encode_hresult(hr);
			hr = m_device->m_device->CreateCommandList(0, encode_command_queue_type(queue->m_desc.type), m_ca.Get(), NULL, IID_PPV_ARGS(&m_li));
			if (FAILED(hr)) return encode_hresult(hr);
			hr = m_device->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
			if (FAILED(hr)) return encode_hresult(hr);
			m_event = ::CreateEventA(NULL, TRUE, TRUE, NULL);
			if (m_event == NULL)
			{
				return BasicError::bad_platform_call();
			}
			m_wait_value = 1;	// The fist wait value.
			return ok;
		}
		void CommandBuffer::write_timestamp(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->EndQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index);
			BufferResource* res = query_heap->m_result_buffer;
			m_li->ResolveQueryData(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index, 1, res->m_res.Get(), index * 8);
		}
		void CommandBuffer::begin_pipeline_statistics_query(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->BeginQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, index);
		}
		void CommandBuffer::end_pipeline_statistics_query(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->EndQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, index);
			BufferResource* res = query_heap->m_result_buffer;
			m_li->ResolveQueryData(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, index, 1, res->m_res.Get(), index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));
		}
		RV CommandBuffer::reset()
		{
			lutsassert();
			if (!m_cmdlist_closed)
			{
				HRESULT hr = m_li->Close();
				if (FAILED(hr)) return encode_hresult(hr);
				m_cmdlist_closed = true;
			}
			HRESULT hr = m_ca->Reset();
			if (FAILED(hr)) return encode_hresult(hr);
			hr = m_li->Reset(m_ca.Get(), NULL);
			m_cmdlist_closed = false;
			if (FAILED(hr)) return encode_hresult(hr);
			m_tracking_system.reset();
			m_objs.clear();
			m_vbs.clear();
			m_ib.reset();
			m_heap_set = false;
			m_graphics_pipeline_layout.reset();
			m_compute_pipeline_layout.reset();
			return ok;
		}
		void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
		{
			lutsassert();
			assert_no_context();
			lutry
			{
				m_occlusion_query_heap_attachment = desc.occlusion_query_heap;
				m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
				m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
				m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
				m_pipeline_statistics_query_heap_attachment = desc.pipeline_statistics_query_heap;
				m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
				if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
				{
					write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
				}
				if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
				{
					begin_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
				}
				// Create render target and depth stencil view.
				D3D12_CPU_DESCRIPTOR_HANDLE rtv[8];
				D3D12_CPU_DESCRIPTOR_HANDLE dsv{ 0 };
				memzero(rtv, sizeof(rtv));
				u8 num_color_attachments = 0;
				for (auto& i : desc.color_attachments)
				{
					if (!i.texture) break;
					++num_color_attachments;
				}
				m_render_pass_context.m_valid = true;
				memzero(m_render_pass_context.m_color_attachments, sizeof(ID3D12DescriptorHeap*) * 8);
				memzero(m_render_pass_context.m_color_attachment_views, sizeof(TextureViewDesc) * 8);
				memzero(m_render_pass_context.m_resolve_attachments, sizeof(ResolveAttachment) * 8);
				m_render_pass_context.m_depth_stencil_attachment = nullptr;
				m_render_pass_context.m_num_color_attachments = num_color_attachments;
				m_render_pass_context.m_tex_size = UInt2U(0, 0);
				for (u8 i = 0; i < num_color_attachments; ++i)
				{
					auto& src = desc.color_attachments[i];
					TextureResource* tex = cast_object<TextureResource>(src.texture->get_object());
					TextureViewDesc view;
					view.texture = src.texture;
					view.type = src.view_type;
					view.format = src.format;
					view.mip_slice = src.mip_slice;
					view.mip_size = 1;
					view.array_slice = src.array_slice;
					view.array_size = desc.array_size;
					luset(m_render_pass_context.m_color_attachments[i], tex->get_rtv(view));
					m_render_pass_context.m_color_attachment_views[i] = view;
					rtv[i] = m_render_pass_context.m_color_attachments[i]->GetCPUDescriptorHandleForHeapStart();
					m_render_pass_context.m_tex_size.x = tex->m_desc.width;
					m_render_pass_context.m_tex_size.y = tex->m_desc.height;
				}
				if (desc.depth_stencil_attachment.texture)
				{
					auto& src = desc.depth_stencil_attachment;
					TextureResource* tex = cast_object<TextureResource>(src.texture->get_object());
					TextureViewDesc view;
					view.texture = src.texture;
					view.type = src.view_type;
					view.format = src.format;
					view.mip_slice = src.mip_slice;
					view.mip_size = 1;
					view.array_slice = src.array_slice;
					view.array_size = desc.array_size;
					luset(m_render_pass_context.m_depth_stencil_attachment, tex->get_dsv(view));
					dsv = m_render_pass_context.m_depth_stencil_attachment->GetCPUDescriptorHandleForHeapStart();
					m_render_pass_context.m_tex_size.x = tex->m_desc.width;
					m_render_pass_context.m_tex_size.y = tex->m_desc.height;
				}
				if (num_color_attachments)
				{
					if (desc.depth_stencil_attachment.texture)
					{
						m_li->OMSetRenderTargets(num_color_attachments, rtv, FALSE, &dsv);
					}
					else
					{
						m_li->OMSetRenderTargets(num_color_attachments, rtv, FALSE, NULL);
					}
				}
				else
				{
					if (desc.depth_stencil_attachment.texture)
					{
						m_li->OMSetRenderTargets(0, NULL, FALSE, &dsv);
					}
					else
					{
						m_li->OMSetRenderTargets(0, NULL, FALSE, NULL);
					}
				}
				// Clear render target and depth stencil if needed.
				for (u32 i = 0; i < num_color_attachments; ++i)
				{
					if ((desc.color_attachments[i].load_op == LoadOp::clear) && m_render_pass_context.m_color_attachments[i])
					{
						m_li->ClearRenderTargetView(m_render_pass_context.m_color_attachments[i]->GetCPUDescriptorHandleForHeapStart(),
							desc.color_attachments[i].clear_value.m, 0, NULL);
					}
				}
				if ((desc.depth_stencil_attachment.depth_load_op == LoadOp::clear) || (desc.depth_stencil_attachment.stencil_load_op == LoadOp::clear))
				{
					if (m_render_pass_context.m_depth_stencil_attachment)
					{
						D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
						if (desc.depth_stencil_attachment.depth_load_op == LoadOp::clear)
						{
							flags |= D3D12_CLEAR_FLAG_DEPTH;
						}
						if (desc.depth_stencil_attachment.stencil_load_op == LoadOp::clear)
						{
							flags |= D3D12_CLEAR_FLAG_STENCIL;
						}
						m_li->ClearDepthStencilView(m_render_pass_context.m_depth_stencil_attachment->GetCPUDescriptorHandleForHeapStart(),
							flags, desc.depth_stencil_attachment.depth_clear_value, desc.depth_stencil_attachment.stencil_clear_value, 0, NULL);
					}
				}
				for (u32 i = 0; i < num_color_attachments; ++i)
				{
					if (desc.resolve_attachments[i].texture)
					{
						m_render_pass_context.m_resolve_attachments[i] = desc.resolve_attachments[i];
					}
				}
			}
			lucatch
			{

			}
		}
		void CommandBuffer::set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout)
		{
			lutsassert();
			assert_graphcis_context();
			lucheck(pipeline_layout);
			PipelineLayout* o = cast_object<PipelineLayout>(pipeline_layout->get_object());
			m_graphics_pipeline_layout = o;
			m_li->SetGraphicsRootSignature(o->m_rs.Get());
		}
		void CommandBuffer::set_graphics_pipeline_state(IPipelineState* pso)
		{
			lutsassert();
			assert_graphcis_context();
			PipelineState* p = cast_object<PipelineState>(pso->get_object());
			m_li->SetPipelineState(p->m_pso.Get());
			D3D12_PRIMITIVE_TOPOLOGY t;
			switch (p->m_primitive_topology)
			{
			case PrimitiveTopology::point_list:
				t = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
				break;
			case PrimitiveTopology::line_list:
				t = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
				break;
			case PrimitiveTopology::line_strip:
				t = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
				break;
			case PrimitiveTopology::triangle_list:
				t = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				break;
			case PrimitiveTopology::triangle_strip:
				t = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				break;
			default:
				lupanic();
				break;
			}
			m_li->IASetPrimitiveTopology(t);
		}
		void CommandBuffer::set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views)
		{
			lutsassert();
			assert_graphcis_context();
			m_vbs.resize(start_slot + views.size());
			for (u32 i = start_slot; i < views.size(); ++i)
			{
				m_vbs[i] = views[i - start_slot];
			}
			D3D12_VERTEX_BUFFER_VIEW* vbv = (D3D12_VERTEX_BUFFER_VIEW*)alloca(sizeof(D3D12_VERTEX_BUFFER_VIEW) * views.size());
			for (u32 i = 0; i < views.size(); ++i)
			{
				vbv[i].BufferLocation = cast_object<BufferResource>(views[i].buffer->get_object())->m_res->GetGPUVirtualAddress() + views[i].offset;
				vbv[i].SizeInBytes = views[i].size;
				vbv[i].StrideInBytes = views[i].element_size;
			}
			m_li->IASetVertexBuffers(start_slot, (UINT)views.size(), vbv);
		}
		void CommandBuffer::set_index_buffer(const IndexBufferView& desc)
		{
			lutsassert();
			assert_graphcis_context();
			BufferResource* b = cast_object<BufferResource>(desc.buffer->get_object());
			m_ib = b;
			D3D12_INDEX_BUFFER_VIEW v;
			v.BufferLocation = b->m_res->GetGPUVirtualAddress() + desc.offset;
			v.Format = encode_format(desc.format);
			v.SizeInBytes = desc.size;
			m_li->IASetIndexBuffer(&v);
		}
		void CommandBuffer::set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
		{
			lutsassert();
			assert_graphcis_context();
			lucheck_msg(m_graphics_pipeline_layout, "Graphics pipeline layout must be set before Graphic Descriptor Set can be bound!");
			lucheck_msg(m_graphics_pipeline_layout->m_descriptor_set_layouts.size() >= start_index + descriptor_sets.size(), "The binding index out of range specified by the pipeline layout.");
			if (!m_heap_set)
			{
				ID3D12DescriptorHeap* heaps[2];
				heaps[0] = m_device->m_cbv_srv_uav_heap.m_heap.Get();
				heaps[1] = m_device->m_sampler_heap.m_heap.Get();
				m_li->SetDescriptorHeaps(2, heaps);
				m_heap_set = true;
			}
			for (u32 index = start_index; index < start_index + (u32)descriptor_sets.size(); ++index)
			{
				auto& info = m_graphics_pipeline_layout->m_descriptor_set_layouts[index];
				DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[index - start_index]->get_object());
				for (u32 i = 0; i < (u32)info.m_memory_types.size(); ++i)
				{
					D3D12_DESCRIPTOR_HEAP_TYPE memory_type = info.m_memory_types[i];
					D3D12_GPU_DESCRIPTOR_HANDLE handle;
					if (memory_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
					{
						handle = m_device->m_cbv_srv_uav_heap.m_gpu_handle;
						// Shift to the set begin.
						handle.ptr += m_device->m_cbv_srv_uav_heap.m_descriptor_size * (set->m_view_heap_offset);
					}
					else if (memory_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
					{
						handle = m_device->m_sampler_heap.m_gpu_handle;
						handle.ptr += m_device->m_sampler_heap.m_descriptor_size * (set->m_sampler_heap_offset);
					}
					else
					{
						lupanic();
					}
					m_li->SetGraphicsRootDescriptorTable(info.m_root_parameter_offset + i, handle);
				}
			}
		}
		void CommandBuffer::set_viewports(Span<const Viewport> viewports)
		{
			lutsassert();
			assert_graphcis_context();
			D3D12_VIEWPORT* vs = (D3D12_VIEWPORT*)alloca(sizeof(D3D12_VIEWPORT) * viewports.size());
			for (u32 i = 0; i < viewports.size(); ++i)
			{
				vs[i].Height = viewports[i].height;
				vs[i].MaxDepth = viewports[i].max_depth;
				vs[i].MinDepth = viewports[i].min_depth;
				vs[i].TopLeftX = viewports[i].top_left_x;
				vs[i].TopLeftY =  viewports[i].top_left_y;
				vs[i].Width = viewports[i].width;
			}
			m_li->RSSetViewports((UINT)viewports.size(), vs);
		}
		void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
		{
			lutsassert();
			assert_graphcis_context();
			D3D12_RECT* rs = (D3D12_RECT*)alloca(sizeof(D3D12_RECT) * rects.size());
			auto tex_sz = m_render_pass_context.m_tex_size;
			for (u32 i = 0; i < rects.size(); ++i)
			{
				rs[i].left = rects[i].offset_x;
				rs[i].right = rects[i].offset_x + rects[i].width;
				rs[i].top = rects[i].offset_y;
				rs[i].bottom = rects[i].offset_y + rects[i].height;
			}
			m_li->RSSetScissorRects((UINT)rects.size(), rs);
		}
		void CommandBuffer::set_blend_factor(const Float4U& blend_factor)
		{
			lutsassert();
			assert_graphcis_context();
			f32 factor[] = {blend_factor.x, blend_factor.y, blend_factor.z, blend_factor.w};
			m_li->OMSetBlendFactor(factor);
		}
		void CommandBuffer::set_stencil_ref(u32 stencil_ref)
		{
			lutsassert();
			assert_graphcis_context();
			m_li->OMSetStencilRef(stencil_ref);
		}
		void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location, i32 base_vertex_location, u32 start_instance_location)
		{
			lutsassert();
			assert_graphcis_context();
			m_li->DrawIndexedInstanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location);
		}
		void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
			u32 start_instance_location)
		{
			lutsassert();
			assert_graphcis_context();
			m_li->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location);
		}
		void CommandBuffer::begin_occlusion_query(OcclusionQueryMode mode, u32 index)
		{
			lutsassert();
			assert_graphcis_context();
			QueryHeap* query_heap = (QueryHeap*)m_occlusion_query_heap_attachment->get_object();
			m_li->BeginQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_OCCLUSION, index);
		}
		void CommandBuffer::end_occlusion_query(u32 index)
		{
			lutsassert();
			assert_graphcis_context();
			QueryHeap* query_heap = (QueryHeap*)m_occlusion_query_heap_attachment->get_object();
			m_li->EndQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_OCCLUSION, index);
			BufferResource* res = query_heap->m_result_buffer;
			m_li->ResolveQueryData(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index, 1, res->m_res.Get(), index * 8);
		}
		void CommandBuffer::end_render_pass()
		{
			lutsassert();
			assert_graphcis_context();
			// Emit barrier.
			Vector<D3D12_RESOURCE_BARRIER> barriers;
			for (u32 i = 0; i < m_render_pass_context.m_num_color_attachments; ++i)
			{
				if (m_render_pass_context.m_resolve_attachments[i].texture)
				{
					auto& src = m_render_pass_context.m_color_attachment_views[i];
					auto& dst = m_render_pass_context.m_resolve_attachments[i];
					u32 num_slices = min(dst.array_size, src.array_size);
					for (u32 i = 0; i < num_slices; ++i)
					{
						D3D12_RESOURCE_BARRIER barrier{};
						barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
						barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						TextureResource* tex = cast_object<TextureResource>(src.texture->get_object());
						barrier.Transition.pResource = tex->m_res.Get();
						barrier.Transition.Subresource = calc_subresource_index(src.mip_slice, src.array_slice + i, tex->m_desc.mip_levels);
						barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
						barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
						barriers.push_back(barrier);
					}
				}
			}
			if (!barriers.empty())
			{
				m_li->ResourceBarrier((UINT)barriers.size(), barriers.data());
			}
			barriers.clear();
			for (u32 i = 0; i < m_render_pass_context.m_num_color_attachments; ++i)
			{
				if (m_render_pass_context.m_resolve_attachments[i].texture)
				{
					auto& src = m_render_pass_context.m_color_attachment_views[i];
					auto& dst = m_render_pass_context.m_resolve_attachments[i];
					TextureResource* src_res = cast_object<TextureResource>(src.texture->get_object());
					TextureResource* dst_res = cast_object<TextureResource>(dst.texture->get_object());
					u32 num_slices = min(dst.array_size, src.array_size);
					for (u32 i = 0; i < num_slices; ++i)
					{
						m_li->ResolveSubresource(
							dst_res->m_res.Get(), calc_subresource_index(dst.mip_slice, dst.array_slice + i, dst_res->m_desc.mip_levels),
							src_res->m_res.Get(), calc_subresource_index(src.mip_slice, src.array_slice + i, src_res->m_desc.mip_levels), encode_format(dst_res->m_desc.format));
					}
				}
			}
			for (u32 i = 0; i < m_render_pass_context.m_num_color_attachments; ++i)
			{
				if (m_render_pass_context.m_resolve_attachments[i].texture)
				{
					auto& src = m_render_pass_context.m_color_attachment_views[i];
					auto& dst = m_render_pass_context.m_resolve_attachments[i];
					u32 num_slices = min(dst.array_size, src.array_size);
					for (u32 i = 0; i < num_slices; ++i)
					{
						D3D12_RESOURCE_BARRIER barrier{};
						barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
						barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						TextureResource* tex = cast_object<TextureResource>(src.texture->get_object());
						barrier.Transition.pResource = tex->m_res.Get();
						barrier.Transition.Subresource = calc_subresource_index(src.mip_slice, src.array_slice + i, tex->m_desc.mip_levels);
						barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
						barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
						barriers.push_back(barrier);
					}
				}
			}
			if (!barriers.empty())
			{
				m_li->ResourceBarrier((UINT)barriers.size(), barriers.data());
			}
			barriers.clear();
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				end_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
			m_occlusion_query_heap_attachment = nullptr;
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_pipeline_statistics_query_heap_attachment = nullptr;
			m_pipeline_statistics_query_index = DONT_QUERY;
			m_render_pass_context.m_valid = false;
		}
		void CommandBuffer::begin_compute_pass(const ComputePassDesc& desc)
		{
			lutsassert();
			assert_no_context();
			m_compute_pass_begin = true;
			m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
			m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
			m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
			m_pipeline_statistics_query_heap_attachment = desc.pipeline_statistics_query_heap;
			m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
			if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				begin_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
		}
		void CommandBuffer::set_compute_pipeline_layout(IPipelineLayout* pipeline_layout)
		{
			lutsassert();
			assert_compute_context();
			lucheck(pipeline_layout);
			PipelineLayout* o = cast_object<PipelineLayout>(pipeline_layout->get_object());
			m_compute_pipeline_layout = o;
			m_li->SetComputeRootSignature(o->m_rs.Get());
		}
		void CommandBuffer::set_compute_pipeline_state(IPipelineState* pso)
		{
			PipelineState* p = cast_object<PipelineState>(pso->get_object());
			lutsassert();
			assert_compute_context();
			m_li->SetPipelineState(p->m_pso.Get());
		}
		void CommandBuffer::set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
		{
			lutsassert();
			assert_compute_context();
			lucheck_msg(m_compute_pipeline_layout, "Compute pipeline layout must be set before Compute Descriptor Set can be attached.");
			lucheck_msg(m_compute_pipeline_layout->m_descriptor_set_layouts.size() > start_index, "The binding index out of range specified by the pipeline layout.");
			if (!m_heap_set)
			{
				ID3D12DescriptorHeap* heaps[2];
				heaps[0] = m_device->m_cbv_srv_uav_heap.m_heap.Get();
				heaps[1] = m_device->m_sampler_heap.m_heap.Get();
				m_li->SetDescriptorHeaps(2, heaps);
				m_heap_set = true;
			}
			for (u32 index = start_index; index < start_index + (u32)descriptor_sets.size(); ++index)
			{
				auto& info = m_compute_pipeline_layout->m_descriptor_set_layouts[index];
				DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[index - start_index]->get_object());
				for (u32 i = 0; i < (u32)info.m_memory_types.size(); ++i)
				{
					D3D12_DESCRIPTOR_HEAP_TYPE memory_type = info.m_memory_types[i];
					D3D12_GPU_DESCRIPTOR_HANDLE handle;
					if (memory_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
					{
						handle = m_device->m_cbv_srv_uav_heap.m_gpu_handle;
						// Shift to the set begin.
						handle.ptr += m_device->m_cbv_srv_uav_heap.m_descriptor_size * (set->m_view_heap_offset);
					}
					else if (memory_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
					{
						handle = m_device->m_sampler_heap.m_gpu_handle;
						handle.ptr += m_device->m_sampler_heap.m_descriptor_size * (set->m_sampler_heap_offset);
					}
					else
					{
						lupanic();
					}
					m_li->SetComputeRootDescriptorTable(info.m_root_parameter_offset + i, handle);
				}
			}
		}
		void CommandBuffer::dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z)
		{
			lutsassert();
			assert_compute_context();
			m_li->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
		}
		void CommandBuffer::end_compute_pass()
		{
			lutsassert();
			assert_compute_context();
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				end_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_pipeline_statistics_query_heap_attachment = nullptr;
			m_pipeline_statistics_query_index = DONT_QUERY;
			m_compute_pass_begin = false;
		}
		void CommandBuffer::begin_copy_pass(const CopyPassDesc& desc)
		{
			lutsassert();
			assert_no_context();
			m_copy_pass_begin = true;
			m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
			m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
			m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
			if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
			}
		}
		void CommandBuffer::copy_resource(IResource* dst, IResource* src)
		{
			lutsassert();
			assert_copy_context();
			lucheck(dst && src);
			{
				BufferResource* d = cast_object<BufferResource>(dst->get_object());
				BufferResource* s = cast_object<BufferResource>(src->get_object());
				if (d && s)
				{
					m_li->CopyResource(d->m_res.Get(), s->m_res.Get());
					return;
				}
			}
			{
				TextureResource* d = cast_object<TextureResource>(dst->get_object());
				TextureResource* s = cast_object<TextureResource>(src->get_object());
				if (d && s)
				{
					m_li->CopyResource(d->m_res.Get(), s->m_res.Get());
				}
			}
		}
		void CommandBuffer::copy_buffer(
			IBuffer* dst, u64 dst_offset,
			IBuffer* src, u64 src_offset,
			u64 copy_bytes)
		{
			lutsassert();
			assert_copy_context();
			BufferResource* d = cast_object<BufferResource>(dst->get_object());
			BufferResource* s = cast_object<BufferResource>(src->get_object());
			m_li->CopyBufferRegion(d->m_res.Get(), dst_offset, s->m_res.Get(), src_offset, copy_bytes);
		}
		void CommandBuffer::copy_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			lutsassert();
			assert_copy_context();
			TextureResource* d = cast_object<TextureResource>(dst->get_object());
			TextureResource* s = cast_object<TextureResource>(src->get_object());
			D3D12_TEXTURE_COPY_LOCATION dsttex;
			D3D12_TEXTURE_COPY_LOCATION srctex;
			dsttex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dsttex.SubresourceIndex = calc_subresource_index(dst_subresource.mip_slice, dst_subresource.array_slice, d->m_desc.mip_levels);
			dsttex.pResource = d->m_res.Get();
			srctex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			srctex.SubresourceIndex = calc_subresource_index(src_subresource.mip_slice, src_subresource.array_slice, s->m_desc.mip_levels);
			srctex.pResource = s->m_res.Get();
			D3D12_BOX src_box;
			src_box.left = src_x;
			src_box.right = src_x + copy_width;
			src_box.top = src_y;
			src_box.bottom = src_y + copy_height;
			src_box.front = src_z;
			src_box.back = src_z + copy_depth;
			m_li->CopyTextureRegion(&dsttex, dst_x, dst_y, dst_z, &srctex, &src_box);
		}
		void CommandBuffer::copy_buffer_to_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			lutsassert();
			assert_copy_context();
			TextureResource* d = cast_object<TextureResource>(dst->get_object());
			BufferResource* s = cast_object<BufferResource>(src->get_object());
			D3D12_TEXTURE_COPY_LOCATION dsttex;
			D3D12_TEXTURE_COPY_LOCATION srctex;
			dsttex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dsttex.SubresourceIndex = calc_subresource_index(dst_subresource.mip_slice, dst_subresource.array_slice, d->m_desc.mip_levels);
			dsttex.pResource = d->m_res.Get();
			srctex.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srctex.PlacedFootprint.Offset = src_offset;
			Format format = d->m_desc.format;
			srctex.PlacedFootprint.Footprint.Format = encode_format(format);
			srctex.PlacedFootprint.Footprint.Width = src_row_pitch * 8 / bits_per_pixel(format);
			srctex.PlacedFootprint.Footprint.Height = src_slice_pitch / src_row_pitch;
			srctex.PlacedFootprint.Footprint.Depth = copy_depth;
			srctex.PlacedFootprint.Footprint.RowPitch = src_row_pitch;
			srctex.pResource = s->m_res.Get();
			D3D12_BOX src_box;
			src_box.left = 0;
			src_box.right = copy_width;
			src_box.top = 0;
			src_box.bottom = copy_height;
			src_box.front = 0;
			src_box.back = copy_depth;
			m_li->CopyTextureRegion(&dsttex, dst_x, dst_y, dst_z, &srctex, &src_box);
		}
		void CommandBuffer::copy_texture_to_buffer(
			IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			lutsassert();
			assert_copy_context();
			BufferResource* d = cast_object<BufferResource>(dst->get_object());
			TextureResource* s = cast_object<TextureResource>(src->get_object());
			D3D12_TEXTURE_COPY_LOCATION dsttex;
			D3D12_TEXTURE_COPY_LOCATION srctex;
			dsttex.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dsttex.PlacedFootprint.Offset = dst_offset;
			Format format = s->m_desc.format;
			dsttex.PlacedFootprint.Footprint.Format = encode_format(format);
			dsttex.PlacedFootprint.Footprint.Width = dst_row_pitch * 8 / bits_per_pixel(format);
			dsttex.PlacedFootprint.Footprint.Height = dst_slice_pitch / dst_row_pitch;
			dsttex.PlacedFootprint.Footprint.Depth = copy_depth;
			dsttex.PlacedFootprint.Footprint.RowPitch = dst_row_pitch;
			dsttex.pResource = d->m_res.Get();
			srctex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			srctex.SubresourceIndex = calc_subresource_index(src_subresource.mip_slice, src_subresource.array_slice, s->m_desc.mip_levels);
			srctex.pResource = s->m_res.Get();
			D3D12_BOX src_box;
			src_box.left = src_x;
			src_box.right = src_x + copy_width;
			src_box.top = src_y;
			src_box.bottom = src_y + copy_height;
			src_box.front = src_z;
			src_box.back = src_z + copy_depth;
			m_li->CopyTextureRegion(&dsttex, 0, 0, 0, &srctex, &src_box);
		}
		void CommandBuffer::end_copy_pass()
		{
			lutsassert();
			assert_copy_context();
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_copy_pass_begin = false;
		}
		void CommandBuffer::resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers)
		{
			lutsassert();
			assert_non_render_pass();
			m_tracking_system.begin_new_barrier_batch();
			for (auto& barrier : buffer_barriers)
			{
				m_tracking_system.pack_buffer(barrier);
			}
			for (auto& barrier : texture_barriers)
			{
				m_tracking_system.pack_texture(barrier);
			}
			if (!m_tracking_system.m_barriers.empty())
			{
				m_li->ResourceBarrier((UINT)m_tracking_system.m_barriers.size(), m_tracking_system.m_barriers.data());
			}
		}
		RV CommandBuffer::submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting)
		{
			lutsassert();
			assert_no_context();
			HRESULT hr;
			hr = m_li->Close();
			if (FAILED(hr)) return encode_hresult(hr);
			m_cmdlist_closed = true;

			auto& queue = m_device->m_command_queues[m_queue];

			for (IFence* f : wait_fences)
			{
				Fence* fence = cast_object<Fence>(f->get_object());
				queue->m_command_queue->Wait(fence->m_fence.Get(), fence->m_wait_value);
			}

			// Resolve barriers.
			m_tracking_system.resolve();

			// Submit commands.
			if (!m_tracking_system.m_barriers.empty())
			{
				ComPtr<ID3D12GraphicsCommandList> li;
				hr = m_device->m_device->CreateCommandList(0, encode_command_queue_type(queue->m_desc.type), m_ca.Get(), NULL, IID_PPV_ARGS(&li));
				if (FAILED(hr)) return encode_hresult(hr);
				li->ResourceBarrier((UINT)m_tracking_system.m_barriers.size(), m_tracking_system.m_barriers.data());
				hr = li->Close();
				if (FAILED(hr)) return encode_hresult(hr);
				ID3D12CommandList* lists[2];
				lists[0] = li.Get();
				lists[1] = m_li.Get();
				queue->m_command_queue->ExecuteCommandLists(2, lists);
			}
			else
			{
				ID3D12CommandList* list = m_li.Get();
				queue->m_command_queue->ExecuteCommandLists(1, &list);
			}

			LockGuard guard(queue->m_lock);
			// Apply barrier changes to the global state.
			m_tracking_system.apply(queue->m_desc.type);
			guard.unlock();

			// Set fences.
			if (allow_host_waiting)
			{
				BOOL b = ::ResetEvent(m_event);
				if (!b)
				{
					return BasicError::bad_platform_call();
				}
				++m_wait_value;
				hr = m_fence->SetEventOnCompletion(m_wait_value, m_event);
				if (FAILED(hr)) return encode_hresult(hr);
				hr = queue->m_command_queue->Signal(m_fence.Get(), m_wait_value);
				if (FAILED(hr)) return encode_hresult(hr);
			}
			for (IFence* f : signal_fences)
			{
				Fence* fence = cast_object<Fence>(f->get_object());
				++fence->m_wait_value;
				hr = queue->m_command_queue->Signal(fence->m_fence.Get(), fence->m_wait_value);
				if (FAILED(hr)) return encode_hresult(hr);
			}
			return ok;
		}
	}
}