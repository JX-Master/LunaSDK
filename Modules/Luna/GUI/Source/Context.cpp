/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.cpp
* @author JXMaster
* @date 2024/3/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Context.hpp"
#include "WidgetList.hpp"
#include "../Widgets.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/Shapes.hpp>

namespace Luna
{
    namespace GUI
    {
        inline constexpr OpCode get_opcode(u32 command)
        {
            return (OpCode)(command & 0xFF);
        }

        inline constexpr u8 get_a(u32 command)
        {
            return (u8)((command >> 8) & 0xFF);
        }

        inline constexpr u8 get_b(u32 command)
        {
            return (u8)((command >> 16) & 0xFF);
        }

        inline constexpr u8 get_c(u32 command)
        {
            return (u8)((command >> 24) & 0xFF);
        }

        inline constexpr u32 get_ax(u32 command)
        {
            return (command >> 8) & 0xFFFFFF;
        }

        RV Context::reset(IWidgetList* widget_list)
        {
            auto buffer = widget_list->get_widget_buffer();
            usize i = 0;
            m_widgets.clear();
            m_widget_stack.clear();
            // Generate a root widget.
            m_root_widget = add_widget(WidgetType::container);
            m_widget_stack.push_back(m_current_widget);
            while(i < buffer.size())
            {
                u32 command = buffer[i];
                ++i;
                OpCode op = get_opcode(command);
                switch(op)
                {
                    case OpCode::nop:

                    break;
                    case OpCode::widget:
                    add_widget(WidgetType::container);
                    break;
                    case OpCode::text:
                    add_widget(WidgetType::text);
                    ((TextWidget*)m_current_widget)->m_text = widget_list->get_text(get_ax(command));
                    break;
                    case OpCode::begin:
                    m_widget_stack.push_back(m_current_widget);
                    break;
                    case OpCode::end:
                    m_widget_stack.pop_back();
                    break;
                    case OpCode::anchor:
                    {
                        RectComponent components = (RectComponent)get_b(command);
                        if(test_flags(components, RectComponent::left))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_anthor.left = value;
                        }
                        if(test_flags(components, RectComponent::top))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_anthor.top = value;
                        }
                        if(test_flags(components, RectComponent::right))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_anthor.right = value;
                        }
                        if(test_flags(components, RectComponent::bottom))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_anthor.bottom = value;
                        }
                    }
                    break;
                    case OpCode::rect:
                    {
                        RectComponent components = (RectComponent)get_b(command);
                        if(test_flags(components, RectComponent::left))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_rect.left = value;
                        }
                        if(test_flags(components, RectComponent::top))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_rect.top = value;
                        }
                        if(test_flags(components, RectComponent::right))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_rect.right = value;
                        }
                        if(test_flags(components, RectComponent::bottom))
                        {
                            f32 value = utof32(buffer[i]);
                            ++i;
                            m_current_widget->m_rect.bottom = value;
                        }
                    }
                    break;
                    case OpCode::color:
                    {
                        ColorType color_type = (ColorType)get_b(command);
                        u32 color = buffer[i];
                        ++i;
                        switch(color_type)
                        {
                            case ColorType::background:
                            m_current_widget->m_background_color = color;
                            break;
                            case ColorType::border:
                            m_current_widget->m_border_color = color;
                            break;
                            case ColorType::text:
                            if (m_current_widget->m_type == WidgetType::text)
                            {
                                TextWidget* w = (TextWidget*)m_current_widget;
                                w->m_text_color = color;
                            }
                            break;
                        }
                    }
                    break;
                    case OpCode::style:
                    {
                        StyleType style_type = (StyleType)get_b(command);
                        f32 value = utof32(buffer[i]);
                        ++i;
                        switch(style_type)
                        {
                            case StyleType::text_size:
                            if(m_current_widget->m_type == WidgetType::text)
                            {
                                ((TextWidget*)m_current_widget)->m_text_size = value;
                            }
                            break;
                        }
                    }
                    break;
                    default:
                    return set_error(BasicError::bad_data(), "Unsupported command detected");
                }
            }
            m_widget_stack.clear();
            m_dirty = true;
            return ok;
        }
        RV Context::update()
        {
            if(m_dirty)
            {
                // Calculates widget size.
                m_root_widget->m_min_x = 0;
                m_root_widget->m_min_y = 0;
                m_root_widget->m_max_x = m_io.width;
                m_root_widget->m_max_y = m_io.height;
                RingDeque<Widget*> widgets;
                for (Widget* w : m_root_widget->m_children)
                {
                    widgets.push_back(w);
                }
                while(!widgets.empty())
                {
                    Widget* w = widgets.front();
                    widgets.pop_front();
                    Widget* p = w->m_parent;
                    // Calculate anthor point.
                    f32 ax1 = lerp(p->m_min_x, p->m_max_x, w->m_anthor.left);
                    f32 ax2 = lerp(p->m_min_x, p->m_max_x, w->m_anthor.right);
                    f32 ay1 = lerp(p->m_min_y, p->m_max_y, w->m_anthor.top);
                    f32 ay2 = lerp(p->m_min_y, p->m_max_y, w->m_anthor.bottom);
                    // Calculate size.
                    w->m_min_x = ax1 + w->m_rect.left;
                    w->m_max_x = ax2 + w->m_rect.right;
                    w->m_min_y = ay1 + w->m_rect.top;
                    w->m_max_y = ay2 + w->m_rect.bottom;
                    if(w->m_type == WidgetType::text)
                    {
                        TextWidget* tw = (TextWidget*)w;
                        // Build font.
                        VG::TextArrangeSection section;
                        section.font_file = Font::get_default_font();
                        section.font_index = 0;
                        section.font_size = tw->m_text_size;
                        section.color = tw->m_text_color;
                        section.char_span = 0;
                        section.line_span = 0;
                        section.num_chars = tw->m_text.size();
                        tw->m_text_arrange_sections.clear();
                        tw->m_text_arrange_sections.push_back(section);
                        RectF rect(w->m_min_x, m_io.height - w->m_max_y, w->m_max_x - w->m_min_x, w->m_max_y - w->m_min_y);
                        tw->m_arrange_result = VG::arrange_text(tw->m_text.c_str(), tw->m_text.size(), 
                            tw->m_text_arrange_sections.cspan(), rect, VG::TextAlignment::center, VG::TextAlignment::begin);
                    }
                    for(Widget* c : w->m_children)
                    {
                        widgets.push_back(c);
                    }
                }
                m_dirty = false;
            }
            return ok;
        }
        RV Context::render(VG::IShapeDrawList* draw_list)
        {
            lutry
            {
                RingDeque<Widget*> draw_widgets;
                for(Widget* w : m_root_widget->m_children)
                {
                    draw_widgets.push_back(w);
                }
                while(!draw_widgets.empty())
                {
                    Widget* widget = draw_widgets.front();
                    draw_widgets.pop_front();
                    switch(widget->m_type)
                    {
                        case WidgetType::container:
                        {
                            Float4 background_color = Color::from_rgba8(widget->m_background_color);
                            if(background_color.w != 0)
                            {
                                auto& points = draw_list->get_shape_points();
                                u32 begin_command = (u32)points.size();
                                f32 screen_min_y = m_io.height - widget->m_max_y;
                                f32 screen_max_y = m_io.height - widget->m_min_y;
                                VG::ShapeBuilder::add_rectangle_filled(points, widget->m_min_x, screen_min_y, widget->m_max_x, screen_max_y);
                                u32 num_commands = (u32)points.size() - begin_command;
                                draw_list->draw_shape(begin_command, num_commands, 
                                    {widget->m_min_x, screen_min_y}, {widget->m_max_x, screen_max_y}, 
                                    {widget->m_min_x, screen_min_y}, {widget->m_max_x, screen_max_y},
                                    widget->m_background_color);
                            }
                        }
                        break;
                        case WidgetType::text:
                        {
                            TextWidget* tw = (TextWidget*)widget;
                            luexp(VG::commit_text_arrange_result(tw->m_arrange_result, tw->m_text_arrange_sections.cspan(), m_font_atlas, draw_list));
                        }
                    }
                    for(Widget* w : widget->m_children)
                    {
                        draw_widgets.push_back(w);
                    }
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_API Ref<IContext> new_context()
        {
            return new_object<Context>();
        }
    }
}