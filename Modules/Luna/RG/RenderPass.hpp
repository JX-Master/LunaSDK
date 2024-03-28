/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderPass.hpp
* @author JXMaster
* @date 2023/3/6
*/
#pragma once
#include <Luna/RHI/RHI.hpp>

#ifndef LUNA_RG_API
#define LUNA_RG_API
#endif

namespace Luna
{
    namespace RG
    {
        //! @addtogroup RG
        //! @{

        //! Specifies one resource type in render graph.
        enum class ResourceType : u8
        {
            //! The resource is a buffer.
            buffer = 0,
            //! The resource is a texture.
            texture = 1,
        };

        //! Describes one render graph resource.
        struct ResourceDesc
        {
            //! The resource type.
            ResourceType type;
            //! The resource memory type.
            RHI::MemoryType memory_type;
            union
            {
                //! Describes the resource if `type` is @ref ResourceType::buffer.
                RHI::BufferDesc buffer;
                //! Describes the resource if `type` is @ref ResourceType::texture.
                RHI::TextureDesc texture;
            };
            //! Creates one resource descriptor for one buffer resource.
            //! @param[in] memory_type The memory type of the resource.
            //! @param[in] desc The buffer resource descriptor.
            //! @return Returns the created resource descriptor.
            static ResourceDesc as_buffer(RHI::MemoryType memory_type, const RHI::BufferDesc& desc)
            {
                ResourceDesc ret;
                ret.type = ResourceType::buffer;
                ret.memory_type = memory_type;
                ret.buffer = desc;
                return ret;
            }
            //! Creates one resource descriptor for one texture resource.
            //! @param[in] memory_type The memory type of the resource.
            //! @param[in] desc The texture resource descriptor.
            //! @return Returns the created resource descriptor.
            static ResourceDesc as_texture(RHI::MemoryType memory_type, const RHI::TextureDesc& desc)
            {
                ResourceDesc ret;
                ret.type = ResourceType::texture;
                ret.memory_type = memory_type;
                ret.texture = desc;
                return ret;
            }
        };

        //! @interface IRenderPassContext
        //! Represents one render graph execution context. This is used only as the parameter of @ref IRenderPass::execute.
        struct IRenderPassContext : virtual Interface
        {
            luiid("{04ab587d-1e50-4816-89e6-6ff676d30bbf}");

            //! Gets the command buffer used to record commands for the render pass.
            //! @return Returns the command buffer.
            virtual RHI::ICommandBuffer* get_command_buffer() = 0;

            //! Gets the input resource of the render pass.
            //! @param[in] name The name of the input resource.
            //! @return Returns the fetched input resource. Returns `nullptr` if not found.
            virtual RHI::IResource* get_input(const Name& name) = 0;

            //! Gets the output resource of the render pass.
            //! @param[in] name The name of the output resource.
            //! @return Returns the fetched output resource. Returns `nullptr` if not found.
            virtual RHI::IResource* get_output(const Name& name) = 0;
            
            //! Gets the timestamp query heap used to track the running time of the render pass.
            //! @param[out] begin_index If not `nullptr`, returns the query heap index to write the beginning timestamp of the render pass.
            //! @param[out] end_index If not `nullptr`, returns the query heap index to write the ending timestamp of the render pass.
            //! @return Returns the timestamp query heap used to track the running time of the render pass. Returns `nullptr` if timestamp query
            //! is not used in this render graph ( @ref RenderGraphCompileConfig::enable_time_profiling is `false`).
            virtual RHI::IQueryHeap* get_timestamp_query_heap(u32* begin_index, u32* end_index) = 0;

            //! Allocates new temporary resource that exists only in the current pass.
            //! @details The allocated The resource will be released when the pass is finished, or the user can 
            //! release it manually using @ref release_temporary_resource.
            virtual R<Ref<RHI::IResource>> allocate_temporary_resource(const ResourceDesc& desc) = 0;

            //! Releases the temporary resource allocated from @ref allocate_temporary_resource.
            //! @details This call will make the resource immediately reusable for other temporary resource allocation calls, so that
            //! another call to @ref allocate_temporary_resource in the same render pass may reuse the released resource, thus reduces
            //! memory comsumption. If the resource is not released by this call, it will be released at the end of the render pass.
            //! @param[in] res The resource to release.
            virtual void release_temporary_resource(RHI::IResource* res) = 0;
        };

