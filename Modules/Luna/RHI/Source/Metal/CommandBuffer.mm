/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.mm
* @author JXMaster
* @date 2023/7/13
*/
#include "CommandBuffer.h"
#include "Resource.h"
#include "PipelineState.h"
#include "DescriptorSet.h"
#include "QueryHeap.h"
#include "Fence.h"
#include <Luna/Runtime/StackAllocator.hpp>

namespace Luna
{
    namespace RHI
    {
        RV CommandBuffer::init(u32 command_queue_index)
        {
            @autoreleasepool
            {
                m_command_queue_index = command_queue_index;
                m_buffer = [m_device->m_queues[command_queue_index].queue commandBuffer];
                if(!m_buffer) return BasicError::bad_platform_call();
                return ok;
            }
        }
        void CommandBuffer::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String: name];
                m_buffer.label = label;
            }
        }
        void CommandBuffer::wait()
        {
            [m_buffer waitUntilCompleted];
        }
        bool CommandBuffer::try_wait()
        {
            auto status = m_buffer.status;
            return status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError;
        }
        RV CommandBuffer::reset()
        {
            @autoreleasepool
            {
                m_objs.clear();
                m_buffer = [m_device->m_queues[m_command_queue_index].queue commandBuffer];
                if(!m_buffer) return BasicError::bad_platform_call();
                return ok;
            }
        }
        void CommandBuffer::attach_device_object(IDeviceChild* obj)
        {
            m_objs.push_back(obj);
        }
        void CommandBuffer::begin_event(const c8* event_name)
        {
            @autoreleasepool
            {
                NSString* string = [NSString stringWithUTF8String: event_name];
                [m_buffer pushDebugGroup: string];
            }
        }
        void CommandBuffer::end_event()
        {
            [m_buffer popDebugGroup];
        }
        void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_render_pass can only be called when no other pass is open.");
            @autoreleasepool
            {
                MTLRenderPassDescriptor* d = [[MTLRenderPassDescriptor alloc] init];
                MTLRenderPassColorAttachmentDescriptorArray* color_attachments = [d colorAttachments];
                MTLRenderPassDepthAttachmentDescriptor* depth_attachment = [d depthAttachment];
                MTLRenderPassStencilAttachmentDescriptor* stencil_attachment = [d stencilAttachment];
                u32 width = 0;
                u32 height = 0;
                for(u32 i = 0; i < 8; ++i)
                {
                    auto& src = desc.color_attachments[i];
                    auto& resolve_src = desc.resolve_attachments[i];
                    if(!src.texture) break;
                    MTLRenderPassColorAttachmentDescriptor* color_attachment = [[MTLRenderPassColorAttachmentDescriptor alloc] init];
                    Texture* tex = cast_object<Texture>(src.texture->get_object());
                    color_attachment.texture = tex->m_texture;
                    color_attachment.level = src.mip_slice;
                    color_attachment.slice = src.array_slice;
                    color_attachment.loadAction = encode_load_action(src.load_op);
                    if(resolve_src.texture)
                    {
                        color_attachment.storeAction = encode_store_action(src.store_op, true);
                        Texture* resolve = cast_object<Texture>(resolve_src.texture->get_object());
                        color_attachment.resolveTexture = resolve->m_texture;
                        color_attachment.resolveLevel = resolve_src.mip_slice;
                        color_attachment.resolveSlice = resolve_src.array_slice;
                    }
                    else
                    {
                        color_attachment.storeAction = encode_store_action(src.store_op, false);
                    }
                    MTLClearColor clear_color;
                    clear_color.red = src.clear_value.x;
                    clear_color.green = src.clear_value.y;
                    clear_color.blue = src.clear_value.z;
                    clear_color.alpha = src.clear_value.w;
                    color_attachment.clearColor = clear_color;
                    color_attachments[i] = color_attachment;
                    width = tex->m_desc.width;
                    height = tex->m_desc.height;
                }
                if(desc.depth_stencil_attachment.texture)
                {
                    auto& src = desc.depth_stencil_attachment;
                    Texture* tex = cast_object<Texture>(src.texture->get_object());
                    depth_attachment.texture = tex->m_texture;
                    depth_attachment.level = src.mip_slice;
                    depth_attachment.slice = src.array_slice;
                    depth_attachment.loadAction = encode_load_action(src.depth_load_op);
                    depth_attachment.storeAction = encode_store_action(src.depth_store_op, false);
                    depth_attachment.clearDepth = src.depth_clear_value;
                    if(is_stencil_format(src.format))
                    {
                        stencil_attachment.texture = tex->m_texture;
                        stencil_attachment.level = src.mip_slice;
                        stencil_attachment.slice = src.array_slice;
                        stencil_attachment.loadAction = encode_load_action(src.stencil_load_op);
                        stencil_attachment.storeAction = encode_store_action(src.stencil_store_op, false);
                        stencil_attachment.clearStencil = src.stencil_clear_value;
                    }
                    width = tex->m_desc.width;
                    height = tex->m_desc.height;
                }
                if(desc.array_size > 1)
                {
                    d.renderTargetArrayLength = desc.array_size;
                }
                if(desc.occlusion_query_heap)
                {
                    BufferQueryHeap* query_heap = cast_object<BufferQueryHeap>(desc.occlusion_query_heap->get_object());
                    d.visibilityResultBuffer = query_heap->m_buffer;
                }
                u32 num_query_heaps = 0;
                if(desc.timestamp_query_heap)
                {
                    CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                    lucheck_msg(heap, "RenderPassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                    if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                    {
                        MTLRenderPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = [d sampleBufferAttachments];
                        MTLRenderPassSampleBufferAttachmentDescriptor* sample_buffer_attachment = [[MTLRenderPassSampleBufferAttachmentDescriptor alloc] init];
                        sample_buffer_attachment.sampleBuffer = heap->m_buffer;
                        sample_buffer_attachment.startOfVertexSampleIndex = desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index;
                        sample_buffer_attachment.endOfVertexSampleIndex = MTLCounterDontSample;
                        sample_buffer_attachment.startOfFragmentSampleIndex = MTLCounterDontSample;
                        sample_buffer_attachment.endOfFragmentSampleIndex = desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index;
                        sample_buffer_attachments[num_query_heaps] = sample_buffer_attachment;
                    }
                    else
                    {
                        m_timestamp_query_heap = heap;
                        m_timestamp_begin_query_index = desc.timestamp_query_begin_pass_write_index;
                        m_timestamp_end_query_index = desc.timestamp_query_end_pass_write_index;
                    }
                    ++num_query_heaps;
                }
                if(desc.pipeline_statistics_query_heap)
                {
                    CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.pipeline_statistics_query_heap->get_object());
                    lucheck_msg(heap, "RenderPassDesc::pipeline_statistics_query_heap must be set to a valid pipeline statistics query heap.");
                    if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                    {
                        MTLRenderPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = [d sampleBufferAttachments];
                        MTLRenderPassSampleBufferAttachmentDescriptor* sample_buffer_attachment = [[MTLRenderPassSampleBufferAttachmentDescriptor alloc] init];
                        sample_buffer_attachment.sampleBuffer = heap->m_buffer;
                        sample_buffer_attachment.startOfVertexSampleIndex = desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2;
                        sample_buffer_attachment.endOfVertexSampleIndex = MTLCounterDontSample;
                        sample_buffer_attachment.startOfFragmentSampleIndex = MTLCounterDontSample;
                        sample_buffer_attachment.endOfFragmentSampleIndex = desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2 + 1;
                        sample_buffer_attachments[num_query_heaps] = sample_buffer_attachment;
                    }
                    else
                    {
                        m_pipeline_statistics_query_heap = heap;
                        m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
                    }
                    ++num_query_heaps;
                }
                d.renderTargetWidth = width;
                d.renderTargetHeight = height;
                d.defaultRasterSampleCount = desc.sample_count == 1 ? 0 : desc.sample_count;
                m_render = [m_buffer renderCommandEncoderWithDescriptor: d];
                if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
                {
                    [m_render sampleCountersInBuffer:m_pipeline_statistics_query_heap->m_buffer atSampleIndex:m_pipeline_statistics_query_index * 2 withBarrier:YES];
                }
                if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
                {
                    [m_render sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_begin_query_index withBarrier:YES];
                }   
            }
        }
        void CommandBuffer::set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout)
        {
            assert_graphcis_context();
        }
        void CommandBuffer::set_graphics_pipeline_state(IPipelineState* pso)
        {
            assert_graphcis_context();
            RenderPipelineState* p = cast_object<RenderPipelineState>(pso->get_object());
            [m_render setRenderPipelineState:p->m_pso];
            [m_render setTriangleFillMode:p->m_fill_mode];
            [m_render setCullMode:p->m_cull_mode];
            [m_render setDepthStencilState:p->m_dss];
            [m_render setFrontFacingWinding:p->m_front_counter_clockwise ? MTLWindingCounterClockwise : MTLWindingClockwise];
            [m_render setDepthBias:p->m_depth_bias slopeScale:p->m_slope_scaled_depth_bias clamp:p->m_depth_bias_clamp];
            auto res = m_device->check_feature(DeviceFeature::rasterizer_depth_clamp);
            if(res.rasterizer_depth_clamp)
            {
                [m_render setDepthClipMode:p->m_depth_clip_mode];
            }
            else if(p->m_depth_clip_mode == MTLDepthClipModeClamp)
            {
                lupanic_msg_always("DeviceFeature::rasterizer_depth_clamp is FALSE on the current device, RasterizerDesc::depth_clamp_enable must be set to `false`.");
            }
            m_primitive_type = p->m_primitive_type;
        }
        void CommandBuffer::set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views)
        {
            assert_graphcis_context();
            StackAllocator salloc;
            id<MTLBuffer> __unsafe_unretained* buffers = (id<MTLBuffer> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLBuffer>) * views.size());
            NSUInteger* offsets = (NSUInteger*)salloc.allocate(sizeof(NSUInteger) * views.size());
            for(usize i = 0; i < views.size(); ++i)
            {
                const VertexBufferView& view = views[i];
                Buffer* buf = cast_object<Buffer>(view.buffer->get_object());
                buffers[i] = buf->m_buffer;
                offsets[i] = view.offset;
            }
            NSRange range;
            range.location = VERTEX_BUFFER_SLOT_OFFSET + start_slot;
            range.length = (NSUInteger)views.size();
            [m_render setVertexBuffers:buffers offsets:offsets withRange:range];
        }
        void CommandBuffer::set_index_buffer(const IndexBufferView& view)
        {
            assert_graphcis_context();
            m_index_buffer_view = view;
        }
        void CommandBuffer::set_graphics_descriptor_set(u32 index, IDescriptorSet* descriptor_set)
        {
            lucheck_msg(index < 16, "Invalid descriptor set index range. Descriptor set index range must be in [0, 16) on Metal.");
            assert_graphcis_context();
            StackAllocator salloc;
            DescriptorSet* set = cast_object<DescriptorSet>(descriptor_set->get_object());
            for(auto& binding : set->m_bindings)
            {
                if(binding.m_resources == nil) continue;
                NSUInteger num_resources = 0;
                id<MTLResource> __unsafe_unretained* resources = (id<MTLResource> __unsafe_unretained*)
                    salloc.allocate(sizeof(id<MTLResource>) * binding.m_resources.count);
                for(usize i = 0; i < binding.m_resources.count; ++i)
                {
                    if([binding.m_resources[i] conformsToProtocol:@protocol(MTLResource)])
                    {
                        id<MTLResource> res = (id<MTLResource>)binding.m_resources[i];
                        resources[num_resources] = res;
                        ++num_resources;
                    }
                }
                [m_render useResources:resources count:num_resources usage:binding.m_usages stages:binding.m_render_stages];
            }
            [m_render setVertexBuffer:set->m_buffer offset:0 atIndex:index];
            [m_render setFragmentBuffer:set->m_buffer offset:0 atIndex:index];
        }
        void CommandBuffer::set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
        {
            lucheck_msg(start_index + descriptor_sets.size() < 16, "Invalid descriptor set index range. Descriptor set index range must be in [0, 16) on Metal.");
            assert_graphcis_context();
            StackAllocator salloc;
            id<MTLBuffer> __unsafe_unretained* buffers = (id<MTLBuffer> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLBuffer>) * descriptor_sets.size());
            NSUInteger* offsets = (NSUInteger*)salloc.allocate(sizeof(NSUInteger) * descriptor_sets.size());
            for(usize i = 0; i < descriptor_sets.size(); ++i)
            {
                DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[i]->get_object());
                buffers[i] = set->m_buffer;
                offsets[i] = 0;
                for(auto& binding : set->m_bindings)
                {
                    if(binding.m_resources == nil) continue;
                    id<MTLResource> __unsafe_unretained* resources = (id<MTLResource> __unsafe_unretained*)
                        salloc.allocate(sizeof(id<MTLResource>) * binding.m_resources.count);
                    NSUInteger num_resources = 0;
                    for(usize i = 0; i < binding.m_resources.count; ++i)
                    {
                        if([binding.m_resources[i] conformsToProtocol:@protocol(MTLResource)])
                        {
                            id<MTLResource> res = (id<MTLResource>)binding.m_resources[i];
                            resources[num_resources] = res;
                            ++num_resources;
                        }
                    }
                    [m_render useResources:resources count:num_resources usage:binding.m_usages stages:binding.m_render_stages];
                }
            }
            NSRange range;
            range.location = start_index;
            range.length = (NSUInteger)descriptor_sets.size();
            [m_render setVertexBuffers:buffers offsets:offsets withRange:range];
            [m_render setFragmentBuffers:buffers offsets:offsets withRange:range];
        }
        void CommandBuffer::set_viewport(const Viewport& viewport)
        {
            assert_graphcis_context();
            MTLViewport vp;
            vp.width = viewport.width;
            vp.height = viewport.height;
            vp.originX = viewport.top_left_x;
            vp.originY = viewport.top_left_y;
            vp.znear = viewport.min_depth;
            vp.zfar = viewport.max_depth;
            [m_render setViewport:vp];
        }
        void CommandBuffer::set_viewports(Span<const Viewport> viewports)
        {
            assert_graphcis_context();
            StackAllocator salloc;
            MTLViewport* vps = (MTLViewport*)salloc.allocate(sizeof(MTLViewport) * viewports.size());
            for(usize i = 0; i < viewports.size(); ++i)
            {
                MTLViewport& dst = vps[i];
                const Viewport& src = viewports[i];
                dst.width = src.width;
                dst.height = src.height;
                dst.originX = src.top_left_x;
                dst.originY = src.top_left_y;
                dst.znear = src.min_depth;
                dst.zfar = src.max_depth;
            }
            [m_render setViewports:vps count:(NSUInteger)viewports.size()];
        }
        void CommandBuffer::set_scissor_rect(const RectI& rect)
        {
            assert_graphcis_context();
            MTLScissorRect dst;
            dst.width = rect.width;
            dst.height = rect.height;
            dst.x = rect.offset_x;
            dst.y = rect.offset_y;
            [m_render setScissorRect:dst];
        }
        void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
        {
            assert_graphcis_context();
            StackAllocator salloc;
            MTLScissorRect* dsts = (MTLScissorRect*)salloc.allocate(sizeof(MTLScissorRect) * rects.size());
            for(usize i = 0; i < rects.size(); ++i)
            {
                MTLScissorRect& dst = dsts[i];
                const RectI& src = rects[i];
                dst.width = src.width;
                dst.height = src.height;
                dst.x = src.offset_x;
                dst.y = src.offset_y;
            }
            [m_render setScissorRects:dsts count:(NSUInteger)rects.size()];
        }
        void CommandBuffer::set_blend_factor(const Float4U& blend_factor)
        {
            assert_graphcis_context();
            [m_render setBlendColorRed:blend_factor.x green:blend_factor.y blue:blend_factor.z alpha:blend_factor.w];
        }
        void CommandBuffer::set_stencil_ref(u32 stencil_ref)
        {
            assert_graphcis_context();
            [m_render setStencilReferenceValue:stencil_ref];
        }
        void CommandBuffer::draw(u32 vertex_count, u32 start_vertex_location)
        {
            assert_graphcis_context();
            [m_render drawPrimitives:m_primitive_type vertexStart:(NSUInteger)start_vertex_location vertexCount:(NSUInteger)vertex_count];
        }
        void CommandBuffer::draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location)
        {
            assert_graphcis_context();
            Buffer* buffer = cast_object<Buffer>(m_index_buffer_view.buffer->get_object());
            MTLIndexType type = encode_index_type(m_index_buffer_view.format);
            start_index_location *= type == MTLIndexTypeUInt16 ? 2 : 4;
            [m_render drawIndexedPrimitives:m_primitive_type 
                indexCount:(NSUInteger)index_count 
                indexType:type 
                indexBuffer:buffer->m_buffer 
                indexBufferOffset:(NSUInteger)start_index_location 
                instanceCount:1 
                baseVertex:(NSInteger)base_vertex_location 
                baseInstance:0];
        }
        void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
                u32 start_instance_location)
        {
            assert_graphcis_context();
            [m_render drawPrimitives:m_primitive_type
                vertexStart:(NSUInteger)start_vertex_location
                vertexCount:(NSUInteger)vertex_count_per_instance 
                instanceCount:(NSUInteger)instance_count
                baseInstance:(NSUInteger)start_instance_location];
        }
        void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
                i32 base_vertex_location, u32 start_instance_location)
        {
            assert_graphcis_context();
            Buffer* buffer = cast_object<Buffer>(m_index_buffer_view.buffer->get_object());
            MTLIndexType type = encode_index_type(m_index_buffer_view.format);
            start_index_location *= type == MTLIndexTypeUInt16 ? 2 : 4;
            [m_render drawIndexedPrimitives:m_primitive_type 
                indexCount:(NSUInteger)index_count_per_instance 
                indexType:type 
                indexBuffer:buffer->m_buffer 
                indexBufferOffset:(NSUInteger)start_index_location 
                instanceCount:(NSUInteger)instance_count 
                baseVertex:(NSInteger)base_vertex_location 
                baseInstance:(NSUInteger)start_instance_location];
        }
        void CommandBuffer::begin_occlusion_query(OcclusionQueryMode mode, u32 index)
        {
            assert_graphcis_context();
            MTLVisibilityResultMode m;
            switch(mode)
            {
                case OcclusionQueryMode::binary: m = MTLVisibilityResultModeBoolean; break;
                case OcclusionQueryMode::counting: m = MTLVisibilityResultModeCounting; break;
            }
            [m_render setVisibilityResultMode:m offset:(NSUInteger)index * 8];
        }
        void CommandBuffer::end_occlusion_query(u32 index)
        {
            [m_render setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:(NSUInteger)index * 8];
        }
        void CommandBuffer::end_render_pass()
        {
            assert_graphcis_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    [m_render sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_end_query_index withBarrier:YES];
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                if(m_pipeline_statistics_query_index != DONT_QUERY)
                {
                    [m_render sampleCountersInBuffer:m_pipeline_statistics_query_heap->m_buffer atSampleIndex:m_pipeline_statistics_query_index * 2 + 1 withBarrier:YES];
                }
                m_pipeline_statistics_query_heap = nullptr;
                m_pipeline_statistics_query_index = DONT_QUERY;
            }
            [m_render endEncoding];
            m_render = nil;
        }
        void CommandBuffer::begin_compute_pass(const ComputePassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_compute_pass can only be called when no other pass is open.");
            @autoreleasepool
            {
                MTLComputePassDescriptor* d = [[MTLComputePassDescriptor alloc]init];
                d.dispatchType = MTLDispatchTypeConcurrent;
                u32 num_query_heaps = 0;
                if(desc.timestamp_query_heap)
                {
                    CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                    lucheck_msg(heap, "ComputePassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                    if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                    {
                        MTLComputePassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = [d sampleBufferAttachments];
                        MTLComputePassSampleBufferAttachmentDescriptor* sample_buffer_attachment = [[MTLComputePassSampleBufferAttachmentDescriptor alloc]init];
                        sample_buffer_attachment.sampleBuffer = heap->m_buffer;
                        sample_buffer_attachment.startOfEncoderSampleIndex = desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index;
                        sample_buffer_attachment.endOfEncoderSampleIndex = desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index;
                        sample_buffer_attachments[num_query_heaps] = sample_buffer_attachment;
                    }
                    else
                    {
                        m_timestamp_query_heap = heap;
                        m_timestamp_begin_query_index = desc.timestamp_query_begin_pass_write_index;
                        m_timestamp_end_query_index = desc.timestamp_query_end_pass_write_index;
                    }
                    ++num_query_heaps;
                }
                if(desc.pipeline_statistics_query_heap)
                {
                    CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.pipeline_statistics_query_heap->get_object());
                    lucheck_msg(heap, "ComputePassDesc::pipeline_statistics_query_heap must be set to a valid pipeline statistics query heap.");
                    if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                    {
                        MTLComputePassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = [d sampleBufferAttachments];
                        MTLComputePassSampleBufferAttachmentDescriptor* sample_buffer_attachment = [[MTLComputePassSampleBufferAttachmentDescriptor alloc]init];
                        sample_buffer_attachment.sampleBuffer = heap->m_buffer;
                        sample_buffer_attachment.startOfEncoderSampleIndex = desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2;
                        sample_buffer_attachment.endOfEncoderSampleIndex = desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2 + 1;
                        sample_buffer_attachments[num_query_heaps] = sample_buffer_attachment;
                    }
                    else
                    {
                        m_pipeline_statistics_query_heap = heap;
                        m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
                    }
                    ++num_query_heaps;
                }
                m_compute = [m_buffer computeCommandEncoderWithDescriptor:d];
                if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
                {
                    [m_compute sampleCountersInBuffer:m_pipeline_statistics_query_heap->m_buffer atSampleIndex:m_pipeline_statistics_query_index * 2 withBarrier:YES];
                }
                if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
                {
                    [m_compute sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_begin_query_index withBarrier:YES];
                }
            }
        }
        void CommandBuffer::set_compute_pipeline_layout(IPipelineLayout* pipeline_layout)
        {
            assert_compute_context();
        }
        void CommandBuffer::set_compute_pipeline_state(IPipelineState* pso)
        {
            assert_compute_context();
            ComputePipelineState* p = cast_object<ComputePipelineState>(pso->get_object());
            [m_compute setComputePipelineState: p->m_pso];
            m_num_threads_per_group = p->m_num_threads_per_group;
        }
        void CommandBuffer::set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set)
        {
            assert_compute_context();
            StackAllocator salloc;
            DescriptorSet* set = cast_object<DescriptorSet>(descriptor_set->get_object());
            for(auto& binding : set->m_bindings)
            {
                if(binding.m_resources == nil) continue;
                id<MTLResource> __unsafe_unretained* resources = (id<MTLResource> __unsafe_unretained*)
                    salloc.allocate(sizeof(id<MTLResource>) * binding.m_resources.count);
                NSUInteger num_resources = 0;
                for(usize i = 0; i < binding.m_resources.count; ++i)
                {
                    if([binding.m_resources[i] conformsToProtocol:@protocol(MTLResource)])
                    {
                        id<MTLResource> res = (id<MTLResource>)binding.m_resources[i];
                        resources[num_resources] = res;
                        ++num_resources;
                    }
                }
                [m_compute useResources:resources count:num_resources usage:binding.m_usages];
            }
            [m_compute setBuffer:set->m_buffer offset:0 atIndex:index];
        }
        void CommandBuffer::set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
        {
            assert_compute_context();
            StackAllocator salloc;
            id<MTLBuffer> __unsafe_unretained* buffers = (id<MTLBuffer> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLBuffer>) * descriptor_sets.size());
            NSUInteger* offsets = (NSUInteger*)salloc.allocate(sizeof(NSUInteger) * descriptor_sets.size());
            for(usize i = 0; i < descriptor_sets.size(); ++i)
            {
                DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[i]->get_object());
                buffers[i] = set->m_buffer;
                offsets[i] = 0;
                for(auto& binding : set->m_bindings)
                {
                    if(binding.m_resources == nil) continue;
                    id<MTLResource> __unsafe_unretained* resources = (id<MTLResource> __unsafe_unretained*)
                        salloc.allocate(sizeof(id<MTLResource>) * binding.m_resources.count);
                    NSUInteger num_resources = 0;
                    for(usize i = 0; i < binding.m_resources.count; ++i)
                    {
                        if([binding.m_resources[i] conformsToProtocol:@protocol(MTLResource)])
                        {
                            id<MTLResource> res = (id<MTLResource>)binding.m_resources[i];
                            resources[num_resources] = res;
                            ++num_resources;
                        }
                    }
                    [m_compute useResources:resources count:num_resources usage:binding.m_usages];
                }
            }
            NSRange range;
            range.location = start_index;
            range.length = (NSUInteger)descriptor_sets.size();
            [m_compute setBuffers:buffers offsets:offsets withRange:range];
        }
        void CommandBuffer::dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z)
        {
            assert_compute_context();
            MTLSize size;
            size.width = thread_group_count_x;
            size.height = thread_group_count_y;
            size.depth = thread_group_count_z;
            MTLSize group;
            group.width = m_num_threads_per_group.x;
            group.height = m_num_threads_per_group.y;
            group.depth = m_num_threads_per_group.z;
            [m_compute dispatchThreadgroups:size threadsPerThreadgroup:group];
        }
        void CommandBuffer::end_compute_pass()
        {
            assert_compute_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    [m_compute sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_end_query_index withBarrier:YES];
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                if(m_pipeline_statistics_query_index != DONT_QUERY)
                {
                    [m_compute sampleCountersInBuffer:m_pipeline_statistics_query_heap->m_buffer atSampleIndex:m_pipeline_statistics_query_index * 2 + 1 withBarrier:YES];
                }
                m_pipeline_statistics_query_heap = nullptr;
                m_pipeline_statistics_query_index = DONT_QUERY;
            }
            [m_compute endEncoding];
            m_compute = nil;
        }
        void CommandBuffer::begin_copy_pass(const CopyPassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_copy_pass can only be called when no other pass is open.");
            @autoreleasepool
            {
                MTLBlitPassDescriptor* d = [[MTLBlitPassDescriptor alloc]init];
                u32 num_query_heaps = 0;
                if(desc.timestamp_query_heap)
                {
                    CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                    lucheck_msg(heap, "CopyPassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                    if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                    {
                        MTLBlitPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = [d sampleBufferAttachments];
                        MTLBlitPassSampleBufferAttachmentDescriptor* sample_buffer_attachment = [[MTLBlitPassSampleBufferAttachmentDescriptor alloc]init];
                        sample_buffer_attachment.sampleBuffer = heap->m_buffer;
                        sample_buffer_attachment.startOfEncoderSampleIndex = desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index;
                        sample_buffer_attachment.endOfEncoderSampleIndex = desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index;
                        sample_buffer_attachments[num_query_heaps] = sample_buffer_attachment;
                    }
                    else
                    {
                        m_timestamp_query_heap = heap;
                        m_timestamp_begin_query_index = desc.timestamp_query_begin_pass_write_index;
                        m_timestamp_end_query_index = desc.timestamp_query_end_pass_write_index;
                    }
                    ++num_query_heaps;
                }
                m_blit = [m_buffer blitCommandEncoderWithDescriptor:d];
                if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::blit))
                {
                    [m_blit sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_begin_query_index withBarrier:YES];
                }
            }
        }
        void CommandBuffer::copy_resource(IResource* dst, IResource* src)
        {
            assert_copy_context();
            {
                Buffer* d = cast_object<Buffer>(dst->get_object());
                Buffer* s = cast_object<Buffer>(src->get_object());
                if(d && s)
                {
                    [m_blit copyFromBuffer:s->m_buffer sourceOffset:0 toBuffer:d->m_buffer destinationOffset:0 size:min(d->m_desc.size, s->m_desc.size)];
                    return;
                }
            }
            {
                Texture* d = cast_object<Texture>(dst->get_object());
                Texture* s = cast_object<Texture>(src->get_object());
                if(d && s)
                {
                    [m_blit copyFromTexture:s->m_texture toTexture:d->m_texture];
                }
            }
        }
        void CommandBuffer::copy_buffer(
            IBuffer* dst, u64 dst_offset,
            IBuffer* src, u64 src_offset,
            u64 copy_bytes)
        {
            assert_copy_context();
            Buffer* d = cast_object<Buffer>(dst->get_object());
            Buffer* s = cast_object<Buffer>(src->get_object());
            [m_blit copyFromBuffer:s->m_buffer sourceOffset:src_offset toBuffer:d->m_buffer destinationOffset:dst_offset size:copy_bytes];
        }
        void CommandBuffer::copy_texture(
            ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
            ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
            u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Texture* d = cast_object<Texture>(dst->get_object());
            Texture* s = cast_object<Texture>(src->get_object());
            MTLOrigin src_origin;
            src_origin.x = src_x;
            src_origin.y = src_y;
            src_origin.z = src_z;
            MTLSize copy_size;
            copy_size.width = copy_width;
            copy_size.height = copy_height;
            copy_size.depth = copy_depth;
            MTLOrigin dst_origin;
            dst_origin.x = dst_x;
            dst_origin.y = dst_y;
            dst_origin.z = dst_z;
            [m_blit copyFromTexture:s->m_texture 
                sourceSlice:src_subresource.array_slice 
                sourceLevel:src_subresource.mip_slice 
                sourceOrigin:src_origin 
                sourceSize:copy_size 
                toTexture:d->m_texture 
                destinationSlice:dst_subresource.array_slice 
                destinationLevel:dst_subresource.mip_slice 
                destinationOrigin:dst_origin];
        }
        void CommandBuffer::copy_buffer_to_texture(
            ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
            IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
            u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Texture* d = cast_object<Texture>(dst->get_object());
            Buffer* s = cast_object<Buffer>(src->get_object());
            MTLSize copy_size;
            copy_size.width = copy_width;
            copy_size.height = copy_height;
            copy_size.depth = copy_depth;
            MTLOrigin dst_origin;
            dst_origin.x = dst_x;
            dst_origin.y = dst_y;
            dst_origin.z = dst_z;
            [m_blit copyFromBuffer:s->m_buffer 
                sourceOffset:src_offset 
                sourceBytesPerRow:src_row_pitch 
                sourceBytesPerImage:src_slice_pitch 
                sourceSize:copy_size 
                toTexture:d->m_texture 
                destinationSlice:dst_subresource.array_slice 
                destinationLevel:dst_subresource.mip_slice 
                destinationOrigin:dst_origin];
        }
        void CommandBuffer::copy_texture_to_buffer(
            IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
            ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
            u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Buffer* d = cast_object<Buffer>(dst->get_object());
            Texture* s = cast_object<Texture>(src->get_object());
            MTLOrigin src_origin;
            src_origin.x = src_x;
            src_origin.y = src_y;
            src_origin.z = src_z;
            MTLSize copy_size;
            copy_size.width = copy_width;
            copy_size.height = copy_height;
            copy_size.depth = copy_depth;
            [m_blit copyFromTexture:s->m_texture 
                sourceSlice:src_subresource.array_slice 
                sourceLevel:src_subresource.mip_slice 
                sourceOrigin:src_origin 
                sourceSize:copy_size 
                toBuffer:d->m_buffer 
                destinationOffset:dst_offset 
                destinationBytesPerRow:dst_row_pitch 
                destinationBytesPerImage:dst_slice_pitch];
        }
        void CommandBuffer::end_copy_pass()
        {
            assert_copy_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::blit))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    [m_blit sampleCountersInBuffer:m_timestamp_query_heap->m_buffer atSampleIndex:m_timestamp_end_query_index withBarrier:YES];
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            [m_blit endEncoding];
            m_blit = nil;
        }
        void CommandBuffer::resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers)
        {
            StackAllocator salloc;
            if(m_compute)
            {
                usize num_resources = buffer_barriers.size() + texture_barriers.size();
                id<MTLResource> __unsafe_unretained* resources = (id<MTLResource> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLResource>) * num_resources);
                NSUInteger i = 0;
                for(const BufferBarrier& barrier : buffer_barriers)
                {
                    Buffer* res = cast_object<Buffer>(barrier.buffer->get_object());
                    resources[i] = res->m_buffer;
                    ++i;
                }
                for(const TextureBarrier& barrier : texture_barriers)
                {
                    Texture* res = cast_object<Texture>(barrier.texture->get_object());
                    resources[i] = res->m_texture;
                    ++i;
                }
                [m_compute memoryBarrierWithResources:resources count:i];
            }
        }
        RV CommandBuffer::submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting)
        {
            @autoreleasepool
            {
                if(!wait_fences.empty())
                {
                    id<MTLCommandBuffer> wait_buffer = [m_device->m_queues[m_command_queue_index].queue commandBuffer];
                    id<MTLBlitCommandEncoder> encoder = [wait_buffer blitCommandEncoder];
                    for(IFence* fence : wait_fences)
                    {
                        Fence* f = cast_object<Fence>(fence->get_object());
                        [encoder waitForFence:f->m_fence];
                    }
                    [encoder endEncoding];
                    [wait_buffer commit];
                }
                if(!signal_fences.empty())
                {
                    id<MTLBlitCommandEncoder> encoder = [m_buffer blitCommandEncoder];
                    for(IFence* fence : signal_fences)
                    {
                        Fence* f = cast_object<Fence>(fence->get_object());
                        [encoder updateFence:f->m_fence];
                    }
                    [encoder endEncoding];
                }
                [m_buffer commit];
                return ok;
            }
        }
    }
}
