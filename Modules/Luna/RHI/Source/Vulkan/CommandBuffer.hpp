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
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
	namespace RHI
	{
		struct QueueTransferTracker
		{
			VkDevice m_device = VK_NULL_HANDLE;
			VolkDeviceTable* m_funcs = nullptr;
			VkCommandPool m_command_pool = VK_NULL_HANDLE;
			VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
			VkSemaphore m_semaphore = VK_NULL_HANDLE;

			RV init(u32 queue_family_index);
			R<VkSemaphore> submit_barrier(VkQueue queue, IMutex* queue_mtx, Span<const VkBufferMemoryBarrier> buffer_barriers, Span<const VkImageMemoryBarrier> texture_barriers);
			~QueueTransferTracker();
		};
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

			// The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

			// Controled by begin_render_pass/end_render_pass.
			bool m_render_pass_begin = false;
			u32 m_rt_width = 0;
			u32 m_rt_height = 0;
			u32 m_num_color_attachments = 0;
			u32 m_num_resolve_attachments = 0;
			ImageView* m_color_attachments[8] = { nullptr };
			ImageView* m_resolve_attachments[8] = { nullptr };
			ImageView* m_dsv = nullptr;
			Vector<VkFramebuffer> m_fbos;

			IPipelineLayout* m_graphics_pipeline_layout = nullptr;
			IPipelineLayout* m_compute_pipeline_layout = nullptr;

			IQueryHeap* m_occlusion_query_heap_attachment = nullptr;
			IQueryHeap* m_timestamp_query_heap_attachment = nullptr;
			IQueryHeap* m_pipeline_statistics_query_heap_attachment = nullptr;
			u32 m_timestamp_query_begin_index = DONT_QUERY;
			u32 m_timestamp_query_end_index = DONT_QUERY;
			u32 m_pipeline_statistics_query_index = DONT_QUERY;

			bool m_compute_pass_begin = false;
			bool m_copy_pass_begin = false;

			bool m_recording = true;

			RV init(u32 command_queue_index);
			~CommandBuffer();

			RV begin_command_buffer();

			R<QueueTransferTracker*> get_transfer_tracker(u32 queue_family_index);

			void assert_graphcis_context()
			{
				lucheck_msg(m_render_pass_begin, "A graphics command can only be submitted between begin_render_pass and end_render_pass.");
			}
			void assert_compute_context()
			{
				lucheck_msg(m_compute_pass_begin, "A compute command can only be submitted between begin_compute_pass and end_compute_pass.");
			}
			void assert_copy_context()
			{
				lucheck_msg(m_copy_pass_begin, "A copy command can only be submitted between begin_copy_pass and end_copy_pass.");
			}
			void assert_non_render_pass()
			{
				lucheck_msg(!m_render_pass_begin, "This command cannot be submitted between begin_render_pass and end_render_pass.");
			}
			void write_timestamp(IQueryHeap* heap, u32 index);
			void begin_pipeline_statistics_query(IQueryHeap* heap, u32 index);
			void end_pipeline_statistics_query(IQueryHeap* heap, u32 index);

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
			virtual void wait() override;
			virtual bool try_wait() override;
			virtual u32 get_command_queue_index() override { return m_queue_index; }
			virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override;
			virtual void begin_event(const c8* event_name) override;
			virtual void end_event() override;
			virtual void begin_render_pass(const RenderPassDesc& desc) override;
			virtual void set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout) override;
			virtual void set_graphics_pipeline_state(IPipelineState* pso) override;
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views) override;
			virtual void set_index_buffer(const IndexBufferView& view) override;
			virtual void set_graphics_descriptor_set(u32 start_index, IDescriptorSet* descriptor_set) override
			{
				set_graphics_descriptor_sets(start_index, { &descriptor_set, 1 });
			}
			virtual void set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void set_viewport(const Viewport& viewport) override;
			virtual void set_viewports(Span<const Viewport> viewports) override;
			virtual void set_scissor_rect(const RectI& rect) override;
			virtual void set_scissor_rects(Span<const RectI> rects) override;
			virtual void set_blend_factor(const Float4U& blend_factor) override;
			virtual void set_stencil_ref(u32 stencil_ref) override;
			virtual void draw(u32 vertex_count, u32 start_vertex_location) override;
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) override;
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) override;
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) override;
			virtual void begin_occlusion_query(OcclusionQueryMode mode, u32 index) override;
			virtual void end_occlusion_query(u32 index) override;
			virtual void end_render_pass() override;
			virtual void begin_compute_pass(const ComputePassDesc& desc) override;
			virtual void set_compute_pipeline_layout(IPipelineLayout* pipeline_layout) override;
			virtual void set_compute_pipeline_state(IPipelineState* pso) override;
			virtual void set_compute_descriptor_set(u32 start_index, IDescriptorSet* descriptor_set) override
			{
				set_compute_descriptor_sets(start_index, { &descriptor_set, 1 });
			}
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) override;
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) override;
			virtual void end_compute_pass() override;
			virtual void begin_copy_pass(const CopyPassDesc& desc) override;
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
			virtual void end_copy_pass() override;
			virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) override;
			virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) override;
		};
	}
}