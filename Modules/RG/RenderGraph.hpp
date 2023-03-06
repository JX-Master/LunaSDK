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
#include "RenderPass.hpp"

namespace Luna
{
    namespace RG
    {
        struct RenderGraphNode
        {
            lustruct("RG::RenderGraphNode", "{f466d136-bc5d-45a9-bb46-0316f182f9f8}");
            Name name; // The node name.
            Name type; // The name of the render pass.
        };

        struct RenderGraphConnection
        {
            lustruct("RG::RenderGraphConnection", "{1af7f40a-77b3-404d-bc48-d8086f2d5fa3}");
            Name source_node;
            Name source_parameter;
            Name destination_node;
            Name destination_parameter;
        };

        struct RenderGraphParameter
        {
            lustruct("RG::RenderGraphParameter", "{09fa206b-4f57-4e69-af90-3d3a5380f0ed}");
            Name name;
            Name exported_node;
            Name exported_parameter;
        };

        //! Describes one render graph.
        struct RenderGraphDesc
        {
            lustruct("RG::RenderGraphDesc", "{01bc2d53-606a-41d5-bd90-1557912de524}");
            Vector<RenderGraphNode> nodes;
            Vector<RenderGraphConnection> connections;
            Vector<RenderGraphParameter> input_parameters;
            Vector<RenderGraphParameter> output_parameters;
        };

        struct IRenderGraph : virtual RHI::IDeviceChild
        {
            luiid("{ad007d31-b655-4276-8b11-db09a93db278}");

            virtual const RenderGraphDesc& get_desc() = 0;

            virtual void set_desc(const RenderGraphDesc& desc) = 0;

            virtual void set_graph_input_resource_desc(const Name& parameter, const RHI::ResourceDesc* desc) = 0;

            virtual RV compile() = 0;

            virtual void set_graph_input_resource(const Name& parameter, RHI::IResource* resource) = 0;

            virtual RV execute(RHI::ICommandBuffer* cmdbuf) = 0;

            virtual RHI::IResource* get_graph_output_resource(const Name& parameter) = 0;
        };

        LUNA_RG_API Ref<IRenderGraph> new_render_graph(RHI::IDevice* device);
    }
}