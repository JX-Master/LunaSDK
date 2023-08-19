/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderGraph.cpp
* @author JXMaster
* @date 2023/3/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RG_API LUNA_EXPORT
#include "RenderGraph.hpp"
#include "RenderPass.hpp"

namespace Luna
{
    namespace RG
    {
        struct RenderGraphPassNodeExtractKey
        {
            const Name& operator()(const RenderGraphPassNode& rhs) const
            {
                return rhs.name;
            }
        };

        struct ResourceTrackData
        {
            // The output parameter this resource is attached to.
            // Name parameter;
            // The index of the node that creates resource.
            // If `first_access` is `USIZE_MAX`, this resource is never used and should not be tracked.
            usize first_access = USIZE_MAX;
            // The index of the node that last accesses the resource.
            usize last_access = 0;
            // All passes that writes to this resource.
            Vector<usize> write_passes;
        };

        inline bool is_resource_desc_valid(const ResourceDesc& desc)
        {
            // The resource size cannot be 0, which means unintialized.
            if(desc.type == ResourceType::texture && desc.texture.type == RHI::TextureType::tex2d)
            {
                if(!desc.texture.width || !desc.texture.height) return false; 
            }
            else if(desc.type == ResourceType::texture && desc.texture.type == RHI::TextureType::tex3d)
            {
                if(!desc.texture.width || !desc.texture.height || !desc.texture.depth) return false;
            }
            else if(desc.type == ResourceType::texture && desc.texture.type == RHI::TextureType::tex1d)
            {
                if(!desc.texture.width) return false;
            }
            else if (desc.type == ResourceType::buffer)
            {
                if (!desc.buffer.size) return false;
            }
            return true;
        }

