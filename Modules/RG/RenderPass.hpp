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
#include <RHI/RHI.hpp>

#ifndef LUNA_RG_API
#define LUNA_RG_API
#endif

namespace Luna
{
    namespace RG
    {
        struct IRenderPassContext : virtual Interface
        {
            luiid("{04ab587d-1e50-4816-89e6-6ff676d30bbf}");

            virtual RHI::ICommandBuffer* get_command_buffer() = 0;

            virtual RHI::IResource* get_input(const Name& name) = 0;

            virtual RHI::IResource* get_output(const Name& name) = 0;

            //! Allocates new temporary resource that exists only in the current pass.
            //! The resource will be released when the pass is finished, or the user can 
            //! release it manually using `release_temporary_resource`.
            virtual R<Ref<RHI::IResource>> allocate_temporary_resource(const RHI::ResourceDesc& desc) = 0;

            virtual void release_temporary_resource(RHI::IResource* res) = 0;
        };

        struct IRenderPass : virtual Interface
        {
            luiid("{e8392032-e97e-4557-a40a-a5e22f5d0f2f}");

            virtual RV execute(IRenderPassContext* ctx) = 0;
        };

        constexpr usize INVALID_RESOURCE = (usize)-1;

        struct IRenderGraphCompiler : virtual Interface
        {
            luiid("{158df588-6b27-4438-ba8a-8913cebacaca}");

            virtual usize get_input_resource(const Name& parameter) = 0;
            virtual usize get_output_resource(const Name& parameter) = 0;

            virtual RHI::ResourceDesc get_resource_desc(usize resource) = 0;
            virtual void set_resource_desc(usize resource, const RHI::ResourceDesc& desc) = 0;

            virtual void set_render_pass_object(IRenderPass* render_pass) = 0;
        };

        using render_pass_compile_func_t = RV(object_t userdata, IRenderGraphCompiler* compiler);

        struct RenderPassTypeParameter
        {
            Name name;
            Name desc;
        };
        
        struct RenderPassTypeDesc
        {
            Name name;  // The name of the render pass.
            Name desc;  // A brief description of the render pass.
            //! The resource that is used as inputs of the node.
            Vector<RenderPassTypeParameter> input_parameters;
            //! The resource that is used as outputs of the node.
            Vector<RenderPassTypeParameter> output_parameters;
            render_pass_compile_func_t* compile;
            ObjRef userdata;
        };

        LUNA_RG_API void register_render_pass_type(const RenderPassTypeDesc& desc);

        LUNA_RG_API void get_render_pass_types(Vector<Name>& out_render_pass_types);

        LUNA_RG_API R<RenderPassTypeDesc> get_render_pass_type_desc(const Name& render_pass);
    }
}