/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TabBar.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cstring>

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            struct TabHeaderData
            {
                c8* label = nullptr;
                i32 index = 0;
                i32* selected_index = nullptr;
                bool enabled = true;
            };

            static f32 tab_width(GUI::IContext* context, const GUI::ElementHandle& element, const c8* label)
            {
                f32 font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                f32 padding = style_scalar(context, element, "gui.tab.padding_x", 14.0f);
                return max((f32)strlen(label ? label : "") * font_size * 0.58f + padding * 2.0f, 56.0f);
            }

            static f32 tab_width_scale(GUI::IContext* context, const GUI::ElementHandle& element,
                const TabAction& action, f32 available_width)
            {
                if(action.fitting_mode != TabBarFittingMode::shrink || available_width <= 0.0f) return 1.0f;
                f32 natural_width = 0.0f;
                for(const String& label : action.state->header_labels)
                    natural_width += tab_width(context, element, label.c_str());
                return natural_width > available_width ? available_width / natural_width : 1.0f;
            }

            static GUI::MeasureResult measure_tab_bar(GUI::IContext*, const GUI::ElementHandle&,
                const Float2U&, void*)
            {
                GUI::MeasureResult result;
                result.minimum = Float2U(80.0f, 40.0f);
                result.desired = Float2U(320.0f, 220.0f);
                return result;
            }

            static RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 left = max(a.offset_x, b.offset_x);
                f32 top = max(a.offset_y, b.offset_y);
                f32 right = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 bottom = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(left, top, max(right - left, 0.0f), max(bottom - top, 0.0f));
            }

            static bool is_header(const TabState& state, id_t id, usize* index = nullptr)
            {
                for(usize i = 0; i < state.header_ids.size(); ++i)
                {
                    if(state.header_ids[i] == id)
                    {
                        if(index) *index = i;
                        return true;
                    }
                }
                return false;
            }

            static RV layout_tab_bar(GUI::IContext* context, const GUI::ElementHandle& element,
                const RectF& rect, void* userdata)
            {
                TabAction* action = (TabAction*)userdata;
                if(!action || !action->state) return BasicError::bad_arguments();
                const GUI::Element* bar = context->get_element(element.index);
                if(!bar) return BasicError::bad_arguments();
                f32 header_height = style_scalar(context, element, "gui.tab.height", 32.0f);
                f32 x = rect.offset_x;
                f32 scale = tab_width_scale(context, element, *action, rect.width);
                RectF clip = bar->layout_result.clip_rect;
                for(u32 child_index = bar->first_child; child_index != GUI::INVALID_ELEMENT;)
                {
                    const GUI::Element* child = context->get_element(child_index);
                    if(!child) break;
                    GUI::LayoutResult result;
                    usize header_index = 0;
                    if(is_header(*action->state, child->id, &header_index))
                    {
                        f32 width = tab_width(context, GUI::ElementHandle { child->id, child_index,
                            element.generation }, action->state->header_labels[header_index].c_str()) * scale;
                        result.rect = RectF(x, rect.offset_y, min(width, max(rect.offset_x + rect.width - x, 0.0f)),
                            header_height);
                        x += width;
                    }
                    else
                    {
                        result.rect = RectF(rect.offset_x, rect.offset_y + header_height, rect.width,
                            max(rect.height - header_height, 0.0f));
                    }
                    result.clip_rect = intersect_rect(result.rect, clip);
                    result.content_size = Float2U(result.rect.width, result.rect.height);
                    context->set_layout_result(GUI::ElementHandle { child->id, child_index, element.generation }, result);
                    child_index = child->next_sibling;
                }
                GUI::LayoutResult result = bar->layout_result;
                result.content_size = Float2U(rect.width, rect.height);
                context->set_layout_result(element, result);
                return ok;
            }

            static RV draw_tab_bar(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                TabAction* action = (TabAction*)userdata;
                if(!action || !action->state || action->state->header_ids.empty()) return ok;
                f32 header_height = style_scalar(context, element, "gui.tab.height", 32.0f);
                const GUI::Element* tab_bar = context->get_element(element.index);
                f32 scale = tab_bar ? tab_width_scale(context, element, *action, tab_bar->layout_result.rect.width) : 1.0f;
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = RectF(0.0f, 0.0f, 0.0f, header_height);
                command.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 0.0f);
                command.color = style_color(context, element, "gui.tab_bar.background",
                    Float4U(0.08f, 0.10f, 0.13f, 1.0f));
                context->draw(command);

                i32 left_index = clamp((i32)floor(action->state->animated_index), 0,
                    (i32)action->state->header_ids.size() - 1);
                i32 right_index = clamp(left_index + 1, 0, (i32)action->state->header_ids.size() - 1);
                f32 factor = action->state->animated_index - floor(action->state->animated_index);
                auto position = [&](i32 index, f32& x, f32& width)
                {
                    x = 0.0f;
                    for(i32 i = 0; i < index; ++i)
                        x += tab_width(context, element, action->state->header_labels[(usize)i].c_str()) * scale;
                    width = tab_width(context, element, action->state->header_labels[(usize)index].c_str()) * scale;
                };
                f32 left_x = 0.0f, left_width = 0.0f, right_x = 0.0f, right_width = 0.0f;
                position(left_index, left_x, left_width);
                position(right_index, right_x, right_width);
                command.type = GUI::DrawCommandType::rounded_rect;
                command.rect = RectF(left_x + (right_x - left_x) * factor, 1.0f,
                    left_width + (right_width - left_width) * factor, header_height - 1.0f);
                command.rect_layout_scale = Float4U(0.0f);
                command.color = style_color(context, element, "gui.tab.selected",
                    Float4U(0.16f, 0.35f, 0.58f, 1.0f));
                command.radius = 4.0f;
                context->draw(command);
                return ok;
            }

            static RV draw_tab_header(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                TabHeaderData* data = (TabHeaderData*)userdata;
                if(!data) return ok;
                GUI::InteractionState interaction = context->get_interaction_state(element.id);
                bool selected = data->selected_index && *data->selected_index == data->index;
                if(data->enabled && interaction.hovered && !selected)
                {
                    GUI::DrawCommand background;
                    background.type = GUI::DrawCommandType::rect;
                    background.rect_reference = GUI::DrawCommandRectReference::element;
                    background.color = style_color(context, element, "gui.tab.hovered",
                        Float4U(0.13f, 0.19f, 0.27f, 1.0f));
                    context->draw(background);
                }
                GUI::DrawCommand text;
                text.type = GUI::DrawCommandType::text;
                text.rect_reference = GUI::DrawCommandRectReference::element;
                text.rect = RectF(8.0f, 0.0f, -16.0f, 0.0f);
                text.text = data->label ? data->label : "";
                text.font = style_name(context, element, "gui.font");
                text.font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                text.color = data->enabled ? style_color(context, element, "gui.text.color", Float4U(0.86f, 0.88f, 0.92f, 1.0f)) :
                    style_color(context, element, "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                text.horizontal_alignment = VG::TextAlignment::center;
                text.vertical_alignment = VG::TextAlignment::center;
                context->draw(text);
                return ok;
            }

            bool resolve_tab_action(GUI::IContext* context, TabAction& action)
            {
                if(!action.selected_index || !action.state || action.state->header_ids.empty()) return false;
                bool changed = false;
                if(action.enabled)
                {
                    for(usize i = 0; i < action.state->header_ids.size(); ++i)
                    {
                        if(context->get_interaction_state(action.state->header_ids[i]).clicked &&
                            *action.selected_index != (i32)i)
                        {
                            *action.selected_index = (i32)i;
                            changed = true;
                            break;
                        }
                    }
                }
                *action.selected_index = clamp(*action.selected_index, 0, (i32)action.state->header_ids.size() - 1);
                if(!action.state->initialized)
                {
                    action.state->animated_index = (f32)*action.selected_index;
                    action.state->initialized = true;
                }
                else
                {
                    action.state->animated_index = smooth_step(action.state->animated_index,
                        (f32)*action.selected_index, 10.0f, context->get_frame_desc().delta_time);
                }
                return changed;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_tab_bar(GUI::IContext* context, id_t id, i32* selected_index,
            const GUI::LayoutConfig& layout, const TabBarDesc& desc)
        {
            luassert(context && id && selected_index);
            GUI::ElementHandle bar = Internal::begin_element(context, id, "Tab Bar", layout);
            Ref<Internal::TabState> state = Internal::widget_state<Internal::TabState>(context, id);
            state->header_ids.clear();
            state->header_labels.clear();
            Internal::TabAction* action = Internal::allocate_frame<Internal::TabAction>(context);
            action->id = id;
            action->selected_index = selected_index;
            action->enabled = desc.enabled;
            action->fitting_mode = desc.fitting_mode;
            action->state = state.get();
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.tab_bar");
            callbacks.measure_callback = Internal::measure_tab_bar;
            callbacks.callback = Internal::layout_tab_bar;
            callbacks.userdata = action;
            context->set_layout_callback_config(bar, callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.tab_bar");
            draw.callback = Internal::draw_tab_bar;
            draw.userdata = action;
            context->set_draw_config(bar, draw);
            Internal::TabBuildScope scope;
            scope.bar = bar;
            scope.data = action;
            Internal::frame_state(context)->tab_stack.push_back(scope);
            return bar;
        }

        LUNA_EDITOR_GUI_API bool begin_tab_item(GUI::IContext* context, id_t id, const c8* label, const TabItemDesc& desc)
        {
            luassert(context && id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->tab_stack.empty());
            Internal::TabBuildScope& scope = frame->tab_stack.back();
            luassert(!scope.content_open);
            i32 index = (i32)scope.data->state->header_ids.size();
            if(desc.selected)
            {
                *scope.data->selected_index = index;
            }
            scope.data->state->header_ids.push_back(id);
            scope.data->state->header_labels.push_back(String(label ? label : ""));
            GUI::LayoutConfig header_layout;
            header_layout.width.kind = GUI::SizeKind::fit;
            header_layout.height.kind = GUI::SizeKind::fixed;
            header_layout.height.value = Internal::style_scalar(context, scope.bar, "gui.tab.height", 32.0f);
            GUI::ElementHandle header = Internal::begin_element(context, id, label ? label : "Tab", header_layout);
            Internal::set_interactable(context, header, scope.data->enabled);
            Internal::TabHeaderData* data = Internal::allocate_frame<Internal::TabHeaderData>(context);
            data->label = Internal::copy_frame_string(context, label);
            data->index = index;
            data->selected_index = scope.data->selected_index;
            data->enabled = scope.data->enabled;
            GUI::DrawConfig draw;
            draw.name = Name("gui.tab_header");
            draw.callback = Internal::draw_tab_header;
            draw.userdata = data;
            context->set_draw_config(header, draw);
            context->end_element();
            bool selected = index == *scope.data->selected_index;
            if(selected)
            {
                GUI::LayoutConfig content_layout;
                content_layout.width.kind = GUI::SizeKind::percent;
                content_layout.width.value = 1.0f;
                content_layout.height.kind = GUI::SizeKind::percent;
                content_layout.height.value = 1.0f;
                scope.content = Internal::begin_element(context, GUI::make_scoped_id(id, "content"),
                    "Tab Content", content_layout);
                scope.content_open = true;
            }
            return selected;
        }

        LUNA_EDITOR_GUI_API void end_tab_item(GUI::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->tab_stack.empty() && frame->tab_stack.back().content_open);
            Internal::TabBuildScope& scope = frame->tab_stack.back();
            GUI::FlexLayoutDesc flex;
            flex.axis = GUI::LayoutAxis::y;
            flex.cross_alignment = GUI::FlexAlignment::stretch;
            Internal::set_flex_layout(context, scope.content, flex, GUI::LayoutAxis::y);
            scope.content_open = false;
        }

        LUNA_EDITOR_GUI_API void end_tab_bar(GUI::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->tab_stack.empty() && !frame->tab_stack.back().content_open);
            Internal::TabBuildScope scope = frame->tab_stack.back();
            frame->tab_stack.pop_back();
            if(scope.data->state->header_ids.empty()) *scope.data->selected_index = -1;
            else *scope.data->selected_index = clamp(*scope.data->selected_index, 0,
                (i32)scope.data->state->header_ids.size() - 1);
            context->end_element();
            Internal::add_action(context, Internal::ActionType::tab_bar, scope.bar.id, scope.data);
        }
    }
}
