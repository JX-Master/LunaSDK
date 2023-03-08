/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.hpp
* @author JXMaster
* @date 2020/3/11
*/
#pragma once

#ifdef LUNA_RHI_D3D12
#include "CommandQueue.hpp"
#include <Runtime/TSAssert.hpp>
#include <Runtime/HashMap.hpp>
#include "Resource.hpp"
#include "DescriptorSet.hpp"
#include "PipelineState.hpp"
#include "ShaderInputLayout.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ResourceKey
		{
			Resource* m_res;
			u32 m_subres;
		};

		inline bool operator==(const ResourceKey& lhs, const ResourceKey& rhs)
		{
			return (lhs.m_res == rhs.m_res) && (lhs.m_subres == rhs.m_subres);
		}
	}

	template<>
	struct hash<RHI::ResourceKey>
	{
		usize operator()(const RHI::ResourceKey& key)
		{
			return (usize)(key.m_res) ^ (usize)(key.m_subres);
		}
	};

	namespace RHI
	{
		// Only for Non-Simultaneous-Access Textures.
		inline bool is_texture_decayable_to_common(ResourceState s)
		{
			/*if (s == ResourceState::common ||
				s == ResourceState::shader_resource_non_pixel ||
				s == ResourceState::shader_resource_pixel ||
				s == ResourceState::copy_source)
			{
				return true;
			}*/
			return false;
		}

		class ResourceStateTrackingSystem
		{
		public:
			//! One table for unresolved resources. Unlike most implementations in other library, because
			//! we don't know when the list will be submitted to the queue, we defer the resolving of this 
			//! to the time when the list is actually submitted.
			HashMap<ResourceKey, ResourceState> m_unresloved;
			//! One table for the current state of resources.
			HashMap<ResourceKey, ResourceState> m_current;

			Vector<D3D12_RESOURCE_BARRIER> m_barriers;

			ResourceStateTrackingSystem() {}

		private:
			//! Translates one transition into the D3D12 transition barrier, and ignores the implicit promotion and decay.
			void append_transition(Resource* res, u32 subresource, ResourceState before, ResourceState after, ResourceBarrierFlag flags);

		public:

			void reset()
			{
				m_unresloved.clear();
				m_current.clear();
			}

			void begin_new_transition_batch()
			{
				m_barriers.clear();
			}

			R<ResourceState> get_state(Resource* res, u32 subresource)
			{
				ResourceKey k;
				k.m_res = res;
				k.m_subres = subresource;
				auto iter = m_current.find(k);
				if (iter == m_current.end())
				{
					return BasicError::not_found();
				}
				return iter->second;
			}

			//! Appends one barrier that transits the specified subresources' state to after
			//! state, and records the change into the tracking system.
			//! 
			//! If any of `begin_only` and `end_only` flag is specified: 
			//! 1. If the specified subresource is not resolved, the `begin_only` call is ignored, and the `end_only`
			//!    call will be converted to a full call (flag `end_only` dropped).
			//! 2. If the specified subresource has been resolved by previous states, both `begin_only` and `end_only`
			//!	   calls will be recorded, but the changes will only be applied to the tracking system when `end_only`
			//!	   is called.
			void pack_transition(Resource* res, u32 subresource, ResourceState after, ResourceBarrierFlag flags);

			//! Appends any barrier.
			void pack_barrier(const ResourceBarrierDesc& desc)
			{
				switch (desc.type)
				{
				case ResourceBarrierType::transition:
				{
					Resource* res = const_cast<Resource*>(static_cast<const Resource*>(desc.transition.resource->get_object()));
					pack_transition(res, desc.transition.subresource, desc.transition.after, desc.flags);
					break;
				}
				case ResourceBarrierType::aliasing:
				{
					if (desc.aliasing.resource)
					{
						D3D12_RESOURCE_BARRIER ba;
						ba.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
						ba.Aliasing.pResourceBefore = nullptr;
						ba.Aliasing.pResourceAfter = static_cast<const Resource*>(desc.aliasing.resource->get_object())->m_res.Get();
						ba.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						m_barriers.push_back(ba);
					}
					break;
				}
				case ResourceBarrierType::uav:
				{
					D3D12_RESOURCE_BARRIER ba;
					ba.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
					ba.UAV.pResource = static_cast<Resource*>(desc.uav.resource->get_object())->m_res.Get();
					ba.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					m_barriers.push_back(ba);
					break;
				}
				default:
					lupanic();
					break;
				}
			}

			//! Resolves all unresolved transitions into m_transitions based on their current state.
			void resolve();

			//! Applies all after state back to the resource global state.
			void apply(CommandQueueType type);
		};

		struct RenderPassContext
		{
			bool m_valid = false;
			UInt2U m_tex_size;
			RenderTargetView* m_rtvs[8];
			DepthStencilView* m_dsv;
			u8 num_render_targets;
		};

		struct CommandBuffer : ICommandBuffer
		{
			lustruct("RHI::D3D12::CommandBuffer", "{2aa94bb6-f36d-4aa2-826b-3076026c2cec}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			Ref<CommandQueue> m_queue;

			// Resource Tracking System.
			ResourceStateTrackingSystem m_tracking_system;

			ComPtr<ID3D12CommandAllocator> m_ca;
			ComPtr<ID3D12GraphicsCommandList> m_li;

			//! The fence used for wait/set from GPU 
			ComPtr<ID3D12Fence> m_fence;
			//! The event used for wait from CPU.
			HANDLE m_event;
			//! The next value to wait for by CPU/GPU.
			u64 m_wait_value;

			//! Checks if the command list is closed.
			bool m_cmdlist_closed;

			// Render Pass Context.
			RenderPassContext m_render_pass_context;

			//! The current set vertex buffer.
			Vector<VertexBufferViewDesc> m_vbs;
			//! The current bound index buffer.
			Ref<Resource> m_ib;
			//! The current bound graphic pipeline state.
			//Ref<GraphicPipelineState> m_graphic_pipeline_state;
			//! The current bound compute pipeline state.
			//Ref<ComputePipelineState> m_compute_pipeline_state;
			//! The current bound graphic shader input layout.
			Ref<ShaderInputLayout> m_graphic_shader_input_layout;
			//! The current bound compute shader input layout.
			Ref<ShaderInputLayout> m_compute_shader_input_layout;

			//! The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

			bool m_heap_set;

			CommandBuffer() :
				m_event(NULL),
				m_cmdlist_closed(false),
				m_heap_set(false) {}

			~CommandBuffer()
			{
				if (m_event)
				{
					::CloseHandle(m_event);
					m_event = NULL;
				}
			}

			RV init();

			void wait()
			{
				DWORD res = ::WaitForSingleObject(m_event, INFINITE);
				if (res != WAIT_OBJECT_0)
				{
					lupanic_msg_always("WaitForSingleObject failed.");
				}
			}
			bool try_wait()
			{
				DWORD res = ::WaitForSingleObject(m_event, 0);
				if (res == WAIT_OBJECT_0)
				{
					return true;
				}
				return false;
			}
			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) { set_object_name(m_ca.Get(), name); set_object_name(m_li.Get(), name); }
			CommandQueueType get_type()
			{
				return m_queue->m_type;
			}
			RV reset();
			void attach_graphic_object(IDeviceChild* obj)
			{
				m_objs.push_back(obj);
			}
			void begin_event(const Name& event_name)
			{
				usize len = utf8_to_utf16_len(event_name.c_str(), event_name.size());
				wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
				utf8_to_utf16((c16*)buf, len + 1, event_name.c_str(), event_name.size());
				m_li->BeginEvent(0, buf, sizeof(wchar_t) * (len + 1));
			}
			void end_event()
			{
				m_li->EndEvent();
			}
			void begin_render_pass(const RenderPassDesc& desc);
			void set_pipeline_state(IPipelineState* pso);
			void set_graphic_shader_input_layout(IShaderInputLayout* shader_input_layout);
			void set_vertex_buffers(u32 start_slot, Span<const VertexBufferViewDesc> views);
			void set_index_buffer(IResource* buffer, u32 offset_in_bytes, u32 size_in_bytes, Format format);
			void set_graphic_descriptor_set(usize index, IDescriptorSet* descriptor_set);
			void set_primitive_topology(PrimitiveTopology primitive_topology);
			void set_stream_output_targets(u32 start_slot, Span<const StreamOutputBufferView> views);
			void set_viewport(const Viewport& viewport)
			{
				set_viewports({ &viewport, 1 });
			}
			void set_viewports(Span<const Viewport> viewports);
			void set_scissor_rect(const RectI& rects)
			{
				return set_scissor_rects({ &rects, 1 });
			}
			void set_scissor_rects(Span<const RectI> rects);
			void set_blend_factor(Span<const f32, 4> blend_factor);
			void set_stencil_ref(u32 stencil_ref);
			void draw(u32 vertex_count, u32 start_vertex_location)
			{
				draw_instanced(vertex_count, 1, start_vertex_location, 0);
			}
			void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location)
			{
				draw_indexed_instanced(index_count, 1, start_index_location, base_vertex_location, 0);
			}
			void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location);
			void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location);
			void clear_depth_stencil_view(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects);
			void clear_render_target_view(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects);
			void end_render_pass();
			void copy_resource(IResource* dest, IResource* src);
			void copy_buffer_region(IResource* dest, u64 dest_offset, IResource* src, u64 src_offset, u64 num_bytes);
			void copy_texture_region(const TextureCopyLocation& dst, u32 dst_x, u32 dst_y, u32 dst_z,
				const TextureCopyLocation& src, const BoxU* src_box = nullptr);
			void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout);
			void set_compute_descriptor_set(usize index, IDescriptorSet* descriptor_set);
			void resource_barrier(const ResourceBarrierDesc& barrier);
			void resource_barriers(Span<const ResourceBarrierDesc> barriers);
			void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z);
			RV submit();
		};
	}
}

#endif