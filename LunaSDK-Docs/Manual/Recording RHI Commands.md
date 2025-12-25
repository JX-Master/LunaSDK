This section describes commands that can be added to one command buffer. In RHI, we have four kinds of commands:

1. Pass begin/end commands.
1. Render commands, which can only be added in render passes.
1. Compute commands, which can only be added in compute passes.
1. Copy command, which can only be added in copy passes.
1. The barrier command.
1. Profile commands.

## Pass begin/end commands
Passes are device-specific states (contexts) that is set up in order to perform a certain kind of tasks. For example, in order to draw primitives, the device need a render pass setup, so that it can prepare framebuffers, pipeline states and other states for the draw call. In RHI, we have three kinds of passes: **render pass**, **compute pass** and **copy pass**. All passes are opened by `begin_xxx_pass`, and are closed by `end_xxx_pass`, one command buffer can only have one open pass at one time, the user should close the last pass begin opening a new pass.

Render passes are opened by `ICommandBuffer::begin_render_pass(desc)`, and are closed by `ICommandBuffer::end_render_pass()`. When beginning a new render pass, the user should provide a render pass descriptor (`RenderPassDesc`) which specifies a set of attachments and query heaps used for the render pass. Attachments are textures that are used to store the render result of the render pass, including:

1. Color attachments: One render pass can specify up to 8 color attachments used for storing render results.
1. Depth stencil attachments: Stores the depth stencil information of the render result and used for depth stencil testing during the draw process.
1. Resolve attachments: If MSAA is enabled, the user can specify resolve attachments that stores the resolve result of color attachments automatically when the render pass ends. Currently, this is the only way to resolve MSAA textures.

Query heaps can be used to query statistics during a render pass, including:

1. Timestamp: Queries the GPU timestamp at the beginning and end of the render pass to measure the GPU time cost for the pass.
1. Pipeline statistics: Queries the pipeline statistics information during the render pass, like how many time vertex shaders and pixel shaders are invoked, how many primitives are drawn, etc.
1. Occlusion: Queries the number of pixels that pass depth/stencil test.

All attachments and query heaps are bound to the render pass until `ICommandBuffer::end_render_pass()` is called. The user cannot change such bindings during a render pass.

Compute passes are opened by `ICommandBuffer::begin_compute_pass(desc)`, and are closed by `ICommandBuffer::end_compute_pass()`. When beginning a new compute pass, the user can provide a compute pass descriptor (`ComputePassDesc`) which specifies query heaps used for the compute pass, including:

1. Timestamp: Queries the GPU timestamp at the beginning and end of the compute pass to measure the GPU time cost for the pass.
1. Pipeline statistics: Queries the number of compute shader invocations during the compute pass.

Copy passes are opened by `ICommandBuffer::begin_copy_pass(desc)`, and and are closed by `ICommandBuffer::end_copy_pass()`. When beginning a new copy pass, the user can provide a copy pass descriptor (`CopyPassDesc`) which specifies query heaps used for the copy pass, which can query for GPU timestamps and beginning and end of the copy pass.

All pipeline states must be valid only in a pass scope, when one pass is closed, all pipeline states will be invalidated and must be set again in the next pass. Such state includes:

1. Pipeline layout objectss.
1. Pipeline state objects.
1. Descriptor sets.
1. Vertex buffers and index buffers.
1. Viewports and scissor rects.
1. Blend factors.
1. Stencil reference values.

### Render commands
Render commands are commands used for rendering tasks, they can only be recorded between `ICommandBuffer::begin_render_pass(desc)` and `ICommandBuffer::end_render_pass()`. Render commands have three types: graphics pipeline setup commands, draw commands and occlusion query commands. The following commands are graphics pipeline setup commands:

1. `ICommandBuffer::set_graphics_pipeline_layout(pipeline_layout)`
1. `ICommandBuffer::set_graphics_pipeline_state(pso)`
1. `ICommandBuffer::set_vertex_buffers(start_slot, views)`
1. `ICommandBuffer::set_index_buffer(view)`
1. `ICommandBuffer::set_graphics_descriptor_set(index, descriptor_set)`
1. `ICommandBuffer::set_graphics_descriptor_sets(start_index, descriptor_sets)`
1. `ICommandBuffer::set_viewport(viewport)`
1. `ICommandBuffer::set_viewports(viewports)`
1. `ICommandBuffer::set_scissor_rect(rect)`
1. `ICommandBuffer::set_scissor_rects(rects)`
1. `ICommandBuffer::set_blend_factor(blend_factor)`
1. `ICommandBuffer::set_stencil_ref(stencil_ref)`

