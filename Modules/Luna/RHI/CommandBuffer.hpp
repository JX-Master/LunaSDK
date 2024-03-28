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
        //! @addtogroup RHI
        //! @{

        //! Specifies additional flags for one buffer or texture barrier.
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

        //! Specifies buffer resource states before and after one barrier.
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
            //! Used as a read-write resource for pixel shader.
            //! This is a combination of @ref shader_read_ps and @ref shader_write_ps.
            shader_read_write_ps = shader_read_ps | shader_write_ps,
            //! Used as a read-write resource for compute shader.
            //! This is a combination of @ref shader_read_cs and @ref shader_write_cs.
            shader_read_write_cs = shader_read_cs | shader_write_cs,
        };

        //! Specifies texture resource states before and after one barrier.
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
            //! Used as a read-write resource for pixel shader.
            //! This is a combination of @ref shader_read_ps and @ref shader_write_ps.
            shader_read_write_ps = shader_read_ps | shader_write_ps,
            //! Used as a read-write resource for compute shader.
            //! This is a combination of @ref shader_read_cs and @ref shader_write_cs.
            shader_read_write_cs = shader_read_cs | shader_write_cs,
        };
        //! A special number that selects all subresources of one resource for one barrier operation.
        constexpr SubresourceIndex TEXTURE_BARRIER_ALL_SUBRESOURCES = {U32_MAX, U32_MAX};
        //! Describes one texture barrier.
        struct TextureBarrier
        {
            //! The resource the barrier is applied to.
            ITexture* texture;
            //! The subresource in the resource the barrier is applied to.
            //! Specify @ref TEXTURE_BARRIER_ALL_SUBRESOURCES will apply the barrier to all subresources 
            //! of the resource.
            SubresourceIndex subresource;
            //! The states of the subresource(s) before this barrier takes place.
            //! Specify @ref TextureStateFlag::automatic to let the system decide the before state, see
            //! docs of @ref TextureStateFlag::automatic for details.
            TextureStateFlag before;
            //! The states of the subresource(s) after this barrier takes place.
            TextureStateFlag after;
            //! Additional flags for this barrier.
            ResourceBarrierFlag flags;
            TextureBarrier() = default;
            TextureBarrier(ITexture* texture, SubresourceIndex subresource, TextureStateFlag before, TextureStateFlag after, ResourceBarrierFlag flags = ResourceBarrierFlag::none) :
                texture(texture),
                subresource(subresource),
                before(before),
                after(after),
                flags(flags) {}
        };
        //! Describes one buffer barrier.
        struct BufferBarrier
        {
            //! The resource the barrier is applied to.
            IBuffer* buffer;
            //! The states of the resource before this barrier takes place.
            //! Specify @ref BufferStateFlag::automatic to let the system decide the before state, see
            //! docs of @ref BufferStateFlag::automatic for details.
            BufferStateFlag before;
            //! The states of the resource after this barrier takes place.
            BufferStateFlag after;
            //! Additional flags for this barrier.
            ResourceBarrierFlag flags;
            BufferBarrier() = default;
            BufferBarrier(IBuffer* buffer, BufferStateFlag before, BufferStateFlag after, ResourceBarrierFlag flags = ResourceBarrierFlag::none) :
                buffer(buffer),
                before(before),
                after(after),
                flags(flags) {}
        };
        //! Describes one viewport used by @ref ICommandBuffer::set_viewport and @ref ICommandBuffer::set_viewports.
        struct Viewport
        {
            //! The X position, in pixels, of the top left corner of the viewport, relative to the top left corner of the render
            //! target (X axis points to right).
            f32 top_left_x;
            //! The Y position, in pixels, of the top left corner of the viewport, relative to the top left corner of the render
            //! target (Y axis points to down).
            f32 top_left_y;
            //! The width of the viewport in pixels.
            f32 width;
            //! The height of the viewport in pixels.
            f32 height;
            //! The minimum depth value of the viewport. The value must between [0.0, 1.0].
            f32 min_depth;
            //! The maximum depth value of the viewport. The value must between [0.0, 1.0].
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
        //! Describes one color attachment when beginning a render pass.
        struct ColorAttachment
        {
            //! The texture to bind for this attachment.
            //! The texture must have @ref TextureUsageFlag::color_attachment being set.
            ITexture* texture = nullptr;
            //! The load operation to perform when reading data from attachment.
            LoadOp load_op = LoadOp::dont_care;
            //! The store operation to perform when writing data to attachment.
            StoreOp store_op = StoreOp::dont_care;
            //! The clear value to use if `load_op` is @ref LoadOp::clear. Otherwise this value is ignored.
            Float4U clear_value = { 0, 0, 0, 0 };
            //! The texture view type to view the texture as an attachment.
            //! If @ref TextureViewType::unspecified is set, the system loads the texture's texture type as 
            //! view type.
            TextureViewType view_type = TextureViewType::unspecified;
            //! The format used for the texture view.
            Format format = Format::unknown;
            //! The mip slice used for the texture view.
            u32 mip_slice = 0;
            //! The array slice used for the texture view.
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
        //! Describes one depth stencil attachment when beginning a render pass.
        struct DepthStencilAttachment
        {
            //! The texture to bind for this attachment.
            //! The texture must have @ref TextureUsageFlag::depth_stencil_attachment being set.
            ITexture* texture = nullptr;
            //! The load operation to perform for depth component when reading data from attachment.
            LoadOp depth_load_op = LoadOp::dont_care;
            //! The store operation to perform for depth component when writing data to attachment.
            StoreOp depth_store_op = StoreOp::dont_care;
            //! The load operation to perform for stencil component when reading data from attachment.
            LoadOp stencil_load_op = LoadOp::dont_care;
            //! The store operation to perform for stencil component when writing data to attachment.
            StoreOp stencil_store_op = StoreOp::dont_care;
            //! The depth clear value to use if `depth_load_op` is @ref LoadOp::clear. Otherwise this value is ignored.
            f32 depth_clear_value = 0.0f;
            //! The stencil clear value to use if `stencil_load_op` is @ref LoadOp::clear. Otherwise this value is ignored.
            u8 stencil_clear_value = 0;
            //! Whether this is a read-only depth stencil attachment.
            bool read_only = false;
            //! The texture view type to view the texture as an attachment.
            //! If @ref TextureViewType::unspecified is set, the system loads the texture's texture type as 
            //! view type.
            TextureViewType view_type = TextureViewType::unspecified;
            //! The format used for the texture view.
            Format format = Format::unknown;
            //! The mip slice used for the texture view.
            u32 mip_slice = 0;
            //! The array slice used for the texture view.
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
        //! Describes one resolve attachment when beginning a render pass.
        //! @details A resolve attachment is used to resolve data in one MSAA texture into one normal texture.
        struct ResolveAttachment
        {
            //! The texture to bind for this attachment. This texture is used as resolve target.
            //! The texture must have @ref TextureUsageFlag::resolve_attachment being set.
            ITexture* texture = nullptr;
            //! The mip slice used for the texture view.
            u32 mip_slice = 0;
            //! The first slice in the texture view.
            u32 array_slice = 0;
            //! The number of array elements in the texture view, range in [array_slice, array_slice + array_size).
            u32 array_size = 1;
            ResolveAttachment() = default;
            ResolveAttachment(ITexture* texture, u32 mip_slice = 0, u32 array_slice = 1) :
                texture(texture),
                mip_slice(mip_slice),
                array_slice(array_slice) {}
        };
        //! A special number that identifies the query operation is disabled.
        constexpr u32 DONT_QUERY = U32_MAX;
        //! Specifies the occlusion query working mode.
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
        //! Describes one render pass.
        struct RenderPassDesc
        {
            //! The color attachments to set.
            ColorAttachment color_attachments[8];
            //! The resolve attachments to set.
            ResolveAttachment resolve_attachments[8];
            //! The depth stencil attachment to set.
            DepthStencilAttachment depth_stencil_attachment;
            //! The occlustion query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* occlusion_query_heap = nullptr;
            //! The timestamp query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* timestamp_query_heap = nullptr;
            //! The pipeline statistics query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* pipeline_statistics_query_heap = nullptr;
            //! The index of the element in timestamp query heap that the render pass begin timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            //! The index of the element in timestamp query heap that the render pass end timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
            //! The index of the element in pipeline statistics query heap that the pipeline statistics will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 pipeline_statistics_query_write_index = DONT_QUERY;
            //! The number of texture array elements that will be bound for all attachments.
            u32 array_size = 1;
            //! The sample count for every pixel of the render pass.
            //! Specify any value greater than 1 enables MSAA.
            u8 sample_count = 1;
        };
        //! Describes one compute pass.
        struct ComputePassDesc
        {
            //! The timestamp query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* timestamp_query_heap = nullptr;
            //! The pipeline statistics query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* pipeline_statistics_query_heap = nullptr;
            //! The index of the element in timestamp query heap that the compute pass begin timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            //! The index of the element in timestamp query heap that the compute pass end timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
            //! The index of the element in pipeline statistics query heap that the pipeline statistics will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 pipeline_statistics_query_write_index = DONT_QUERY;
        };
        //! Describes one copy pass.
        struct CopyPassDesc
        {
            //! The timestamp query heap that the query data will be written to if not `nullptr`.
            IQueryHeap* timestamp_query_heap = nullptr;
            //! The index of the element in timestamp query heap that the copy pass begin timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_begin_pass_write_index = DONT_QUERY;
            //! The index of the element in timestamp query heap that the copy pass end timestamp will be written to.
            //! If this is @ref DONT_QUERY, the query data will not be written.
            u32 timestamp_query_end_pass_write_index = DONT_QUERY;
        };
        //! Describes one vertex buffer view when binding one vertex buffer to the render pipeline.
        struct VertexBufferView
        {
            //! The vertex buffer resource.
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
        //! Describes one index buffer view when binding one index buffer to the render pipeline.
        struct IndexBufferView
        {
            //! The index buffer resource.
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
        //! Used to allocate memory for commands, record commands, submitting 
        //! commands to GPU and tracks the state of the submitted commands.
        //! @details Command buffer is not thread safe. If the user need to record commands simultaneously, she 
        //! should create multiple command buffers, one per thread. 
        //! 
        //! All synchroizations for command buffers are performed explicitly, for instance:
        //! 1. Use @ref ICommandBuffer::wait to wait for one command buffer from host side, 
        //! 2. Use fence objects to wait for one command buffer from another command buffer.
        //! 3. Only call @ref ICommandBuffer::reset after the command buffer is not submitted, or is finished by GPU.
        struct ICommandBuffer : virtual IDeviceChild, virtual IWaitable
        {
            luiid("{2970a4c8-d905-4e58-9247-46ba6a33b220}");

            //! Gets the command queue index of the command queue attached to 
            //! this buffer.
            virtual u32 get_command_queue_index() = 0;

            //! Resets the command buffer. This call clears all commands in the command buffer, resets the state tracking
            //! infrastructure and reopens the command buffer for recording new commands.
            //! @details You should only call this after the command buffer has finished execution by the command queue,
            //! or the behavior is undefined. In order to make sure all commands are executed by GPU, call
            //! `wait` to block the thread until this buffer gets finished, or you can use `try_wait` to test
            //! if the buffer has finished execution.
            virtual RV reset() = 0;

            //! Attaches one graphic device object to this command buffer. The command buffer keeps a strong reference 
            //! to the object until the next `reset` is called.
            //! @details This is mainly used to keep references to the graphic objects used by the current command buffer, so they
            //! will not be released before GPU finishes accessing them.
            virtual void attach_device_object(IDeviceChild* obj) = 0;

            //! Begins a new event. This is for use in diagnostic tools like RenderDoc, PIX, XCode, etc to group commands into hierarchical
            //! sections.
            virtual void begin_event(const c8* event_name) = 0;

            //! Ends the latest event opened by @ref begin_event that has not been ended.
            virtual void end_event() = 0;

            //! Starts a new render pass. The previous render pass should be closed before beginning another one.
            //! @param[in] desc The render pass descriptor object.
            //! @details The following functions can only be called in between @ref begin_render_pass and @ref end_render_pass:
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
            //! 
            //! The following functions can only be called outside of one render pass range:
            //! * submit
            //! * set_context
            //! * resource_barrier
            virtual void begin_render_pass(const RenderPassDesc& desc) = 0;

            //! Sets the graphic pipeline layout.
            //! @param[in] pipeline_layout The pipeline layout to set.
            virtual void set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout) = 0;

            //! Sets the pipeline state for graphics pipeline.
            //! @param[in] pso The pipeline state object to set.
            virtual void set_graphics_pipeline_state(IPipelineState* pso) = 0;

            //! Sets vertex buffers.
            //! @param[in] start_slot The start slot of the vertex buffer to set.
            //! @param[in] views An array of vertex buffer views to set, each describes one vertex buffer range to bind.
            //! The vertex buffer views will be set in slot [start_slot, start_slot + views.size()).
            virtual void set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views) = 0;

            //! Sets the index buffer.
            //! @param[in] view The index buffer view to set.
            virtual void set_index_buffer(const IndexBufferView& view) = 0;

            //! Sets the descriptor set to be used by the graphic pipeline.
            //! @details This behaves the same as calling @ref set_graphics_descriptor_sets with only one element.
            //! @param[in] index The binding index of the descriptor set. This must be smaller than the size of
            //! @ref PipelineLayoutDesc::descriptor_set_layouts of the binding pipeline layout.
            //! @param[in] descriptor_set The descriptor set to set.
            //! @par Valid Usage
            //! * This must be called after @ref set_graphics_pipeline_state and @ref set_graphics_pipeline_layout.
            virtual void set_graphics_descriptor_set(u32 index, IDescriptorSet* descriptor_set) = 0;

            //! Sets descriptor sets to be used by the graphic pipeline.
            //! @param[in] start_index The binding index of the first descriptor set in the array. Descriptor sets in the array
            //! will be bound in range [`start_index`, `start_index + descriptor_sets.size()`).
            //! @param[in] descriptor_sets The descriptor sets to set.
            //! @par Valid Usage
            //! * This must be called after @ref set_graphics_pipeline_state and @ref set_graphics_pipeline_layout.
            virtual void set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

            //! Bind one viewport to the rasterizer stage of the pipeline.
            //! @details This operation behaves the same as calling @ref set_viewports with only one viewport.
            //! @param[in] viewport The viewport to set.
            virtual void set_viewport(const Viewport& viewport) = 0;

            //! Bind an array of viewports to the rasterizer stage of the pipeline.
            //! @details All viewports must be set in one call. Any call to @ref set_viewport or @ref set_viewports
            //! clears all existing viewports before setting new viewports.
            virtual void set_viewports(Span<const Viewport> viewports) = 0;

            //! Binds one scissor rectangle to the rasterizer stage. The scissor rectangle points are relative to the top-left corner of the render target, 
            //! with x-axis points to right and y-axis points to down.
            //! @details This operation behaves the same as calling @ref set_scissor_rects with only one scissor rect.
            //! @param[in] rect The scissor rectangle to set.
            virtual void set_scissor_rect(const RectI& rect) = 0;

            //! Binds an array of scissor rectangles to the rasterizer stage. The scissor rectangle points are relative to the top-left corner of the render target, 
            //! with x-axis points to right and y-axis points to down.
            //! @details All scissor rectangles must be set in one call. Any call to @ref set_scissor_rect or @ref set_scissor_rects
            //! clears all existing scissor rectangles before setting new scissor rectangles.
            //! @param[in] rects The scissor rectangles to set.
            virtual void set_scissor_rects(Span<const RectI> rects) = 0;

            //! Sets the blend factor of the graphics pipeline.
            //! @details The blend factor is used if @ref BlendFactor::blend_factor or @ref BlendFactor::one_minus_blend_factor 
            //! is used for any blending operation in @ref BlendDesc of the binding graphics pipeline state object.
            //! @param[in] blend_factor The blend factor (in RGBA order) to set.
            virtual void set_blend_factor(const Float4U& blend_factor) = 0;

            //! Sets the reference value for stencil testing.
            //! @param[in] stencil_ref The stencil reference value to set.
            virtual void set_stencil_ref(u32 stencil_ref) = 0;

            //! Draw primitives.
            //! @param[in] vertex_count The number of vertices to draw.
            //! @param[in] start_vertex_location The position of the first vertex to draw. Vertices in range
            //! [`start_vertex_location`, `start_vertex_location + vertex_count`) will be drawn.
            virtual void draw(u32 vertex_count, u32 start_vertex_location) = 0;

            //! Draw indexed primitives.
            //! @param[in] index_count The number of indices to draw.
            //! @param[in] start_index_location The position of the first index to draw. Indices in range
            //! [`start_index_location`, `start_index_location + index_count`) will be drawn.
            //! @param[in] base_vertex_location An offset that will be added to all indices numbers before 
            //! dereferring vertex data from their indices. This can be used to batch vertices of multiple meshes into
            //! one vertex buffer, and use offsets to draw each of them separately.
            virtual void draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location) = 0;

            //! Draws non-indexed, instanced primitives.
            //! @param[in] vertex_count_per_instance The number of vertices to draw for every instance.
            //! @param[in] instance_count The number of instances to draw.
            //! @param[in] start_vertex_location The position of the first per-vertex data to use for instance drawing.
            //! Vertex data in range [`start_vertex_location`, `start_vertex_location + vertex_count_per_instance`) will be used.
            //! @param[in] start_instance_location The index of the first per-instance data to use for instance drawing.
            //! Instance data in range [`start_instance_location`, `start_instance_location + instance_count`) will be used.
            virtual void draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
                u32 start_instance_location) = 0;

            //! Draws indexed, instanced primitives.
            //! @param[in] index_count_per_instance The number of indices to draw for every instance.
            //! @param[in] instance_count The number of instances to draw.
            //! @param[in] start_index_location The position of the index pointing to first per-vertex data to use for instance drawing.
            //! Vertex data pointed by index in range [`start_index_location`, `start_index_location + index_count_per_instance`) will be used.
            //! @param[in] base_vertex_location An offset that will be added to all indices numbers before 
            //! dereferring vertex data from their indices. This can be used to batch vertices of multiple meshes into
            //! one vertex buffer, and use offsets to draw each of them separately.
            //! @param[in] start_instance_location The index of the first per-instance data to use for instance drawing.
            //! Instance data in range [`start_instance_location`, `start_instance_location + instance_count`) will be used.
            virtual void draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
                i32 base_vertex_location, u32 start_instance_location) = 0;
            
            //! Starts one occlusion query.
            //! @param[in] mode The working mode of the new occlusion query.
            //! @param[in] index The position to write occlusion query result to.
            //! Multiple occlusion queries can exist at the same time, so long as each of them takes one separate 
            //! index.
            virtual void begin_occlusion_query(OcclusionQueryMode mode, u32 index) = 0;
            
            //! Ends one existing occlusion query.
            //! @param[in] index The position that was passed in @ref begin_occlusion_query when 
            //! starting the occlusion query.
            virtual void end_occlusion_query(u32 index) = 0;

            //! Finishes the current render pass.
            virtual void end_render_pass() = 0;

            //! Begins a compute pass.
            //! @details The following functions can only be called in between `begin_compute_pass` and `end_compute_pass`:
            //! * set_compute_pipeline_layout
            //! * set_compute_pipeline_state
            //! * set_compute_descriptor_set
            //! * set_compute_descriptor_sets
            //! * dispatch
            //! @param[in] desc The compute pass descriptor.
            virtual void begin_compute_pass(const ComputePassDesc& desc = ComputePassDesc()) = 0;

            //! Sets the compute pipeline layout.
            //! @param[in] pipeline_layout The pipeline layout to set.
            virtual void set_compute_pipeline_layout(IPipelineLayout* pipeline_layout) = 0;

            //! Sets the pipeline state for compute pipeline.
            //! @param[in] pso The pipeline state object to set.
            virtual void set_compute_pipeline_state(IPipelineState* pso) = 0;

            //! Sets the descriptor set to be used by the compute pipeline.
            //! @details This behaves the same as calling @ref set_computes_descriptor_sets with only one element.
            //! @param[in] index The binding index of the descriptor set. This must be smaller than the size of
            //! @ref PipelineLayoutDesc::descriptor_set_layouts of the binding pipeline layout.
            //! @param[in] descriptor_set The descriptor set to set.
            //! @par Valid Usage
            //! * This must be called after @ref set_computes_pipeline_state and @ref set_computes_pipeline_layout.
            virtual void set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set) = 0;

            //! Sets descriptor sets to be used by the compute pipeline.
            //! @param[in] start_index The binding index of the first descriptor set in the array. Descriptor sets in the array
            //! will be bound in range [`start_index`, `start_index + descriptor_sets.size()`).
            //! @param[in] descriptor_sets The descriptor sets to set.
            //! @par Valid Usage
            //! * This must be called after @ref set_computes_pipeline_state and @ref set_computes_pipeline_layout.
            virtual void set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets) = 0;

            //! Dispatches one compute task.
            //! @param[in] thread_group_count_x The number of thread groups to emit in the first dimension.
            //! @param[in] thread_group_count_y The number of thread groups to emit in the second dimension.
            //! @param[in] thread_group_count_z The number of thread groups to emit in the third dimension.
            virtual void dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z) = 0;

            //! Ends a compute pass.
            virtual void end_compute_pass() = 0;

            //! Begins a copy pass.
            //! @details The following functions can only be called in between `begin_copy_pass` and `end_copy_pass`:
            //! * copy_resource
            //! * copy_buffer
            //! * copy_texture
            //! * copy_buffer_to_texture
            //! * copy_texture_to_buffer
            //! @param[in] desc The copy pass descriptor.
            virtual void begin_copy_pass(const CopyPassDesc& desc = CopyPassDesc()) = 0;

            //! Copies the entire contents of the source resource to the destination resource.
            //! @param[in] dst The resource to copy data to.
            //! @param[in] src The resource to copy data from.
            //! @par Valid Usage
            //! * The source resource and destination resource must have exactly the same resource format and dimension, that is,
            //! have the same size for buffers, or the same type, width, height, depth, format, mip count, array count and sample count for textures.
            virtual void copy_resource(IResource* dst, IResource* src) = 0;

            //! Copies buffer data region from one buffer to another.
            //! @param[in] dst The buffer to copy data to.
            //! @param[in] dst_offset The offset, in bytes, of the first data byte to copy data to in `dst`.
            //! @param[in] src The buffer to copy data from.
            //! @param[in] src_offset  The offset, in bytes, of the first data byte to copy data from to in `src`.
            //! @param[in] copy_bytes The number of bytes to copy.
            virtual void copy_buffer(
                IBuffer* dst, u64 dst_offset,
                IBuffer* src, u64 src_offset,
                u64 copy_bytes) = 0;

            //! Copies texture data region from one texture to another.
            //! @param[in] dst The texture to copy data to.
            //! @param[in] dst_subresource The subresource in `dst` to copy data to.
            //! @param[in] dst_x The X position of the first pixel to copy data to.
            //! @param[in] dst_y The Y position of the first pixel to copy data to.
            //! @param[in] dst_z The Z position of the first pixel to copy data to.
            //! @param[in] src The texture to copy data from.
            //! @param[in] src_subresource The subresource in `src` to copy data from.
            //! @param[in] src_x The X position of the first pixel to copy data from.
            //! @param[in] src_y The Y position of the first pixel to copy data from.
            //! @param[in] src_z The Z position of the first pixel to copy data from.
            //! @param[in] copy_width The number of pixels to copy in X dimension.
            //! @param[in] copy_height The number of pixels to copy in Y dimension.
            //! @param[in] copy_depth The number of pixels to copy in Z dimension.
            virtual void copy_texture(
                ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
                ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
                u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

            //! Copies texture data region from one buffer to one texture. Texture data in the buffer is
            //! interpreted in row-major arrangement.
            //! @param[in] dst The texture to copy data to.
            //! @param[in] dst_subresource The subresource in `dst` to copy data to.
            //! @param[in] dst_x The X position of the first pixel to copy data to.
            //! @param[in] dst_y The Y position of the first pixel to copy data to.
            //! @param[in] dst_z The Z position of the first pixel to copy data to.
            //! @param[in] src The buffer to copy data from.
            //! @param[in] src_offset The offset, in bytes, of the first pixel data to copy from in `src`.
            //! @param[in] src_row_pitch The number of bytes to advance between every row of data in `src`.
            //! @param[in] src_slice_pitch The number of bytes to advance between every slice (row * column) of data in `src`.
            //! @param[in] copy_width The number of pixels to copy for each row.
            //! @param[in] copy_height The number of rows to copy for each slice.
            //! @param[in] copy_depth The number of slices to copy.
            virtual void copy_buffer_to_texture(
                ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
                IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
                u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

            //! Copies texture data region from one texture to one buffer. Texture data is written to the buffer
            //! in row-major arrangement.
            //! @param[in] dst The buffer to copy data to.
            //! @param[in] dst_offset The offset, in bytes, of the first data byte to copy data to in `dst`.
            //! @param[in] dst_row_pitch The number of bytes to advance between every row of data in `dst`.
            //! @param[in] dst_slice_pitch The number of bytes to advance between every slice (row * column) of data in `dst`.
            //! @param[in] src The texture to copy data from.
            //! @param[in] src_subresource The subresource in `src` to copy data from.
            //! @param[in] src_x The X position of the first pixel to copy data from.
            //! @param[in] src_y The Y position of the first pixel to copy data from.
            //! @param[in] src_z The Z position of the first pixel to copy data from.
            //! @param[in] copy_width The number of pixels to copy for each row.
            //! @param[in] copy_height The number of rows to copy for each slice.
            //! @param[in] copy_depth The number of slices to copy.
            virtual void copy_texture_to_buffer(
                IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
                ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
                u32 copy_width, u32 copy_height, u32 copy_depth) = 0;

            //! Ends a copy pass.
            virtual void end_copy_pass() = 0;

            //! Issues one resource barrier that synchronizes GPU pipeline access to multiple resources.
            //! @param[in] buffer_barriers The buffer barriers to submitl.
            //! @param[in] texture_barriers The texture barriers to submit.
            virtual void resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers) = 0;

            //! Submits the recorded commands in this command buffer to the attached command queue.
            //! @details The command buffer can only be submitted once, and the only allowed operation after submit is to 
            //! reset the command buffer after it is executed by command queue.
            //! @param[in] wait_fences The fence objects to wait before this command buffer can be processed by the system.
            //! @param[in] signal_fences The fence objects to signal after this command buffer is completed.
            //! @param[in] allow_host_waiting Whether @ref ICommandBuffer::wait can be used to wait for the command buffer 
            //! from host side. If this is `false`, the command buffer cannot be waited from host, and the behavior of 
            //! calling @ref ICommandBuffer::wait is undefined. Setting this to `false` may improve queue performance, and 
            //! the command buffer can still be waited by other command buffers using fences.
            //! 
            //! @remark Command buffers submitted to the same command queue are processed by their submission order without overlapping, so
            //! that one command buffer will not be executed until all previous command buffers in the same command queue are 
            //! completely finished, and their writes are made visible to this command buffer.
            //! 
            //! If `signal_fences` is not empty, the system guarantees that all commands in the submission is finished, and all 
            //! writes to the memory in the submission is made visible before fences are signaled.
            virtual RV submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting) = 0;
        };

        //! @}
    }
}
