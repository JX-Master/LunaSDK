/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RG.cpp
* @author JXMaster
* @date 2023/3/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RG_API LUNA_EXPORT
#include "RenderPass.hpp"
#include <Luna/Runtime/Module.hpp>
#include "RenderGraphImpl.hpp"
#include "RG.meta.generated.hpp"
#include "../RG.hpp"
namespace Luna
{
    namespace RG
    {
        struct RGModule : public Module
        {
            virtual const c8* get_name() override { return "RG"; }
            virtual RV on_register() override
            {
                return add_dependency_module(this, module_rhi());
            }
            virtual RV on_init() override
            {
                Meta::register_RG_types();
                g_render_pass_types_mtx = new_mutex();
                return ok;
            }
            virtual void on_close() override
            {
                g_render_pass_types.clear();
                g_render_pass_types.shrink_to_fit();
                g_render_pass_types_mtx.reset();
            }
        };
    }
    LUNA_RG_API Module* module_rg()
    {
        static RG::RGModule m;
        return &m;
    }
}
