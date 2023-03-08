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

#ifdef LUNA_RHI_D3D12

#include "QueryHeap.hpp"

namespace Luna
{
	namespace RHI
	{
		void ResourceStateTrackingSystem::append_transition(Resource* res, u32 subresource, ResourceState before, ResourceState after, ResourceBarrierFlag flags)
		{
			// Early out for unnecessary calls.
			if (before == after)
			{
				return;
			}
			// Use implicit transition whenever possible.
			// Refs: https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions
			if (before == ResourceState::common)
			{
				if (res->m_states.empty())
				{
					// Buffers or textures with EResourceUsageFlag::simultaneous_access
					return;
				}
				else if (
					(after == ResourceState::shader_resource_non_pixel) ||
					(after == ResourceState::shader_resource_pixel) ||
					(after == ResourceState::copy_dest) ||
					(after == ResourceState::copy_source))
				{
					return;
				}
			}
			D3D12_RESOURCE_BARRIER t;
			t.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			t.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if ((flags & ResourceBarrierFlag::begin_only) != ResourceBarrierFlag::none)
			{
				t.Flags |= D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
			}
			if ((flags & ResourceBarrierFlag::end_only) != ResourceBarrierFlag::none)
			{
				t.Flags |= D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			}
			t.Transition.StateBefore = encode_resource_state(before);
			t.Transition.StateAfter = encode_resource_state(after);
			if (subresource == resource_barrier_all_subresources_v)
			{
				t.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			else
			{
				t.Transition.Subresource = subresource;
			}
			t.Transition.pResource = res->m_res.Get();
			m_barriers.push_back(t);
		}

		void ResourceStateTrackingSystem::pack_transition(Resource* res, u32 subresource, ResourceState after, ResourceBarrierFlag flags)
		{
			if (subresource == resource_barrier_all_subresources_v)
			{
				u32 num_subresources = res->count_subresources();
				for (u32 i = 0; i < num_subresources; ++i)
				{
					pack_transition(res, i, after, flags);
				}
			}
			else
			{
				// Checks if the subresource is resolved.
				ResourceKey k;
				k.m_res = res;
				k.m_subres = subresource;
				auto citer = m_current.find(k);
				if (citer == m_current.end())
				{
					// The subresource is not resolved.

					// The `begin_only` call is discarded for unresolved resource.
					if ((flags & ResourceBarrierFlag::begin_only) != ResourceBarrierFlag::none)
					{
						return;
					}

					// The `end_only` flag will be dropped.
					if ((flags & ResourceBarrierFlag::end_only) != ResourceBarrierFlag::none)
					{
						flags &= (~ResourceBarrierFlag::end_only);
					}

					if (res->m_states.empty())
					{
						// If this resource does not have a global state, always proceed as common.
						append_transition(res, subresource, ResourceState::common, after, flags);
					}
					else
					{
						// If the resource has global states, the transition is deferred to submit time.
						m_unresloved.insert(make_pair(k, after));
					}
					m_current.insert(make_pair(k, after));
				}
				else
				{
					// The subresource is resolved.
					// Insert a transition always.
					append_transition(res, subresource, citer->second, after, flags);

					// Ignore applying changes to tracking system for `begin_only`.
					if ((flags & ResourceBarrierFlag::begin_only) == ResourceBarrierFlag::none)
					{
						citer->second = after;
					}
				}
			}
		}

		void ResourceStateTrackingSystem::resolve()
		{
			begin_new_transition_batch();
			for (auto& i : m_unresloved)
			{
				Resource* res = i.first.m_res;
				luassert(!res->m_states.empty());
				append_transition(res, i.first.m_subres, res->m_states[i.first.m_subres], i.second, ResourceBarrierFlag::none);
			}
		}

		void ResourceStateTrackingSystem::apply(CommandQueueType type)
		{
			for (auto& i : m_current)
			{
				if (i.first.m_res->m_states.empty())
				{
					continue;
				}
				// Any resources accessed by Copy queue can be implicitly decayed to common state.
				// Any read state that can be implicitly promoted from common state can be implicitly decayed to common state.
				if (type == CommandQueueType::copy /* || is_texture_decayable_to_common(i.second) */)
				{
					i.first.m_res->m_states[i.first.m_subres] = ResourceState::common;
				}
				else
				{
					i.first.m_res->m_states[i.first.m_subres] = i.second;
				}
			}
		}

		RV CommandBuffer::init()
		{
			HRESULT hr;
			hr = m_device->m_device->CreateCommandAllocator(encode_command_list_type(m_queue->m_type), IID_PPV_ARGS(&m_ca));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_device->m_device->CreateCommandList(0, encode_command_list_type(m_queue->m_type), m_ca.Get(), NULL, IID_PPV_ARGS(&m_li));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_device->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			m_event = ::CreateEventA(NULL, TRUE, FALSE, NULL);
			if (m_event == NULL)
			{
				return BasicError::bad_platform_call();
			}
			m_wait_value = 1;	// The fist wait value.
			return ok;
		}

		RV CommandBuffer::reset()
		{
			lutsassert();
			BOOL b = ::ResetEvent(m_event);
			if (!b)
			{
				return BasicError::bad_platform_call();
			}
			++m_wait_value;
			if (!m_cmdlist_closed)
			{
				HRESULT hr = m_li->Close();
				if (FAILED(hr))
				{
					return BasicError::bad_platform_call();
				}
				m_cmdlist_closed = true;
			}
			HRESULT hr = m_ca->Reset();
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_li->Reset(m_ca.Get(), NULL);
			m_cmdlist_closed = false;
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			m_tracking_system.reset();
			m_objs.clear();

			m_vbs.clear();
			m_ib.reset();
			m_heap_set = false;
			m_graphic_shader_input_layout.reset();
			m_compute_shader_input_layout.reset();
			return ok;
		}

		void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
		{
			lutsassert();
			lucheck_msg(!m_render_pass_context.m_valid, "The last render pass is not correctly closed.");
			// Create render target and depth stencil view.
			D3D12_CPU_DESCRIPTOR_HANDLE rtv[8];
			D3D12_CPU_DESCRIPTOR_HANDLE dsv{ 0 };
			memzero(rtv, sizeof(rtv));
			u8 num_render_targets = 0;
			for (auto& i : desc.rtvs)
			{
				if (!i) break;
				++num_render_targets;
			}
			m_render_pass_context.m_valid = true;
			for (auto& i : m_render_pass_context.m_rtvs) i = nullptr;
			m_render_pass_context.m_dsv = nullptr;
			m_render_pass_context.num_render_targets = num_render_targets;

			for (u8 i = 0; i < num_render_targets; ++i)
			{
				m_render_pass_context.m_rtvs[i] = static_cast<RenderTargetView*>(desc.rtvs[i]->get_object());
				rtv[i] = m_render_pass_context.m_rtvs[i]->m_heap->GetCPUDescriptorHandleForHeapStart();
			}
			if (desc.dsv)
			{
				m_render_pass_context.m_dsv = static_cast<DepthStencilView*>(desc.dsv->get_object());
				dsv = m_render_pass_context.m_dsv->m_heap->GetCPUDescriptorHandleForHeapStart();
			}

			if (m_render_pass_context.m_rtvs[0])
			{
				auto d = m_render_pass_context.m_rtvs[0]->m_resource->get_desc();
				m_render_pass_context.m_tex_size.x = (u32)d.width_or_buffer_size;
				m_render_pass_context.m_tex_size.y = (u32)d.height;
			}
			else if(m_render_pass_context.m_dsv)
			{
				auto d = m_render_pass_context.m_dsv->m_resource->get_desc();
				m_render_pass_context.m_tex_size.x = (u32)d.width_or_buffer_size;
				m_render_pass_context.m_tex_size.y = (u32)d.height;
			}
			else
			{
				m_render_pass_context.m_tex_size = UInt2U(0, 0);
			}

			if (num_render_targets)
			{
				if (desc.dsv)
				{
					m_li->OMSetRenderTargets(num_render_targets, rtv, FALSE, &dsv);
				}
				else
				{
					m_li->OMSetRenderTargets(num_render_targets, rtv, FALSE, NULL);
				}
			}
			else
			{
				if (desc.dsv)
				{
					m_li->OMSetRenderTargets(0, NULL, FALSE, &dsv);
				}
				else
				{
					m_li->OMSetRenderTargets(0, NULL, FALSE, NULL);
				}
			}

			// Clear render target and depth stencil if needed.
			for (u32 i = 0; i < num_render_targets; ++i)
			{
				if ((desc.rt_load_ops[i] == LoadOp::clear) && m_render_pass_context.m_rtvs[i])
				{
					m_li->ClearRenderTargetView(m_render_pass_context.m_rtvs[i]->m_heap->GetCPUDescriptorHandleForHeapStart(),
						desc.rt_clear_values[i].m, 0, NULL);
				}
			}
			if ((desc.depth_load_op == LoadOp::clear) || (desc.stencil_load_op == LoadOp::clear))
			{
				if (m_render_pass_context.m_dsv)
				{
					D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
					if (desc.depth_load_op == LoadOp::clear)
					{
						flags |= D3D12_CLEAR_FLAG_DEPTH;
					}
					if (desc.stencil_load_op == LoadOp::clear)
					{
						flags |= D3D12_CLEAR_FLAG_STENCIL;
					}
					m_li->ClearDepthStencilView(m_render_pass_context.m_dsv->m_heap->GetCPUDescriptorHandleForHeapStart(), 
						flags, desc.depth_clear_value, desc.stencil_clear_value, 0, NULL);
				}
			}
		}

		void CommandBuffer::set_pipeline_state(IPipelineState* pso)
		{
			PipelineState* p = static_cast<PipelineState*>(pso->get_object());
			lutsassert();
			m_li->SetPipelineState(p->m_pso.Get());
		}

		void CommandBuffer::set_graphic_shader_input_layout(IShaderInputLayout* shader_input_layout)
		{
			lutsassert();
			lucheck(shader_input_layout);
			ShaderInputLayout* o = static_cast<ShaderInputLayout*>(shader_input_layout->get_object());
			m_graphic_shader_input_layout = o;
			m_li->SetGraphicsRootSignature(o->m_rs.Get());
		}

		void CommandBuffer::set_vertex_buffers(u32 start_slot, Span<const VertexBufferViewDesc> views)
		{
			lutsassert();
			m_vbs.resize(start_slot + views.size());
			for (u32 i = start_slot; i < views.size(); ++i)
			{
				m_vbs[i] = views[i - start_slot];
			}

			D3D12_VERTEX_BUFFER_VIEW* vbv = (D3D12_VERTEX_BUFFER_VIEW*)alloca(sizeof(D3D12_VERTEX_BUFFER_VIEW) * views.size());
			for (u32 i = 0; i < views.size(); ++i)
			{
				vbv[i].BufferLocation = static_cast<Resource*>(views[i].resource->get_object())->m_res->GetGPUVirtualAddress() + views[i].offset_in_bytes;
				vbv[i].SizeInBytes = views[i].size_in_bytes;
				vbv[i].StrideInBytes = views[i].stride_in_bytes;
			}
			m_li->IASetVertexBuffers(start_slot, (UINT)views.size(), vbv);
		}

		void CommandBuffer::set_index_buffer(IResource* buffer, u32 offset_in_bytes, u32 size_in_bytes, Format format)
		{
			lutsassert();
			Resource* b = static_cast<Resource*>(buffer->get_object());
			m_ib = b;
			D3D12_INDEX_BUFFER_VIEW v;
			v.BufferLocation = b->m_res->GetGPUVirtualAddress() + offset_in_bytes;
			v.Format = encode_pixel_format(format);
			v.SizeInBytes = size_in_bytes;
			m_li->IASetIndexBuffer(&v);
		}

		void CommandBuffer::set_graphic_descriptor_set(usize index, IDescriptorSet* descriptor_set)
		{
			lutsassert();
			lucheck_msg(m_graphic_shader_input_layout, "Graphic Shader Input Layout must be set before Graphic View Set can be bound!");

			if (!m_heap_set)
			{
				ID3D12DescriptorHeap* heaps[2];
				heaps[0] = m_device->m_cbv_srv_uav_heap.m_heap.Get();
				heaps[1] = m_device->m_sampler_heap.m_heap.Get();
				m_li->SetDescriptorHeaps(2, heaps);
				m_heap_set = true;
			}

			lucheck_msg(m_graphic_shader_input_layout->m_descriptor_set_layouts.size() > index, "The binding index out of range specified by the shader input layout.");

			auto& info = m_graphic_shader_input_layout->m_descriptor_set_layouts[index];
			DescriptorSet* set = static_cast<DescriptorSet*>(descriptor_set->get_object());
			
			for (u32 i = 0; i < (u32)info.m_heap_types.size(); ++i)
			{
				D3D12_DESCRIPTOR_HEAP_TYPE heap_type = info.m_heap_types[i];
				D3D12_GPU_DESCRIPTOR_HANDLE handle;
				if (heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
				{
					handle = m_device->m_cbv_srv_uav_heap.m_gpu_handle;
					// Shift to the set begin.
					handle.ptr += m_device->m_cbv_srv_uav_heap.m_descriptor_size * (set->m_view_heap_offset);
				}
				else if (heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
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

		void CommandBuffer::set_primitive_topology(PrimitiveTopology primitive_topology)
		{
			lutsassert();
			D3D12_PRIMITIVE_TOPOLOGY t;
			switch (primitive_topology)
			{
			case PrimitiveTopology::undefined:
				t = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
				break;
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
			case PrimitiveTopology::line_list_adj:
				t = D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
				break;
			case PrimitiveTopology::line_strip_adj:
				t = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
				break;
			case PrimitiveTopology::triangle_list_adj:
				t = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
				break;
			case PrimitiveTopology::triangle_strip_adj:
				t = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
				break;
			case PrimitiveTopology::patchlist_1_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_2_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_3_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_4_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_5_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_6_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_7_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_8_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_9_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_10_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_11_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_12_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_13_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_14_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_15_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_16_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_17_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_18_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_19_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_20_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_21_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_22_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_23_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_24_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_25_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_26_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_27_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_28_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_29_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_30_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_31_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
				break;
			case PrimitiveTopology::patchlist_32_control_point:
				t = D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
				break;
			default:
				lupanic();
				break;
			}
			m_li->IASetPrimitiveTopology(t);
		}

		void CommandBuffer::set_stream_output_targets(u32 start_slot, Span<const StreamOutputBufferView> views)
		{
			lutsassert();
			D3D12_STREAM_OUTPUT_BUFFER_VIEW* vs = (D3D12_STREAM_OUTPUT_BUFFER_VIEW*)alloca(sizeof(D3D12_STREAM_OUTPUT_BUFFER_VIEW) * views.size());
			for (u32 i = 0; i < views.size(); ++i)
			{
				vs[i].BufferLocation = static_cast<Resource*>(views[i].soresource->get_object())->m_res->GetGPUVirtualAddress() + views[i].offset_in_bytes;
				vs[i].SizeInBytes = views[i].size_in_bytes;
				vs[i].BufferFilledSizeLocation = static_cast<Resource*>(views[i].buffer_filled_size_resource->get_object())->m_res->GetGPUVirtualAddress() + views[i].buffer_filled_size_offset;
			}
			m_li->SOSetTargets(start_slot, (UINT)views.size(), vs);
		}

		void CommandBuffer::set_viewports(Span<const Viewport> viewports)
		{
			lutsassert();
			D3D12_VIEWPORT* vs = (D3D12_VIEWPORT*)alloca(sizeof(D3D12_VIEWPORT) * viewports.size());
			for (u32 i = 0; i < viewports.size(); ++i)
			{
				vs[i].Height = viewports[i].height;
				vs[i].MaxDepth = viewports[i].max_depth;
				vs[i].MinDepth = viewports[i].min_depth;
				vs[i].TopLeftX = viewports[i].top_left_x;
				vs[i].TopLeftY = viewports[i].top_left_y;
				vs[i].Width = viewports[i].width;
			}
			m_li->RSSetViewports((UINT)viewports.size(), vs);
		}

		void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "set_scissor_rects must be called between `begin_render_pass` and `end_render_pass`.");
			D3D12_RECT* rs = (D3D12_RECT*)alloca(sizeof(D3D12_RECT) * rects.size());
			auto tex_sz = m_render_pass_context.m_tex_size;
			for (u32 i = 0; i < rects.size(); ++i)
			{
				rs[i].top = tex_sz.y - (rects[i].offset_y + rects[i].height);
				rs[i].bottom = tex_sz.y - rects[i].offset_y;
				rs[i].left = rects[i].offset_x;
				rs[i].right = rects[i].offset_x + rects[i].width;
			}
			m_li->RSSetScissorRects((UINT)rects.size(), rs);
		}

		void CommandBuffer::set_blend_factor(Span<const f32, 4> blend_factor)
		{
			lutsassert();
			m_li->OMSetBlendFactor(blend_factor.data());
		}

		void CommandBuffer::set_stencil_ref(u32 stencil_ref)
		{
			lutsassert();
			m_li->OMSetStencilRef(stencil_ref);
		}

		RV CommandBuffer::submit()
		{
			lutsassert();
			HRESULT hr;
			hr = m_li->Close();
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			m_cmdlist_closed = true;

			// Resolve barriers.
			m_tracking_system.resolve();

			// Submit commands.
			if (!m_tracking_system.m_barriers.empty())
			{
				ComPtr<ID3D12GraphicsCommandList> li;
				hr = m_device->m_device->CreateCommandList(0, encode_command_list_type(m_queue->m_type), m_ca.Get(), NULL, IID_PPV_ARGS(&li));
				if (FAILED(hr))
				{
					return BasicError::bad_platform_call();
				}
				li->ResourceBarrier((UINT)m_tracking_system.m_barriers.size(), m_tracking_system.m_barriers.data());
				hr = li->Close();
				if (FAILED(hr))
				{
					return BasicError::bad_platform_call();
				}
				ID3D12CommandList* lists[2];
				lists[0] = li.Get();
				lists[1] = m_li.Get();
				m_queue->m_queue->ExecuteCommandLists(2, lists);
			}
			else
			{
				m_queue->m_queue->ExecuteCommandLists(1, (ID3D12CommandList**)m_li.GetAddressOf());
			}

			MutexGuard guard(m_queue->m_mtx);

			// Apply barrier changes to the global state.
			m_tracking_system.apply(m_queue->m_type);

			guard.unlock();

			// Set fences.
			hr = m_fence->SetEventOnCompletion(m_wait_value, m_event);
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_queue->m_queue->Signal(m_fence.Get(), m_wait_value);
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}

			return ok;
		}

		void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location, i32 base_vertex_location, u32 start_instance_location)
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "draw_indexed_instanced must be called between `begin_render_pass` and `end_render_pass`.");
			m_li->DrawIndexedInstanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location);
		}

		void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
			u32 start_instance_location)
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "draw_instanced must be called between `begin_render_pass` and `end_render_pass`.");
			m_li->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location);
		}

		void CommandBuffer::clear_depth_stencil_view(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects)
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "clear_depth_stencil_view must be called between `begin_render_pass` and `end_render_pass`.");
			D3D12_CPU_DESCRIPTOR_HANDLE h = m_render_pass_context.m_dsv->m_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_RECT* d3drects = (D3D12_RECT*)alloca(sizeof(D3D12_RECT) * rects.size());
			auto tex_sz = m_render_pass_context.m_tex_size;
			for (u32 i = 0; i < rects.size(); ++i)
			{
				d3drects[i].bottom = tex_sz.y - rects[i].offset_y;
				d3drects[i].left = rects[i].offset_x;
				d3drects[i].right = rects[i].offset_x + rects[i].width;
				d3drects[i].top = tex_sz.y - (rects[i].offset_y + rects[i].height);
			}
			D3D12_CLEAR_FLAGS f;
			if ((clear_flags & ClearFlag::depth) != ClearFlag::none)
			{
				if ((clear_flags & ClearFlag::stencil) != ClearFlag::none)
				{
					f = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
				}
				else
				{
					f = D3D12_CLEAR_FLAG_DEPTH;
				}
			}
			else if ((clear_flags & ClearFlag::stencil) != ClearFlag::none)
			{
				f = D3D12_CLEAR_FLAG_STENCIL;
			}
			else
			{
				return;
			}
			m_li->ClearDepthStencilView(h, f, depth, stencil, (UINT)rects.size(), d3drects);
		}

		void CommandBuffer::clear_render_target_view(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects)
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "clear_render_target_view must be called between `begin_render_pass` and `end_render_pass`.");
			D3D12_CPU_DESCRIPTOR_HANDLE h = m_render_pass_context.m_rtvs[index]->m_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_RECT* d3drects = (D3D12_RECT*)alloca(sizeof(D3D12_RECT) * rects.size());
			auto tex_sz = m_render_pass_context.m_tex_size;
			for (u32 i = 0; i < rects.size(); ++i)
			{
				d3drects[i].bottom = tex_sz.y - rects[i].offset_y;
				d3drects[i].left = rects[i].offset_x;
				d3drects[i].right = rects[i].offset_x + rects[i].width;
				d3drects[i].top = tex_sz.y - (rects[i].offset_y + rects[i].height);
			}
			m_li->ClearRenderTargetView(h, color_rgba.data(), (UINT)rects.size(), d3drects);
		}

		void CommandBuffer::end_render_pass()
		{
			lutsassert();
			lucheck_msg(m_render_pass_context.m_valid, "`begin_render_pass` must be called before `end_render_pass`.");
			m_render_pass_context.m_valid = false;
		}

		void CommandBuffer::copy_resource(IResource* dest, IResource* src)
		{
			lutsassert();
			lucheck(dest && src);
			m_li->CopyResource(static_cast<Resource*>(dest->get_object())->m_res.Get(), static_cast<Resource*>(src->get_object())->m_res.Get());
		}

		void CommandBuffer::copy_buffer_region(IResource* dest, u64 dest_offset, IResource* src, u64 src_offset, u64 num_bytes)
		{
			lutsassert();
			Resource* d = static_cast<Resource*>(dest->get_object());
			Resource* s = static_cast<Resource*>(src->get_object());
			m_li->CopyBufferRegion(d->m_res.Get(), dest_offset, s->m_res.Get(), src_offset, num_bytes);
		}

		void CommandBuffer::copy_texture_region(const TextureCopyLocation& dst, u32 dst_x, u32 dst_y, u32 dst_z, const TextureCopyLocation& src, const BoxU* src_box)
		{
			lutsassert();
			Resource* d = static_cast<Resource*>(dst.resource->get_object());
			Resource* s = static_cast<Resource*>(src.resource->get_object());
			D3D12_BOX* pb = nullptr;
			D3D12_BOX b;
			if (src_box)
			{
				b.left = src_box->offset_x;
				b.right = src_box->offset_x + src_box->width;
				b.top = src_box->offset_y;
				b.bottom = src_box->offset_y + src_box->height;
				b.front = src_box->offset_z;
				b.back = src_box->offset_z + src_box->depth;
				pb = &b;
			}
			D3D12_TEXTURE_COPY_LOCATION desttex;
			D3D12_TEXTURE_COPY_LOCATION srctex;
			desttex.pResource = d->m_res.Get();
			if (dst.type == TextureCopyType::placed_footprint)
			{
				desttex.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				desttex.PlacedFootprint.Offset = dst.placed_footprint.offset;
				desttex.PlacedFootprint.Footprint.Format = encode_pixel_format(dst.placed_footprint.footprint.format);
				desttex.PlacedFootprint.Footprint.Width = dst.placed_footprint.footprint.width;
				desttex.PlacedFootprint.Footprint.Height = dst.placed_footprint.footprint.height;
				desttex.PlacedFootprint.Footprint.Depth = dst.placed_footprint.footprint.depth;
				desttex.PlacedFootprint.Footprint.RowPitch = dst.placed_footprint.footprint.row_pitch;
			}
			else
			{
				desttex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				desttex.SubresourceIndex = dst.subresource_index;
			}
			srctex.pResource = s->m_res.Get();
			if (src.type == TextureCopyType::placed_footprint)
			{
				srctex.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				srctex.PlacedFootprint.Offset = src.placed_footprint.offset;
				srctex.PlacedFootprint.Footprint.Format = encode_pixel_format(src.placed_footprint.footprint.format);
				srctex.PlacedFootprint.Footprint.Width = src.placed_footprint.footprint.width;
				srctex.PlacedFootprint.Footprint.Height = src.placed_footprint.footprint.height;
				srctex.PlacedFootprint.Footprint.Depth = src.placed_footprint.footprint.depth;
				srctex.PlacedFootprint.Footprint.RowPitch = src.placed_footprint.footprint.row_pitch;
			}
			else
			{
				srctex.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				srctex.SubresourceIndex = src.subresource_index;
			}
			m_li->CopyTextureRegion(&desttex, dst_x, dst_y, dst_z, &srctex, pb);
		}

		void CommandBuffer::set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout)
		{
			lutsassert();
			lucheck(shader_input_layout);
			ShaderInputLayout* o = static_cast<ShaderInputLayout*>(shader_input_layout->get_object());
			m_compute_shader_input_layout = o;
			m_li->SetComputeRootSignature(o->m_rs.Get());
		}

		void CommandBuffer::set_compute_descriptor_set(usize index, IDescriptorSet* descriptor_set)
		{
			lutsassert();
			lucheck_msg(m_compute_shader_input_layout, "Compute Shader Input Layout must be set before Compute View Set can be attached.");

			if (!m_heap_set)
			{
				ID3D12DescriptorHeap* heaps[2];
				heaps[0] = m_device->m_cbv_srv_uav_heap.m_heap.Get();
				heaps[1] = m_device->m_sampler_heap.m_heap.Get();
				m_li->SetDescriptorHeaps(2, heaps);
				m_heap_set = true;
			}

			lucheck_msg(m_compute_shader_input_layout->m_descriptor_set_layouts.size() > index, "The binding index out of range specified by the shader input layout.");

			auto& info = m_compute_shader_input_layout->m_descriptor_set_layouts[index];
			DescriptorSet* set = static_cast<DescriptorSet*>(descriptor_set->get_object());

			for (u32 i = 0; i < (u32)info.m_heap_types.size(); ++i)
			{
				D3D12_DESCRIPTOR_HEAP_TYPE heap_type = info.m_heap_types[i];
				D3D12_GPU_DESCRIPTOR_HANDLE handle;
				if (heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
				{
					handle = m_device->m_cbv_srv_uav_heap.m_gpu_handle;
					// Shift to the set begin.
					handle.ptr += m_device->m_cbv_srv_uav_heap.m_descriptor_size * (set->m_view_heap_offset);
				}
				else if (heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
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

		void CommandBuffer::resource_barrier(const ResourceBarrierDesc& barrier)
		{
			lutsassert();
			m_tracking_system.begin_new_transition_batch();
			m_tracking_system.pack_barrier(barrier);
			if (!m_tracking_system.m_barriers.empty())
			{
				m_li->ResourceBarrier((UINT)m_tracking_system.m_barriers.size(), m_tracking_system.m_barriers.data());
			}
		}

		void CommandBuffer::resource_barriers(Span<const ResourceBarrierDesc> barriers)
		{
			lutsassert();
			m_tracking_system.begin_new_transition_batch();
			for (auto& barrier : barriers)
			{
				m_tracking_system.pack_barrier(barrier);
			}
			if (!m_tracking_system.m_barriers.empty())
			{
				m_li->ResourceBarrier((UINT)m_tracking_system.m_barriers.size(), m_tracking_system.m_barriers.data());
			}
		}

		void CommandBuffer::dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z)
		{
			lutsassert();
			m_li->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
		}

		void CommandBuffer::write_timestamp(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->EndQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index);
			Resource* res = (Resource*)query_heap->m_result_buffer->get_object();
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
			Resource* res = (Resource*)query_heap->m_result_buffer->get_object();
			m_li->ResolveQueryData(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, index, 1, res->m_res.Get(), index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));
		}
		void CommandBuffer::begin_occlusion_query(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->BeginQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_OCCLUSION, index);
		}
		void CommandBuffer::end_occlusion_query(IQueryHeap* heap, u32 index)
		{
			lutsassert();
			QueryHeap* query_heap = (QueryHeap*)heap->get_object();
			m_li->EndQuery(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_OCCLUSION, index);
			Resource* res = (Resource*)query_heap->m_result_buffer->get_object();
			m_li->ResolveQueryData(query_heap->m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index, 1, res->m_res.Get(), index * 8);
		}
	}
}

#endif