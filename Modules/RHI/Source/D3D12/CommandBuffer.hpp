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

#include <Runtime/TSAssert.hpp>
#include <Runtime/HashMap.hpp>
#include "Resource.hpp"
#include "DescriptorSet.hpp"
#include "PipelineState.hpp"
#include "ShaderInputLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		struct TextureKey
		{
			TextureResource* m_res;
			u32 m_subres;
		};

		inline bool operator==(const TextureKey& lhs, const TextureKey& rhs)
		{
			return (lhs.m_res == rhs.m_res) && (lhs.m_subres == rhs.m_subres);
		}
	}

	template<>
	struct hash<RHI::TextureKey>
	{
		usize operator()(const RHI::TextureKey& key)
		{
			return (usize)(key.m_res) ^ (usize)(key.m_subres);
		}
	};

	namespace RHI
	{
		class ResourceStateTrackingSystem
		{
		public:
			//! One table for unresolved textures. Unlike most implementations in other library, because
			//! we don't know when the list will be submitted to the queue, we defer the resolving of this 
			//! to the time when the list is actually submitted.
			HashMap<TextureKey, TextureBarrier> m_unresolved_texture_states;
			//! One table for the current state of resources.
			HashMap<BufferResource*, BufferStateFlag> m_current_buffer_states;
			HashMap<TextureKey, TextureStateFlag> m_current_texture_states;

			//! Packed barriers.
			Vector<D3D12_RESOURCE_BARRIER> m_barriers;

			ResourceStateTrackingSystem() {}

		private:
			void append_buffer(BufferResource* buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, ResourceBarrierFlag flags);
			void append_texture(TextureResource* texture, u32 subresource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, ResourceBarrierFlag flags);

			void pack_buffer_internal(BufferResource* res, const BufferBarrier& barrier, D3D12_RESOURCE_STATES recorded_before_state);
			void pack_texture_internal(TextureResource* res, u32 subresource, const TextureBarrier& barrier, D3D12_RESOURCE_STATES recorded_before_state);

		public:

			void reset()
			{
				m_unresolved_texture_states.clear();
				m_current_buffer_states.clear();
				m_current_texture_states.clear();
			}

			void begin_new_barrier_batch()
			{
				m_barriers.clear();
			}

			/*R<ResourceState> get_state(Resource* res, u32 subresource)
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
			}*/

			void pack_buffer(const BufferBarrier& barrier);
			void pack_texture(const TextureBarrier& barrier);

			//! Resolves all unresolved transitions into m_transitions based on their current state.
			void resolve();

			//! Applies all after state back to the resource global state.
			void apply(CommandQueueType type);
		};

		struct RenderPassContext
		{
			bool m_valid = false;
			UInt2U m_tex_size;
			ID3D12DescriptorHeap* m_color_attachments[8] = { nullptr };
			TextureViewDesc m_color_attachment_views[8];
			ResolveAttachment m_resolve_attachments[8];
			ID3D12DescriptorHeap* m_depth_stencil_attachment = nullptr;
			u8 m_num_color_attachments;
		};

		struct CommandBuffer : ICommandBuffer
		{
			lustruct("RHI::CommandBuffer", "{2aa94bb6-f36d-4aa2-826b-3076026c2cec}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;
			u32 m_queue;

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
			Vector<VertexBufferView> m_vbs;
			//! The current bound index buffer.
			Ref<IBuffer> m_ib;
			//! The current bound graphic pipeline state.
			//Ref<GraphicPipelineState> m_graphics_pipeline_state;
			//! The current bound compute pipeline state.
			//Ref<ComputePipelineState> m_compute_pipeline_state;
			//! The current bound graphic shader input layout.
			Ref<ShaderInputLayout> m_graphics_shader_input_layout;
			//! The current bound compute shader input layout.
			Ref<ShaderInputLayout> m_compute_shader_input_layout;

			//! The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

			CommandBufferContextType m_context = CommandBufferContextType::none;

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
			void assert_graphcis_context()
			{
				lucheck_msg(m_context == CommandBufferContextType::graphics, "A graphics command is submiited in a non-graphics context.");
			}
			void assert_compute_context()
			{
				lucheck_msg(m_context == CommandBufferContextType::compute, "A compute command is submiited in a non-compute context.");
			}
			void assert_copy_context()
			{
				lucheck_msg(m_context == CommandBufferContextType::copy, "A copy command is submiited in a non-copy context.");
			}
			void assert_non_render_pass()
			{
				lucheck_msg(!m_render_pass_context.m_valid, "This command cannot be submitted within a render pass.");
			}
			virtual IDevice* get_device() override
			{
				return m_device.as<IDevice>();
			}
			virtual void set_name(const Name& name) override { set_object_name(m_ca.Get(), name); set_object_name(m_li.Get(), name); }
			virtual u32 get_command_queue_index() override
			{
				return m_queue;
			}
			virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override
			{
				m_objs.push_back(obj);
			}
			virtual void begin_event(const Name& event_name) override
			{
				usize len = utf8_to_utf16_len(event_name.c_str(), event_name.size());
				wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
				utf8_to_utf16((c16*)buf, len + 1, event_name.c_str(), event_name.size());
				m_li->BeginEvent(0, buf, sizeof(wchar_t) * (len + 1));
			}
			virtual void end_event() override
			{
				m_li->EndEvent();
			}
			virtual CommandBufferContextType get_context_type() override { return m_context; }
			virtual void set_context(CommandBufferContextType new_context) override { m_context = new_context; }
			virtual void begin_render_pass(const RenderPassDesc& desc) override;
			virtual void set_graphics_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_graphics_pipeline_state(IPipelineState* pso) override;
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views) override;
			virtual void set_index_buffer(const IndexBufferView& view) override;
			virtual void set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void set_viewport(const Viewport& viewport) override
			{
				set_viewports({ &viewport, 1 });
			}
			virtual void set_viewports(Span<const Viewport> viewports) override;
			virtual void set_scissor_rect(const RectI& rects) override
			{
				return set_scissor_rects({ &rects, 1 });
			}
			virtual void set_scissor_rects(Span<const RectI> rects) override;
			virtual void set_blend_factor(Span<const f32, 4> blend_factor) override;
			virtual void set_stencil_ref(u32 stencil_ref) override;
			virtual void draw(u32 vertex_count, u32 start_vertex_location) override
			{
				draw_instanced(vertex_count, 1, start_vertex_location, 0);
			}
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) override
			{
				draw_indexed_instanced(index_count, 1, start_index_location, base_vertex_location, 0);
			}
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) override;
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) override;
			virtual void clear_depth_stencil_attachment(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects) override;
			virtual void clear_color_attachment(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects) override;
			virtual void end_render_pass() override;
			virtual void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_compute_pipeline_state(IPipelineState* pso) override;
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) override;
			virtual void copy_resource(IResource* dst, IResource* src) override;
			virtual void copy_buffer(
				IBuffer* dst, u64 dst_offset,
				IBuffer* src, u64 src_offset,
				u64 copy_bytes) override;
			virtual void copy_texture(
				ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) override;
			virtual void copy_buffer_to_texture(
				ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
				IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
				u32 copy_width, u32 copy_height, u32 copy_depth) override;
			virtual void copy_texture_to_buffer(
				IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) override;
			virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) override;
			virtual void write_timestamp(IQueryHeap* heap, u32 index) override;
			virtual void begin_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void end_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void begin_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual void end_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) override;
		};
	}
}