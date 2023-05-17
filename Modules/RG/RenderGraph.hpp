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
        struct RenderGraphPassNode
        {
            Name name; // The pass name.
            Name type; // The type name of the pass.
        };

        enum class RenderGraphResourceType : u8
        {
            //! This resource is used to hold temporal data during the render graph execution.
            //! The render graph allocates this resource at first access, and releases the resource after last 
            //! access.
            transient = 0,
            //! This resource is persistent. Such resources are used to hold data between render graph executions.
            //! The render graph allocates this resource when the graph is being compiled, 
            //! and does not release it after the render graph is finished.
            persistent = 1,
            //! This resource is imported to the render graph. 
            //! The render graph does not manage the resource lifetime.
            external = 2,
        };

        enum class RenderGraphResourceFlag : u8
        {
            none = 0x00,
            //! This resource is the output of the render graph.
            //! This is used to determine passes that should be retained during culling when the render graph
            //! is being compiled.
            output = 0x01,
        };

        struct RenderGraphResourceNode
        {
            RenderGraphResourceType type;
            RenderGraphResourceFlag flags;
            Name name; // The name of the resource.
            ResourceDesc desc;
        };

        struct RenderGraphConnection
        {
            usize pass;
            Name parameter;
            usize resource;
        };

        //! Describes one render graph.
        struct RenderGraphDesc
        {
            Vector<RenderGraphPassNode> passes;
            Vector<RenderGraphResourceNode> resources;
            Vector<RenderGraphConnection> input_connections;
            Vector<RenderGraphConnection> output_connections;
        };

        struct RenderGraphCompileConfig
        {
            bool enable_time_profiling = false;
        };

        struct IRenderGraph : virtual Interface
        {
            luiid("{ad007d31-b655-4276-8b11-db09a93db278}");

            virtual RHI::IDevice* get_device() = 0;

            virtual const RenderGraphDesc& get_desc() = 0;

            virtual void set_desc(const RenderGraphDesc& desc) = 0;

            virtual RV compile(const RenderGraphCompileConfig& config) = 0;

            virtual void get_enabled_render_passes(Vector<usize>& render_passes) = 0;

            virtual IRenderPass* get_render_pass(usize index) = 0;

            virtual void set_external_resource(usize index, RHI::IResource* resource) = 0;

            virtual RV execute(RHI::ICommandBuffer* cmdbuf) = 0;

            virtual RHI::IResource* get_persistent_resource(usize index) = 0;

            virtual RV get_pass_time_intervals(Vector<u64>& pass_times) = 0;
        };

        LUNA_RG_API Ref<IRenderGraph> new_render_graph(RHI::IDevice* device);
    }
}