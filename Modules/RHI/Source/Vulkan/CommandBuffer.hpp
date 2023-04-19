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
#include "CommandQueue.hpp"

namespace Luna
{
	namespace RHI
	{
		struct CommandBuffer : ICommandBuffer
		{
			lustruct("RHI::CommandBuffer", "{057DBF2F-5817-490B-9683-18A0D3C4C5CB}");

			Ref<Device> m_device;
			Ref<CommandQueue> m_queue;
			Name m_name;

			VkCommandPool m_command_pool = VK_NULL_HANDLE;
			VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;

			RV init(CommandQueue* queue);
			~CommandBuffer();

			//! The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

			RV begin_command_buffer();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual void wait() override;
			virtual bool try_wait() override;
			virtual ICommandQueue* get_command_queue() override { return m_queue.get(); }
			virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override;
			virtual void begin_event(const Name& event_name) override;
			virtual void end_event() override;
			virtual void begin_render_pass(const RenderPassDesc& desc) override;
			virtual void set_pipeline_state(IPipelineState* pso) override;
			virtual void set_graphics_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferViewDesc> views) override;
			virtual void set_index_buffer(const IndexBufferViewDesc& desc) override;
			virtual void set_graphics_descriptor_set(u32 index, IDescriptorSet* descriptor_set) override;
			virtual void set_primitive_topology(PrimitiveTopology primitive_topology) override;
			virtual void set_stream_output_targets(u32 start_slot, Span<const StreamOutputBufferView> views) override;
			virtual void set_viewport(const Viewport& viewport) override;
			virtual void set_viewports(Span<const Viewport> viewports) override;
			virtual void set_scissor_rect(const RectI& rect) override;
			virtual void set_scissor_rects(Span<const RectI> rects) override;
			virtual void set_blend_factor(Span<const f32, 4> blend_factor) override;
			virtual void set_stencil_ref(u32 stencil_ref) override;
			virtual void draw(u32 vertex_count, u32 start_vertex_location) override;
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) override;
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) override;
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) override;
			virtual void clear_depth_stencil_view(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects) override;
			virtual void clear_render_target_view(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects) override;
			virtual void end_render_pass() override;
			virtual void copy_resource(IResource* dest, IResource* src) override;
			virtual void copy_buffer_region(IResource* dest, u64 dest_offset, IResource* src, u64 src_offset, u64 num_bytes) override;
			virtual void copy_texture_region(const TextureCopyLocation& dst, u32 dst_x, u32 dst_y, u32 dst_z,
				const TextureCopyLocation& src, const BoxU* src_box = nullptr) override;
			virtual void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout) override;
			virtual void set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set) override;
			virtual void resource_barrier(const ResourceBarrierDesc& barrier) override;
			virtual void resource_barriers(Span<const ResourceBarrierDesc> barriers) override;
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) override;
			virtual void write_timestamp(IQueryHeap* heap, u32 index) override;
			virtual void begin_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void end_pipeline_statistics_query(IQueryHeap* heap, u32 index) override;
			virtual void begin_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual void end_occlusion_query(IQueryHeap* heap, u32 index) override;
			virtual RV submit() override;
		};
	}
}