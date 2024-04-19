/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.cpp
* @author JXMaster
* @date 2024/3/30
*/
#include "../Widgets.hpp"
#include "../WidgetList.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void begin_widget(IWidgetList* list)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::widget, 0, 0, 0));
            cmds.push_back(make_command(OpCode::begin, 0, 0, 0));
        }
        LUNA_GUI_API void end(IWidgetList* list)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::end, 0, 0, 0));
        }
        LUNA_GUI_API void set_anthor(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom, Condition condition, RectComponent components)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::anchor, (u8)condition, (u8)components, 0));
            if(test_flags(components, RectComponent::left))
            {
                cmds.push_back(ftou32(left));
            }
            if(test_flags(components, RectComponent::top))
            {
                cmds.push_back(ftou32(top));
            }
            if(test_flags(components, RectComponent::right))
            {
                cmds.push_back(ftou32(right));
            }
            if(test_flags(components, RectComponent::bottom))
            {
                cmds.push_back(ftou32(bottom));
            }
        }
        LUNA_GUI_API void set_rect(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom, Condition condition, RectComponent components)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::rect, (u8)condition, (u8)components, 0));
            if(test_flags(components, RectComponent::left))
            {
                cmds.push_back(ftou32(left));
            }
            if(test_flags(components, RectComponent::top))
            {
                cmds.push_back(ftou32(top));
            }
            if(test_flags(components, RectComponent::right))
            {
                cmds.push_back(ftou32(right));
            }
            if(test_flags(components, RectComponent::bottom))
            {
                cmds.push_back(ftou32(bottom));
            }
        }
        LUNA_GUI_API void set_color(IWidgetList* list, ColorType color_type, u32 color, Condition condition)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::color, (u8)condition, (u8)color_type, 0));
            cmds.push_back(color);
        }
        LUNA_GUI_API void text(IWidgetList* list, const Name& text)
        {
            u32 i = list->add_text(text);
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::text, i));
        }
        LUNA_GUI_API void set_style(IWidgetList* list, StyleType type, f32 value, Condition condition)
        {
            auto& cmds = list->get_widget_buffer();
            cmds.push_back(make_command(OpCode::style, (u8)condition, (u8)type, 0));
            cmds.push_back(ftou32(value));
        }
    }
}