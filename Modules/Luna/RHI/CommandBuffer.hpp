/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.hpp
* @author JXMaster
* @date 2020/3/10
*/
#pragma once
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "PipelineLayout.hpp"
#include <Luna/Runtime/Waitable.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include "QueryHeap.hpp"
#include <Luna/Runtime/Span.hpp>
#include "Fence.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class ClearFlag : u8
		{
			none = 0x00,
			depth = 0x01,
			stencil = 0x02
		};
		enum class ResourceBarrierFlag : u8
		{
			none = 0,
			//! Submits an aliasing barrier for this resource.
			//! 
			//! One aliasing barrier is required when you want to use one resource that shares part of its memory with other resources,
			//! and other resources are previously used in the same command buffer. In such state, the device should finish all operations
			//! on the previous resource before it can process commands on the new resource.
			//! 
			//! When submitting aliasing barriers, `buffer` or `texture` should be specified to the new resource being used. The `before` state 
			//! is a combination of states of all previouly used resources on the command buffer. The `before` state can be 
			//! `BufferStateFlag::automatic` or `TextureStateFlag::automatic`, which always emits one full pipeline barrier, but specify 
			//! `before` states precisely may improve performance by avoiding waiting for pipeline stages that does not use the aliasing resource. 
			//! The `after` state is the initial state of the new resource. The resource content is always unspecified, despite whether `discard_content` 
			//! is specified.
			aliasing = 0x01,
			//! Tells the device that the old content of the specified resource does not need to be preserved. The resource content is 
			//! uninitialized after this barrier and should be overwritten in the following commands.
			//! 
			//! Specify this state may help preventing unnecessary availability operations and image layout transfer operations during
			//! the resource barrier, thus improve performance.
			discard_content = 0x02,
		};
		enum class BufferStateFlag : u32
		{
			//! This resource is not used by the pipeline.
			//! Resources with no state flag cannot be used by the pipeline, and must be transfered to one valid state 
			//! before it can be used.
			none = 0x00,
			//! Used as a indirect argument buffer.
			indirect_argument = 0x01,
			//! Used as a vertex buffer.
			vertex_buffer = 0x02,
			//! Used as a index buffer.
			index_buffer = 0x04,
			//! Used as a uniform buffer for vertex shader.
			uniform_buffer_vs = 0x08,
			//! Used as a read-only resource for vertex shader.
			shader_read_vs = 0x10,
			//! Used as a uniform buffer for pixel shader.
			uniform_buffer_ps = 0x20,
			//! Used as a read-only resource for pixel shader.
			shader_read_ps = 0x40,
			//! Used as a write-only resource for pixel shader. Enabled only if pixel shader write feature is supported.
			shader_write_ps = 0x80,
			//! Used as a uniform buffer for compute shader.
			uniform_buffer_cs = 0x0100,
			//! Used as a read-only resource for compute shader.
			shader_read_cs = 0x0200,
			//! Used as a write-only resource for compute shader.
			shader_write_cs = 0x0400,
			//! Used as a copy destination.
			copy_dest = 0x0800,
			//! Used as a copy source.
			copy_source = 0x1000,
			//! If this is specified as the before state, the system determines the before state automatically using the last state 
			//! specified in the same command buffer for the resource. If this is the first time the resource is used in the current
			//! command buffer, the system loads the resource's global state automatically.
			//! 
			//! This state cannot be set as the after state of the resource. This state cannot be set along with other state flags.
			//! state.
			automatic = 0x80000000,

			shader_read_write_ps = shader_read_ps | shader_write_ps,
			shader_read_write_cs = shader_read_cs | shader_write_cs,
		};
		enum class TextureStateFlag : u32
		{
			//! This resource is not used by the pipeline.
			//! Resources with no state flag cannot be used by the pipeline, and must be transfered to one valid state 
			//! before it can be used.
			none = 0x00,
			//! Used as a sampled texture for vertex shader.
			shader_read_vs = 0x01,
			//! Used as a read-only resource for pixel shader.
			shader_read_ps = 0x02,
			//! Used as a write-only resource for pixel shader. Enabled only if pixel shader write feature is supported.
			shader_write_ps = 0x04,
			//! Used as a color attachment with read access.
			color_attachment_read = 0x08,
			//! Used as a color attachment with write access.
			color_attachment_write = 0x10,
			//! Used as a depth stencil attachment with read access.
			depth_stencil_attachment_read = 0x20,
			//! Used as a depth stencil attachment with write access.
			depth_stencil_attachment_write = 0x40,
			//! Used as a resolve attachment with write access.
			resolve_attachment = 0x80,
			//! Used as a shader resource for compute shader.
			shader_read_cs = 0x0100,
			//! Used as a read-only unordered access for compute shader.
			shader_write_cs = 0x0200,
			//! Used as a copy destination.
			copy_dest = 0x0400,
			//! Used as a copy source.
			copy_source = 0x0800,
			//! Used for swap chain presentation.
			present = 0x1000,
			//! If this is specified as the before state, the system determines the before state automatically using the last state 
			//! specified in the same command buffer for the resource. If this is the first time the resource is used in the current
			//! command buffer, the system loads the resource's global state automatically.
			//! 
			//! This state cannot be set as the after state of the resource. This state cannot be set along with other state flags.
			automatic = 0x80000000,

			shader_read_write_ps = shader_read_ps | shader_write_ps,
			shader_read_write_cs = shader_read_cs | shader_write_cs,
		};
		constexpr SubresourceIndex TEXTURE_BARRIER_ALL_SUBRESOURCES = {U32_MAX, U32_MAX};
		struct TextureBarrier
		{
			ITexture* texture;
			SubresourceIndex subresource;
			TextureStateFlag before;
			TextureStateFlag after;
			ResourceBarrierFlag flags;
			TextureBarrier() = default;
			TextureBarrier(ITexture* texture, SubresourceIndex subresource, TextureStateFlag before, TextureStateFlag after, ResourceBarrierFlag flags = ResourceBarrierFlag::none) :
				texture(texture),
				subresource(subresource),
				before(before),
				after(after),
				flags(flags) {}
		};
		struct BufferBarrier
		{
			IBuffer* buffer;
			BufferStateFlag before;
			BufferStateFlag after;
			ResourceBarrierFlag flags;
			BufferBarrier() = default;
			BufferBarrier(IBuffer* buffer, BufferStateFlag before, BufferStateFlag after, ResourceBarrierFlag flags = ResourceBarrierFlag::none) :
				buffer(buffer),
				before(before),
				after(after),
				flags(flags) {}
		};
		struct Viewport
		{
			f32 top_left_x;
			f32 top_left_y;
			f32 width;
			f32 height;
			f32 min_depth;
			f32 max_depth;

			Viewport() = default;
			Viewport(
				f32 top_left_x,
				f32 top_left_y,
				f32 width,
				f32 height,
				f32 min_depth,
				f32 max_depth) :
				top_left_x(top_left_x),
				top_left_y(top_left_y),
				width(width),
				height(height),
				min_depth(min_depth),
				max_depth(max_depth) {}
		};
		//! The operation to perform when the attachment is loaded to GPU.
		enum class LoadOp : u8
		{
			//! The previous contents of the attachment does not need to be preserved. 
			//! If this is specified, your application should not have any assumptions on 
			//! the initial data of the attachment, and should overwrite the data in render
			//! pass.
			dont_care = 0,
			//! The previous contents of the attachment shall be preserved.
			load = 1,
			//! The contents within of the attachment will be cleared to a uniform value.
			clear = 2,
		};
		//! The operation to perform when the render texture is written back to resource memory.
		enum class StoreOp : u8
		{
			//! The content of the attachment will be discarded and not stored back to the resource 
			//! after ending the render pass.
			//! This is used for attachments like depth buffer that is only used for the current 
			//! render pass.
			dont_care = 0,
			//! Stores the content of the attachment to the resource after ending the render pass.
			store = 1
		};
		struct ColorAttachment
		{
			ITexture* texture = nullptr;
			LoadOp load_op = LoadOp::dont_care;
			StoreOp store_op = StoreOp::dont_care;
			Float4U clear_value = { 0, 0, 0, 0 };
			TextureViewType view_type = TextureViewType::unspecified;
			Format format = Format::unknown;
			u32 mip_slice = 0;
			u32 array_slice = 0;
			ColorAttachment() = default;
			ColorAttachment(ITexture* texture, 
				LoadOp load_op = LoadOp::load, StoreOp store_op = StoreOp::store, const Float4U& clear_value = Float4U(0), 
				TextureViewType view_type = TextureViewType::unspecified, Format format = Format::unknown, u32 mip_slice = 0, u32 array_slice = 0) :
				texture(texture),
				load_op(load_op),
				store_op(store_op),
				clear_value(clear_value),
				view_type(view_type),
				format(format),
				mip_slice(mip_slice),
				array_slice(array_slice) {}
		};
		struct DepthStencilAttachment
		{
			ITexture* texture = nullptr;
			LoadOp depth_load_op = LoadOp::dont_care;
			StoreOp depth_store_op = StoreOp::dont_care;
			LoadOp stencil_load_op = LoadOp::dont_care;
			StoreOp stencil_store_op = StoreOp::dont_care;
			f32 depth_clear_value = 0.0f;
			u8 stencil_clear_value = 0;
			bool read_only = false;
			TextureViewType view_type = TextureViewType::unspecified;
			Format format = Format::unknown;
			u32 mip_slice = 0;
			u32 array_slice = 0;
			DepthStencilAttachment() = default;
			DepthStencilAttachment(ITexture* texture, bool read_only,
				LoadOp depth_load_op = LoadOp::load, StoreOp depth_store_op = StoreOp::store, f32 depth_clear_value = 1.0f,
				LoadOp stencil_load_op = LoadOp::dont_care, StoreOp stencil_store_op = StoreOp::dont_care, u8 stencil_clear_value = 0,
				TextureViewType view_type = TextureViewType::unspecified, Format format = Format::unknown, u32 mip_slice = 0, u32 array_slice = 0
				) :
				texture(texture),
				read_only(read_only),
				depth_load_op(depth_load_op),
				depth_store_op(depth_store_op),
				depth_clear_value(depth_clear_value),
				stencil_load_op(stencil_load_op),
				stencil_store_op(stencil_store_op),
				stencil_clear_value(stencil_clear_value),
				view_type(view_type),
				format(format),
				mip_slice(mip_slice),
				array_slice(array_slice) {}
		};
		struct ResolveAttachment
		{
			ITexture* texture = nullptr;
			u32 mip_slice = 0;
			u32 array_slice = 0;
			u32 array_size = 0;
			ResolveAttachment() = default;
			ResolveAttachment(ITexture* texture, u32 mip_slice = 0, u32 array_slice = 0) :
				texture(texture),
				mip_slice(mip_slice),
				array_slice(array_slice) {}
		};
        constexpr u32 DONT_QUERY = U32_MAX;
		enum class OcclusionQueryMode : u8
		{
			//! Begins a binary occlusion query. In this query mode, the stored value will be 0 if no pixel passes the depth/stencil test, 
			//! and will be non-zero if any pixel passes the depth/stencil test. Note that the stored value is platform-dependent if it is not 0, 
			//! and may not always be 1.
			binary = 0,
			//! Begins a counting occlusion query. In this query mode, the exact number of pixels that pass the depth/stencil test will
			//! be stored.
			counting = 1,
		};
		//! Parameters passed to `begin_render_pass`.
		struct RenderPassDesc
		{
			//! The color attachments to set.
			ColorAttachment color_attachments[8];
			//! The resolve attachments to set.
			ResolveAttachment resolve_attachments[8];
			//! The depth stencil attachment to set.
			DepthStencilAttachment depth_stencil_attachment;
			//! The occlustion query heap that accepts the query data if not `nullptr`.
			IQueryHeap* occlusion_query_heap = nullptr;
            IQueryHeap* timestamp_query_heap = nullptr;
            IQueryHeap* pipeline_statistics_query_heap = nullptr;
			//! The occlusion query writing index.
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
            u32 pipeline_statistics_query_write_index = DONT_QUERY;
			//! The number of array slices that will be bound for all attachments.
			u32 array_size = 1;
			//! The sample count of the render pass.
			u8 sample_count = 1;
		};
    
        struct ComputePassDesc
        {
            IQueryHeap* timestamp_query_heap = nullptr;
            IQueryHeap* pipeline_statistics_query_heap = nullptr;
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
            u32 pipeline_statistics_query_write_index = DONT_QUERY;
        };
    
        struct CopyPassDesc
        {
            IQueryHeap* timestamp_query_heap = nullptr;
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
        };
    
		struct VertexBufferView
		{
			IResource* buffer;
			//! The offset, in bytes, of the first vertex from the beginning of the buffer.
			usize offset;
			//! The size, in bytes, of the vertex buffer range to bind.
			u32 size;
			//! The size, in butes, of every vertex element in the vertex buffer.
			//! The element size must be equal to `InputBindingDesc::element_size` of the 
			//! pipeline state object used to draw this vertex buffer.
			u32 element_size;

			VertexBufferView() = default;
			VertexBufferView(IResource* buffer, usize offset, u32 size, u32 element_size) :
				buffer(buffer),
				offset(offset),
				size(size),
				element_size(element_size) {}
		};
		struct IndexBufferView
		{
			IResource* buffer;
			//! The offset, in bytes, of the first vertex from the beginning of the buffer.
			usize offset;
			//! The size, in bytes, of the vertex buffer range to bind.
			u32 size;
			//! The index format.
			Format format;

			IndexBufferView() = default;
			IndexBufferView(IResource * buffer, usize offset, u32 size, Format format) :
				buffer(buffer),
				offset(offset),
				size(size),
				format(format) {}
		};
		//! @interface ICommandBuffer
		//! The command buffer is used to allocate memory for commands, record commands, submitting 
		//! commands to GPU and tracks the state of the submitted commands.
		//! 
		//! Command buffer is not thread safe. If the user need to record commands simultaneously, she 
		//! should create multiple command buffers, one per thread. 
		//! 
		//! All synchroizations for command buffers are performed explicitly, for instance:
		//! 1. Use `ICommandBuffer::wait` to wait for one command buffer from host side, 
		//! 2. Use fence objects to wait for one command buffer from another command buffer.
		//! 3. Only call `ICommandBuffer::reset` after the command buffer is not submitted, or is finished by GPU.
		struct ICommandBuffer : virtual IDeviceChild, virtual IWaitable
		{
			luiid("{2970a4c8-d905-4e58-9247-46ba6a33b220}");

			//! Gets the command queue index of the command queue attached to 
			//! this buffer.
			virtual u32 get_command_queue_index() = 0;

			//! Resets the command buffer. This call clears all commands in the command buffer, resets the state tracking
			//! infrastructure and reopens the command buffer for recording new commands.
			//! 
			//! You should only call this after the command buffer has finished execution by the command queue,
			//! or the behavior is undefined. In order to make sure all commands are executed by GPU, call
			//! `wait` to block the thread until this buffer gets finished, or you can use `try_wait` to test
			//! if the buffer has finished execution.
			virtual RV reset() = 0;

			//! Attaches one graphic device object to this command buffer. The command buffer keeps a strong reference 
			//! to the object until the next `reset` is called.
			//! This is mainly used to keep references to the graphic objects used by the current command buffer, so they
			//! will not be released before GPU finishes accessing them.
			virtual void attach_device_object(IDeviceChild* obj) = 0;

			//! Begins a new event. This is for use in diagnostic tools like RenderDoc, PIX, etc to group commands into hierarchical
			//! sections.
			virtual void begin_event(const c8* event_name) = 0;

			//! Ends the latest event opened with `begin_event` that has not been ended.
			virtual void end_event() = 0;

			//! Starts a new render pass. The previous render pass should be closed before beginning another one.
			//! @param[in] desc The render pass descriptor object.
			//! @ResourceStateFlag:: to let LoadOp::clear works, the render textures that need to be cleared must be in
			//! `ResourceStateFlag::render_target` state and the depth stencil texture that needs to be cleared must be in 
			//! `ResourceState::depth_write` state.
			//! 
			//! The following functions can only be called in between `begin_render_pass` and `end_render_pass`:
			//! * set_graphics_pipeline_layout
			//! * set_graphics_pipeline_state
			//! * set_vertex_buffers
			//! * set_index_buffer
			//! * set_graphics_descriptor_set
			//! * set_graphics_descriptor_sets
			//! * set_viewport
			//! * set_viewports
			//! * set_scissor_rect
			//! * set_scissor_rects
			//! * set_blend_factor
			//! * set_stencil_ref
			//! * draw
			//! * draw_indexed
			//! * draw_instanced
			//! * draw_indexed_instanced
			//! * clear_color_attachment
			//! * clear_depth_stencil_attachment
			//! The following functions can only be called outside of one render pass range:
			//! * submit
			//! * set_context
			//! * resource_barrier
			virtual void begin_render_pass(const RenderPassDesc& desc) = 0;

			//! Sets the graphic pipeline layout.
			virtual void set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout) = 0;

			//! Sets the pipeline state for graphics pipeline.
			virtual void set_graphics_pipeline_state(IPipelineState* pso) = 0;

			//! Sets vertex buffers.
			//! @param[in] start_slot The start slot of the vertex buffer to set.
			//! @param[in] views An array of vertex buffer views to set, each describes one vertex buffer range to bind.
			//! The vertex buffer views will be set in slot [start_slot, start_slot + views.size()).
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views) = 0;

			//! Sets index buffer.
			//! @param[in] buffer The index buffer to set.
			//! @param[in] offset The offset, in bytes, of the first index from the beginning of the buffer.
			//! @param[in] size 
			virtual void set_index_buffer(const IndexBufferView& view) = 0;

			//! Sets the descriptor set to be used by the graphic pipeline.
			//! This behaves the same as calling `set_graphics_descriptor_sets` with only one element.
			virtual void set_graphics_descriptor_set(u32 index, IDescriptorSet* descriptor_set) = 0;

			//! Sets descriptor sets to be used by the graphic pipeline.
			//! This must be called after `set_graphics_pipeline_state` and `set_graphics_pipeline_layout`.
			virtual void set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

			//! Bind one viewport to the rasterizer stage of the pipeline.
			//! This operation behaves the same as calling `set_viewports` with only one viewport.
			virtual void set_viewport(const Viewport& viewport) = 0;

			//! Bind an array of viewports to the rasterizer stage of the pipeline.
			//! All viewports must be set atomically as one operation. Any viewports not defined by the call are disabled.
			virtual void set_viewports(Span<const Viewport> viewports) = 0;

			//! Binds one scissor rectangle to the rasterizer stage.
			//! This operation behaves the same as calling `set_scissor_rects` with only one scissor rect.
			virtual void set_scissor_rect(const RectI& rect) = 0;

			//! Binds an array of scissor rectangles to the rasterizer stage.
			//! The scissor rectangle points are relative to the top-left corner of the render target, 
			//! with x-axis points to right and y-axis points to down.
			//! All scissor rectangles must be set atomically as one operation. Any scissor rectangles not defined by the call are disabled.
			virtual void set_scissor_rects(Span<const RectI> rects) = 0;

			//! Sets the blend factor that modulate values for a pixel shader, render target, or both.
			virtual void set_blend_factor(const Float4U& blend_factor) = 0;

			//! Sets the reference value for depth stencil tests.
			virtual void set_stencil_ref(u32 stencil_ref) = 0;

			//! Draw primitives.
			virtual void draw(u32 vertex_count, u32 start_vertex_location) = 0;

			//! Draw indexed primitives.
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) = 0;

			//! Draws non-indexed, instanced primitives.
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) = 0;

			//! Draws indexed, instanced primitives.
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) = 0;
            
            virtual void begin_occlusion_query(OcclusionQueryMode mode, u32 index) = 0;

            virtual void end_occlusion_query(u32 index) = 0;

			//! Finishes the current render pass.
			virtual void end_render_pass() = 0;

			//! Begins a compute pass.
			//! The following functions can only be called in between `begin_compute_pass` and `end_compute_pass`:
			//! * set_compute_pipeline_layout
			//! * set_compute_pipeline_state
			//! * set_compute_descriptor_set
			//! * set_compute_descriptor_sets
			//! * dispatch
			virtual void begin_compute_pass(const ComputePassDesc& desc = ComputePassDesc()) = 0;

			//! Sets the compute pipeline layout.
			virtual void set_compute_pipeline_layout(IPipelineLayout* pipeline_layout) = 0;

			//! Sets the pipeline state for compute pipeline.
			virtual void set_compute_pipeline_state(IPipelineState* pso) = 0;

			//! Sets the descriptor set to be used by the compute pipeline.
			//! This behaves the same as calling `set_compute_descriptor_sets` with only one element.
			virtual void set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set) = 0;

			//! Sets descriptor sets to be used by the compute pipeline.
			//! This must be called after `set_compute_pipeline_state` and `set_compute_pipeline_layout`.
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

			//! Executes a command list from a thread group.
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) = 0;

			//! Ends a compute pass.
			virtual void end_compute_pass() = 0;

			//! Begins a copy pass.
			//! The following functions can only be called in between `begin_copy_pass` and `end_copy_pass`:
			//! * copy_resource
			//! * copy_buffer
			//! * copy_texture
			//! * copy_buffer_to_texture
			//! * copy_texture_to_buffer
			virtual void begin_copy_pass(const CopyPassDesc& desc = CopyPassDesc()) = 0;

			//! Copies the entire contents of the source resource to the destination resource.
			//! The source resource and destination resource must be the same `ResourceType`, have the same size for buffers, or 
			//! the same width, height, depth, mip count and array count for textures.
			virtual void copy_resource(IResource* dst, IResource* src) = 0;

			//! Copies a region of a buffer from one resource to another.
			virtual void copy_buffer(
				IBuffer* dst, u64 dst_offset,
				IBuffer* src, u64 src_offset,
				u64 copy_bytes) = 0;

			//! This method uses the GPU to copy texture data between two locations. 
			virtual void copy_texture(
				ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

			virtual void copy_buffer_to_texture(
				ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
				IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
				u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

			virtual void copy_texture_to_buffer(
				IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

			//! Ends a copy pass.
			virtual void end_copy_pass() = 0;

			//! Issues one resource barrier.
			virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) = 0;

			//! Submits the recorded content in this command buffer to the attached command queue.
			//! The command buffer can only be submitted once, and the only operation after the submit is to 
			//! reset the command buffer after it is executed by command queue.
			//! @param[in] wait_fences The fence objects to wait before this command buffer can be processed by the system.
			//! @param[in] signal_fences The fence objects to signal after this command buffer is completed.
			//! @param[in] allow_host_waiting Whether `ICommandBuffer::wait` can be used to wait for the command buffer 
			//! from host side. If this is `false`, the command buffer cannot be waited from host, and the behavior of 
			//! calling `ICommandBuffer::wait` is undefined. Setting this to `false` may improve queue performance, and 
			//! the command buffer can still be waited by other command buffers using fences.
			//! 
			//! @remark Command buffers submitted to the same command queue are processed by their submission order.
			//! The system is allowed to merge commands in adjacent submissions as if they are recorded in the same command buffer,
			//! thus different submissions may execute out of order if they have no memory dependency. To prevent synchronization hazard,
			//! always insert resource barriers before using resources in command buffers.
			//! 
			//! If `signal_fences` is not empty, the system guarantees that all commands in the submission is finished, and all 
			//! writes to the memory in the submission is made visible before fences are signaled.
			virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) = 0;
		};
	}
}
