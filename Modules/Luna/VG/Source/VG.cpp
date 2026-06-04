/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VG.cpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "FontAtlasImpl.hpp"
#include "ShapeDrawListImpl.hpp"
#include "ShapeRendererImpl.hpp"
#include "ShapeBufferImpl.hpp"
#include "VG.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>

namespace Luna
{
    namespace VG
    {
        struct VGModule : public Module
        {
            virtual const c8* get_name() override { return "VG"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_rhi_utility()});
            }
            virtual RV on_init() override
            {
                Meta::register_VG_types();
                return init_render_resources();
            }
            virtual void on_close() override
            {
                deinit_render_resources();
            }
        };
    }

    LUNA_VG_API Module* module_vg()
    {
        static VG::VGModule m;
        return &m;
    }
}
