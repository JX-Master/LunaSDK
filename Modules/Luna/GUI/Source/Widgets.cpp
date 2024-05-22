/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.cpp
* @author JXMaster
* @date 2024/3/30
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Widgets.hpp"
#include "../WidgetList.hpp"
#include "../Widgets/Rectangle.hpp"
#include "../Widgets/Text.hpp"
#include "../Widgets/ResizableWindow.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void begin_resizable_window(IWidgetList* list)
        {
            Ref<ResizableWindow> widget = new_object<ResizableWindow>();
            list->add_widget(widget);
            list->push_widget(widget);
        }
        LUNA_GUI_API void begin_rectangle(IWidgetList* list)
        {
            Ref<Rectangle> widget = new_object<Rectangle>();
            list->add_widget(widget);
            list->push_widget(widget);
        }
        LUNA_GUI_API void end(IWidgetList* list)
        {
            list->pop_widget();
        }
        LUNA_GUI_API void set_anthor(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(list, VATTR_ANTHOR, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void set_offset(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(list, VATTR_OFFSET, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void text(IWidgetList* list, const Name& text)
        {
            Ref<Text> widget = new_object<Text>();
            list->add_widget(widget);
            set_tattr(list, TATTR_TEXT, text);
        }
        LUNA_GUI_API void set_sattr(IWidgetList* list, u32 kind, f32 value)
        {
            Widget* widget = list->get_current_widget();
            widget->sattrs.insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_vattr(IWidgetList* list, u32 kind, const Float4U& value)
        {
            Widget* widget = list->get_current_widget();
            widget->vattrs.insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_tattr(IWidgetList* list, u32 kind, const Name& value)
        {
            Widget* widget = list->get_current_widget();
            widget->tattrs.insert_or_assign(kind, value);
        }
    }
}