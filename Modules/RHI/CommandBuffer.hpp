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

		enum class TextureCopyType : u8
		{
			subresource_index,	// When the referencing resource is a texture.
			placed_footprint	// When the referencing resource is a buffer.
		};

		struct SubresourceFootprint
		{
			Format format;
			u32 width;
			u32 height;
			u32 depth;
			u32 row_pitch;
		};

		struct PlacedResourceFootprint
		{
			u64 offset;
			SubresourceFootprint footprint;
		};

		struct TextureCopyLocation
		{
			IResource* resource;
			TextureCopyType type;
			union
			{
				PlacedResourceFootprint placed_footprint;
				SubresourceIndex subresource_index;
			};
			static TextureCopyLocation as_placed_foorprint(IResource* _resource, u64 _offset, Format _format, u32 _width, u32 _height, u32 _depth, u32 _row_pitch)
			{
				TextureCopyLocation r;
				r.type = TextureCopyType::placed_footprint;
				r.resource = _resource;
				r.placed_footprint.offset = _offset;
				r.placed_footprint.footprint.format = _format;
				r.placed_footprint.footprint.width = _width;
				r.placed_footprint.footprint.height = _height;
				r.placed_footprint.footprint.depth = _depth;
				r.placed_footprint.footprint.row_pitch = _row_pitch;
				return r;
			}
			static TextureCopyLocation as_subresource_index(IResource* _resource, const SubresourceIndex& _subresource_index)
			{
				TextureCopyLocation r;
				r.type = TextureCopyType::subresource_index;
				r.resource = _resource;
				r.subresource_index = _subresource_index;
				return r;
			}
		};

		//! `ResourceStateFlag` defines how resources are bound to the pipeline. One resource may be bind to multiple stages of the 
		//! pipeline, which can be expressed by bitwise-OR of `ResourceStateFlag` flags.
		//! At the beginning of each command buffer, no resources is bound to the pipeline. All resources start with 
		//! `ResourceBindFlag::none`, and will be reset to `ResourceBindFlag::none` automatically at the end of the command buffer.
		//! In each command buffer, before the user binds the resource for the first time, she must issue one resource barrier with
		//! `ResourceBindFlag::none` as the `before` state for the resource to be correctly bind.
		enum class ResourceStateFlag : u32
		{
			//! The content of the resource is undefined.
			//! This can only be specified as the before state of the resource barrier, which tells the system to discard the old 
			//! content of the resource. The resource data is then undefined and should be overwritten in the following render passes.
			undefined = 0x00,
			//! Used as a indirect argument buffer.
			indirect_argument = 0x01,
			//! Used as a vertex buffer.
			vertex_buffer = 0x02,
			//! Used as a index buffer.
			index_buffer = 0x04,
			//! Used as a constant buffer for vertex shader.
			constant_buffer_vs = 0x08,
			//! Used as a shader resource for vertex shader.
			shader_resource_vs = 0x10,
			//! Used as a constant buffer for pixel shader.
			constant_buffer_ps = 0x20,
			//! Used as a shader resource for pixel shader.
			shader_resource_ps = 0x40,
			//! Used as a read-only unordered access for pixel shader.
			unordered_access_read_ps = 0x80,
			//! Used as a write-only unordered access for pixel shader.
			unordered_access_write_ps = 0x0100,
			//! Used as a color attachment with read access.
			color_attachment_read = 0x0200,
			//! Used as a color attachment with write access.
			color_attachment_write = 0x0400,
			//! Used as a depth stencil attachment with read access.
			depth_stencil_attachment_read = 0x0800,
			//! Used as a depth stencil attachment with write access.
			depth_stencil_attachment_write = 0x1000,
			//! Used as a resolve attachment with write access.
			resolve_attachment = 0x2000,
			//! Used as a constant buffer for compute shader.
			constant_buffer_cs = 0x4000,
			//! Used as a shader resource for compute shader.
			shader_resource_cs = 0x8000,
			//! Used as a read-only unordered access for compute shader.
			unordered_access_read_cs = 0x00010000,
			//! Used as a write-only unordered access for compute shader.
			unordered_access_write_cs = 0x00020000,
			//! Used as a copy destination.
			copy_dest = 0x00040000,
			//! Used as a copy source.
			copy_source = 0x00800000,

			//! If this is specified as the before state, the system determines the before state automatically using the last state 
			//! specified in the same command buffer for the resource. If this is the first time the resource is used in the current
			//! command buffer, the system loads the resource's global state automatically.
			//! 
			//! This state cannot be set as the after state of the resource. This state cannot be set along with other state flags.
			automatic = 0x80000000,

			// Combinations.

			constant_buffer = constant_buffer_vs | constant_buffer_ps | constant_buffer_cs,
			shader_resource = shader_resource_vs | shader_resource_ps | shader_resource_cs,
			unordered_access_read = unordered_access_read_ps | unordered_access_read_cs,
			unordered_access_write = unordered_access_write_ps | unordered_access_write_cs,
			unordered_access = unordered_access_read | unordered_access_write,
		};

		constexpr SubresourceIndex RESOURCE_BARRIER_ALL_SUBRESOURCES = {U32_MAX, U32_MAX};
		struct IResource;

		struct ResourceBarrierDesc
		{
			IResource* resource;
			SubresourceIndex subresource;
			ResourceStateFlag before;
			ResourceStateFlag after;
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
			virtual void set_vertex_buffers(u32 start_slot, u32 num_slots, IResource** buffers, const usize* offsets) = 0;

			//! Sets index buffer.
			virtual void set_index_buffer(IResource* buffer, usize offset_in_bytes, Format index_format) = 0;

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
			virtual void copy_resource(IResource* dest, IResource* src) = 0;

			//! Copies a region of a buffer from one resource to another.
			virtual void copy_buffer_region(IResource* dest, u64 dest_offset, IResource* src, u64 src_offset, u64 num_bytes) = 0;

			//! This method uses the GPU to copy texture data between two locations. 
			virtual void copy_texture_region(const TextureCopyLocation& dst, u32 dst_x, u32 dst_y, u32 dst_z,
				const TextureCopyLocation& src, const BoxU* src_box = nullptr) = 0;

			//! Sets the compute shader input layout.
			virtual void set_compute_shader_input_layout(IShaderInputLayout* shader_input_layout) = 0;

			//! Sets the view set to be used by the compute pipeline.
			//! This must be called after `set_pipeline_state`.
			virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

			//! Issues one resource barrier.
			virtual void resource_barrier(Span<const ResourceBarrierDesc> barriers) = 0;

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