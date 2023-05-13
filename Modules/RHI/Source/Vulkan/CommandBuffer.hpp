/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CommandBuffer.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "Common.hpp"
#include "../../CommandBuffer.hpp"
#include "Device.hpp"
#include "ResourceStateTrackingSystem.hpp"
#include <Runtime/UniquePtr.hpp>

namespace Luna
{
	namespace RHI
	{
		struct CommandBuffer : ICommandBuffer
		{
			lustruct("RHI::CommandBuffer", "{057DBF2F-5817-490B-9683-18A0D3C4C5CB}");
			luiimpl();

			Ref<Device> m_device;
			u32 m_queue_index;
			CommandQueue m_queue;
			Name m_name;

			ResourceStateTrackingSystem m_track_system;
			HashMap<u32, UniquePtr<QueueTransferTracker>> m_transfer_trackers;

			VkCommandPool m_command_pool = VK_NULL_HANDLE;
			VkCommandBuffer m_resolve_buffer = VK_NULL_HANDLE;
			VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
			VkFence m_fence = VK_NULL_HANDLE;

			//! The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

			// Controled by begin_render_pass/end_render_pass.
			bool m_render_pass_begin = false;
			u32 m_rt_width = 0;
			u32 m_rt_height = 0;
			u32 m_num_color_attachments = 0;
			u32 m_num_resolve_attachments = 0;
			IRenderTargetView* m_color_attachments[8] = { nullptr };
			IResolveTargetView* m_resolve_attachments[8] = { nullptr };
			IDepthStencilView* m_dsv = nullptr;

			// Set by set_pipeline_state.
			u32 m_num_viewports = 0;

			IShaderInputLayout* m_graphics_shader_input_layout = nullptr;
			IShaderInputLayout* m_compute_shader_input_layout = nullptr;

			bool m_recording = true;

			RV init(u32 command_queue_index);
			~CommandBuffer();

			RV begin_command_buffer();

			R<QueueTransferTracker*> get_transfer_tracker(u32 queue_family_index);

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual void wait() override;
			virtual bool try_wait() override;
			virtual u32 get_command_queue_index() override { return m_queue_index; }
			virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override;
			virtual void begin_event(const Name& event_name) override {}
			virtual void end_event() override {}
			virtual void begin_render_pass(const RenderPassDesc& desc) override;
			virtual void set_pipeline_state(PipelineStateBindPoint bind_point, IPipelineState* pso) override;
			virtual void set_graphics_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_vertex_buffers(u32 start_slot, u32 num_slots, IBuffer** buffers, const usize* offsets) override;
			virtual void set_index_buffer(IBuffer* buffer, usize offset_in_bytes, Format index_format) override;
			virtual void set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void set_viewport(const Viewport& viewport) override;
			virtual void set_viewports(Span<const Viewport> viewports) override;
			virtual void set_scissor_rect(const RectI& rect) override;
			virtual void set_scissor_rects(Span<const RectI> rects) override;
			virtual void set_blend_factor(Span<const f32, 4> blend_factor) override;
			virtual void set_stencil_ref(u32 stencil_ref) override;
			virtual void draw(u32 vertex_count, u32 start_vertex_location) override;
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) override;
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) override;
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) override;

			virtual void clear_depth_stencil_attachment(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects) override;
			virtual void clear_color_attachment(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects) override;
			virtual void end_render_pass() override;
			virtual void copy_resource(IResource* dest, IResource* src) override;
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
				IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_depth_pitch,
				u32 copy_width, u32 copy_height, u32 copy_depth) override;
			virtual void copy_texture_to_buffer(
				IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) override;
			virtual void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) override;
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) override;
			virtual void write_timestamp(IQueryHeap* heap, u32 index) override;
			virtual void begin_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void end_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void begin_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual void end_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) override;
		};
	}
}