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
#include <Runtime/Span.hpp>

namespace Luna
{
	namespace RHI
	{
		enum class CommandQueueType : u32
		{
			graphic = 1,
			compute = 2,
			copy = 3,
		};

		enum class ClearFlag : u32
		{
			none = 0x00,
			depth = 0x01,
			stencil = 0x02
		};

		enum class TextureCopyType : u32
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
				u32 subresource_index;
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

			static TextureCopyLocation as_subresource_index(IResource* _resource, u32 _subresource_index)
			{
				TextureCopyLocation r;
				r.type = TextureCopyType::subresource_index;
				r.resource = _resource;
				r.subresource_index = _subresource_index;
				return r;
			}
		};

		enum class ResourceBarrierType : u32
		{
			//! Translates the state of the resource to a new state. In most of the time you don't need to call this 
			//! manually, since the resource state tracking system built in RHI tracks the resource state and inserts
			//! proper barriers for you, however you can explicitly specify this if you need the resource to be in a 
			//! specific state that cannot be converted automatically by system, or you can explicitly specify this
			//! to do some optimizations.
			transition = 1,
			//! Issues an aliasing barrier when a new resource whose memory overlaps with the old resource is about
			//! to be access by the graphic pipeline. This call inserts a memory barrier so that the access to the new
			//! resource is deferred until all access to the old resource finished to prevent race condition.
			aliasing = 2,
			//! Issues an uav barrier that makes the previous dispatch call to access the specified resource finishes 
			//! before the next dispatch call that access the resource can start. This this to prevent race condition
			//! between multiple dispatch calls.
			uav = 3
		};

		enum class ResourceState : u32
		{
			//! The state used for passing resources between different graphic engine types (graphic/compute/copy).
			//! This state is also required if you need to read/write data in CPU side.
			common = 0,
			//! Used as vertex and constant buffer in 3D pipeline by vertex buffer view and index buffer view.
			//! Read-only.
			vertex_and_constant_buffer,
			//! Used as index buffer in graphic pipeline.
			//! Read-only.
			index_buffer,
			//! Used for render target in graphic pipeline via RTV or clear render target.
			//! Write-only.
			render_target,
			//! Used for unordered access in pipeline via UAV.
			//! Read/Write.
			unordered_access,
			//! Used for depth write in graphic pipeline via DSV or clear depth stencil.
			//! Write-only for depth.
			depth_stencil_write,
			//! Used for depth write is disabled.
			//! Read-only for depth.
			depth_stencil_read,
			//! Used as shader resource for non-pixel shader via SRV.
			//! Read-only.
			shader_resource_non_pixel,
			//! Used as shader resource for pixel shader via SRV.
			//! Read-only.
			shader_resource_pixel,
			//! Used as stream output.
			//! Write-only.
			stream_out,
			//! Used as indirect argument.
			//! Read-only.
			indirect_argument,
			//! Used as a copy destination.
			//! Write-only.
			copy_dest,
			//! Used as a copy source.
			//! Read-only.
			copy_source,
			//! Used as destination in a resolve operation.
			resolve_dest,
			//! Used as source in a resolve operation.
			resolve_src,
		};

		enum class ResourceBarrierFlag : u32
		{
			none = 0x00,
			begin_only = 0x01,
			end_only = 0x02,
		};

		constexpr u32 resource_barrier_all_subresources_v = U32_MAX;
		struct IResource;

		struct ResourceTransitionBarrierDesc
		{
			IResource* resource;
			u32 subresource;
			ResourceState after;
		};

		struct ResourceUAVBarrierDesc
		{
			IResource* resource;
		};

		struct ResourceAliasingBarrierDesc
		{
			IResource* resource;
		};

		struct ResourceBarrierDesc
		{
			ResourceBarrierType type;
			ResourceBarrierFlag flags;
			ResourceTransitionBarrierDesc transition;
			ResourceAliasingBarrierDesc aliasing;
			ResourceUAVBarrierDesc uav;

