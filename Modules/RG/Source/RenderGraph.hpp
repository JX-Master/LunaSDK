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
#include "../TransientResourceHeap.hpp"

namespace Luna
{
    namespace RG
    {
        struct RenderGraph : IRenderGraph, IRenderPassContext
        {
            lustruct("RG::RenderGraph", "{feefd806-4b82-48cd-b350-f8fc9387fc65}");
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Ref<ITransientResourceHeap> m_transient_heap;
            RenderGraphDesc m_desc;
            HashMap<Name, RHI::ResourceDesc> m_input_resource_descs;

            enum class ResourceType : u8
            {
                transient = 0,
                input,
                output,
            };

            // Referred by nodes to attach inputs/outputs.
            struct Resource
            {
                ResourceType type;
                RHI::ResourceDesc resource_desc; // Null for input resource.
                Ref<RHI::IResource> resource;
            };
            Vector<Resource> m_resources;
            using ResourceHandle = usize;

            // Produced by compiling the render graph.
            struct NodeData
            {
                HashMap<Name, ResourceHandle> m_input_resources;
                HashMap<Name, ResourceHandle> m_output_resources;
                // The indices of `m_transient_resources` that should be released when this node is finished.
                Vector<ResourceHandle> m_create_resources;
                Vector<ResourceHandle> m_release_resources;
            };
            Vector<NodeData> m_node_data;
            HashMap<Name, ResourceHandle> m_input_resources;
            HashMap<Name, ResourceHandle> m_output_resources;

            // Execution context.
            Ref<RHI::ICommandBuffer> m_cmdbuf;
            Vector<Ref<RHI::IResource>> m_temporary_resources;
            usize m_current_pass;

            ResourceHandle new_resource_entry()
            {
                m_resources.emplace_back();
                return m_resources.size() - 1;
            }

            Resource& get_resource(ResourceHandle handle)
            {
                return m_resources[handle];
            }

            R<NodeData*> find_node(const Name& name, usize* index = nullptr)
            {
                for(usize i = 0; i < m_desc.nodes.size(); ++i)
                {
                    if(m_desc.nodes[i].name == name)
                    {
                        if(index) *index = i;
                        return m_node_data.data() + i;
                    }
                }
                return set_error(BasicError::not_found(), "Render pass \"%s\" cannot be found.", name.c_str());
            }

            void find_output_targets(const Name& node, const Name& output, Vector<Pair<Name, Name>>& result)
            {
                result.clear();
                for(auto& i : m_desc.connections)
                {
                    if(i.source_node == node && i.source_parameter == output)
                    {
                        result.push_back(make_pair(i.destination_node, i.destination_parameter));
                    }
                }
            }

            bool find_input_source(const Name& node, const Name& input, Name& output_node, Name& output_parameter)
            {
                for(auto& i : m_desc.connections)
                {
                    if(i.destination_node == node && i.destination_parameter == input)
                    {
                        output_node = i.source_node;
                        output_parameter = i.source_parameter;
                        return true;
                    }
                }
                return false;
            }

            virtual RHI::IDevice* get_device() override { return m_device.get(); }
            virtual const RenderGraphDesc& get_desc() override { return m_desc; }
            virtual void set_desc(const RenderGraphDesc& desc) override { m_desc = desc; }
            virtual void set_graph_input_resource_desc(const Name& parameter, const RHI::ResourceDesc* desc) override
            {
                if(desc) m_input_resource_descs.insert_or_assign(parameter, *desc);
                else m_input_resource_descs.erase(parameter);
            }
            virtual RV compile() override;
            virtual void set_graph_input_resource(const Name& parameter, RHI::IResource* resource) override
            {
                auto iter = m_input_resources.find(parameter);
                if(iter != m_input_resources.end())
                {
                    auto h = iter->second;
                    auto& res = get_resource(h);
                    res.resource = resource;
                }
            }
            virtual RV execute(RHI::ICommandBuffer* cmdbuf) override;
            virtual RHI::IResource* get_graph_output_resource(const Name& parameter) override
            {
                auto iter = m_output_resources.find(parameter);
                if(iter == m_output_resources.end()) return nullptr;
                auto h = iter->second;
                auto& res = get_resource(h);
                return res.resource;
            }

            virtual RHI::ICommandBuffer* get_command_buffer() override { return m_cmdbuf; }
            virtual Ref<RHI::IResource> get_input(const Name& name) override
            {
                auto& data = m_node_data[m_current_pass];
                auto iter = data.m_input_resources.find(name);
                if(iter == data.m_input_resources.end()) return nullptr;
                auto h = iter->second;
                auto& res = get_resource(h);
                return res.resource;
            }
            virtual Ref<RHI::IResource> get_output(const Name& name) override
            {
                auto& data = m_node_data[m_current_pass];
                auto iter = data.m_output_resources.find(name);
                if(iter == data.m_output_resources.end()) return nullptr;
                auto h = iter->second;
                auto& res = get_resource(h);
                return res.resource;
            }
            virtual R<Ref<RHI::IResource>> allocate_temporary_resource(const RHI::ResourceDesc& desc) override;
            virtual void release_temporary_resource(RHI::IResource* res) override;
        };
    }
}