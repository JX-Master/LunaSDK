/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Menus.cpp
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
        struct MenuItemDrawData
        {
            c8* label = nullptr;
            c8* shortcut = nullptr;
            bool selected = false;
            bool enabled = true;
            bool top_level = false;
            bool submenu = false;
            bool open = false;
            bool hovered = false;
            bool active = false;
            bool draw_text = true;
        };

        static void draw_menu_rect(GUI::IContext* context, const Float4U& color,
            GUI::paint_order_id_t paint_order_id, f32 radius = 0.0f)
        {
            GUI::DrawCommand command;
            command.type = radius > 0.0f ? GUI::DrawCommandType::rounded_rect : GUI::DrawCommandType::rect;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.color = color;
            command.radius = radius;
            context->draw(command, paint_order_id);
        }

        static void draw_menu_text(GUI::IContext* context, const RectF& rect, const c8* value,
            const Float4U& color, f32 size, GUI::paint_order_id_t paint_order_id,
            VG::TextAlignment alignment = VG::TextAlignment::begin)
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::text;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = value ? value : "";
            context->draw(command, paint_order_id);
        }

        static GUI::LayoutConfig menu_item_layout(GUI::IContext* context,
            const GUI::LayoutConfig& source, bool top_level, const c8* label)
        {
            GUI::LayoutConfig result = source;
            GUI::ElementHandle style_element;
            f32 height = Internal::style_scalar(context, style_element,
                top_level ? "gui.control.height" : "gui.control.small_height", top_level ? 32.0f : 28.0f);
            f32 font_size = Internal::style_scalar(context, style_element, "gui.menu_item.font_size", 13.0f);
            if(result.width.kind == GUI::SizeKind::fit)
            {
                result.width.kind = top_level ? GUI::SizeKind::fixed : GUI::SizeKind::percent;
                f32 horizontal_padding = top_level ? max(height * 0.32f, 10.0f) : 30.0f;
                result.width.value = top_level ? max(height, (f32)(label ? strlen(label) : 0) *
                    font_size * 0.58f + horizontal_padding * 2.0f) : 1.0f;
            }
            if(result.height.kind == GUI::SizeKind::fit)
            {
                result.height.kind = GUI::SizeKind::fixed;
                result.height.value = height;
            }
            return result;
        }

        static R<GUI::paint_order_id_t> draw_menu_item(GUI::IContext* context,
            const GUI::ElementHandle& element, GUI::DrawPhase, GUI::paint_order_id_t paint_order_id,
            void* userdata)
        {
            MenuItemDrawData* data = (MenuItemDrawData*)userdata;
            if(!data) return paint_order_id;
            if(data->enabled && (data->open || data->hovered || data->active))
            {
                draw_menu_rect(context, Internal::style_color(context, element,
                    data->active || data->open ? "gui.menu_item.active" : "gui.menu_item.hovered",
                    data->active || data->open ? Float4U(0.18f, 0.36f, 0.62f, 1.0f) :
                    Float4U(0.14f, 0.22f, 0.32f, 1.0f)),
                    paint_order_id, Internal::style_scalar(context, element, "gui.menu_item.radius", 3.0f));
            }
            if(!data->draw_text) return paint_order_id;
            Float4U color = Internal::style_color(context, element,
                data->enabled ? "gui.menu_item.text" : "gui.menu_item.text_disabled",
                data->enabled ? Float4U(0.95f, 0.96f, 0.98f, 1.0f) : Float4U(0.50f, 0.56f, 0.64f, 1.0f));
            f32 size = Internal::style_scalar(context, element, "gui.menu_item.font_size", 15.0f);
            f32 left = data->top_level ? 10.0f : 30.0f;
            draw_menu_text(context, RectF(left, 0.0f, data->top_level ? -20.0f : -94.0f, 0.0f),
                data->label, color, size, paint_order_id + 1);
            if(data->selected && !data->top_level) draw_menu_text(context,
                RectF(6.0f, 0.0f, 20.0f, 0.0f), "v", color, size, paint_order_id + 1,
                VG::TextAlignment::center);
            if(data->submenu && !data->top_level) draw_menu_text(context,
                RectF(-26.0f, 0.0f, 20.0f, 0.0f), ">", color, size, paint_order_id + 1,
                VG::TextAlignment::center);
            else if(data->shortcut && data->shortcut[0] && !data->top_level) draw_menu_text(context,
                RectF(-88.0f, 0.0f, 78.0f, 0.0f), data->shortcut, color, size - 1.0f,
                paint_order_id + 1, VG::TextAlignment::end);
            return paint_order_id + 1;
        }

        static void configure_menu_item_draw(GUI::IContext* context, const GUI::ElementHandle& element,
            const c8* label, const c8* shortcut, bool selected, bool enabled, bool top_level,
            bool submenu, bool open, const GUI::InteractionState& interaction, bool draw_text = true)
        {
            MenuItemDrawData* data = Internal::allocate_frame<MenuItemDrawData>(context);
            data->label = Internal::copy_frame_string(context, label);
            data->shortcut = shortcut ? Internal::copy_frame_string(context, shortcut) : nullptr;
            data->selected = selected;
            data->enabled = enabled;
            data->top_level = top_level;
            data->submenu = submenu;
            data->open = open;
            data->hovered = interaction.hovered;
            data->active = interaction.active;
            data->draw_text = draw_text;
            GUI::DrawConfig draw;
            draw.name = Name("gui.menu_item");
            draw.callback = draw_menu_item;
            draw.userdata = data;
            context->set_draw_config(element, draw);
        }

        static void close_menu_stack(GUI::IContext* context)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            for(const Internal::PopupBuildScope& scope : frame->popup_stack) close_popup(context, scope.popup_id);
        }

        static GUI::ElementHandle begin_menu_item_container(GUI::IContext* context, id_t id,
            const c8* label, const MenuItemDesc& desc, const GUI::LayoutConfig& layout)
        {
            GUI::InteractionState interaction = context->get_interaction_state(id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            if(interaction.hovered && !frame->popup_stack.empty())
            {
                frame->popup_stack.back().hovered_menu_item = id;
            }
            if(desc.enabled && interaction.clicked) close_menu_stack(context);

            GUI::LayoutConfig resolved_layout = menu_item_layout(context, layout, false, label);
            if(resolved_layout.padding.x == 0.0f && resolved_layout.padding.y == 0.0f &&
                resolved_layout.padding.z == 0.0f && resolved_layout.padding.w == 0.0f)
            {
                f32 horizontal_padding = Internal::style_scalar(context, GUI::ElementHandle(),
                    "gui.menu_item.padding_x", 10.0f);
                resolved_layout.padding = Float4U(horizontal_padding, 0.0f, horizontal_padding, 0.0f);
            }
            GUI::ElementHandle item = Internal::begin_element(context, id,
                label ? label : "Menu Item", resolved_layout);
            Internal::set_interactable(context, item, desc.enabled);
            configure_menu_item_draw(context, item, nullptr, nullptr, false, desc.enabled,
                false, false, false, interaction, false);
            GUI::FlexLayoutDesc* flex = Internal::allocate_frame<GUI::FlexLayoutDesc>(context);
            flex->axis = GUI::LayoutAxis::x;
            flex->main_axis_gap = Internal::style_scalar(context, item, "gui.control.content_gap", 8.0f);
            flex->cross_alignment = GUI::FlexAlignment::center;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.menu_item");
            callbacks.measure_callback = GUI::measure_flex;
            callbacks.callback = GUI::layout_flex;
            callbacks.userdata = flex;
            context->set_layout_callback_config(item, callbacks);
            return item;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_menu_bar(GUI::IContext* context, id_t id,
            const c8* label, const GUI::LayoutConfig& layout, const MenuBarDesc& desc)
        {
            GUI::LayoutConfig resolved_layout = layout;
            if(resolved_layout.height.kind == GUI::SizeKind::fit)
            {
                resolved_layout.height.kind = GUI::SizeKind::fixed;
                resolved_layout.height.value = Internal::style_scalar(context, GUI::ElementHandle(),
                    "gui.control.height", 32.0f);
            }
            GUI::ElementHandle bar = Internal::begin_element(context, id, label ? label : "Menu Bar",
                resolved_layout);
            context->set_child_paint_order_mode(bar, GUI::ChildPaintOrderMode::shared);
            GUI::ElementVisualEffect visual;
            visual.command.type = GUI::DrawCommandType::rect;
            visual.command.rect_reference = GUI::DrawCommandRectReference::element;
            visual.command.color = Internal::style_color(context, bar, "gui.menu_bar.background",
                Float4U(0.08f, 0.10f, 0.13f, 1.0f));
            GUI::ElementVisualConfig visual_config;
            visual_config.before_children = Span<const GUI::ElementVisualEffect>(&visual, 1);
            lupanic_if_failed(context->set_element_visual_config(bar, visual_config));
            Internal::MenuBarBuildScope scope;
            scope.root = bar;
            scope.gap = desc.gap >= 0.0f ? desc.gap :
                Internal::style_scalar(context, bar, "gui.menu_bar.gap", 4.0f);
            Internal::frame_state(context)->menu_bar_stack.push_back(scope);
            return bar;
        }

        LUNA_EDITOR_GUI_API void end_menu_bar(GUI::IContext* context, const GUI::ElementHandle& menu_bar)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->menu_bar_stack.empty() && frame->menu_bar_stack.back().root.id == menu_bar.id);
            Internal::MenuBarBuildScope scope = frame->menu_bar_stack.back();
            frame->menu_bar_stack.pop_back();
            GUI::FlexLayoutDesc flex;
            flex.axis = GUI::LayoutAxis::x;
            flex.main_axis_gap = scope.gap;
            flex.cross_alignment = GUI::FlexAlignment::stretch;
            Internal::set_flex_layout(context, menu_bar, flex, GUI::LayoutAxis::x);
        }

        LUNA_EDITOR_GUI_API bool begin_menu(GUI::IContext* context, id_t id, const c8* label,
            const MenuItemDesc& desc, GUI::ElementHandle* out_handle, const GUI::LayoutConfig& layout)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            bool top_level = !frame->menu_bar_stack.empty() && frame->popup_stack.empty();
            id_t popup_id = Internal::derived_id(id, "menu.popup");
            Ref<Internal::PopupPlacementState> placement = Internal::widget_state<Internal::PopupPlacementState>(context,
                popup_id);
            GUI::InteractionState interaction = context->get_interaction_state(id);
            Ref<Internal::MenuBarState> menu_bar_state;
            if(top_level)
            {
                menu_bar_state = Internal::widget_state<Internal::MenuBarState>(context,
                    frame->menu_bar_stack.back().root.id);
                if(menu_bar_state->active_popup && !is_popup_open(context, menu_bar_state->active_popup))
                {
                    menu_bar_state->active_popup = 0;
                }
            }
            else if(interaction.hovered && !frame->popup_stack.empty())
            {
                frame->popup_stack.back().hovered_menu_item = id;
            }
            if(!top_level && !interaction.hovered && !frame->popup_stack.empty() &&
                frame->popup_stack.back().hovered_menu_item &&
                frame->popup_stack.back().hovered_menu_item != id)
            {
                close_popup(context, popup_id);
            }
            bool switch_top_level = top_level && interaction.hovered && menu_bar_state->active_popup &&
                menu_bar_state->active_popup != popup_id;
            if(desc.enabled && (interaction.clicked || switch_top_level || (!top_level && interaction.hovered)))
            {
                bool should_open = !is_popup_open(context, popup_id) || (!top_level && interaction.hovered);
                if(switch_top_level)
                {
                    close_popup(context, menu_bar_state->active_popup);
                    should_open = true;
                }
                if(should_open)
                {
                    const Float2U& pointer = interaction.clicked ? interaction.clicked_screen_position :
                        interaction.pointer_screen_position;
                    const Float2U& local = interaction.clicked ? interaction.clicked_element_position :
                        interaction.pointer_element_position;
                    const RectF& rect = interaction.clicked ? interaction.clicked_element_rect :
                        interaction.pointer_element_rect;
                    placement->position = top_level ? Float2U(pointer.x - local.x, pointer.y - local.y + rect.height) :
                        Float2U(pointer.x - local.x + rect.width, pointer.y - local.y);
                    open_popup(context, popup_id);
                    if(top_level) menu_bar_state->active_popup = popup_id;
                }
                else if(interaction.clicked)
                {
                    close_popup(context, popup_id);
                    if(top_level) menu_bar_state->active_popup = 0;
                }
            }
            bool open = is_popup_open(context, popup_id);
            GUI::ElementHandle item = Internal::begin_element(context, id, label ? label : "Menu",
                menu_item_layout(context, layout, top_level, label));
            Internal::set_interactable(context, item, desc.enabled);
            configure_menu_item_draw(context, item, label, nullptr, false, desc.enabled, top_level,
                true, open, interaction);
            context->end_element();
            if(out_handle) *out_handle = item;
            if(!open) return false;

            PopupDesc popup_desc;
            popup_desc.position = placement->position;
            popup_desc.layout.width.kind = GUI::SizeKind::fixed;
            popup_desc.layout.width.value = 220.0f;
            popup_desc.layout.height.kind = GUI::SizeKind::fixed;
            popup_desc.layout.height.value = 260.0f;
            return begin_popup(context, popup_id, popup_desc);
        }

        LUNA_EDITOR_GUI_API RV end_menu(GUI::IContext* context, const RectF& rect)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->popup_stack.empty());
            return end_popup(context, frame->popup_stack.back().root, rect);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_menu_item(GUI::IContext* context, id_t id,
            const c8* label, const MenuItemDesc& desc, const GUI::LayoutConfig& layout)
        {
            luassert(context && id);
            return begin_menu_item_container(context, id, label, desc, layout);
        }

        LUNA_EDITOR_GUI_API void end_menu_item(GUI::IContext* context)
        {
            luassert(context);
            context->end_element();
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle menu_item(GUI::IContext* context, id_t id, const c8* label,
            bool selected, const MenuItemDesc& desc, const GUI::LayoutConfig& layout)
        {
            GUI::InteractionState interaction = context->get_interaction_state(id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            if(interaction.hovered && !frame->popup_stack.empty())
            {
                frame->popup_stack.back().hovered_menu_item = id;
            }
            if(desc.enabled && interaction.clicked) close_menu_stack(context);
            GUI::ElementHandle item = Internal::begin_element(context, id, label ? label : "Menu Item",
                menu_item_layout(context, layout, false, label));
            Internal::set_interactable(context, item, desc.enabled);
            configure_menu_item_draw(context, item, label, desc.shortcut, selected, desc.enabled,
                false, false, false, interaction);
            context->end_element();
            return item;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle menu_item(GUI::IContext* context, id_t id, const c8* label,
            bool* selected, const MenuItemDesc& desc, const GUI::LayoutConfig& layout)
        {
            if(selected && desc.enabled && context->get_interaction_state(id).clicked) *selected = !*selected;
            return menu_item(context, id, label, selected && *selected, desc, layout);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle menu_separator(GUI::IContext* context, id_t id,
            const GUI::LayoutConfig& layout)
        {
            GUI::LayoutConfig resolved = layout;
            if(resolved.width.kind == GUI::SizeKind::fit)
            {
                resolved.width.kind = GUI::SizeKind::percent;
                resolved.width.value = 1.0f;
            }
            if(resolved.height.kind == GUI::SizeKind::fit)
            {
                resolved.height.kind = GUI::SizeKind::fixed;
                resolved.height.value = Internal::style_scalar(context, GUI::ElementHandle(),
                    "gui.control.small_height", 28.0f) * 0.32f;
            }
            GUI::ElementHandle separator = Internal::begin_element(context, id, "Menu Separator", resolved);
            GUI::ElementVisualEffect visual;
            visual.command.type = GUI::DrawCommandType::rect;
            visual.command.rect_reference = GUI::DrawCommandRectReference::element;
            visual.command.rect = RectF(8.0f, 0.0f, -16.0f, 1.0f);
            visual.command.rect_layout_scale = Float4U(0.0f, 0.5f, 1.0f, 0.0f);
            visual.command.color = Internal::style_color(context, separator,
                "gui.menu_separator.color", Float4U(0.24f, 0.30f, 0.38f, 1.0f));
            GUI::ElementVisualConfig visual_config;
            visual_config.before_children = Span<const GUI::ElementVisualEffect>(&visual, 1);
            lupanic_if_failed(context->set_element_visual_config(separator, visual_config));
            context->end_element();
            return separator;
        }
    }
}
