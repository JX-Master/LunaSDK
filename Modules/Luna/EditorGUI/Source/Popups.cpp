/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Popups.cpp
* @author JXMaster
* @date 2026/7/15
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
            struct PopupPanelDrawData
            {
                const c8* prefix = nullptr;
                Float4U background_fallback = Float4U(0.0f);
            };

            static Ref<PopupState> popup_state(GUI::IContext* context, id_t id, bool create)
            {
                id_t state_id = GUI::make_state_id<PopupState>(id);
                if(object_t object = context->get_state(state_id))
                {
                    object_retain(object);
                    Ref<PopupState> result;
                    result.attach(object);
                    return result;
                }
                if(!create) return nullptr;
                Ref<PopupState> result = new_object<PopupState>();
                lupanic_if_failed(context->set_state(state_id, result.object(), GUI::StateLifetime::context));
                return result;
            }

            static RV draw_panel(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                PopupPanelDrawData* data = (PopupPanelDrawData*)userdata;
                if(!data || !data->prefix) return BasicError::bad_arguments();
                const c8* prefix = data->prefix;
                String name;
                strprintf(name, "gui.%s.background", prefix);
                Float4U background = style_color(context, element, name.c_str(), data->background_fallback);
                strprintf(name, "gui.%s.border", prefix);
                Float4U border = style_color(context, element, name.c_str(), Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                strprintf(name, "gui.%s.radius", prefix);
                f32 radius = style_scalar(context, element, name.c_str(), 5.0f);
                strprintf(name, "gui.%s.backdrop_softness", prefix);
                bool draw_backdrop = style_scalar(context, element, name.c_str(), 0.0f) > 0.0f;
                f32 shadow_softness = style_scalar(context, element, "gui.shadow.softness", 6.0f);
                RoundedRectEffect outer_effects[2];
                outer_effects[0].shadow = true;
                outer_effects[0].color = style_color(context, element, "gui.shadow.dark",
                    Float4U(0.0f, 0.0f, 0.0f, 0.28f));
                outer_effects[0].color.w = min(outer_effects[0].color.w * 1.18f, 0.46f);
                outer_effects[0].shadow_desc.offset = Float2U(0.0f, shadow_softness * 1.4f);
                outer_effects[0].shadow_desc.softness = shadow_softness * 3.0f;
                outer_effects[1].color = border;
                if(RV result = draw_rounded_rect_effects(context, element, RectF(), Float4U(), radius,
                    Span<const RoundedRectEffect>(outer_effects, 2)); failed(result))
                {
                    return result;
                }
                if(draw_backdrop)
                {
                    GUI::DrawCommand backdrop;
                    backdrop.type = GUI::DrawCommandType::backdrop_blur;
                    backdrop.rect_reference = GUI::DrawCommandRectReference::element;
                    backdrop.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                    backdrop.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
                    backdrop.radius = max(radius - 1.0f, 0.0f);
                    context->draw(backdrop);
                }

                RoundedRectEffect fill;
                fill.color = background;
                return draw_rounded_rect_effects(context, element,
                    RectF(1.0f, 1.0f, -2.0f, -2.0f), Float4U(), max(radius - 1.0f, 0.0f),
                    Span<const RoundedRectEffect>(&fill, 1));
            }

            static void set_panel_draw_config(GUI::IContext* context,
                const GUI::ElementHandle& element, const c8* prefix,
                const Float4U& background_fallback)
            {
                PopupPanelDrawData* data = allocate_frame<PopupPanelDrawData>(context);
                data->prefix = prefix;
                data->background_fallback = background_fallback;
                GUI::DrawConfig draw;
                draw.name = Name("gui.popup_panel");
                draw.callback = draw_panel;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                String name;
                strprintf(name, "gui.%s.backdrop_softness", prefix);
                f32 softness = style_scalar(context, element, name.c_str(), 0.0f);
                if(softness > 0.0f)
                {
                    strprintf(name, "gui.%s.backdrop_downsample_level", prefix);
                    f32 downsample_level =
                        style_scalar(context, element, name.c_str(), 1.0f);
                    GUI::BackdropBlurCaptureDesc capture;
                    capture.softness = softness;
                    capture.downsample_level = (u8)clamp(
                        (i32)round(downsample_level), 0, 4);
                    context->set_backdrop_blur_capture(element, capture);
                }
            }

            static GUI::LayoutConfig panel_layout(GUI::IContext* context,
                const GUI::ElementHandle& element, const GUI::LayoutConfig& input, const c8* prefix)
            {
                GUI::LayoutConfig result = input;
                String name;
                strprintf(name, "gui.%s.padding", prefix);
                f32 padding = style_scalar(context, element, name.c_str(), 7.0f);
                result.padding = Float4U(padding);
                return result;
            }
        }

        LUNA_EDITOR_GUI_API void open_popup(GUI::IContext* context, id_t id)
        {
            luassert(context && id);
            Ref<Internal::PopupState> state = Internal::popup_state(context, id, true);
            state->open = true;
            lupanic_if_failed(context->set_state(GUI::make_state_id<Internal::PopupState>(id), state.object(),
                GUI::StateLifetime::context));
        }

        LUNA_EDITOR_GUI_API void close_popup(GUI::IContext* context, id_t id)
        {
            luassert(context && id);
            context->clear_state(GUI::make_state_id<Internal::PopupState>(id));
        }

        LUNA_EDITOR_GUI_API bool is_popup_open(GUI::IContext* context, id_t id)
        {
            luassert(context && id);
            Ref<Internal::PopupState> state = Internal::popup_state(context, id, false);
            return state && state->open;
        }

        LUNA_EDITOR_GUI_API bool begin_popup(GUI::IContext* context, id_t id, const PopupDesc& desc,
            GUI::ElementHandle* out_handle)
        {
            luassert(context && id);
            if(!is_popup_open(context, id))
            {
                if(out_handle) *out_handle = GUI::ElementHandle();
                return false;
            }
            context->push_layer(id, desc.position);
            context->set_layer_debug_name(id, Name("GUI Popup"));
            GUI::ElementHandle root = Internal::begin_element(context,
                Internal::derived_id(id, "popup.root"), "Popup", desc.layout);
            context->set_layout_config(root, Internal::panel_layout(context, root, desc.layout, "popup"));
            GUI::Interactable interactable;
            interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
            set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
            context->set_interactable(root, interactable);
            Internal::set_panel_draw_config(context, root, "popup",
                Float4U(0.08f, 0.10f, 0.13f, 0.96f));
            Ref<Internal::PopupState> state = Internal::popup_state(context, id, true);
            state->flags = desc.flags;
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            Internal::PopupBuildScope scope;
            scope.popup_id = id;
            scope.root = root;
            frame->popup_stack.push_back(scope);
            if(out_handle) *out_handle = root;
            return true;
        }

        LUNA_EDITOR_GUI_API RV end_popup(GUI::IContext* context, const GUI::ElementHandle& popup, const RectF& rect)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->popup_stack.empty() && frame->popup_stack.back().root.id == popup.id);
            Internal::PopupBuildScope scope = frame->popup_stack.back();
            frame->popup_stack.pop_back();
            GUI::FlexLayoutDesc flex;
            flex.axis = GUI::LayoutAxis::y;
            flex.main_axis_gap = Internal::style_scalar(context, popup, "gui.popup.gap", 4.0f);
            Internal::set_flex_layout(context, popup, flex, GUI::LayoutAxis::y);
            context->pop_layer();
            lutry
            {
                luexp(context->apply_layout(popup, rect));
                Ref<Internal::PopupState> state = Internal::popup_state(context, scope.popup_id, true);
                Internal::PopupAction* action = Internal::allocate_frame<Internal::PopupAction>(context);
                action->id = scope.popup_id;
                action->root = popup;
                action->flags = state->flags;
                Internal::add_action(context, Internal::ActionType::popup, scope.popup_id, action);
            }
            lucatchret;
            return ok;
        }

        LUNA_EDITOR_GUI_API bool begin_tooltip(GUI::IContext* context, id_t id,
            const GUI::ElementHandle& owner, const TooltipDesc& desc, GUI::ElementHandle* out_handle)
        {
            luassert(context && id);
            Ref<Internal::TooltipState> state = Internal::widget_state<Internal::TooltipState>(context, id);
            GUI::InteractionState interaction = context->get_interaction_state(owner.id);
            bool hovered = owner.id && (interaction.hovered || interaction.subtree_hovered);
            if(hovered && state->hovered_owner == owner.id)
            {
                state->hover_time += max(context->get_frame_desc().delta_time, 0.0f);
            }
            else
            {
                state->hovered_owner = hovered ? owner.id : 0;
                state->hover_time = 0.0f;
            }
            if(!hovered || state->hover_time < max(desc.delay, 0.0f))
            {
                if(out_handle) *out_handle = GUI::ElementHandle();
                return false;
            }
            Float2U position = context->get_pointer_position() + desc.offset;
            context->push_layer(id, position);
            context->set_layer_debug_name(id, Name("GUI Tooltip"));
            GUI::ElementHandle root = Internal::begin_element(context,
                Internal::derived_id(id, "tooltip.root"), "Tooltip", desc.layout);
            context->set_layout_config(root, Internal::panel_layout(context, root, desc.layout, "tooltip"));
            Internal::set_panel_draw_config(context, root, "tooltip",
                Float4U(0.05f, 0.06f, 0.07f, 0.96f));
            if(out_handle) *out_handle = root;
            return true;
        }

        LUNA_EDITOR_GUI_API RV end_tooltip(GUI::IContext* context, const GUI::ElementHandle& tooltip,
            const RectF& rect)
        {
            luassert(context);
            GUI::FlexLayoutDesc flex;
            flex.axis = GUI::LayoutAxis::y;
            flex.main_axis_gap = Internal::style_scalar(context, tooltip, "gui.tooltip.gap", 4.0f);
            Internal::set_flex_layout(context, tooltip, flex, GUI::LayoutAxis::y);
            context->pop_layer();
            return context->apply_layout(tooltip, rect);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle set_item_tooltip(GUI::IContext* context, id_t id,
            const GUI::ElementHandle& owner, const c8* content, const TooltipDesc& desc)
        {
            GUI::ElementHandle root;
            if(!begin_tooltip(context, id, owner, desc, &root)) return root;
            usize length = content ? strlen(content) : 0;
            f32 width = min(max((f32)length * 8.0f + 20.0f, 80.0f), max(desc.max_width, 80.0f));
            usize characters_per_line = max((usize)((width - 14.0f) / 8.0f), (usize)1);
            usize line_count = max((length + characters_per_line - 1) / characters_per_line, (usize)1);
            f32 height = max((f32)line_count * 20.0f + 14.0f, 36.0f);
            GUI::LayoutConfig text_layout;
            text_layout.width.kind = GUI::SizeKind::fixed;
            text_layout.width.value = width - 14.0f;
            text_layout.height.kind = GUI::SizeKind::fixed;
            text_layout.height.value = height - 14.0f;
            text(context, Internal::derived_id(id, "tooltip.text"), content ? content : "", text_layout);
            lupanic_if_failed(end_tooltip(context, root, RectF(0.0f, 0.0f, width, height)));
            return root;
        }
    }
}
