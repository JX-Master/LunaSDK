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
        //! @addtogroup RG
        //! @{
        
        //! Describes one render pass node in one render graph.
        struct RenderGraphPassNode
        {
            //! The render pass node name.
            Name name;
            //! The render pass type.
            Name type;
        };

        //! Specifies the residency type of one resource in one render graph.
        enum class RenderGraphResourceType : u8
        {
            //! This resource is used to hold temporal data during the render graph execution.
            //! The render graph allocates this resource at first access, and releases the resource after last 
            //! access.
            transient = 0,
            //! This resource is persistent. Such resources are used to hold data between render graph executions.
            //! The render graph allocates this resource when the graph is being compiled, 
            //! and does not release it after the render graph execution is finished.
            //! 
            //! This resource will be released when the render graph is destructed or recompiled.
            persistent = 1,
            //! This resource is imported to the render graph. 
            //! The render graph does not manage the resource lifetime.
            external = 2,
        };

        //! Specifies flags of one resource in one render graph.
        enum class RenderGraphResourceFlag : u8
        {
            none = 0x00,
            //! This resource is the output of the render graph.
            //! This is used to determine passes that should be retained during culling when the render graph
            //! is being compiled.
            output = 0x01,
        };

        //! Describes one resource node in one render graph.
        struct RenderGraphResourceNode
        {
            //! The resource type.
            RenderGraphResourceType type;
            //! The resource flags.
            RenderGraphResourceFlag flags;
            //! The resource node name.
            Name name;
            //! The resource descriptor.
            ResourceDesc desc;
        };

        //! Describes one input or output connection between the resource node and the render pass node in one render graph.
        struct RenderGraphConnection
        {
            //! The index of the connected render pass node in @ref RenderGraphDesc::passes.
            usize pass;
            //! The name of the render pass parameter to bind the resource to.
            Name parameter;
            //! The index of the connected resource node in @ref RenderGraphDesc::resources.
            usize resource;
        };

        //! Describes one render graph.
        struct RenderGraphDesc
        {
            //! The array of passed in this render graph.
            //! The order of elements this array will be the execution order of render passes.
            Vector<RenderGraphPassNode> passes;
            //! The array of resources in this render graph.
            Vector<RenderGraphResourceNode> resources;
            //! The array of input connections (resource -> pass).
            Vector<RenderGraphConnection> input_connections;
            //! The array of output connection (pass -> resource).
            Vector<RenderGraphConnection> output_connections;
        };

        //! Describes render graph compiling configurations.
        struct RenderGraphCompileConfig
        {
            //! Whether to enable render pass time profiling. If this is `true`, the render graph creates one query heap
            //! that can be used by the render pass object to record start and end time of the render pass.
            bool enable_time_profiling = false;
        };

        //! @interface IRenderGraph
        //! Represents one render graph that can be used to schedule render passes and reuse in-frame 
        //! transient render resources to reduce memory comsumption. 
        struct IRenderGraph : virtual Interface
        {
            luiid("{ad007d31-b655-4276-8b11-db09a93db278}");

            //! Gets the RHI device attached to this render graph.
            //! @return Returns the RHI device attached to this render graph.
            virtual RHI::IDevice* get_device() = 0;

            //! Gets the descriptor of this render graph.
            //! @return Returns the descriptor of this render graph.
            virtual const RenderGraphDesc& get_desc() = 0;

            //! Sets the descriptor of this render graph.
            //! @details The user should call @ref compile after setting the descriptor in order
            //! to make the new descriptor take effect.
            //! @param[in] desc The descriptor to set.
            virtual void set_desc(const RenderGraphDesc& desc) = 0;

            //! Compiles the render graph.
            //! @details This function does the following steps in order:
            //! 1. Set up internal render pass and resource data.
            //! 2. Cull out unused passes and resources by scanning all dependency render passes and resources of
            //! output resources.
            //! 3. Determines the lifetime of every transient resource.
            //! 4. Initialize resource descriptors using user-defined descriptors.
            //! 5. Calls the compile callback of every render pass in execution order to get render pass objects.
            //! 6. Create persistent resources.
            //! 7. Create time query heap if needed.
            //! @param[in] config The compilation configuration.
            virtual RV compile(const RenderGraphCompileConfig& config) = 0;

            //! Gets all enabled render passes.
            //! @details This should be called after @ref compile, or no render pass will be returned.
            //! @param[out] render_passes Returns the array of indices of enabled render passes.
            virtual void get_enabled_render_passes(Vector<usize>& render_passes) = 0;

            //! Gets the render pass object of the specified render pass.
            //! @param[in] index The index of the render pass to get.
            //! @return Returns the render pass object of the specified render pass.
            //! Returns `nullptr` if the index is not valid, or @ref compile is not called.
            virtual IRenderPass* get_render_pass(usize index) = 0;

            //! Sets external resource.
            //! @details Since external resources are not allocated and managed by the render graph, they must
            //! be set explicitly before executing the render graph.
            //! @param[in] index The index of the resource to set.
            //! @param[in] resource The resource to set.
            //! @par Valid Usage
            //! * `index` must specify one resource with @ref RenderGraphResourceType::external.
            virtual void set_external_resource(usize index, RHI::IResource* resource) = 0;

            //! Executes the render graph.
            //! @details This will execute all enabled render passes in order.
            //! @param[in] cmdbuf The command buffer used to record render commands of this render graph.
            //! @par Valid Usage
            //! * @ref compile must be called before calling this function.
            virtual RV execute(RHI::ICommandBuffer* cmdbuf) = 0;

            //! Gets one persistent resource.
            //! @param[in] index The index of the resource to get.
            //! @return Returns the persistent resource of the specified index. Returns `nullptr` if the index
            //! is not valid or does not specify one persistent resource.
            virtual RHI::IResource* get_persistent_resource(usize index) = 0;

            //! Gets the time used for every active render pass.
            //! @param[out] pass_times The array to receive the elapsed time for every active render pass.
            //! The size of this array is the same as the size of the array returned by @ref get_enabled_render_passes, and 
            //! every time span in this array maps to the corresponding render pass returned by @ref get_enabled_render_passes.
            //! The time is measured in GPU ticks, and can be converted to seconds by dividing with @ref RHI::IDevice::get_command_queue_timestamp_frequency.
            virtual RV get_pass_time_intervals(Vector<u64>& pass_times) = 0;
        };

        //! Creates one new render graph.
        //! @param[in] device The RHI device to bind to the render graph.
        //! @return Returns the created render graph.
        LUNA_RG_API Ref<IRenderGraph> new_render_graph(RHI::IDevice* device);

        //! @}
    }
}