        //! @interface IRenderPass
        //! Represents one user-implemented render pass.
        struct IRenderPass : virtual Interface
        {
            luiid("{e8392032-e97e-4557-a40a-a5e22f5d0f2f}");

            //! Executes the render pass. This is called by the render graph in @ref IRenderGraph::execute, the user
            //! should not call this manually.
            //! @param[in] ctx The render graph execution context that can be used by the render pass to read input / output resources, 
            //! get command buffers, allocate temporary resources and so on.
            virtual RV execute(IRenderPassContext* ctx) = 0;
        };

        //! A specifal value that identifies one invalid resource name in @ref IRenderGraphCompiler.
        constexpr usize INVALID_RESOURCE = (usize)-1;

        //! @interface IRenderGraphCompiler
        //! The render graph compile context. This is used only as the parameter for @ref render_pass_compile_func_t.
        struct IRenderGraphCompiler : virtual Interface
        {
            luiid("{158df588-6b27-4438-ba8a-8913cebacaca}");

            //! Gets the resource ID of one input resource.
            //! @param[in] name The name of the input resource.
            //! @return Returns the resource ID of the resource. Returns @ref INVALID_RESOURCE if the name is not specified as one input resource.
            virtual usize get_input_resource(const Name& name) = 0;
            //! Gets the resource ID of one output resource.
            //! @param[in] name The name of the output resource.
            //! @return Returns the resource ID of the resource. Returns @ref INVALID_RESOURCE if the name is not specified as one output resource.
            virtual usize get_output_resource(const Name& name) = 0;
            //! Gets the resource descriptor of the specified resource.
            //! @param[in] resource The resource id returned by @ref get_input_resource or @ref get_output_resource.
            //! @return Returns the resource descriptor of the specified resource.
            //! @par Valid Usage
            //! * `resource` must specify one valid resource.
            virtual ResourceDesc get_resource_desc(usize resource) = 0;
            //! Sets the resource descriptor of the specified resource.
            //! @param[in] resource The resource id returned by @ref get_input_resource or @ref get_output_resource.
            //! @param[in] desc The resource descriptor to set for the resource.
            //! @par Valid Usage
            //! * `resource` must specify one valid resource.
            virtual void set_resource_desc(usize resource, const ResourceDesc& desc) = 0;
            //! Sets the render pass object used for executing render commands for this render pass.
            //! @details The render pass object must be set before @ref render_pass_compile_func_t returns, or the compilation fails.
            //! @param[in] render_pass The render pass to set.
            //! The render graph will keep a strong reference to this object until the render graph is destructed or recompiled.
            virtual void set_render_pass_object(IRenderPass* render_pass) = 0;
        };

        //! The function called by the render graph to produce the render pass object using input and output resources.
        //! @param[in] userdata The userdata specified in @ref RenderPassTypeDesc.
        //! @param[in] compiler The render graph compilation context.
        using render_pass_compile_func_t = RV(object_t userdata, IRenderGraphCompiler* compiler);

        //! Describes one parameter of one render pass.
        struct RenderPassTypeParameter
        {
            //! The name of the parameter.
            Name name;
            //! A short description of the parameter, this is used only for visualizing and debugging purpose.
            Name desc;
        };
        
        //! Describes one render pass type
        struct RenderPassTypeDesc
        {
            // The name of the render pass.
            Name name;
            // A brief description of the render pass. This is used only for visualizing and debugging purpose.
            Name desc;
            //! The resource that is used as inputs of the node.
            Vector<RenderPassTypeParameter> input_parameters;
            //! The resource that is used as outputs of the node.
            Vector<RenderPassTypeParameter> output_parameters;
            //! The render pass compile callback function.
            render_pass_compile_func_t* compile;
            //! The optional userdata that will be passed to the compile callback function.
            //! The system keeps a strong reference to this object until module shutdown.
            ObjRef userdata;
        };

        //! Registers one new render pass type.
        //! @param[in] desc The render pass type descriptor.
        LUNA_RG_API void register_render_pass_type(const RenderPassTypeDesc& desc);

        //! Gets a list of render pass types registered to the system.
        //! @param[out] out_render_pass_types The vector used to receive render pass types.
        LUNA_RG_API void get_render_pass_types(Vector<Name>& out_render_pass_types);

        //! Gets the descriptor of the specified render pass.
        //! @param[in] render_pass The name of the render pass to fetch.
        //! @return Returns the fetched render pass descriptor.
        LUNA_RG_API R<RenderPassTypeDesc> get_render_pass_type_desc(const Name& render_pass);

        //! @}
    }
}