			ResourceBarrierDesc() = default;
			ResourceBarrierDesc(const ResourceBarrierDesc&) = default;
			ResourceBarrierDesc(ResourceBarrierDesc&&) = default;
			ResourceBarrierDesc& operator=(const ResourceBarrierDesc&) = default;
			ResourceBarrierDesc& operator=(ResourceBarrierDesc&&) = default;
			ResourceBarrierDesc(const ResourceTransitionBarrierDesc& transition, ResourceBarrierFlag flags) :
				type(ResourceBarrierType::transition),
				transition(transition),
				flags(flags) {}
			ResourceBarrierDesc(const ResourceAliasingBarrierDesc& aliasing, ResourceBarrierFlag flags) :
				type(ResourceBarrierType::aliasing),
				aliasing(aliasing),
				flags(flags) {}
			ResourceBarrierDesc(const ResourceUAVBarrierDesc& uav, ResourceBarrierFlag flags) :
				type(ResourceBarrierType::uav),
				uav(uav),
				flags(flags) {}

			static ResourceBarrierDesc as_transition(IResource* resource, ResourceState after, u32 subresource = resource_barrier_all_subresources_v, ResourceBarrierFlag flags = ResourceBarrierFlag::none)
			{
				ResourceTransitionBarrierDesc desc;
				desc.resource = resource;
				desc.subresource = subresource;
				desc.after = after;
				return ResourceBarrierDesc(desc, flags);
			}

			static ResourceBarrierDesc as_aliasing(IResource* resource, ResourceBarrierFlag flags = ResourceBarrierFlag::none)
			{
				ResourceAliasingBarrierDesc desc;
				desc.resource = resource;
				return ResourceBarrierDesc(desc, flags);
			}

			static ResourceBarrierDesc as_uav(IResource* resource, ResourceBarrierFlag flags = ResourceBarrierFlag::none)
			{
				ResourceUAVBarrierDesc desc;
				desc.resource = resource;
				return ResourceBarrierDesc(desc, flags);
			}

		};

		struct VertexBufferViewDesc
		{
			IResource* resource;
			//! The offset of the buffer from resource start in bytes.
			u64 offset_in_bytes;
			//! The size of the resource in bytes.
			u32 size_in_bytes;
			//! The size of one element in the buffer in bytes.
			u32 stride_in_bytes;

			VertexBufferViewDesc() = default;
			VertexBufferViewDesc(IResource* _resource,
				u64 _offset_in_bytes,
				u32 _size_in_bytes,
				u32 _stride_in_bytes) :
				resource(_resource),
				offset_in_bytes(_offset_in_bytes),
				size_in_bytes(_size_in_bytes),
				stride_in_bytes(_stride_in_bytes) {}
		};

		enum class PrimitiveTopology : u32
		{
			undefined,
			point_list,
			line_list,
			line_strip,
			triangle_list,
			triangle_strip,
			line_list_adj,
			line_strip_adj,
			triangle_list_adj,
			triangle_strip_adj,
			patchlist_1_control_point,
			patchlist_2_control_point,
			patchlist_3_control_point,
			patchlist_4_control_point,
			patchlist_5_control_point,
			patchlist_6_control_point,
			patchlist_7_control_point,
			patchlist_8_control_point,
			patchlist_9_control_point,
			patchlist_10_control_point,
			patchlist_11_control_point,
			patchlist_12_control_point,
			patchlist_13_control_point,
			patchlist_14_control_point,
			patchlist_15_control_point,
			patchlist_16_control_point,
			patchlist_17_control_point,
			patchlist_18_control_point,
			patchlist_19_control_point,
			patchlist_20_control_point,
			patchlist_21_control_point,
			patchlist_22_control_point,
			patchlist_23_control_point,
			patchlist_24_control_point,
			patchlist_25_control_point,
			patchlist_26_control_point,
			patchlist_27_control_point,
			patchlist_28_control_point,
			patchlist_29_control_point,
			patchlist_30_control_point,
			patchlist_31_control_point,
			patchlist_32_control_point,
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

