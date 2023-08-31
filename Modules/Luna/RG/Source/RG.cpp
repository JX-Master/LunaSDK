/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RG.cpp
* @author JXMaster
* @date 2023/3/6
*/
#include "RenderPass.hpp"
#include <Luna/Runtime/Module.hpp>
#include "RenderGraph.hpp"
namespace Luna
{
    namespace RG
    {
        RV init()
        {
            register_boxed_type<RenderGraph>();
            impl_interface_for_type<RenderGraph, IRenderGraph, IRenderPassContext, IRenderGraphCompiler>();
            g_render_pass_types_mtx = new_mutex();
            return ok;
        }
        void close()
        {
            g_render_pass_types.clear();
            g_render_pass_types.shrink_to_fit();
            g_render_pass_types_mtx.reset();
        }
        LUNA_STATIC_REGISTER_MODULE(RG, "RHI", init, close);
    }
}