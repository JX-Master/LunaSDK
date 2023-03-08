/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderGraph.cpp
* @author JXMaster
* @date 2023/3/6
*/
#include <Runtime/PlatformDefines.hpp>
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
            usize first_access = USIZE_MAX;
            // The index of the node that last accesses the resource.
            usize last_access = 0;
            // All passes that writes to this resource.
            Vector<usize> write_passes;
        };

        inline bool is_resource_desc_valid(const RHI::ResourceDesc& desc)
        {
            // The resource size cannot be 0, which means unintialized.
            if(desc.type == RHI::ResourceType::texture_2d)
            {
                if(!desc.width_or_buffer_size || !desc.height) return false; 
            }
            else if(desc.type == RHI::ResourceType::texture_3d)
            {
                if(!desc.width_or_buffer_size || !desc.height || !desc.depth_or_array_size) return false;
            }
            else
            {
                if(!desc.width_or_buffer_size) return false; 
            }
            return true;
        }

        RV RenderGraph::compile()
        {
            lutry
            {
                m_resource_data.clear();
                m_resource_data.resize(m_desc.resources.size());
                m_pass_data.clear();
                m_pass_data.resize(m_desc.passes.size());
                Vector<ResourceTrackData> resource_track_data(m_resource_data.size());
                // Apply connections.
                for(auto& i : m_desc.input_connections)
                {
                    m_pass_data[i.pass].m_input_resources.insert(make_pair(i.parameter, i.resource));
                    auto& res = resource_track_data[i.resource];
                    res.first_access = min(res.first_access, i.pass);
                    res.last_access = max(res.last_access, i.pass);
                }
                for(auto& i : m_desc.output_connections)
                {
                    m_pass_data[i.pass].m_output_resources.insert(make_pair(i.parameter, i.resource));
                    auto& res = resource_track_data[i.resource];
                    res.first_access = min(res.first_access, i.pass);
                    res.last_access = max(res.last_access, i.pass);
                    res.write_passes.push_back(i.pass);
                }
                // Check alive passes (cull out unrequired passes).
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    if(m_desc.resources[i].type == RenderGraphResourceType::output)
                    {
                        for(usize pass : resource_track_data[i].write_passes)
                        {
                            m_pass_data[pass].m_enabled = true;
                        }
                    }
                }
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
                // Apply user-defined descs.
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    m_resource_data[i].m_resource_desc = m_desc.resources[i].desc;
                }
                // Compile every node in execution order.
                for(usize i = 0; i < m_desc.passes.size(); ++i)
                {
                    m_current_compile_pass = i;
                    if(m_pass_data[i].m_enabled)
                    {
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
                    if(m_desc.resources[i].type == RenderGraphResourceType::internal && res.first_access != USIZE_MAX)
                    {
                        m_pass_data[resource_track_data[i].first_access].m_create_resources.push_back(i);
                        m_pass_data[resource_track_data[i].last_access].m_release_resources.push_back(i);
                    }
                }
                // Create output resources.
                for(usize i = 0; i < m_desc.resources.size(); ++i)
                {
                    if(m_desc.resources[i].type == RenderGraphResourceType::output)
                    {
                        auto& res = m_resource_data[i];
                        if(is_resource_desc_valid(res.m_resource_desc))
                        {
                            luset(res.m_resource, m_device->new_resource(res.m_resource_desc));
                            if (m_desc.resources[i].name) res.m_resource->set_name(m_desc.resources[i].name);
                        }
                        else
                        {
                            return set_error(BasicError::bad_data(), "Cannot create output resource %s because the resource layout is not specified.", m_desc.resources[i].name.c_str());
                        }
                    }
                }
            }
            lucatchret;
            return ok;
        }
        RV RenderGraph::execute(RHI::ICommandBuffer* cmdbuf)
        {
            lutry
            {
                m_cmdbuf = cmdbuf;
                for(usize i = 0; i < m_pass_data.size(); ++i)
                {
                    auto& data = m_pass_data[i];
                    if(!data.m_enabled) continue;
                    // Allocates resources.
                    Vector<RHI::ResourceBarrierDesc> barriers;
                    for(usize h : data.m_create_resources)
                    {
                        auto& res = m_resource_data[h];
                        if(is_resource_desc_valid(res.m_resource_desc))
                        {
                            luset(res.m_resource, m_transient_heap->allocate(res.m_resource_desc));
                            if(m_desc.resources[h].name) res.m_resource->set_name(m_desc.resources[h].name);
                        }
                        else
                        {
                            return set_error(BasicError::bad_data(), "Cannot create transient resource %s because the resource layout is not specified.", m_desc.resources[h].name.c_str());
                        }
                        barriers.push_back(
                            RHI::ResourceBarrierDesc::as_aliasing(res.m_resource)
                        );
                    }
                    if(!barriers.empty()) cmdbuf->resource_barriers({ barriers.data(), barriers.size() });
                    m_current_pass = i;
                    if (m_desc.passes[i].name) cmdbuf->begin_event(m_desc.passes[i].name);
                    luexp(m_pass_data[i].m_render_pass->execute(this));
                    if (m_desc.passes[i].name) cmdbuf->end_event();
                    for(auto& res : m_temporary_resources)
                    {
                        m_transient_heap->release(res);
                    }
                    m_temporary_resources.clear();
                    // Release resources.
                    for(usize h : data.m_release_resources)
                    {
                        auto& res = m_resource_data[h];
                        m_transient_heap->release(res.m_resource);
                    }
                }
            }
            lucatchret;
            return ok;
        }
        R<Ref<RHI::IResource>> RenderGraph::allocate_temporary_resource(const RHI::ResourceDesc& desc)
        {
            Ref<RHI::IResource> ret;
            lutry
            {
                luset(ret, m_transient_heap->allocate(desc));
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
            ret->m_transient_heap = new_transient_resource_heap(device);
            return ret;
        }
    }
}