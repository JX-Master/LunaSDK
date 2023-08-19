/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderGraph.hpp
* @author JXMaster
* @date 2023/3/6
*/
#pragma once
#include "../RenderGraph.hpp"
namespace Luna
{
    namespace RG
    {
        struct RenderGraph : IRenderGraph, IRenderGraphCompiler, IRenderPassContext
        {
            lustruct("RG::RenderGraph", "{feefd806-4b82-48cd-b350-f8fc9387fc65}");
            luiimpl();

            Ref<RHI::IDevice> m_device;
            RenderGraphDesc m_desc;

            // Produced by compiling the render graph.
            struct PassData
            {
                HashMap<Name, usize> m_input_resources;
                HashMap<Name, usize> m_output_resources;
                // The indices of `m_transient_resources` that should be released when this node is finished.
                Vector<usize> m_create_resources;
                Vector<usize> m_release_resources;
                Ref<IRenderPass> m_render_pass;
                bool m_enabled = false;
            };
            struct ResourceData
            {
                ResourceDesc m_resource_desc;
                Ref<RHI::IResource> m_resource;
            };
            Vector<PassData> m_pass_data;
            Vector<ResourceData> m_resource_data;
            bool m_enable_time_profiling;

            Ref<RHI::IQueryHeap> m_time_query_heap;
            u32 m_time_query_heap_capacity = 0;
            u32 m_current_time_query_index = 0;
            u32 m_num_enabled_passes;

            // Compile context.
            usize m_current_compile_pass;

            // Execution context.
            Ref<RHI::ICommandBuffer> m_cmdbuf;
            Vector<Ref<RHI::IResource>> m_temporary_resources;
            usize m_current_pass;

            Vector<Ref<RHI::IDeviceMemory>> m_transient_memory;
            R<Ref<RHI::IResource>> allocate_transient_resource(const ResourceDesc& desc)
            {
                // Try to reuse one memory block.
                Ref<RHI::IResource> ret;
                auto iter = m_transient_memory.begin();
                while (iter != m_transient_memory.end())
                {
                    if (desc.type == ResourceType::texture)
                    {
                        auto r = m_device->new_aliasing_texture(*iter, desc.texture);
                        if (succeeded(r))
                        {
                            ret = r.get();
                            m_transient_memory.erase(iter);
                            break;
                        }
                    }
                    else
                    {
                        auto r = m_device->new_aliasing_buffer(*iter, desc.buffer);
                        if (succeeded(r))
                        {
                            ret = r.get();
                            m_transient_memory.erase(iter);
                            break;
                        }
                    }
                    ++iter;
                }
                if (!ret)
                {
                    // Try to allocate one new block.
                    if (desc.type == ResourceType::texture)
                    {
                        auto tex = desc.texture;
                        tex.flags |= RHI::ResourceFlag::allow_aliasing;
                        auto r = m_device->new_texture(desc.memory_type, tex);
                        if (failed(r)) return r.errcode();
                        ret = r.get();
                    }
                    else
                    {
                        auto buffer = desc.buffer;
                        buffer.flags |= RHI::ResourceFlag::allow_aliasing;
                        auto r = m_device->new_buffer(desc.memory_type, buffer);
                        if (failed(r)) return r.errcode();
                        ret = r.get();
                    }
                }
                return ret;
            }
            void release_transient_resource(RHI::IResource* resource)
            {
                m_transient_memory.push_back(resource->get_memory());
            }
            virtual RHI::IDevice* get_device() override { return m_device.get(); }
            virtual const RenderGraphDesc& get_desc() override { return m_desc; }
            virtual void set_desc(const RenderGraphDesc& desc) override { m_desc = desc; }
            virtual RV compile(const RenderGraphCompileConfig& config) override;
            virtual void get_enabled_render_passes(Vector<usize>& render_passes) override;
            virtual IRenderPass* get_render_pass(usize index) override
            {
                return m_pass_data[index].m_render_pass;
            }
            virtual void set_external_resource(usize index, RHI::IResource* resource) override
            {
                if(m_desc.resources[index].type == RenderGraphResourceType::external)
                {
                    m_resource_data[index].m_resource = resource;
                }
            }
            virtual RV execute(RHI::ICommandBuffer* cmdbuf) override;
            virtual RHI::IResource* get_persistent_resource(usize index) override
            {
                if(m_desc.resources[index].type == RenderGraphResourceType::persistent)
                {
                    return m_resource_data[index].m_resource;
                }
                return nullptr;
            }
            virtual RV get_pass_time_intervals(Vector<u64>& pass_time_intervals) override;

            virtual usize get_input_resource(const Name& parameter) override
            {
                auto& pass = m_pass_data[m_current_compile_pass];
                auto iter = pass.m_input_resources.find(parameter);
                return iter == pass.m_input_resources.end() ? INVALID_RESOURCE : iter->second;
            }
            virtual usize get_output_resource(const Name& parameter) override
            {
                auto& pass = m_pass_data[m_current_compile_pass];
                auto iter = pass.m_output_resources.find(parameter);
                return iter == pass.m_output_resources.end() ? INVALID_RESOURCE : iter->second;
            }

            virtual ResourceDesc get_resource_desc(usize resource) override
            {
                lucheck(resource < m_resource_data.size());
                auto& res = m_resource_data[resource];
                return res.m_resource_desc;
            }
            virtual void set_resource_desc(usize resource, const ResourceDesc& desc) override
            {
                if(resource >= m_resource_data.size()) return;
                auto& res = m_resource_data[resource];
                res.m_resource_desc = desc;
            }
            virtual void set_render_pass_object(IRenderPass* render_pass) override
            {
                m_pass_data[m_current_compile_pass].m_render_pass = render_pass;
            }

            virtual RHI::ICommandBuffer* get_command_buffer() override { return m_cmdbuf; }
            virtual RHI::IResource* get_input(const Name& name) override
            {
                auto& data = m_pass_data[m_current_pass];
                auto iter = data.m_input_resources.find(name);
                if(iter == data.m_input_resources.end()) return nullptr;
                auto h = iter->second;
                return m_resource_data[h].m_resource;
            }
            virtual RHI::IResource* get_output(const Name& name) override
            {
                auto& data = m_pass_data[m_current_pass];
                auto iter = data.m_output_resources.find(name);
                if(iter == data.m_output_resources.end()) return nullptr;
                auto h = iter->second;
                return m_resource_data[h].m_resource;
            }
            virtual RHI::IQueryHeap* get_timestamp_query_heap(u32* begin_index, u32* end_index) override
            {
                if(m_enable_time_profiling)
                {
                    if(begin_index) *begin_index = m_current_time_query_index * 2;
                    if(end_index) *end_index = m_current_time_query_index * 2 + 1;
                    return m_time_query_heap;
                }
                if(begin_index) *begin_index = 0;
                if(end_index) *end_index = 0;
                return nullptr;
            }
            virtual R<Ref<RHI::IResource>> allocate_temporary_resource(const ResourceDesc& desc) override;
            virtual void release_temporary_resource(RHI::IResource* res) override;
        };
    }
}
