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
#include "ShaderInputLayout.hpp"
#include <Runtime/Waitable.hpp>
#include <Runtime/Math/Vector.hpp>
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "QueryHeap.hpp"
#include <Runtime/Span.hpp>
#include "ResolveTargetView.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class CommandQueueType : u8
		{
			graphics = 1,
			compute = 2,
			copy = 3,
		};

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
			//! When submitting aliasing barriers, `resource` should be specified to the new resource being used. The `before` state 
			//! is a combination of states of all previouly used resources on the command buffer. The `before` state can be 
			//! `BufferStateFlag::automatic`, which always emits one full pipeline barrier, but specify before states precisely may 
			//! improve performance by avoiding waiting for pipeline stages that does not use the aliasing resource. The `after` state
			//! is the initial state of the new resource. The resource content is always unspecified, despite whether `discard_content` 
			//! is specified.
			aliasing = 0x01,
			//! Tells the device that the old content of the specified resource does not need to be preserved. The resource content is 
			//! uninitialized after this barrier and should be overwritten in the following commands.
			//! 
			//! Specify this state may help preventing unnecessary availability operations and image layout transfer operations during
			//! the resource barrier, thus improve performance.
			discard_content = 0x02,
		};

		//! `BufferStateFlag` defines how resources are bound to the pipeline. One resource may be bind to multiple stages of the 
		//! pipeline, which can be expressed by bitwise-OR of `ResourceStateFlag` flags.
		//! At the beginning of each command buffer, no resources is bound to the pipeline. All resources start with 
		//! `ResourceBindFlag::none`, and will be reset to `ResourceBindFlag::none` automatically at the end of the command buffer.
		//! In each command buffer, before the user binds the resource for the first time, she must issue one resource barrier with
		//! `ResourceBindFlag::none` as the `before` state for the resource to be correctly bind.
		enum class BufferStateFlag : u32
		{
			//! If this is specified as the before state, the system determines the before state automatically using the last state 
			//! specified in the same command buffer for the resource. If this is the first time the resource is used in the current
			//! command buffer, the system loads the resource's global state automatically.
			//! 
			//! This state cannot be set as the after state of the resource. Any non-zero states are not considered to be automatic 
			//! state.
			automatic = 0x00,
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
			//! Used as a uniform buffer for compute shader.
			uniform_buffer_cs = 0x80,
			//! Used as a read-only resource for compute shader.
			shader_read_cs = 0x0100,
			//! Used as a write-only resource for compute shader.
			shader_write_cs = 0x0200,
			//! Used as a copy destination.
			copy_dest = 0x0400,
			//! Used as a copy source.
			copy_source = 0x0800,
		};

		enum class TextureStateFlag : u32
		{
			//! If this is specified as the before state, the system determines the before state automatically using the last state 
			//! specified in the same command buffer for the resource. If this is the first time the resource is used in the current
			//! command buffer, the system loads the resource's global state automatically.
			//! 
			//! This state cannot be set as the after state of the resource. This state cannot be set along with other state flags.
			automatic = 0x00,
			//! Used as a sampled texture for vertex shader.
			shader_read_vs = 0x10,
			//! Used as a shader resource for pixel shader.
			shader_read_ps = 0x20,
			//! Used as a color attachment with read access.
			color_attachment_read = 0x40,
			//! Used as a color attachment with write access.
			color_attachment_write = 0x80,
			//! Used as a depth stencil attachment with read access.
			depth_stencil_attachment_read = 0x0100,
			//! Used as a depth stencil attachment with write access.
			depth_stencil_attachment_write = 0x0200,
			//! Used as a resolve attachment with write access.
			resolve_attachment = 0x0400,
			//! Used as a shader resource for compute shader.
			shader_read_cs = 0x0800,
			//! Used as a read-only unordered access for compute shader.
			shader_write_cs = 0x1000,
			//! Used as a copy destination.
			copy_dest = 0x2000,
			//! Used as a copy source.
			copy_source = 0x4000,
			//! Used for swap chain presentation.
			present = 0x8000,
		};

		constexpr SubresourceIndex RESOURCE_BARRIER_ALL_SUBRESOURCES = {U32_MAX, U32_MAX};

		struct TextureBarrier
		{
			ITexture* texture;
			SubresourceIndex subresource;
			TextureStateFlag before;
			TextureStateFlag after;
			ResourceBarrierFlag flags;
		};

		struct BufferBarrier
		{
			IBuffer* buffer;
			BufferStateFlag before;
			BufferStateFlag after;
			ResourceBarrierFlag flags;
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

		//! Parameters passed to `begin_render_pass`.
		struct RenderPassDesc
		{
			//! The color attachment to set.
			IRenderTargetView* color_attachments[8] = { nullptr };
			//! The color attachment to set.
			IResolveTargetView* resolve_attachments[8] = { nullptr };
			//! The depth stencil attachment to set.
			IDepthStencilView* depth_stencil_attachment = nullptr;
			//! The load operations for color attachments. 
			//! If the corresponding color attachment resource is `nullptr`, this operation is ignored.
			LoadOp color_load_ops[8] = { LoadOp::dont_care };
			//! The store operations for color attachments.
			//! If the corresponding color attachment resource is `nullptr`, this operation is ignored.
			StoreOp color_store_ops[8] = { StoreOp::dont_care };
			//! The load operation for depth component.
			//! If the depth stencil attachment is `nullptr`, this operation is ignored.
			LoadOp depth_load_op = LoadOp::dont_care;
			//! The store operation for depth component.
			//! If the depth stencil attachment is `nullptr`, this operation is ignored.
			StoreOp depth_store_op = StoreOp::dont_care;
			//! The load operation for stencil component.
			//! If the depth stencil attachment is `nullptr`, this operation is ignored.
			LoadOp stencil_load_op = LoadOp::dont_care;
			//! The store operation for stencil component.
			//! If the depth stencil attachment is `nullptr`, this operation is ignored.
			StoreOp stencil_store_op = StoreOp::dont_care;
			//! The clear value for color attachments if `color_load_ops` specified for 
			//! the color attachment is `LoadOp::clear`.
			Float4U color_clear_values[8] = { Float4U(0.0f, 0.0f, 0.0f, 0.0f) };
			//! The depth value to use for clear if `depth_load_op` specified in render pass
			//! is `LoadOp::clear`.
			f32 depth_clear_value = 0.0f;
			//! The stencil value to use for clear if `stencil_load_op` specified in render pass
			//! is `LoadOp::clear`.
			u8 stencil_clear_value = 0;
			//! The sample count of the render pass.
			u8 sample_count = 1;
		};

		enum class PipelineStateBindPoint : u8
		{
			graphics = 0,
			compute = 1
		};

		struct ICommandQueue;

		struct VertexBufferBind
		{
			IResource* buffer;
			usize offset_in_bytes;
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

			virtual ICommandQueue* get_command_queue() = 0;

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
			virtual void begin_event(const Name& event_name) = 0;

			//! Ends the latest event begun with `begin_event` that has not benn ended.
			virtual void end_event() = 0;

			//! Starts a new render pass. The previous render pass should be closed before beginning another one.
			//! @param[in] desc The render pass descriptor object.
			//! @ResourceStateFlag:: to let LoadOp::clear works, the render textures that need to be cleared must be in
			//! `ResourceStateFlag::render_target` state and the depth stencil texture that needs to be cleared must be in 
			//! `ResourceState::depth_write` state.
			//! 
			//! The following functions can only be called in between `begin_render_pass` and `end_render_pass`:
			//! * draw
			//! * draw_indexed
			//! * draw_instanced
			//! * draw_indexed_instanced
			//! * clear_color_attachment
			//! * clear_depth_stencil_attachment
			//! * set_scissor_rects
			//! The following functions can only be called outside of one render pass range:
			//! * submit
			virtual void begin_render_pass(const RenderPassDesc& desc) = 0;

			//! Sets the pipeline state.
			virtual void set_pipeline_state(PipelineStateBindPoint bind_point, IPipelineState* pso) = 0;

			//! Sets the graphic shader input layout.
			virtual void set_graphics_shader_input_layout(IShaderInputLayout* shader_input_layout) = 0;

			//! Sets vertex buffers.
			//! @param[in] start_slot The start slot of the vertex buffer to set.
			//! @param[in] num_slots The number of vertex buffers to set.
			//! Vertex buffers in range [start_slot, start_slot + num_slots) will be replaced by new buffer.
			//! @param[in] buffers An array of vertex buffers to bind.
			//! @param[in] offsets An array of offsets for reading vertices from each vertex buffer in bytes.
			virtual void set_vertex_buffers(u32 start_slot, u32 num_slots, IBuffer** buffers, const usize* offsets) = 0;

			//! Sets index buffer.
			virtual void set_index_buffer(IBuffer* buffer, usize offset_in_bytes, Format index_format) = 0;

			//! Sets the descriptor set to be used by the graphic pipeline.
			//! This must be called after `set_pipeline_state` and `set_graphics_shader_input_layout`.
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
			//! The scissor rectangle points are relative to the bottom-left corner of the render target, 
			//! with x-axis points to right and y-axis points to up.
			//! All scissor rectangles must be set atomically as one operation. Any scissor rectangles not defined by the call are disabled.
			virtual void set_scissor_rects(Span<const RectI> rects) = 0;

			//! Sets the blend factor that modulate values for a pixel shader, render target, or both.
			virtual void set_blend_factor(Span<const f32, 4> blend_factor) = 0;

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

			//! Preforms one indirect draw.
			//virtual void draw_indexed_indirect(IResource* buffer, usize argment_offset, u32 draw_count, u32 stride) = 0;

			//! Clears the depth stencil view bound to the current render pass.
			virtual void clear_depth_stencil_attachment(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects) = 0;

			//! Clears the render target view bound to the current render pass.
			//! @param[in] index The index of the render target view to clear in the frame buffers.
			virtual void clear_color_attachment(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects) = 0;

			//! Finishes the current render pass.
			virtual void end_render_pass() = 0;

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
				IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_depth_pitch,
				u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

			virtual void copy_texture_to_buffer(
				IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
				ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
				u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

			virtual void copy_texture_to_buffer() = 0;

			//! Sets the compute shader input layout.
			virtual void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout) = 0;

			//! Sets the view set to be used by the compute pipeline.
			//! This must be called after `set_pipeline_state`.
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

			//! Issues one resource barrier.
			virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) = 0;

			//! Executes a command list from a thread group.
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) = 0;

			//! Writes the current GPU queue timestamp to the specified query heap.
			//! @param[in] heap The query heap to write to.
			//! @param[in] index The index of the query entry to write in the heap.
			virtual void write_timestamp(IQueryHeap* heap, u32 index) = 0;

			virtual void begin_pipeline_statistics_query(IQueryHeap* heap, u32 index) = 0;

			virtual void end_pipeline_statistics_query(IQueryHeap* heap, u32 index) = 0;

			virtual void begin_occlusion_query(IQueryHeap* heap, u32 index) = 0;

			virtual void end_occlusion_query(IQueryHeap* heap, u32 index) = 0;

			//virtual void dispatch_indirect(IResource* buffer, usize offset) = 0;

			//! Submits the recorded content in this command buffer to the attached command queue.
			//! The command buffer can only be submitted once, and the only operation after the submit is to 
			//! reset the command buffer after it is executed by command queue.
			//! @param[in] wait_fences The fence objects to wait before this command buffer can be processed.
			//! @param[in] signal_fences The fence objects to signal after this command buffer is completed.
			//! @param[in] allow_host_waiting Whether `ICommandBuffer::wait` can be used to wait for the command buffer 
			//! from host side. If this is `false`, the command buffer cannot be waited from host, and the behavior of 
			//! calling `ICommandBuffer::wait` is undefined. Setting this to `false` may improve queue performance, and 
			//! the command buffer can still be waited by other command buffers using fences.
			//! 
			//! @remark Synchronizations between command buffers are specified explicitly using fences when submitting
			//! command buffers. For command buffers (A) and (B) submitted to the same command queue in order (A -> B), 
			//! the system is allowed to execute (B) after (A) is started and before (A) is completed. If you want to
			//! defer the execution of (B) until (A) is completed, using one fence (F) as the signal fence of (A) and the
			//! wait fence of (B) to synchronize them explicitly.
			virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) = 0;
		};
	}
}