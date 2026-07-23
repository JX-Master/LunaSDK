/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include "GUI.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace GUI
    {
        struct GUIModule : Module
        {
            virtual const c8* get_name() override
            {
                return "GUI";
            }

            virtual RV on_register() override
            {
                return add_dependency_modules(this,
                    { module_rhi(), module_vg(), module_font(), GUICore::module_gui_core() });
            }

            virtual RV on_init() override
            {
                Meta::register_GUI_types();
                return Internal::initialize_icons();
            }

            virtual void on_close() override
            {
                Internal::close_icons();
            }
        };

        LUNA_GUI_API Module* module_gui()
        {
            static GUIModule module;
            return &module;
        }
    }
}
