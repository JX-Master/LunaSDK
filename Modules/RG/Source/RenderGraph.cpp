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
        struct RenderGraphNodeExtractKey
        {
            const Name& operator()(const RenderGraphNode& rhs) const
            {
                return rhs.name;
            }
        };

        struct OutputResource
        {
            // The output parameter this resource is attached to.
            Name parameter;
            // The index of the node that creates resource.
            usize creator;
            // The index of the node that last accesses the resource.
            usize last_access;
        };

        RV RenderGraph::compile()
        {
            lutry
            {
                // Check node names.
                {
                    SelfIndexedHashMap<Name, RenderGraphNode, RenderGraphNodeExtractKey> nodes;
                    for(auto& i : m_desc.nodes)
                    {
                        auto result = nodes.insert(i);
                        if(result.second == false) return set_error(BasicError::already_exists(), "Multiple nodes have the same name \"%s\", which is not allowed.", i.name.c_str());
                    }
                }
                m_resources.clear();
                m_node_data.clear();
                m_node_data.resize(m_desc.nodes.size());
                m_input_resources.clear();
                m_output_resources.clear();
                HashMap<ResourceHandle, OutputResource> output_resources;
                // Create input resource entry.
                for(auto& i : m_desc.input_parameters)
                {
                    auto iter = m_input_resource_descs.find(i.name);
                    if(iter != m_input_resource_descs.end())
                    {
                        ResourceHandle handle = new_resource_entry();
                        auto& res = get_resource(handle);
                        res.type = ResourceType::input;
                        res.resource_desc = iter->second;
                        m_input_resources.insert(make_pair(i.name, handle));
                        lulet(node_data, find_node(i.exported_node));
                        node_data->m_input_resources.insert(make_pair(i.exported_parameter, handle));
                    }
                }
                // Compile every node in execution order.
                for(usize i = 0; i < m_desc.nodes.size(); ++i)
                {
                    MutexGuard guard(g_render_pass_types_mtx);
                    auto iter = g_render_pass_types.find(m_desc.nodes[i].type);
                    if(iter == g_render_pass_types.end())
                    {
                        return set_error(BasicError::not_found(), "Render pass type \"%s\" is not found.", m_desc.nodes[i].type.c_str());
                    }
                    RenderPassInputInfo input_info;
                    RenderPassOutputInfo output_info;
                    for(auto input : m_node_data[i].m_input_resources)
                    {
                        auto& res = get_resource(input.second);
                        input_info.input_parameters.insert(make_pair(input.first, res.resource_desc));
                    }
                    luset(output_info, iter->compile(iter->userdata.get(), input_info));
                    guard.unlock();
                    // Apply outputs.
                    Vector<Pair<Name, Name>> targets;
                    for(auto& o : output_info.output_parameters)
                    {
                        ResourceHandle handle;
                        bool valid_output = false;
                        if(o.second.type == RenderPassOutputParameterType::default)
                        {
                            // Create a new output resource.
                            valid_output = true;
                            handle = new_resource_entry();
                            auto& res = get_resource(handle);
                            res.type = ResourceType::transient;
                            res.resource_desc = o.second.resource_desc;
                            OutputResource out_res;
                            out_res.creator = i;
                            out_res.last_access = i;
                            out_res.parameter = o.first;
                            output_resources.insert(make_pair(handle, out_res));
                        }
                        else if(o.second.type == RenderPassOutputParameterType::input)
                        {
                            // Finds the resource entry.
                            Name output_node;
                            Name output_parameter;
                            bool found = find_input_source(m_desc.nodes[i].name, o.second.input_name, output_node, output_parameter);
                            if(found)
                            {
                                valid_output = true;
                                lulet(node_data, find_node(output_node));
                                auto iter = node_data->m_output_resources.find(output_parameter);
                                if(iter == node_data->m_output_resources.end())
                                {
                                    return set_error(BasicError::not_found(), "Output parameter \"%s\" is not found on render pass \"%s\".", output_parameter.c_str(), output_node.c_str());
                                }
                                handle = iter->second;
                                auto iter2 = output_resources.find(handle);
                                if(iter2 != output_resources.end())
                                {
                                    iter2->second.last_access = max(iter2->second.last_access, i);
                                }
                            }
                        }
                        else 
                        {
                            lupanic(); 
                        }
                        if(valid_output)
                        {
                            m_node_data[i].m_output_resources.insert(make_pair(o.first, handle));
                            // Broadcast output to connected inputs.
                            find_output_targets(m_desc.nodes[i].name, o.first, targets);
                            for(auto& t : targets)
                            {
                                lulet(t_node, find_node(t.first));
                                t_node->m_input_resources.insert(make_pair(t.second, handle));
                            }
                        }
                    }
                }
                // Apply global outputs.
                for(auto& output : m_desc.output_parameters)
                {
                    for(auto iter = output_resources.begin(); iter != output_resources.end(); ++iter)
                    {
                        if(output.exported_node == m_desc.nodes[iter->second.creator].name && 
                            output.exported_parameter == iter->second.parameter)
                        {
                            m_output_resources.insert(make_pair(output.name, iter->first));
                            auto& res = get_resource(iter->first);
                            if(res.type == ResourceType::transient)
                            {
                                // We do not modify input type to output, since they are not 
                                // managed by render graph.
                                res.type = ResourceType::output;
                            }
                            break;
                        }
                    }
                }
                // Resolve transient resource lifetime.
                for(auto& out_res : output_resources)
                {
                    auto& res = get_resource(out_res.first);
                    if(res.type == ResourceType::transient)
                    {
                        m_node_data[out_res.second.creator].m_create_resources.push_back(out_res.first);
                        m_node_data[out_res.second.last_access].m_release_resources.push_back(out_res.first);
                    }
                }
                // Create output resources.
                for(auto& out_res : m_output_resources)
                {
                    auto& res = get_resource(out_res.second);
                    if(res.type == ResourceType::output)
                    {
                        luset(res.resource, m_device->new_resource(res.resource_desc));
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
                for(usize i = 0; i < m_node_data.size(); ++i)
                {
                    auto& data = m_node_data[i];
                    // Allocates resources.
                    for(ResourceHandle h : data.m_create_resources)
                    {
                        auto& res = get_resource(h);
                        luset(res.resource, m_transient_heap->allocate(res.resource_desc));
                    }
                    m_current_pass = i;
                    MutexGuard guard(g_render_pass_types_mtx);
                    auto iter = g_render_pass_types.find(m_desc.nodes[i].type);
                    if(iter == g_render_pass_types.end())
                    {
                        return set_error(BasicError::not_found(), "Render pass type \"%s\" is not found.", m_desc.nodes[i].type.c_str());
                    }
                    luexp(iter->execute(this));
                    for(auto& res : m_temporary_resources)
                    {
                        m_transient_heap->release(res);
                    }
                    m_temporary_resources.clear();
                    // Release resources.
                    for(ResourceHandle h : data.m_release_resources)
                    {
                        auto& res = get_resource(h);
                        m_transient_heap->release(res.resource);
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