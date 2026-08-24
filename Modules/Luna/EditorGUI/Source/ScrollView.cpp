/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ScrollView.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            constexpr f32 SCROLLBAR_SIZE = 12.0f;

            struct ScrollbarData
            {
                GUI::id_t viewport_id = 0;
                bool vertical = false;
                ScrollBarMode mode = ScrollBarMode::dynamic_overlay;
                ScrollState* state = nullptr;
            };

            static RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 left = max(a.offset_x, b.offset_x);
                f32 top = max(a.offset_y, b.offset_y);
                f32 right = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 bottom = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(left, top, max(right - left, 0.0f), max(bottom - top, 0.0f));
            }

            static RV layout_scroll_view(GUI::IContext* context, const GUI::ElementHandle& element,
                const RectF& rect, void* userdata)
            {
                ScrollAction* action = (ScrollAction*)userdata;
                if(!action || !action->layout_desc) return E_BAD_ARGUMENTS;
                lutry(GUI::layout_scroll_viewport(context, element, rect, action->layout_desc));
                const GUI::Element* viewport = context->get_element(element.index);
                if(!viewport) return E_BAD_ARGUMENTS;
                u32 content_index = viewport->first_child;
                Float2U content_size(0.0f);
                if(content_index != GUI::INVALID_ELEMENT)
                {
                    const GUI::Element* content = context->get_element(content_index);
                    if(content)
                    {
                        content_size = Float2U(content->layout_result.rect.width + content->layout.margin.x +
                            content->layout.margin.z, content->layout_result.rect.height + content->layout.margin.y +
                            content->layout.margin.w);
                    }
                }
                f32 right_inset = action->desc.scrollbar_mode == ScrollBarMode::dynamic_overlay ? 2.0f : 0.0f;
                f32 bottom_inset = right_inset;
                for(u32 child_index = content_index == GUI::INVALID_ELEMENT ? GUI::INVALID_ELEMENT :
                    context->get_element(content_index)->next_sibling;
                    child_index != GUI::INVALID_ELEMENT;)
                {
                    const GUI::Element* child = context->get_element(child_index);
                    if(!child) break;
                    GUI::LayoutResult result;
                    if(child->id == action->vertical_bar_id)
                    {
                        f32 horizontal_reserve = action->desc.horizontal ? SCROLLBAR_SIZE : 0.0f;
                        result.rect = RectF(rect.offset_x + rect.width - SCROLLBAR_SIZE - right_inset,
                            rect.offset_y + 2.0f, SCROLLBAR_SIZE,
                            max(rect.height - horizontal_reserve - 4.0f - bottom_inset, 0.0f));
                    }
                    else
                    {
                        f32 vertical_reserve = action->desc.vertical ? SCROLLBAR_SIZE : 0.0f;
                        result.rect = RectF(rect.offset_x + 2.0f,
                            rect.offset_y + rect.height - SCROLLBAR_SIZE - bottom_inset,
                            max(rect.width - vertical_reserve - 4.0f - right_inset, 0.0f), SCROLLBAR_SIZE);
                    }
                    result.clip_rect = intersect_rect(result.rect, rect);
                    result.content_size = Float2U(result.rect.width, result.rect.height);
                    context->set_layout_result(GUI::ElementHandle { child->id, child_index, element.generation }, result);
                    child_index = child->next_sibling;
                }
                GUI::LayoutResult parent_result = viewport->layout_result;
                parent_result.content_size = content_size;
                context->set_layout_result(element, parent_result);
                return ok;
            }

            static RV draw_scrollbar(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                ScrollbarData* data = (ScrollbarData*)userdata;
                if(!data || !data->state) return ok;
                const GUI::Element* viewport = context->find_element(data->viewport_id);
                const GUI::Element* bar = context->find_element(element.id);
                if(!viewport || !bar) return ok;
                f32 visibility = data->mode == ScrollBarMode::always_visible ? 1.0f : data->state->visibility;
                Float2U viewport_size(viewport->layout_result.rect.width - viewport->layout.padding.x -
                    viewport->layout.padding.z, viewport->layout_result.rect.height - viewport->layout.padding.y -
                    viewport->layout.padding.w);
                f32 content = data->vertical ? viewport->layout_result.content_size.y : viewport->layout_result.content_size.x;
                f32 visible = data->vertical ? viewport_size.y : viewport_size.x;
                if(visibility <= 0.001f || content <= visible + 0.5f) return ok;
                GUI::InteractionState interaction = context->get_interaction_state(element.id);
                f32 thickness = interaction.hovered || interaction.active ? SCROLLBAR_SIZE - 2.0f : 4.0f;
                f32 track_length = data->vertical ? bar->layout_result.rect.height : bar->layout_result.rect.width;
                f32 thumb_length = max(track_length * visible / max(content, 1.0f), 20.0f);
                f32 maximum_offset = max(content - visible, 1.0f);
                f32 offset = data->vertical ? data->state->offset.y : data->state->offset.x;
                f32 thumb_offset = (track_length - thumb_length) * clamp(offset / maximum_offset, 0.0f, 1.0f);
                GUI::DrawCommand command;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                if(interaction.hovered || interaction.active || data->mode == ScrollBarMode::always_visible)
                {
                    command.type = GUI::DrawCommandType::rounded_rect;
                    command.color = style_color(context, element, "gui.scrollbar.track",
                        Float4U(0.04f, 0.05f, 0.07f, 0.72f));
                    command.color.w *= visibility;
                    command.radius = SCROLLBAR_SIZE * 0.5f;
                    context->draw(command);
                }
                command.type = GUI::DrawCommandType::rounded_rect;
                command.color = style_color(context, element, "gui.scrollbar.thumb",
                    Float4U(0.42f, 0.50f, 0.60f, 0.72f));
                command.color.w *= visibility;
                if(data->vertical)
                    command.rect = RectF((SCROLLBAR_SIZE - thickness) * 0.5f, thumb_offset, thickness, thumb_length);
                else
                    command.rect = RectF(thumb_offset, (SCROLLBAR_SIZE - thickness) * 0.5f, thumb_length, thickness);
                command.radius = thickness * 0.5f;
                context->draw(command);
                return ok;
            }

            static bool contains(const RectF& rect, const Float2U& point)
            {
                return point.x >= rect.offset_x && point.y >= rect.offset_y &&
                    point.x < rect.offset_x + rect.width && point.y < rect.offset_y + rect.height;
            }

            bool resolve_scroll_action(GUI::IContext* context, ScrollAction& action)
            {
                if(!action.state || !action.layout_desc) return false;
                ScrollState& state = *action.state;
                const GUI::Element* viewport = context->find_element(action.id);
                if(!viewport) return false;
                Float2U viewport_size(max(viewport->layout_result.rect.width - viewport->layout.padding.x -
                    viewport->layout.padding.z, 0.0f), max(viewport->layout_result.rect.height -
                    viewport->layout.padding.y - viewport->layout.padding.w, 0.0f));
                Float2U content_size = viewport->layout_result.content_size;
                Float2U maximum(max(content_size.x - viewport_size.x, 0.0f),
                    max(content_size.y - viewport_size.y, 0.0f));
                Float2U old_offset = state.offset;
                bool activity = false;
                for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(action.id))
                {
                    if(routed.event.type == GUI::InputEventType::pointer_wheel)
                    {
                        if(action.desc.horizontal) state.offset.x -= routed.event.wheel_delta.x * action.desc.wheel_scale;
                        if(action.desc.vertical) state.offset.y -= routed.event.wheel_delta.y * action.desc.wheel_scale;
                        activity = true;
                    }
                }
                auto resolve_bar = [&](id_t bar_id, bool vertical)
                {
                    const GUI::Element* bar = context->find_element(bar_id);
                    if(!bar) return;
                    for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(bar_id))
                    {
                        if(!routed.has_element_position) continue;
                        if((routed.event.type == GUI::InputEventType::pointer_down &&
                            routed.event.button == GUI::PointerButton::left) ||
                            (routed.event.type == GUI::InputEventType::pointer_move &&
                            context->is_pointer_button_down(GUI::PointerButton::left)))
                        {
                            f32 length = vertical ? bar->layout_result.rect.height : bar->layout_result.rect.width;
                            f32 position = vertical ? routed.element_position.y : routed.element_position.x;
                            f32 fraction = clamp(position / max(length, 1.0f), 0.0f, 1.0f);
                            if(vertical) state.offset.y = maximum.y * fraction;
                            else state.offset.x = maximum.x * fraction;
                            activity = true;
                        }
                    }
                };
                resolve_bar(action.horizontal_bar_id, false);
                resolve_bar(action.vertical_bar_id, true);
                state.offset.x = action.desc.horizontal ? clamp(state.offset.x, 0.0f, maximum.x) : 0.0f;
                state.offset.y = action.desc.vertical ? clamp(state.offset.y, 0.0f, maximum.y) : 0.0f;
                f32 delta_time = max(context->get_frame_desc().delta_time, 0.0f);
                if(action.desc.scrollbar_mode == ScrollBarMode::always_visible)
                {
                    state.visibility = 1.0f;
                }
                else
                {
                    bool pointer_motion_inside = contains(viewport->layout_result.rect, context->get_pointer_position()) &&
                        (context->get_pointer_delta().x != 0.0f || context->get_pointer_delta().y != 0.0f);
                    bool scrollbar_hovered = context->get_interaction_state(action.horizontal_bar_id).hovered ||
                        context->get_interaction_state(action.vertical_bar_id).hovered;
                    if(activity || pointer_motion_inside || scrollbar_hovered)
                    {
                        state.idle_time = 0.0f;
                        state.visibility = smooth_step(state.visibility, 1.0f, 14.0f, delta_time);
                    }
                    else
                    {
                        state.idle_time += delta_time;
                        f32 target = state.idle_time > 0.85f ? 0.0f : 1.0f;
                        state.visibility = smooth_step(state.visibility, target, 8.0f, delta_time);
                    }
                }
                action.layout_desc->scroll_offset = state.offset;
                return old_offset.x != state.offset.x || old_offset.y != state.offset.y;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_scroll_view(GUI::IContext* context, id_t id, const c8* label,
            const GUI::LayoutConfig& layout, const ScrollViewDesc& desc)
        {
            luassert(context && id);
            GUI::LayoutConfig viewport_layout = layout;
            if(desc.scrollbar_mode == ScrollBarMode::always_visible)
            {
                if(desc.vertical) viewport_layout.padding.z += Internal::SCROLLBAR_SIZE;
                if(desc.horizontal) viewport_layout.padding.w += Internal::SCROLLBAR_SIZE;
            }
            GUI::ElementHandle viewport = Internal::begin_element(context, id, label ? label : "Scroll View",
                viewport_layout);
            GUI::Interactable interactable;
            interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
            set_flags(interactable.flags, GUI::InteractableFlag::scrollable);
            context->set_interactable(viewport, interactable);
            Ref<Internal::ScrollState> state = Internal::widget_state<Internal::ScrollState>(context, id);
            Internal::ScrollAction* action = Internal::allocate_frame<Internal::ScrollAction>(context);
            action->id = id;
            action->horizontal_bar_id = GUI::make_scoped_id(id, "horizontal_scrollbar");
            action->vertical_bar_id = GUI::make_scoped_id(id, "vertical_scrollbar");
            action->desc = desc;
            action->state = state.get();
            action->layout_desc = Internal::allocate_frame<GUI::ScrollViewportLayoutDesc>(context);
            action->layout_desc->scroll_offset = state->offset;
            action->layout_desc->max_scroll_delta = desc.max_scroll_delta;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.scroll_view");
            callbacks.callback = Internal::layout_scroll_view;
            callbacks.userdata = action;
            context->set_layout_callback_config(viewport, callbacks);

            GUI::LayoutConfig content_layout;
            content_layout.width.kind = GUI::SizeKind::fit;
            content_layout.height.kind = GUI::SizeKind::fit;
            GUI::ElementHandle content = Internal::begin_element(context, GUI::make_scoped_id(id, "content"),
                "Scroll Content", content_layout);
            Internal::ScrollBuildScope scope;
            scope.viewport = viewport;
            scope.content = content;
            scope.data = action;
            Internal::frame_state(context)->scroll_stack.push_back(scope);
            return viewport;
        }

        LUNA_EDITOR_GUI_API void end_scroll_view(GUI::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->scroll_stack.empty());
            Internal::ScrollBuildScope scope = frame->scroll_stack.back();
            frame->scroll_stack.pop_back();
            GUI::FlexLayoutDesc content_flex;
            content_flex.axis = GUI::LayoutAxis::y;
            content_flex.cross_alignment = GUI::FlexAlignment::stretch;
            Internal::set_flex_layout(context, scope.content, content_flex, GUI::LayoutAxis::y);
            auto add_bar = [&](id_t bar_id, bool vertical, bool enabled)
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::fixed;
                layout.width.value = Internal::SCROLLBAR_SIZE;
                layout.height.kind = GUI::SizeKind::fixed;
                layout.height.value = Internal::SCROLLBAR_SIZE;
                GUI::ElementHandle bar = Internal::begin_element(context, bar_id,
                    vertical ? "Vertical Scrollbar" : "Horizontal Scrollbar", layout);
                Internal::set_interactable(context, bar, enabled);
                Internal::ScrollbarData* data = Internal::allocate_frame<Internal::ScrollbarData>(context);
                data->viewport_id = scope.viewport.id;
                data->vertical = vertical;
                data->mode = scope.data->desc.scrollbar_mode;
                data->state = scope.data->state;
                GUI::DrawConfig draw;
                draw.name = Name(vertical ? "gui.scrollbar.vertical" : "gui.scrollbar.horizontal");
                draw.callback = Internal::draw_scrollbar;
                draw.userdata = data;
                context->set_draw_config(bar, draw);
                context->end_element();
            };
            bool visible = scope.data->desc.scrollbar_mode == ScrollBarMode::always_visible ||
                scope.data->state->visibility > 0.01f;
            if(scope.data->desc.horizontal)
            {
                add_bar(scope.data->horizontal_bar_id, false, visible);
            }
            if(scope.data->desc.vertical)
            {
                add_bar(scope.data->vertical_bar_id, true, visible);
            }
            context->end_element();
            Internal::add_action(context, Internal::ActionType::scroll_view, scope.viewport.id, scope.data);
        }

        LUNA_EDITOR_GUI_API RectF get_scroll_view_visible_rect(GUI::IContext* context,
            const GUI::ElementHandle& scroll_view)
        {
            return GUI::get_scroll_viewport_visible_rect(context, scroll_view);
        }
    }
}