The following commands are draw commands:

1. `ICommandBuffer::draw(vertex_count, start_vertex_location)`
1. `ICommandBuffer::draw_indexed(index_count, start_index_location, base_vertex_location)`
1. `ICommandBuffer::draw_instanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location)`
1. `ICommandBuffer::draw_indexed_instanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location)`

The following commands are occlusion query commands:

1. `ICommandBuffer::begin_occlusion_query(mode, index)`
1. `ICommandBuffer::end_occlusion_query(index)`

The graphics pipeline behaves like a state machine: states set by `ICommandBuffer::set_xxx` stays in the state until changed by another set state call. All graphics pipeline states are undefined at the beginning of one render pass, the user should set all necessary states explicitly before she issues the draw call. All states will be lost at the end of the render pass, the user should set such states again for one new render pass.

When setting pipeline states, the user should following the following rules:

1. `ICommandBuffer::set_graphics_descriptor_set(index, descriptor_set)` and `ICommandBuffer::set_graphics_descriptor_sets(start_index, descriptor_sets)` must be called after a valid graphics pipeline layout is set by `ICommandBuffer::set_graphics_pipeline_layout(pipeline_layout)`.

The following graphics pipeline setup order is suggested:

1. `ICommandBuffer::set_graphics_pipeline_layout(pipeline_layout)`
1. `ICommandBuffer::set_graphics_pipeline_state(pso)`
1. `ICommandBuffer::set_vertex_buffers(start_slot, views)` / `ICommandBuffer::set_index_buffer(view)`  / `ICommandBuffer::set_graphics_descriptor_set(index, descriptor_set)` / `ICommandBuffer::set_graphics_descriptor_sets(start_index, descriptor_sets)`
1. `ICommandBuffer::set_viewport(viewport)` / `ICommandBuffer::set_viewports(viewports)` / `ICommandBuffer::set_scissor_rect(rect)` / `ICommandBuffer::set_scissor_rects(rects)` / `ICommandBuffer::set_blend_factor(blend_factor)` / `ICommandBuffer::set_stencil_ref(stencil_ref)`

### Compute commands
Compute commands are commands used for computing tasks, they can only be recorded between `ICommandBuffer::begin_compute_pass(desc)` and `ICommandBuffer::end_compute_pass()`. Compute commands have two types: compute pipeline setup commands and the dispatch command. The following commands are compute pipeline setup commands:

1. `ICommandBuffer::set_compute_pipeline_layout(pipeline_layout)`
1. `ICommandBuffer::set_compute_pipeline_state(pso)`
1. `ICommandBuffer::set_compute_descriptor_set(index, descriptor_set)`
1. `ICommandBuffer::set_compute_descriptor_sets(start_index, descriptor_sets)`

The dispath command is:
1. `ICommandBuffer::dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z)`

The compute pipeline behaves like a state machine: states set by `ICommandBuffer::set_xxx` stays in the state until changed by another set state call. All compute pipeline states are undefined at the beginning of one compute pass, the user should set all necessary states explicitly before she dispatches the compute task. All states will be lost at the end of the compute pass, the user should set such states again for one new compute pass.

When setting compute pipeline states, the user should following the following rules:

1. `ICommandBuffer::set_compute_descriptor_set(index, descriptor_set)` and `ICommandBuffer::set_compute_descriptor_sets(start_index, descriptor_sets)` must be called after a valid compute pipeline layout is set by `ICommandBuffer::set_compute_pipeline_layout(pipeline_layout)`.

The following compute pipeline setup order is suggested:

1. `ICommandBuffer::set_compute_pipeline_layout(pipeline_layout)`
1. `ICommandBuffer::set_compute_pipeline_state(pso)`
1. `ICommandBuffer::set_compute_descriptor_set(index, descriptor_set)` / `ICommandBuffer::set_compute_descriptor_sets(start_index, descriptor_sets)`

### Copy commands
Copy commands are commands used for coping data between resources, they can only be recorded between `ICommandBuffer::begin_compute_pass(desc)` and `ICommandBuffer::end_compute_pass()`. The following commands are copy commands:

1. `copy_resource(dst, src)`
1. `copy_buffer(dst, dst_offset, src, src_offset, copy_bytes)`
1. `copy_texture(dst, dst_subresource, dst_x, dst_y, dst_z, src, src_subresource, src_x, src_y, src_z, copy_width, copy_height, copy_depth)`
1. `copy_buffer_to_texture(dst, dst_subresource, dst_x, dst_y, dst_z, src, src_offset, src_row_pitch, src_slice_pitch, copy_width, copy_height, copy_depth)`
1. `copy_texture_to_buffer(dst, dst_offset, dst_row_pitch, dst_slice_pitch, src, src_subresource, src_x, src_y, src_z, copy_width, copy_height, copy_depth)`