        RV RenderGraph::compile(const RenderGraphCompileConfig& config)
        {
            lutry
            {
                m_resource_data.clear();
                m_resource_data.resize(m_desc.resources.size());
                m_pass_data.clear();
                m_pass_data.resize(m_desc.passes.size());
                m_enable_time_profiling = config.enable_time_profiling;
                Vector<ResourceTrackData> resource_track_data(m_resource_data.size());
                // Initialize pass data and resource track data.
                for (auto& i : m_desc.input_connections)
                {
                    m_pass_data[i.pass].m_input_resources.insert(make_pair(i.parameter, i.resource));
                }
                for (auto& i : m_desc.output_connections)
                {
                    m_pass_data[i.pass].m_output_resources.insert(make_pair(i.parameter, i.resource));
                    auto& res = resource_track_data[i.resource];
                    res.write_passes.push_back(i.pass);
                }
                // Cull out unrequired passes.
                // Scan the resource to find output resources, all passes that writes to output resources should be enabled.
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    if(test_flags(m_desc.resources[i].flags, RenderGraphResourceFlag::output))
                    {
                        for(usize pass : resource_track_data[i].write_passes)
                        {
                            m_pass_data[pass].m_enabled = true;
                        }
                    }
                }
                // Scan the pass queue in reverse order, all passes that write to input resources of enabled passes should also be enabled.
                for(usize i = 0; i < m_desc.passes.size(); ++i)
                {
                    auto& pass = m_pass_data[m_desc.passes.size() - i - 1];
                    if(pass.m_enabled)
                    {
                        for(auto r : pass.m_input_resources)
                        {
                            for(usize prior_pass : resource_track_data[r.second].write_passes)
                            {
                                m_pass_data[prior_pass].m_enabled = true;
                            }
                        }
                    }
                }
                // Determine transient resource lifetime.
                for (auto& i : m_desc.input_connections)
                {
                    if (m_pass_data[i.pass].m_enabled)
                    {
                        auto& res = resource_track_data[i.resource];
                        res.first_access = min(res.first_access, i.pass);
                        res.last_access = max(res.last_access, i.pass);
                    }   
                }
                for (auto& i : m_desc.output_connections)
                {
                    if (m_pass_data[i.pass].m_enabled)
                    {
                        auto& res = resource_track_data[i.resource];
                        res.first_access = min(res.first_access, i.pass);
                        res.last_access = max(res.last_access, i.pass);
                    }
                }
                // Apply user-defined descs.
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    m_resource_data[i].m_resource_desc = m_desc.resources[i].desc;
                }
                // Compile every node in execution order.
                u32 num_enabled_passes = 0;
                for(usize i = 0; i < m_desc.passes.size(); ++i)
                {
                    m_current_compile_pass = i;
                    if(m_pass_data[i].m_enabled)
                    {
                        ++num_enabled_passes;
                        MutexGuard guard(g_render_pass_types_mtx);
                        auto iter = g_render_pass_types.find(m_desc.passes[i].type);
                        if(iter == g_render_pass_types.end())
                        {
                            return set_error(BasicError::not_found(), "Render pass type \"%s\" is not found.", m_desc.passes[i].type.c_str());
                        }
                        luexp(iter->compile(iter->userdata.get(), this));
                    }
                }
                // Resolve transient resource lifetime.
                for(usize i = 0; i < resource_track_data.size(); ++i)
                {
                    auto& res = resource_track_data[i];
                    if(m_desc.resources[i].type == RenderGraphResourceType::transient && res.first_access != USIZE_MAX)
                    {
                        m_pass_data[resource_track_data[i].first_access].m_create_resources.push_back(i);
                        m_pass_data[resource_track_data[i].last_access].m_release_resources.push_back(i);
                    }
                }
                // Create output resources.
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    if(m_desc.resources[i].type == RenderGraphResourceType::persistent)
                    {
                        auto& res = m_resource_data[i];
                        if(is_resource_desc_valid(res.m_resource_desc))
                        {
                            if (res.m_resource_desc.type == ResourceType::buffer)
                            {
                                luset(res.m_resource, m_device->new_buffer(res.m_resource_desc.memory_type, res.m_resource_desc.buffer));
                            }
                            else
                            {
                                luset(res.m_resource, m_device->new_texture(res.m_resource_desc.memory_type, res.m_resource_desc.texture));
                            }
                            if (m_desc.resources[i].name) res.m_resource->set_name(m_desc.resources[i].name.c_str());
                        }
                        else
                        {
                            return set_error(BasicError::bad_data(), "Cannot create output resource %s because the resource layout is not specified.", m_desc.resources[i].name.c_str());
                        }
                    }
                }
                // Recreate time query heap.
                if (m_enable_time_profiling)
                {
                    if (!m_time_query_heap || m_time_query_heap_capacity < num_enabled_passes)
                    {
                        RHI::QueryHeapDesc desc;
                        desc.type = RHI::QueryType::timestamp;
                        desc.count = num_enabled_passes * 2;
                        luset(m_time_query_heap, m_device->new_query_heap(desc));
                        m_time_query_heap_capacity = num_enabled_passes;
                    }
                }
                m_num_enabled_passes = num_enabled_passes;
            }
            lucatchret;
            return ok;
        }
        void RenderGraph::get_enabled_render_passes(Vector<usize>& render_passes)
        {
            render_passes.clear();
            for (usize i = 0; i < m_pass_data.size(); ++i)
            {
                if (m_pass_data[i].m_enabled)
                {
                    render_passes.push_back(i);
                }
            }
        }
        RV RenderGraph::execute(RHI::ICommandBuffer* cmdbuf)
        {
            lutry
            {
                m_transient_memory.clear();
                m_cmdbuf = cmdbuf;
                m_current_time_query_index = 0;
                for(usize i = 0; i < m_pass_data.size(); ++i)
                {
                    auto& data = m_pass_data[i];
                    if(!data.m_enabled) continue;
                    // Allocates resources.
                    Vector<RHI::BufferBarrier> buffer_barriers;
                    Vector<RHI::TextureBarrier> texture_barriers;
                    for(usize h : data.m_create_resources)
                    {
                        auto& res = m_resource_data[h];
                        if(is_resource_desc_valid(res.m_resource_desc))
                        {
                            luset(res.m_resource, allocate_transient_resource(res.m_resource_desc));
                            cmdbuf->attach_device_object(res.m_resource);
                            if(m_desc.resources[h].name) res.m_resource->set_name(m_desc.resources[h].name.c_str());
                        }
                        else
                        {
                            return set_error(BasicError::bad_data(), "Cannot create transient resource %s because the resource layout is not specified.", m_desc.resources[h].name.c_str());
                        }
                        if (res.m_resource_desc.type == ResourceType::texture)
                        {
                            Ref<RHI::ITexture> tex = res.m_resource;
                            texture_barriers.push_back({ tex, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::none, RHI::ResourceBarrierFlag::aliasing });
                        }
                        else
                        {
                            Ref<RHI::IBuffer> buf = res.m_resource;
                            buffer_barriers.push_back({ buf, RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::none, RHI::ResourceBarrierFlag::aliasing });
                        }
                    }
                    if (!buffer_barriers.empty() || !texture_barriers.empty()) cmdbuf->resource_barrier({ buffer_barriers.data(), buffer_barriers.size() }, {texture_barriers.data(), texture_barriers.size()});
                    m_current_pass = i;
                    if (m_desc.passes[i].name) cmdbuf->begin_event(m_desc.passes[i].name.c_str());
                    luexp(m_pass_data[i].m_render_pass->execute(this));
                    if (m_desc.passes[i].name) cmdbuf->end_event();
                    for(auto& res : m_temporary_resources)
                    {
                        release_transient_resource(res);
                    }
                    m_temporary_resources.clear();
                    // Release resources.
                    for(usize h : data.m_release_resources)
                    {
                        auto& res = m_resource_data[h];
                        release_transient_resource(res.m_resource);
                    }
                    ++m_current_time_query_index;
                }
            }
            lucatchret;
            return ok;
        }
        RV RenderGraph::get_pass_time_intervals(Vector<u64>& pass_time_intervals)
        {
            lutry
            {
                pass_time_intervals.clear();
                if (m_enable_time_profiling)
                {
                    Vector<u64> times((usize)m_num_enabled_passes * 2);
                    luexp(m_time_query_heap->get_timestamp_values(0, m_num_enabled_passes * 2, times.data()));
                    for (usize i = 0; i < m_num_enabled_passes; ++i)
                    {
                        pass_time_intervals.push_back(times[i * 2 + 1] - times[i * 2]);
                    }
                }
            }
            lucatchret;
            return ok;
        }
        R<Ref<RHI::IResource>> RenderGraph::allocate_temporary_resource(const ResourceDesc& desc)
        {
            Ref<RHI::IResource> ret;
            lutry
            {
                luset(ret, allocate_transient_resource(desc));
                m_temporary_resources.push_back(ret);
            }
            lucatchret;
            return ret;
        }
        void RenderGraph::release_temporary_resource(RHI::IResource* res)
        {
            for(auto iter = m_temporary_resources.begin(); iter != m_temporary_resources.end(); ++iter)
            {
                if(iter->get() == res)
                {
                    m_temporary_resources.erase(iter);
                    return;
                }
            }
        }
        LUNA_RG_API Ref<IRenderGraph> new_render_graph(RHI::IDevice* device)
        {
            auto ret = new_object<RenderGraph>();
            ret->m_device = device;
            return ret;
        }
    }
}
