/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorGUI.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include "EditorGUI.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace EditorGUI
    {
        struct EditorGUIModule : Module
        {
            virtual const c8* get_name() override
            {
                return "EditorGUI";
            }

            virtual RV on_register() override
            {
                return add_dependency_modules(this,
                    { module_rhi(), module_vg(), module_font(), GUI::module_gui() });
            }

            virtual RV on_init() override
            {
                Meta::register_EditorGUI_types();
                return Internal::initialize_icons();
            }

            virtual void on_close() override
            {
                Internal::close_icons();
            }
        };

        LUNA_EDITOR_GUI_API Module* module_editor_gui()
        {
            static EditorGUIModule module;
            return &module;
        }
    }
}
