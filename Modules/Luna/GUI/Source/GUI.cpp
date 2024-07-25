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
#include "Context.hpp"
#include <Luna/Font/Font.hpp>
#include "../Widgets/Rectangle.hpp"
#include "../Widgets/Text.hpp"
#include "../Widgets/Canvas.hpp"
#include "../Widgets/HorizontalLayout.hpp"
#include "../Widgets/VerticalLayout.hpp"
#include "DrawList.hpp"
#include "WidgetBuilder.hpp"
#include "../Event.hpp"

namespace Luna
{
    namespace GUI
    {
        struct GUIModule : public Module
        {
            virtual const c8* get_name() override { return "GUI"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_vg(), module_font()});
            }
            virtual RV on_init() override
            {
                register_boxed_type<Context>();
                impl_interface_for_type<Context, IContext>();
                register_boxed_type<Widget>();
                impl_interface_for_type<Widget, IWidget>();
                register_struct_type<Rectangle>({}, typeof<Widget>());
                impl_interface_for_type<Rectangle, IWidget>();
                register_struct_type<Text>({}, typeof<Widget>());
                impl_interface_for_type<Text, IWidget>();
                register_struct_type<Canvas>({}, typeof<Widget>());
                impl_interface_for_type<Canvas, IContainer, IWidget>();
                register_struct_type<HorizontalLayout>({}, typeof<Widget>());
                impl_interface_for_type<HorizontalLayout, IContainer, IWidget>();
                register_struct_type<VerticalLayout>({}, typeof<Widget>());
                impl_interface_for_type<VerticalLayout, IContainer, IWidget>();
                register_boxed_type<DrawList>();
                impl_interface_for_type<DrawList, IDrawList>();
                register_boxed_type<WidgetBuilder>();
                impl_interface_for_type<WidgetBuilder, IWidgetBuilder>();
                register_struct_type<RootWidget>({}, typeof<Widget>());
                impl_interface_for_type<RootWidget, IContainer, IWidget>();
                register_struct_type<MouseEvent>({});
                register_struct_type<MouseMoveEvent>({}, typeof<MouseEvent>());
                register_struct_type<MouseButtonEvent>({}, typeof<MouseEvent>());
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