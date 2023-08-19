/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.hpp
* @author JXMaster
* @date 2023/7/13
*/
#pragma once
#include "Device.hpp"
#include "QueryHeap.hpp"

namespace Luna
{
    namespace RHI
    {
        struct CommandBuffer : ICommandBuffer
        {
            lustruct("RHI::CommandBuffer", "{da3d7c91-2ae4-407e-81c6-276089faeb40}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::CommandBuffer> m_buffer;
            u32 m_command_queue_index;

            // The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

            NSPtr<MTL::RenderCommandEncoder> m_render;
            NSPtr<MTL::ComputeCommandEncoder> m_compute;
            NSPtr<MTL::BlitCommandEncoder> m_blit;

            IndexBufferView m_index_buffer_view;
            MTL::PrimitiveType m_primitive_type;

            UInt3U m_num_threads_per_group;
            
            // Used if stage boundary counter sample is not supported.
            CounterSampleQueryHeap* m_timestamp_query_heap = nullptr;
            CounterSampleQueryHeap* m_pipeline_statistics_query_heap = nullptr;
            u32 m_timestamp_begin_query_index = DONT_QUERY;
            u32 m_timestamp_end_query_index = DONT_QUERY;
            u32 m_pipeline_statistics_query_index = DONT_QUERY;
            

            RV init(u32 command_queue_index);

            void assert_graphcis_context()
			{
				lucheck_msg(m_render, "A graphics command can only be submitted between begin_render_pass and end_render_pass.");
			}
			void assert_compute_context()
			{
				lucheck_msg(m_compute, "A compute command can only be submitted between begin_compute_pass and end_compute_pass.");
			}
			void assert_copy_context()
			{
				lucheck_msg(m_blit, "A copy command can only be submitted between begin_copy_pass and end_copy_pass.");
			}
			void assert_non_render_pass()
			{
				lucheck_msg(!m_render, "This command cannot be submitted between begin_render_pass and end_render_pass.");
			}
            void assert_no_context()
            {
                lucheck_msg(!m_render && !m_compute && !m_blit, "This command cannot be called in a pass context.");
            }

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_buffer.get(), name); }
            virtual void wait() override;
			virtual bool try_wait() override;
			virtual u32 get_command_queue_index() override { return m_command_queue_index; }
            virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override;
			virtual void begin_event(const c8* event_name) override;
			virtual void end_event() override;
            virtual void begin_render_pass(const RenderPassDesc& desc) override;
			virtual void set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout) override;
			virtual void set_graphics_pipeline_state(IPipelineState* pso) override;
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views) override;
			virtual void set_index_buffer(const IndexBufferView& view) override;
			virtual void set_graphics_descriptor_set(u32 index, IDescriptorSet* descriptor_set) override;
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
			virtual void set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set) override;
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