		//! The operation to perform when the render texture is loaded to GPU.
		enum class LoadOp : u8
		{
			//! The previous contents of the texture within the render area will be preserved.
			load = 0,
			//! The contents within the render area will be cleared to a uniform value
			clear = 1,
			//! The previous contents within the area need not be preserved, and we don't clear 
			//! the content before using it.
			dont_care = 2,
		};

		//! The operation to perform when the render texture is written back to resource memory.
		enum class StoreOp : u8
		{
			//! Preserves the content of the texture after ending the render pass.
			store = 0,
			//! The content of the texture will not be preserved after ending the render pass.
			//! This is used for attachments like depth buffer that is only used for the current 
			//! render pass.
			dont_care = 1
		};

		//! Parameters passed to `begin_render_pass`.
		struct RenderPassDesc
		{
			//! The render targets views to set.
			IRenderTargetView* rtvs[8] = { nullptr };
			//! The depth stencil view to set.
			IDepthStencilView* dsv = nullptr;
			//! The load operation for the render target. 
			//! If the corresponding render target resource is `nullptr`, this operation is ignored.
			LoadOp rt_load_ops[8] = { LoadOp::load };
			//! The store operation for the render target.
			//! If the corresponding render target resource is `nullptr`, this operation is ignored.
			StoreOp rt_store_ops[8] = { StoreOp::store };
			//! The load operation for depth component.
			//! If the depth stencil resource is `nullptr`, this operation is ignored.
			LoadOp depth_load_op = LoadOp::load;
			//! The store operation for depth component.
			//! If the depth stencil resource is `nullptr`, this operation is ignored.
			StoreOp depth_store_op = StoreOp::store;
			//! The load operation for stencil component.
			//! If the depth stencil resource is `nullptr`, this operation is ignored.
			LoadOp stencil_load_op = LoadOp::load;
			//! The store operation for stencil component.
			//! If the depth stencil resource is `nullptr`, this operation is ignored.
			StoreOp stencil_store_op = StoreOp::store;
			//! The clear value for render targets if `rt_load_ops` specified for 
			//! the render target is `RenderTargetLoadOp::clear`.
			Float4U rt_clear_values[8] = { Float4U(0.0f, 0.0f, 0.0f, 0.0f) };
			//! The depth value to use for clear if `depth_load_op` specified in render pass
			//! is `RenderTargetLoadOp::clear`.
			f32 depth_clear_value = 0.0f;
			//! The stencil value to use for clear if `stencil_load_op` specified in render pass
			//! is `RenderTargetLoadOp::clear`.
			u8 stencil_clear_value = 0;
		};

		//! @interface ICommandBuffer
		//! The command buffer is used to allocate memory for commands, record commands, submitting 
		//! commands to GPU and tracks the state of the submitted commands.
		//! 
		//! Command buffer is not thread safe. If the user need to record commands simultaneously, she 
		//! should create multiple command buffers, one per thread. 
		//! 
		//! All synchroizations for command buffers are performed explicitly, for instance:
		//! 1. To waits for one command buffer from CPU side, call `ICommandBuffer::wait`.
		//! 2. To waits for one command buffer from another command queue, call `ICommandQueue::wait_command_buffer`.
		//! 3. Only call `ICommandBuffer::reset` after the command buffer is not submitted, or is finished by GPU.
		struct ICommandBuffer : virtual IDeviceChild, virtual IWaitable
		{
			luiid("{2970a4c8-d905-4e58-9247-46ba6a33b220}");

			virtual CommandQueueType get_type() = 0;

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
			virtual void attach_graphic_object(IDeviceChild* obj) = 0;

			//! Starts a new render pass. The previous render pass should be closed before beginning another one.
			//! @param[in] desc The render pass descriptor object.
			//! @remark In order to let LoadOp::clear works, the render textures that need to be cleared must be in
			//! `ResourceState::render_target` state and the depth stencil texture that needs to be cleared must be in 
			//! `ResourceState::depth_write` state.
			//! 
			//! The following functions can only be called in between `begin_render_pass` and `end_render_pass`:
			//! * draw
			//! * draw_indexed
			//! * draw_instanced
			//! * draw_indexed_instanced
			//! * clear_render_target_view
			//! * clear_depth_stencil_view
			//! * set_scissor_rects
			//! The following functions can only be called outside of one render pass range:
			//! * submit
			virtual void begin_render_pass(const RenderPassDesc& desc) = 0;

