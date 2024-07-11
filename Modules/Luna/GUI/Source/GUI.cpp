/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.cpp
* @author JXMaster
* @date 2024/6/28
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/Runtime/Module.hpp>
#include "Context.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/VG/VG.hpp>
#include "../GUI.hpp"
namespace Luna
{
    namespace GUI
    {
        struct GUIModule : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_font(), module_vg()});
            }
            virtual RV on_init() override
            {
                register_boxed_type<Context>();
                impl_interface_for_type<Context, IContext>();
                return ok;
            }
        };
    }
    LUNA_GUI_API Module* module_gui()
    {
        static GUI::GUIModule m;
        return &m;
    }
}