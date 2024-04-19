/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file GUI.cpp
* @author JXMaster
* @date 2024/3/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../GUI.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/VG/VG.hpp>
#include "WidgetList.hpp"
#include "Context.hpp"
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace GUI
    {
        struct GUIModule : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                register_boxed_type<WidgetList>();
                impl_interface_for_type<WidgetList, IWidgetList>();
                register_boxed_type<Context>();
                impl_interface_for_type<Context, IContext>();
                return add_dependency_modules(this, {module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                return ok;
            }
            virtual void on_close() override
            {
            }
        };
    }
    LUNA_GUI_API Module* module_gui()
    {
        static GUI::GUIModule m;
        return &m;
    }
}