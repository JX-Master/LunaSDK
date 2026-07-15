/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Base.hpp"
#include "GUILegacy.meta.generated.hpp"
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace GUI
    {
        struct ModuleImpl : public Module
        {
            virtual const c8* get_name() override { return "GUILegacy"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_vg(), module_font(), GUICore::module_gui_core()});
            }
            virtual RV on_init() override
            {
                Meta::register_GUILegacy_types();
                return ok;
            }
            virtual void on_close() override {}
        };
    }

    namespace GUI
    {
        LUNA_GUI_API Module* module_gui()
        {
            static ModuleImpl m;
            return &m;
        }
    }
}
