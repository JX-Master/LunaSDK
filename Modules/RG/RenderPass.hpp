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
        struct RenderPassInputInfo
        {
            HashMap<Name, RHI::ResourceDesc> input_parameters;
        };

        enum class RenderPassOutputParameterType : u8
        {
            //! The resource is an resource allocated by the render graph before this pass is executed.
            default = 0,
            //! The resource is the input resource of the specified input parameter.
            input = 1
        };

        struct RenderPassOutputParameterInfo
        {
            RenderPassOutputParameterType type;
            //! If `type` is `input`, specify the input name used. Otherwise, this is ignored.
            Name input_name;
            //! If `type` is `default`, specify the resource you want the render graph to create.
            RHI::ResourceDesc resource_desc;
        };

        struct RenderPassOutputInfo
        {
            HashMap<Name, RenderPassOutputParameterInfo> output_parameters;
        };

        struct IRenderPassContext : virtual Interface
        {
            luiid("{04ab587d-1e50-4816-89e6-6ff676d30bbf}");

            virtual RHI::ICommandBuffer* get_command_buffer() = 0;

            virtual Ref<RHI::IResource> get_input(const Name& name) = 0;

            virtual Ref<RHI::IResource> get_output(const Name& name) = 0;

            //! Allocates new temporary resource that exists only in the current pass.
            //! The resource will be released when the pass is finished, or the user can 
            //! release it manually using `release_temporary_resource`.
            virtual R<Ref<RHI::IResource>> allocate_temporary_resource(const RHI::ResourceDesc& desc) = 0;

            virtual void release_temporary_resource(RHI::IResource* res) = 0;
        };

        using render_pass_compile_func_t = R<RenderPassOutputInfo>(object_t userdata, const RenderPassInputInfo& input);
        using render_pass_execute_func_t = RV(IRenderPassContext* ctx);

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
            //! The resource that is used as both input and output of the node.
            Vector<RenderPassTypeParameter> inout_parameters;
            render_pass_compile_func_t* compile;
            render_pass_execute_func_t* execute;
            ObjRef userdata;
        };

        LUNA_RG_API void register_render_pass_type(const RenderPassTypeDesc& desc);

        LUNA_RG_API void get_render_pass_types(Vector<Name>& out_render_pass_types);

        LUNA_RG_API R<RenderPassTypeDesc> get_render_pass_type_desc(const Name& render_pass);
    }
}