Copy passes do not have any state.

### The barrier command
GPU engines are a highly pipelined architecture. When executing commands in command queues, the GPU engine will run multiple commands simultaneously to ensure every pieline stage of the GPU engine is fully occupied to improve performance. The driver only guarantees that commands submitted earier will start executing earier, but there is no guarantee of the finish order of such commands. This can cause problems if commands in the same command queue have dependencies that the later command must be executed after the previous command is fully finished. In modern graphics APIs, the user must emit **barrier commands** explicitly to synchronize such dependencies, so that the driver will block succeeding commands from executing until the specified previous pipeline stage is finished.

In LunaSDK, barriers are specified by declaring resource barriers, and are submitted by `ICommandBuffer::resource_barrier(buffer_barriers, texture_barriers)`. One resource barrier declares one resource, and the state of resource **before** and **after** the barrier. The resource state describes the role the resource plays when being used by one GPU engine, possible resource states for buffers and textures are described by `BufferStateFlag` and `TextureStateFlag`, including:

* `BufferStateFlag::indirect_argument`
* `BufferStateFlag::vertex_buffer`
* `BufferStateFlag::index_buffer`
* `BufferStateFlag::uniform_buffer_vs`
* `BufferStateFlag::shader_read_vs`
* `BufferStateFlag::uniform_buffer_ps`
* `BufferStateFlag::shader_read_ps`
* `BufferStateFlag::shader_write_ps`
* `BufferStateFlag::uniform_buffer_cs`
* `BufferStateFlag::shader_read_cs`
* `BufferStateFlag::shader_write_cs`
* `BufferStateFlag::copy_dest`
* `BufferStateFlag::copy_source`
* `TextureStateFlag::shader_read_vs`
* `TextureStateFlag::shader_read_ps`
* `TextureStateFlag::shader_write_ps`
* `TextureStateFlag::color_attachment_read`
* `TextureStateFlag::color_attachment_write`
* `TextureStateFlag::depth_stencil_attachment_read`
* `TextureStateFlag::depth_stencil_attachment_write`
* `TextureStateFlag::resolve_attachment`
* `TextureStateFlag::shader_read_cs`
* `TextureStateFlag::shader_write_cs`
* `TextureStateFlag::copy_dest`
* `TextureStateFlag::copy_source`
* `TextureStateFlag::present`

Multiple states can be bit-OR combined so long as they are compatible to each other, for example, the resource state can be `TextureStateFlag::shader_read_vs | TextureStateFlag::shader_read_ps` if the texture will be read by both vertex shader and pixel shader. A resource barrier tells the driver to wait for the **before** pipeline stage that uses the resource to finish before allowing the **after** pipeline state to access the resource, this barrier also deals the cache visibility so that the **after** pipeline state always have the latest resource data. Multiple resource barriers can be batched and submitted in one call, which is suggested so that the driver can handle all resource barriers in one physical barrier command.

When specifying resource barriers, the before state of one resource barrier for one resource must match the after state of the last resource barrier for the same resource. RHI also tracks the resource state internally, so that in most cases the user can simple specify `BufferStateFlag::automatic` or `TextureStateFlag::automatic` as the before state of one resource, which tells RHI to read the internal state recorded for the resource on the last source barrier. The resource state tracking is valid even between multiple command buffers and multiple command queues.

There are some rules that must comply when using barriers:

1. One resource can only be used by one command queue at one time. If multiple command queues need to access the same resource, they must be synchronized using fences. See "Multi-queues Synchronization" section of "Command Recording and Submission" for the usage of fences.
1. When using one resource that is previously used by another command queue, always submit a barrier before using the resource, even if the resource state is not changed. This is because different command queues may use different internal texture layouts and may have different cache visibility for the same resource state.
1. Command buffers submitted earier to the command queue must be executed earier than command buffers submitted later if they access the same resource because the resource state tracking system updates resources' global states when submitting command buffers, so that different submission order will cause the track system's recorded state being inconsistent with the resource's real state.
1. For barriers submitted within a render pass, the following additional rules must be complied, according to [Vulkan specifications](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdPipelineBarrier.html):
    1. Buffer barriers cannot be submitted.
    1. Texture barriers can only including textures used as attachments for the render pass.