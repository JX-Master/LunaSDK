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
#include "../Widgets.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/Shapes.hpp>

namespace Luna
{
    template <>
    struct hash<Ref<GUI::Widget>>
    {
        usize operator()(const Ref<GUI::Widget>& rhs)
        {
            return hash<GUI::Widget*>()(rhs.get());
        }
    };
    namespace GUI
    {
        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset)
        {
            // Calculate anthor point.
            f32 ax1 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.left);
            f32 ax2 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.right);
            f32 ay1 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.top);
            f32 ay2 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.bottom);
            // Calculate size.
            OffsetRectF r;
            r.left = ax1 + offset.left;
            r.right = ax2 + offset.right;
            r.top = ay1 + offset.top;
            r.bottom = ay2 + offset.bottom;
            return r;
        }
        void Context::reset(Widget* root_widget)
        {
            m_new_root_widget = root_widget;
        }
        void match_build_data(Widget* widget, WidgetBuildData* build_data, 
            HashMap<Ref<Widget>, Ref<WidgetBuildData>>& out_widget_map)
        {
            if(!build_data) return;
            Widget* old_widget = build_data->widget;
            if(get_object_type(old_widget) == get_object_type(widget))
            {
                // We can reuse the widget build data.
                out_widget_map.insert(make_pair(widget, build_data));
                // We need to recalculate the widget.
                if (!old_widget->equal_to(widget))
                {
                    build_data->dirty = true;
                }
            }
            // match children.
            Vector<bool> matched_children;
            matched_children.resize(build_data->children.size(), false);
            for(usize i = 0; i < widget->children.size(); ++i)
            {
                // Match by ID firstly.
                bool matched = false;
                Name id = widget->children[i]->id;
                if(!id.empty())
                {
                    for(usize j = 0; j < build_data->children.size(); ++j)
                    {
                        if(id == build_data->children[j]->widget->id)
                        {
                            // matched.
                            matched_children[j] = true;
                            match_build_data(widget->children[i], build_data->children[j], out_widget_map);
                            matched = true;
                            break;
                        }
                    }
                }
                if(matched) continue;
                // If ID is empty or not found, match by index.
                // This will skip widgets that are already matched to prevent duplication.
                if(i < build_data->children.size() && !matched_children[i])
                {
                    match_build_data(widget->children[i], build_data->children[i], out_widget_map);
                }
            }
        }
        Ref<WidgetBuildData> construct_build_data_tree(Widget* widget, HashMap<Ref<Widget>, Ref<WidgetBuildData>>& widget_map)
        {
            auto iter = widget_map.find(widget);
            Ref<WidgetBuildData> ret;
            if(iter != widget_map.end())
            {
                ret = iter->second;
                ret->children.clear();
            }
            else
            {
                ret = widget->new_build_data();
            }
            ret->widget = widget;
            // resolve children.
            for(usize i = 0; i < widget->children.size(); ++i)
            {
                Ref<WidgetBuildData> child = construct_build_data_tree(widget->children[i], widget_map);
                child->parent = ret;
                ret->children.push_back(child);
            }
            return ret;
        }
        void Context::diff_widget_tree()
        {
            // match build data against new widget tree.
            HashMap<Ref<Widget>, Ref<WidgetBuildData>> widget_map;
            match_build_data(m_new_root_widget, m_root_widget_build_data, widget_map);
            // construct new build data tree.
            m_root_widget_build_data = construct_build_data_tree(m_new_root_widget, widget_map);
            m_root_widget = m_new_root_widget;
        }
        RV Context::update()
        {
            if(m_root_widget_build_data)
            {
                m_root_widget_build_data->update(this);
            }
            if(m_new_root_widget != m_root_widget)
            {
                // A new widget tree is set.
                diff_widget_tree();

                // // Calculates widget size.
                // m_root_widget->m_min_x = 0;
                // m_root_widget->m_min_y = 0;
                // m_root_widget->m_max_x = m_io.width;
                // m_root_widget->m_max_y = m_io.height;
                // RingDeque<Widget*> widgets;
                // for (Widget* w : m_root_widget->m_children)
                // {
                //     widgets.push_back(w);
                // }
                // while(!widgets.empty())
                // {
                //     Widget* w = widgets.front();
                //     widgets.pop_front();
                //     Widget* p = w->m_parent;
                //     // Calculate anthor point.
                //     f32 ax1 = lerp(p->m_min_x, p->m_max_x, w->m_anthor.left);
                //     f32 ax2 = lerp(p->m_min_x, p->m_max_x, w->m_anthor.right);
                //     f32 ay1 = lerp(p->m_min_y, p->m_max_y, w->m_anthor.top);
                //     f32 ay2 = lerp(p->m_min_y, p->m_max_y, w->m_anthor.bottom);
                //     // Calculate size.
                //     w->m_min_x = ax1 + w->m_rect.left;
                //     w->m_max_x = ax2 + w->m_rect.right;
                //     w->m_min_y = ay1 + w->m_rect.top;
                //     w->m_max_y = ay2 + w->m_rect.bottom;
                //     if(w->m_type == WidgetType::text)
                //     {
                //         TextWidget* tw = (TextWidget*)w;
                //         // Build font.
                //         VG::TextArrangeSection section;
                //         section.font_file = Font::get_default_font();
                //         section.font_index = 0;
                //         section.font_size = tw->m_text_size;
                //         section.color = tw->m_text_color;
                //         section.char_span = 0;
                //         section.line_span = 0;
                //         section.num_chars = tw->m_text.size();
                //         tw->m_text_arrange_sections.clear();
                //         tw->m_text_arrange_sections.push_back(section);
                //         RectF rect(w->m_min_x, m_io.height - w->m_max_y, w->m_max_x - w->m_min_x, w->m_max_y - w->m_min_y);
                //         tw->m_arrange_result = VG::arrange_text(tw->m_text.c_str(), tw->m_text.size(), 
                //             tw->m_text_arrange_sections.cspan(), rect, VG::TextAlignment::center, VG::TextAlignment::begin);
                //     }
                //     for(Widget* c : w->m_children)
                //     {
                //         widgets.push_back(c);
                //     }
                // }
                // m_dirty = false;
            }
            // Recalculate dirty widgets.
            RingDeque<WidgetBuildData*> resolve_data;
            resolve_data.push_back(m_root_widget_build_data);
            while(!resolve_data.empty())
            {
                WidgetBuildData* data = resolve_data.front();
                resolve_data.pop_front();
                if(data->dirty)
                {
                    // resolve this.
                    auto r = data->build(this);
                    if(failed(r))
                    {
                        return r.errcode();
                    }
                }
                else
                {
                    // Push child widgets, since they may be dirty.
                    for(usize i = 0; i < data->children.size(); ++i)
                    {
                        resolve_data.push_back(data->children[i]);
                    }
                }
            }
            return ok;
        }
        RV Context::render(VG::IShapeDrawList* draw_list)
        {
            // lutry
            // {
                
                // RingDeque<WidgetBuildData*> draw_widgets;
                // draw_widgets.push_back(m_root_widget_build_data);
                // while(!draw_widgets.empty())
                // {
                //     WidgetBuildData* widget = draw_widgets.front();
                //     draw_widgets.pop_front();
                //     switch(widget->m_type)
                //     {
                //         case WidgetType::container:
                //         {
                //             Float4 background_color = Color::from_rgba8(widget->m_background_color);
                //             if(background_color.w != 0)
                //             {
                //                 auto& points = draw_list->get_shape_points();
                //                 u32 begin_command = (u32)points.size();
                //                 f32 screen_min_y = m_io.height - widget->m_max_y;
                //                 f32 screen_max_y = m_io.height - widget->m_min_y;
                //                 VG::ShapeBuilder::add_rectangle_filled(points, widget->m_min_x, screen_min_y, widget->m_max_x, screen_max_y);
                //                 u32 num_commands = (u32)points.size() - begin_command;
                //                 draw_list->draw_shape(begin_command, num_commands, 
                //                     {widget->m_min_x, screen_min_y}, {widget->m_max_x, screen_max_y}, 
                //                     {widget->m_min_x, screen_min_y}, {widget->m_max_x, screen_max_y},
                //                     widget->m_background_color);
                //             }
                //         }
                //         break;
                //         case WidgetType::text:
                //         {
                //             TextWidget* tw = (TextWidget*)widget;
                //             luexp(VG::commit_text_arrange_result(tw->m_arrange_result, tw->m_text_arrange_sections.cspan(), m_font_atlas, draw_list));
                //         }
                //     }
                //     for(Widget* w : widget->m_children)
                //     {
                //         draw_widgets.push_back(w);
                //     }
                // }
            // }
            // lucatchret;
            return m_root_widget_build_data->render(this, draw_list);
        }
        LUNA_GUI_API Ref<IContext> new_context()
        {
            return new_object<Context>();
        }
    }
}