			//! Sets the pipeline state.
			virtual void set_pipeline_state(IPipelineState* pso) = 0;

			//! Sets the graphic shader input layout.
			virtual void set_graphic_shader_input_layout(IShaderInputLayout* shader_input_layout) = 0;

			//! Sets vertex buffers.
			virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferViewDesc> views) = 0;

			//! Sets index buffer.
			virtual void set_index_buffer(IResource* buffer, u32 offset_in_bytes, u32 size_in_bytes, Format format) = 0;

			//! Sets the descriptor set to be used by the graphic pipeline.
			//! This must be called after `set_pipeline_state`.
			virtual void set_graphic_descriptor_set(usize index, IDescriptorSet* descriptor_set) = 0;

			//! Bind information about the primitive type, and data order that describes input data for the input assembler stage.
			virtual void set_primitive_topology(PrimitiveTopology primitive_topology) = 0;

			//! Sets the stream output buffer views. 
			virtual void set_stream_output_targets(u32 start_slot, Span<const StreamOutputBufferView> views) = 0;

			//! Bind one viewport to the rasterizer stage of the pipeline.
			virtual void set_viewport(const Viewport& viewport) = 0;

			//! Bind an array of viewports to the rasterizer stage of the pipeline.
			virtual void set_viewports(Span<const Viewport> viewports) = 0;

			//! Binds one scissor rectangle to the rasterizer stage.
			//! The scissor rectangle points are relative to the bottom-left corner of the render target, 
			//! with x-axis points to right and y-axis points to up.
			virtual void set_scissor_rect(const RectI& rect) = 0;

			//! Binds an array of scissor rectangles to the rasterizer stage.
			//! The scissor rectangle points are relative to the bottom-left corner of the render target, 
			//! with x-axis points to right and y-axis points to up.
			virtual void set_scissor_rects(Span<const RectI> rects) = 0;

			//! Sets the blend factor that modulate values for a pixel shader, render target, or both.
			virtual void set_blend_factor(Span<const f32, 4> blend_factor) = 0;

			//! Sets the reference value for depth stencil tests.
			virtual void set_stencil_ref(u32 stencil_ref) = 0;

			//! Draw primitives.
			virtual void draw(u32 vertex_count, u32 start_vertex_location) = 0;

			//! Draw indexed primitives.
			virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) = 0;

			//! Draws indexed, instanced primitives.
			virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location) = 0;

			//! Draws non-indexed, instanced primitives.
			virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location) = 0;

			//! Preforms one indirect draw.
			//virtual void draw_indexed_indirect(IResource* buffer, usize argment_offset, u32 draw_count, u32 stride) = 0;

			//! Clears the depth stencil view bound to the current render pass.
			virtual void clear_depth_stencil_view(ClearFlag clear_flags, f32 depth, u8 stencil, Span<const RectI> rects) = 0;

			//! Clears the render target view bound to the current render pass.
			//! @param[in] index The index of the render target view to clear in the frame buffers.
			virtual void clear_render_target_view(u32 index, Span<const f32, 4> color_rgba, Span<const RectI> rects) = 0;

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
			virtual void set_compute_descriptor_set(usize index, IDescriptorSet* descriptor_set) = 0;

			//! Issues one resource barrier.
			virtual void resource_barrier(const ResourceBarrierDesc& barrier) = 0;

			//! Issues resource barriers.
			virtual void resource_barriers(Span<const ResourceBarrierDesc> barriers) = 0;

			//! Executes a command list from a thread group.
			virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) = 0;

			//virtual void dispatch_indirect(IResource* buffer, usize offset) = 0;

			//! Submits the recorded content in this command buffer to the target command queue.
			//! The command buffer can only be submitted once, and the only operation after the submit is to 
			//! reset the command buffer after it is executed by command queue.
			virtual RV submit() = 0;
		};
	}
}