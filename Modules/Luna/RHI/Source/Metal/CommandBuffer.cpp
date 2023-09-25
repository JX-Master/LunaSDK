/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.cpp
* @author JXMaster
* @date 2023/7/13
*/
#include "CommandBuffer.hpp"
#include "Resource.hpp"
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"

namespace Luna
{
    namespace RHI
    {
        constexpr NS::UInteger MTLCounterDontSample = ((NS::UInteger)-1);
        RV CommandBuffer::init(u32 command_queue_index)
        {
            AutoreleasePool pool;
            m_command_queue_index = command_queue_index;
            m_buffer = retain(m_device->m_queues[command_queue_index].queue->commandBuffer());
            if(!m_buffer) return BasicError::bad_platform_call();
            return ok;
        }
        void CommandBuffer::wait()
        {
            m_buffer->waitUntilCompleted();
        }
        bool CommandBuffer::try_wait()
        {
            auto status = m_buffer->status();
            return status == MTL::CommandBufferStatusCompleted || status == MTL::CommandBufferStatusError;
        }
        RV CommandBuffer::reset()
        {
            AutoreleasePool pool;
            m_objs.clear();
            m_buffer = retain(m_device->m_queues[m_command_queue_index].queue->commandBuffer());
            if(!m_buffer) return BasicError::bad_platform_call();
            return ok;
        }
        void CommandBuffer::attach_device_object(IDeviceChild* obj)
        {
            m_objs.push_back(obj);
        }
        void CommandBuffer::begin_event(const c8* event_name)
        {
            AutoreleasePool pool;
            NS::String* string = NS::String::string(event_name, NS::StringEncoding::UTF8StringEncoding);
            m_buffer->pushDebugGroup(string);
        }
        void CommandBuffer::end_event()
        {
            m_buffer->popDebugGroup();
        }
        void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_render_pass can only be called when no other pass is open.");
            AutoreleasePool pool;
            NSPtr<MTL::RenderPassDescriptor> d = box(MTL::RenderPassDescriptor::alloc()->init());
            MTL::RenderPassColorAttachmentDescriptorArray* color_attachments = d->colorAttachments();
            MTL::RenderPassDepthAttachmentDescriptor* depth_attachment = d->depthAttachment();
            MTL::RenderPassStencilAttachmentDescriptor* stencil_attachment = d->stencilAttachment();
            u32 width = 0;
            u32 height = 0;
            for(u32 i = 0; i < 8; ++i)
            {
                auto& src = desc.color_attachments[i];
                auto& resolve_src = desc.resolve_attachments[i];
                if(!src.texture) break;
                NSPtr<MTL::RenderPassColorAttachmentDescriptor> color_attachment = box(MTL::RenderPassColorAttachmentDescriptor::alloc()->init());
                Texture* tex = cast_object<Texture>(src.texture->get_object());
                color_attachment->setTexture(tex->m_texture.get());
                color_attachment->setLevel(src.mip_slice);
                color_attachment->setSlice(src.array_slice);
                color_attachment->setLoadAction(encode_load_action(src.load_op));
                if(resolve_src.texture)
                {
                    color_attachment->setStoreAction(encode_store_action(src.store_op, true));
                    Texture* resolve = cast_object<Texture>(resolve_src.texture->get_object());
                    color_attachment->setResolveTexture(resolve->m_texture.get());
                    color_attachment->setResolveLevel(resolve_src.mip_slice);
                    color_attachment->setResolveSlice(resolve_src.array_slice);
                }
                else
                {
                    color_attachment->setStoreAction(encode_store_action(src.store_op, false));
                }
                MTL::ClearColor clear_color;
                clear_color.red = src.clear_value.x;
                clear_color.green = src.clear_value.y;
                clear_color.blue = src.clear_value.z;
                clear_color.alpha = src.clear_value.w;
                color_attachment->setClearColor(clear_color);
                color_attachments->setObject(color_attachment.get(), i);
                width = tex->m_desc.width;
                height = tex->m_desc.height;
            }
            if(desc.depth_stencil_attachment.texture)
            {
                auto& src = desc.depth_stencil_attachment;
                Texture* tex = cast_object<Texture>(src.texture->get_object());
                depth_attachment->setTexture(tex->m_texture.get());
                depth_attachment->setLevel(src.mip_slice);
                depth_attachment->setSlice(src.array_slice);
                depth_attachment->setLoadAction(encode_load_action(src.depth_load_op));
                depth_attachment->setStoreAction(encode_store_action(src.depth_store_op, false));
                depth_attachment->setClearDepth(src.depth_clear_value);
                if(is_stencil_format(src.format))
                {
                    stencil_attachment->setTexture(tex->m_texture.get());
                    stencil_attachment->setLevel(src.mip_slice);
                    stencil_attachment->setSlice(src.array_slice);
                    stencil_attachment->setLoadAction(encode_load_action(src.stencil_load_op));
                    stencil_attachment->setStoreAction(encode_store_action(src.stencil_store_op, false));
                    stencil_attachment->setClearStencil(src.stencil_clear_value);
                }
                width = tex->m_desc.width;
                height = tex->m_desc.height;
            }
            if(desc.array_size > 1)
            {
                d->setRenderTargetArrayLength(desc.array_size);
            }
            if(desc.occlusion_query_heap)
            {
                BufferQueryHeap* query_heap = cast_object<BufferQueryHeap>(desc.occlusion_query_heap->get_object());
                d->setVisibilityResultBuffer(query_heap->m_buffer.get());
            }
            u32 num_query_heaps = 0;
            if(desc.timestamp_query_heap)
            {
                CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                lucheck_msg(heap, "RenderPassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                {
                    MTL::RenderPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = d->sampleBufferAttachments();
                    NSPtr<MTL::RenderPassSampleBufferAttachmentDescriptor> sample_buffer_attachment = box(MTL::RenderPassSampleBufferAttachmentDescriptor::alloc()->init());
                    sample_buffer_attachment->setSampleBuffer(heap->m_buffer.get());
                    sample_buffer_attachment->setStartOfVertexSampleIndex(desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index);
                    sample_buffer_attachment->setEndOfVertexSampleIndex(MTLCounterDontSample);
                    sample_buffer_attachment->setStartOfFragmentSampleIndex(MTLCounterDontSample);
                    sample_buffer_attachment->setEndOfFragmentSampleIndex(desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index);
                    sample_buffer_attachments->setObject(sample_buffer_attachment.get(), (NS::UInteger)num_query_heaps);
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
                    MTL::RenderPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = d->sampleBufferAttachments();
                    NSPtr<MTL::RenderPassSampleBufferAttachmentDescriptor> sample_buffer_attachment = box(MTL::RenderPassSampleBufferAttachmentDescriptor::alloc()->init());
                    sample_buffer_attachment->setSampleBuffer(heap->m_buffer.get());
                    sample_buffer_attachment->setStartOfVertexSampleIndex(desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2);
                    sample_buffer_attachment->setEndOfVertexSampleIndex(MTLCounterDontSample);
                    sample_buffer_attachment->setStartOfFragmentSampleIndex(MTLCounterDontSample);
                    sample_buffer_attachment->setEndOfFragmentSampleIndex(desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2 + 1);
                    sample_buffer_attachments->setObject(sample_buffer_attachment.get(), (NS::UInteger)num_query_heaps);
                }
                else
                {
                    m_pipeline_statistics_query_heap = heap;
                    m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
                }
                ++num_query_heaps;
            }
            d->setRenderTargetWidth(width);
            d->setRenderTargetHeight(height);
            d->setDefaultRasterSampleCount(desc.sample_count);
            m_render = retain(m_buffer->renderCommandEncoder(d.get()));
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                m_render->sampleCountersInBuffer(m_pipeline_statistics_query_heap->m_buffer.get(), m_pipeline_statistics_query_index * 2, true);
            }
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                m_render->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_begin_query_index, true);
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
            m_render->setRenderPipelineState(p->m_pso.get());
            m_render->setTriangleFillMode(p->m_fill_mode);
            m_render->setCullMode(p->m_cull_mode);
            m_render->setDepthStencilState(p->m_dss.get());
            m_render->setFrontFacingWinding(p->m_front_counter_clockwise ? MTL::WindingCounterClockwise : MTL::WindingClockwise);
            m_render->setDepthBias(p->m_depth_bias, p->m_slope_scaled_depth_bias, p->m_depth_bias_clamp);
            m_render->setDepthClipMode(p->m_depth_clip_mode);
            m_primitive_type = p->m_primitive_type;
        }
        void CommandBuffer::set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views)
        {
            assert_graphcis_context();
            MTL::Buffer** buffers = (MTL::Buffer**)alloca(sizeof(MTL::Buffer*) * views.size());
            NS::UInteger* offsets = (NS::UInteger*)alloca(sizeof(NS::UInteger) * views.size());
            for(usize i = 0; i < views.size(); ++i)
            {
                const VertexBufferView& view = views[i];
                Buffer* buf = cast_object<Buffer>(view.buffer->get_object());
                buffers[i] = buf->m_buffer.get();
                offsets[i] = view.offset;
            }
            m_render->setVertexBuffers(buffers, offsets, NS::Range::Make(VERTEX_BUFFER_SLOT_OFFSET + start_slot, (NS::UInteger)views.size()));
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
            DescriptorSet* set = cast_object<DescriptorSet>(descriptor_set->get_object());
            Vector<MTL::Resource*> resources;
            for(auto& binding : set->m_bindings)
            {
                if(binding.m_resources.empty()) continue;
                resources.clear();
                for(usize i = 0; i < binding.m_resources.size(); ++i)
                {
                    if(binding.m_resources[i])
                    {
                        resources.push_back(binding.m_resources[i]);
                    }
                }
                m_render->useResources(resources.data(), resources.size(), binding.m_usages, binding.m_render_stages);
            }
            m_render->setVertexBuffer(set->m_buffer.get(), 0, index);
            m_render->setFragmentBuffer(set->m_buffer.get(), 0, index);
        }
        void CommandBuffer::set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
        {
            lucheck_msg(start_index + descriptor_sets.size() < 16, "Invalid descriptor set index range. Descriptor set index range must be in [0, 16) on Metal.");
            assert_graphcis_context();
            MTL::Buffer** buffers = (MTL::Buffer**)alloca(sizeof(MTL::Buffer*) * descriptor_sets.size());
            NS::UInteger* offsets = (NS::UInteger*)alloca(sizeof(NS::UInteger) * descriptor_sets.size());
            Vector<MTL::Resource*> resources;
            for(usize i = 0; i < descriptor_sets.size(); ++i)
            {
                DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[i]->get_object());
                buffers[i] = set->m_buffer.get();
                offsets[i] = 0;
                for(auto& binding : set->m_bindings)
                {
                    if(binding.m_resources.empty()) continue;
                    resources.clear();
                    for(usize i = 0; i < binding.m_resources.size(); ++i)
                    {
                        if(binding.m_resources[i])
                        {
                            resources.push_back(binding.m_resources[i]);
                        }
                    }
                    m_render->useResources(resources.data(), resources.size(), binding.m_usages, binding.m_render_stages);
                }
            }
            m_render->setVertexBuffers(buffers, offsets, NS::Range::Make(start_index, (NS::UInteger)descriptor_sets.size()));
            m_render->setFragmentBuffers(buffers, offsets, NS::Range::Make(start_index, (NS::UInteger)descriptor_sets.size()));
        }
        void CommandBuffer::set_viewport(const Viewport& viewport)
        {
            assert_graphcis_context();
            MTL::Viewport vp;
            vp.width = viewport.width;
            vp.height = viewport.height;
            vp.originX = viewport.top_left_x;
            vp.originY = viewport.top_left_y;
            vp.znear = viewport.min_depth;
            vp.zfar = viewport.max_depth;
            m_render->setViewport(vp);
        }
        void CommandBuffer::set_viewports(Span<const Viewport> viewports)
        {
            assert_graphcis_context();
            MTL::Viewport* vps = (MTL::Viewport*)alloca(sizeof(MTL::Viewport) * viewports.size());
            for(usize i = 0; i < viewports.size(); ++i)
            {
                MTL::Viewport& dst = vps[i];
                const Viewport& src = viewports[i];
                dst.width = src.width;
                dst.height = src.height;
                dst.originX = src.top_left_x;
                dst.originY = src.top_left_y;
                dst.znear = src.min_depth;
                dst.zfar = src.max_depth;
            }
            m_render->setViewports(vps, (NS::UInteger)viewports.size());
        }
        void CommandBuffer::set_scissor_rect(const RectI& rect)
        {
            assert_graphcis_context();
            MTL::ScissorRect dst;
            dst.width = rect.width;
            dst.height = rect.height;
            dst.x = rect.offset_x;
            dst.y = rect.offset_y;
            m_render->setScissorRect(dst);
        }
        void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
        {
            assert_graphcis_context();
            MTL::ScissorRect* dsts = (MTL::ScissorRect*)alloca(sizeof(MTL::ScissorRect) * rects.size());
            for(usize i = 0; i < rects.size(); ++i)
            {
                MTL::ScissorRect& dst = dsts[i];
                const RectI& src = rects[i];
                dst.width = src.width;
                dst.height = src.height;
                dst.x = src.offset_x;
                dst.y = src.offset_y;
            }
            m_render->setScissorRects(dsts, (NS::UInteger)rects.size());
        }
        void CommandBuffer::set_blend_factor(const Float4U& blend_factor)
        {
            assert_graphcis_context();
            m_render->setBlendColor(blend_factor.x, blend_factor.y, blend_factor.z, blend_factor.w);
        }
        void CommandBuffer::set_stencil_ref(u32 stencil_ref)
        {
            assert_graphcis_context();
            m_render->setStencilReferenceValue(stencil_ref);
        }
        void CommandBuffer::draw(u32 vertex_count, u32 start_vertex_location)
        {
            assert_graphcis_context();
            m_render->drawPrimitives(m_primitive_type, (NS::UInteger)start_vertex_location, (NS::UInteger)vertex_count);
        }
        void CommandBuffer::draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location)
        {
            assert_graphcis_context();
            Buffer* buffer = cast_object<Buffer>(m_index_buffer_view.buffer->get_object());
            MTL::IndexType type = encode_index_type(m_index_buffer_view.format);
            start_index_location *= type == MTL::IndexTypeUInt16 ? 2 : 4;
            m_render->drawIndexedPrimitives(m_primitive_type, (NS::UInteger)index_count, type, 
                buffer->m_buffer.get(), (NS::UInteger)start_index_location, 1, (NS::Integer)base_vertex_location, 0);
        }
        void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
				u32 start_instance_location)
        {
            assert_graphcis_context();
            m_render->drawPrimitives(m_primitive_type, (NS::UInteger)start_vertex_location, (NS::UInteger)vertex_count_per_instance, 
                (NS::UInteger)instance_count, (NS::UInteger)start_instance_location);
        }
        void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
				i32 base_vertex_location, u32 start_instance_location)
        {
            assert_graphcis_context();
            Buffer* buffer = cast_object<Buffer>(m_index_buffer_view.buffer->get_object());
            MTL::IndexType type = encode_index_type(m_index_buffer_view.format);
            start_index_location *= type == MTL::IndexTypeUInt16 ? 2 : 4;
            m_render->drawIndexedPrimitives(m_primitive_type, (NS::UInteger)index_count_per_instance, type, 
                buffer->m_buffer.get(), (NS::UInteger)start_index_location, instance_count, (NS::Integer)base_vertex_location, start_instance_location);
        }
        void CommandBuffer::begin_occlusion_query(OcclusionQueryMode mode, u32 index)
        {
            assert_graphcis_context();
            MTL::VisibilityResultMode m;
            switch(mode)
            {
                case OcclusionQueryMode::binary: m = MTL::VisibilityResultModeBoolean; break;
                case OcclusionQueryMode::counting: m = MTL::VisibilityResultModeCounting; break;
            }
            m_render->setVisibilityResultMode(m, (NS::UInteger)index * 8);
        }
        void CommandBuffer::end_occlusion_query(u32 index)
        {
            m_render->setVisibilityResultMode(MTL::VisibilityResultModeDisabled, index * 8);
        }
        void CommandBuffer::end_render_pass()
        {
            assert_graphcis_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    m_render->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_end_query_index, true);
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::draw))
            {
                if(m_pipeline_statistics_query_index != DONT_QUERY)
                {
                    m_render->sampleCountersInBuffer(m_pipeline_statistics_query_heap->m_buffer.get(), m_pipeline_statistics_query_index * 2 + 1, true);
                }
                m_pipeline_statistics_query_heap = nullptr;
                m_pipeline_statistics_query_index = DONT_QUERY;
            }
            m_render->endEncoding();
            m_render.reset();
        }
        void CommandBuffer::begin_compute_pass(const ComputePassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_compute_pass can only be called when no other pass is open.");
            AutoreleasePool pool;
            NSPtr<MTL::ComputePassDescriptor> d = box(MTL::ComputePassDescriptor::alloc()->init());
            d->setDispatchType(MTL::DispatchTypeConcurrent);
            u32 num_query_heaps = 0;
            if(desc.timestamp_query_heap)
            {
                CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                lucheck_msg(heap, "ComputePassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                {
                    MTL::ComputePassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = d->sampleBufferAttachments();
                    NSPtr<MTL::ComputePassSampleBufferAttachmentDescriptor> sample_buffer_attachment = box(MTL::ComputePassSampleBufferAttachmentDescriptor::alloc()->init());
                    sample_buffer_attachment->setSampleBuffer(heap->m_buffer.get());
                    sample_buffer_attachment->setStartOfEncoderSampleIndex(desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index);
                    sample_buffer_attachment->setEndOfEncoderSampleIndex(desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index);
                    sample_buffer_attachments->setObject(sample_buffer_attachment.get(), (NS::UInteger)num_query_heaps);
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
                    MTL::ComputePassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = d->sampleBufferAttachments();
                    NSPtr<MTL::ComputePassSampleBufferAttachmentDescriptor> sample_buffer_attachment = box(MTL::ComputePassSampleBufferAttachmentDescriptor::alloc()->init());
                    sample_buffer_attachment->setSampleBuffer(heap->m_buffer.get());
                    sample_buffer_attachment->setStartOfEncoderSampleIndex(desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2);
                    sample_buffer_attachment->setEndOfEncoderSampleIndex(desc.pipeline_statistics_query_write_index == DONT_QUERY ? MTLCounterDontSample : desc.pipeline_statistics_query_write_index * 2 + 1);
                    sample_buffer_attachments->setObject(sample_buffer_attachment.get(), (NS::UInteger)num_query_heaps);
                }
                else
                {
                    m_pipeline_statistics_query_heap = heap;
                    m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
                }
                ++num_query_heaps;
            }
            m_compute = retain(m_buffer->computeCommandEncoder(d.get()));
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                m_compute->sampleCountersInBuffer(m_pipeline_statistics_query_heap->m_buffer.get(), m_pipeline_statistics_query_index * 2, true);
            }
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                m_compute->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_begin_query_index, true);
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
            m_compute->setComputePipelineState(p->m_pso.get());
            m_num_threads_per_group = p->m_num_threads_per_group;
        }
        void CommandBuffer::set_compute_descriptor_set(u32 index, IDescriptorSet* descriptor_set)
        {
            assert_compute_context();
            DescriptorSet* set = cast_object<DescriptorSet>(descriptor_set->get_object());
            Vector<MTL::Resource*> resources;
            for(auto& binding : set->m_bindings)
            {
                if(binding.m_resources.empty()) continue;
                resources.clear();
                for(usize i = 0; i < binding.m_resources.size(); ++i)
                {
                    if(binding.m_resources[i])
                    {
                        resources.push_back(binding.m_resources[i]);
                    }
                }
                m_compute->useResources(resources.data(), resources.size(), binding.m_usages);
            }
            m_compute->setBuffer(set->m_buffer.get(), 0, index);
        }
        void CommandBuffer::set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
        {
            assert_compute_context();
            MTL::Buffer** buffers = (MTL::Buffer**)alloca(sizeof(MTL::Buffer*) * descriptor_sets.size());
            NS::UInteger* offsets = (NS::UInteger*)alloca(sizeof(NS::UInteger) * descriptor_sets.size());
            Vector<MTL::Resource*> resources;
            for(usize i = 0; i < descriptor_sets.size(); ++i)
            {
                DescriptorSet* set = cast_object<DescriptorSet>(descriptor_sets[i]->get_object());
                buffers[i] = set->m_buffer.get();
                offsets[i] = 0;
                for(auto& binding : set->m_bindings)
                {
                    if(binding.m_resources.empty()) continue;
                    resources.clear();
                    for(usize i = 0; i < binding.m_resources.size(); ++i)
                    {
                        if(binding.m_resources[i])
                        {
                            resources.push_back(binding.m_resources[i]);
                        }
                    }
                    m_compute->useResources(resources.data(), resources.size(), binding.m_usages);
                }
            }
            m_compute->setBuffers(buffers, offsets, NS::Range::Make(start_index, (NS::UInteger)descriptor_sets.size()));
        }
        void CommandBuffer::dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z)
        {
            assert_compute_context();
            m_compute->dispatchThreadgroups(MTL::Size::Make(thread_group_count_x, thread_group_count_y, thread_group_count_z), 
                MTL::Size::Make(m_num_threads_per_group.x, m_num_threads_per_group.y, m_num_threads_per_group.z));
        }
        void CommandBuffer::end_compute_pass()
        {
            assert_compute_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    m_compute->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_end_query_index, true);
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            if(m_pipeline_statistics_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::dispatch))
            {
                if(m_pipeline_statistics_query_index != DONT_QUERY)
                {
                    m_compute->sampleCountersInBuffer(m_pipeline_statistics_query_heap->m_buffer.get(), m_pipeline_statistics_query_index * 2 + 1, true);
                }
                m_pipeline_statistics_query_heap = nullptr;
                m_pipeline_statistics_query_index = DONT_QUERY;
            }
            m_compute->endEncoding();
            m_compute.reset();
        }
        void CommandBuffer::begin_copy_pass(const CopyPassDesc& desc)
        {
            lucheck_msg(!m_render && !m_compute && !m_blit, "begin_copy_pass can only be called when no other pass is open.");
            AutoreleasePool pool;
            NSPtr<MTL::BlitPassDescriptor> d = box(MTL::BlitPassDescriptor::alloc()->init());
            u32 num_query_heaps = 0;
            if(desc.timestamp_query_heap)
            {
                CounterSampleQueryHeap* heap = cast_object<CounterSampleQueryHeap>(desc.timestamp_query_heap->get_object());
                lucheck_msg(heap, "CopyPassDesc::timestamp_query_heap must be set to a valid timestamp query heap.");
                if(test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::stage))
                {
                    MTL::BlitPassSampleBufferAttachmentDescriptorArray* sample_buffer_attachments = d->sampleBufferAttachments();
                    NSPtr<MTL::BlitPassSampleBufferAttachmentDescriptor> sample_buffer_attachment = box(MTL::BlitPassSampleBufferAttachmentDescriptor::alloc()->init());
                    sample_buffer_attachment->setSampleBuffer(heap->m_buffer.get());
                    sample_buffer_attachment->setStartOfEncoderSampleIndex(desc.timestamp_query_begin_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_begin_pass_write_index);
                    sample_buffer_attachment->setEndOfEncoderSampleIndex(desc.timestamp_query_end_pass_write_index == DONT_QUERY ? MTLCounterDontSample : desc.timestamp_query_end_pass_write_index);
                    sample_buffer_attachments->setObject(sample_buffer_attachment.get(), (NS::UInteger)num_query_heaps);
                }
                else
                {
                    m_timestamp_query_heap = heap;
                    m_timestamp_begin_query_index = desc.timestamp_query_begin_pass_write_index;
                    m_timestamp_end_query_index = desc.timestamp_query_end_pass_write_index;
                }
                ++num_query_heaps;
            }
            m_blit = retain(m_buffer->blitCommandEncoder(d.get()));
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::blit))
            {
                m_blit->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_begin_query_index, true);
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
                    m_blit->copyFromBuffer(s->m_buffer.get(), 0, d->m_buffer.get(), 0, min(d->m_desc.size, s->m_desc.size));
                    return;
                }
            }
            {
                Texture* d = cast_object<Texture>(dst->get_object());
                Texture* s = cast_object<Texture>(src->get_object());
                if(d && s)
                {
                    m_blit->copyFromTexture(s->m_texture.get(), d->m_texture.get());
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
            m_blit->copyFromBuffer(s->m_buffer.get(), src_offset, d->m_buffer.get(), dst_offset, copy_bytes);
        }
        void CommandBuffer::copy_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Texture* d = cast_object<Texture>(dst->get_object());
            Texture* s = cast_object<Texture>(src->get_object());
            m_blit->copyFromTexture(s->m_texture.get(), src_subresource.array_slice, src_subresource.mip_slice, 
                MTL::Origin::Make(src_x, src_y, src_z), MTL::Size::Make(copy_width, copy_height, copy_depth),
                d->m_texture.get(), dst_subresource.array_slice, dst_subresource.mip_slice, MTL::Origin::Make(dst_x, dst_y, dst_z));
        }
        void CommandBuffer::copy_buffer_to_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
			u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Texture* d = cast_object<Texture>(dst->get_object());
            Buffer* s = cast_object<Buffer>(src->get_object());
            m_blit->copyFromBuffer(s->m_buffer.get(), src_offset, src_row_pitch, src_slice_pitch, 
                MTL::Size::Make(copy_width, copy_height, copy_depth), 
                d->m_texture.get(), dst_subresource.array_slice, dst_subresource.mip_slice, MTL::Origin::Make(dst_x, dst_y, dst_z));
        }
        void CommandBuffer::copy_texture_to_buffer(
			IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
        {
            assert_copy_context();
            Buffer* d = cast_object<Buffer>(dst->get_object());
            Texture* s = cast_object<Texture>(src->get_object());
            m_blit->copyFromTexture(s->m_texture.get(), src_subresource.array_slice, src_subresource.mip_slice, MTL::Origin::Make(src_x, src_y, src_z),
                MTL::Size::Make(copy_width, copy_height, copy_depth), d->m_buffer.get(), dst_offset, dst_row_pitch, dst_slice_pitch);
        }
        void CommandBuffer::end_copy_pass()
        {
            assert_copy_context();
            if(m_timestamp_query_heap && test_flags(m_device->m_counter_sampling_support_flags, CounterSamplingSupportFlag::blit))
            {
                if(m_timestamp_end_query_index != DONT_QUERY)
                {
                    m_blit->sampleCountersInBuffer(m_timestamp_query_heap->m_buffer.get(), m_timestamp_end_query_index, true);
                }
                m_timestamp_query_heap = nullptr;
                m_timestamp_begin_query_index = DONT_QUERY;
                m_timestamp_end_query_index = DONT_QUERY;
            }
            m_blit->endEncoding();
            m_blit.reset();
        }
        void CommandBuffer::resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers)
        {
             if(m_compute)
             {
                 usize num_resources = buffer_barriers.size() + texture_barriers.size();
                 MTL::Resource** resources = (MTL::Resource**)alloca(sizeof(MTL::Resource*) * num_resources);
                 usize i = 0;
                 for(const BufferBarrier& barrier : buffer_barriers)
                 {
                     Buffer* res = cast_object<Buffer>(barrier.buffer->get_object());
                     resources[i] = res->m_buffer.get();
                     ++i;
                 }
                 for(const TextureBarrier& barrier : texture_barriers)
                 {
                     Texture* res = cast_object<Texture>(barrier.texture->get_object());
                     resources[i] = res->m_texture.get();
                     ++i;
                 }
                 m_compute->memoryBarrier(resources, i);
             }
        }
        RV CommandBuffer::submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting)
        {
            AutoreleasePool pool;
            if(!wait_fences.empty())
            {
                MTL::CommandBuffer* wait_buffer = m_device->m_queues[m_command_queue_index].queue->commandBuffer();
                MTL::BlitCommandEncoder* encoder = wait_buffer->blitCommandEncoder();
                for(IFence* fence : wait_fences)
                {
                    Fence* f = cast_object<Fence>(fence->get_object());
                    encoder->waitForFence(f->m_fence.get());
                }
                encoder->endEncoding();
                wait_buffer->commit();
            }
            if(!signal_fences.empty())
            {
                MTL::BlitCommandEncoder* encoder = m_buffer->blitCommandEncoder();
                for(IFence* fence : signal_fences)
                {
                    Fence* f = cast_object<Fence>(fence->get_object());
                    encoder->updateFence(f->m_fence.get());
                }
                encoder->endEncoding();
            }
            m_buffer->commit();
            return ok;
        }
    }
}
