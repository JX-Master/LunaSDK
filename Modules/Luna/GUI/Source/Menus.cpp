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
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        static void draw_menu_rect(GUICore::IContext* context, const Float4U& color, f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = radius > 0.0f ? GUICore::DrawCommandType::rounded_rect : GUICore::DrawCommandType::rect;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_menu_text(GUICore::IContext* context, const RectF& rect, const c8* value,
            const Float4U& color, f32 size, VG::TextAlignment alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = value ? value : "";
            context->draw(command);
        }

        static GUICore::LayoutConfig menu_item_layout(const GUICore::LayoutConfig& source, bool top_level,
            const c8* label)
        {
            GUICore::LayoutConfig result = source;
            if(result.width.kind == GUICore::SizeKind::fit)
            {
                result.width.kind = top_level ? GUICore::SizeKind::fixed : GUICore::SizeKind::percent;
                result.width.value = top_level ? max(48.0f, (f32)(label ? strlen(label) : 0) * 8.5f + 24.0f) : 1.0f;
            }
            if(result.height.kind == GUICore::SizeKind::fit)
            {
                result.height.kind = GUICore::SizeKind::fixed;
                result.height.value = top_level ? 30.0f : 28.0f;
            }
            return result;
        }

        static void draw_menu_item(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const c8* label, const c8* shortcut, bool selected, bool enabled, bool top_level,
            bool submenu, bool open, const GUICore::InteractionState& interaction)
        {
            if(enabled && (open || interaction.hovered || interaction.active))
            {
                draw_menu_rect(context, Internal::style_color(context, element,
                    interaction.active || open ? "gui.menu_item.active" : "gui.menu_item.hovered",
                    interaction.active || open ? Float4U(0.18f, 0.36f, 0.62f, 1.0f) :
                    Float4U(0.14f, 0.22f, 0.32f, 1.0f)),
                    Internal::style_scalar(context, element, "gui.menu_item.radius", 3.0f));
            }
            Float4U color = Internal::style_color(context, element,
                enabled ? "gui.menu_item.text" : "gui.menu_item.text_disabled",
                enabled ? Float4U(0.95f, 0.96f, 0.98f, 1.0f) : Float4U(0.50f, 0.56f, 0.64f, 1.0f));
            f32 size = Internal::style_scalar(context, element, "gui.menu_item.font_size", 15.0f);
            f32 left = top_level ? 10.0f : 30.0f;
            draw_menu_text(context, RectF(left, 0.0f, top_level ? -20.0f : -94.0f, 0.0f), label, color, size);
            if(selected && !top_level) draw_menu_text(context, RectF(6.0f, 0.0f, 20.0f, 0.0f), "v", color, size,
                VG::TextAlignment::center);
            if(submenu && !top_level) draw_menu_text(context, RectF(-26.0f, 0.0f, 20.0f, 0.0f), ">", color, size,
                VG::TextAlignment::center);
            else if(shortcut && shortcut[0] && !top_level) draw_menu_text(context,
                RectF(-88.0f, 0.0f, 78.0f, 0.0f), shortcut, color, size - 1.0f, VG::TextAlignment::end);
        }

        static void close_menu_stack(GUICore::IContext* context)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            for(const Internal::PopupBuildScope& scope : frame->popup_stack) close_popup(context, scope.popup_id);
        }

        LUNA_GUI_API GUICore::ElementHandle begin_menu_bar(GUICore::IContext* context, id_t id,
            const c8* label, const GUICore::LayoutConfig& layout, const MenuBarDesc& desc)
        {
            GUICore::ElementHandle bar = Internal::begin_element(context, id, label ? label : "Menu Bar", layout);
            draw_menu_rect(context, Internal::style_color(context, bar, "gui.menu_bar.background",
                Float4U(0.08f, 0.10f, 0.13f, 1.0f)));
            Internal::MenuBarBuildScope scope;
            scope.root = bar;
            scope.gap = desc.gap;
            Internal::frame_state(context)->menu_bar_stack.push_back(scope);
            return bar;
        }

        LUNA_GUI_API void end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->menu_bar_stack.empty() && frame->menu_bar_stack.back().root.id == menu_bar.id);
            Internal::MenuBarBuildScope scope = frame->menu_bar_stack.back();
            frame->menu_bar_stack.pop_back();
            GUICore::FlexLayoutDesc flex;
            flex.axis = GUICore::LayoutAxis::x;
            flex.main_axis_gap = scope.gap;
            flex.cross_alignment = GUICore::FlexAlignment::stretch;
            Internal::set_flex_layout(context, menu_bar, flex, GUICore::LayoutAxis::x);
        }

        LUNA_GUI_API bool begin_menu(GUICore::IContext* context, id_t id, const c8* label,
            const MenuItemDesc& desc, GUICore::ElementHandle* out_handle, const GUICore::LayoutConfig& layout)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            bool top_level = !frame->menu_bar_stack.empty() && frame->popup_stack.empty();
            id_t popup_id = Internal::derived_id(id, "menu.popup");
            Ref<Internal::PopupPlacementState> placement = Internal::widget_state<Internal::PopupPlacementState>(context,
                popup_id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
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
            GUICore::ElementHandle item = Internal::begin_element(context, id, label ? label : "Menu",
                menu_item_layout(layout, top_level, label));
            Internal::set_interactable(context, item, desc.enabled);
            draw_menu_item(context, item, label, nullptr, false, desc.enabled, top_level, true, open, interaction);
            context->end_element();
            if(out_handle) *out_handle = item;
            if(!open) return false;

            PopupDesc popup_desc;
            popup_desc.position = placement->position;
            popup_desc.layout.width.kind = GUICore::SizeKind::fixed;
            popup_desc.layout.width.value = 220.0f;
            popup_desc.layout.height.kind = GUICore::SizeKind::fixed;
            popup_desc.layout.height.value = 260.0f;
            return begin_popup(context, popup_id, popup_desc);
        }

        LUNA_GUI_API RV end_menu(GUICore::IContext* context, const RectF& rect)
        {
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->popup_stack.empty());
            return end_popup(context, frame->popup_stack.back().root, rect);
        }

        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool selected, const MenuItemDesc& desc, const GUICore::LayoutConfig& layout)
        {
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            if(interaction.hovered && !frame->popup_stack.empty())
            {
                frame->popup_stack.back().hovered_menu_item = id;
            }
            if(desc.enabled && interaction.clicked) close_menu_stack(context);
            GUICore::ElementHandle item = Internal::begin_element(context, id, label ? label : "Menu Item",
                menu_item_layout(layout, false, label));
            Internal::set_interactable(context, item, desc.enabled);
            draw_menu_item(context, item, label, desc.shortcut, selected, desc.enabled, false, false, false, interaction);
            context->end_element();
            return item;
        }

        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool* selected, const MenuItemDesc& desc, const GUICore::LayoutConfig& layout)
        {
            if(selected && desc.enabled && context->get_interaction_state(id).clicked) *selected = !*selected;
            return menu_item(context, id, label, selected && *selected, desc, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle menu_separator(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout)
        {
            GUICore::LayoutConfig resolved = layout;
            if(resolved.width.kind == GUICore::SizeKind::fit)
            {
                resolved.width.kind = GUICore::SizeKind::percent;
                resolved.width.value = 1.0f;
            }
            if(resolved.height.kind == GUICore::SizeKind::fit)
            {
                resolved.height.kind = GUICore::SizeKind::fixed;
                resolved.height.value = 9.0f;
            }
            GUICore::ElementHandle separator = Internal::begin_element(context, id, "Menu Separator", resolved);
            GUICore::DrawCommand line;
            line.type = GUICore::DrawCommandType::rect;
            line.rect_reference = GUICore::DrawCommandRectReference::element;
            line.rect = RectF(8.0f, 0.0f, -16.0f, 1.0f);
            line.rect_layout_scale = Float4U(0.0f, 0.5f, 1.0f, 0.0f);
            line.color = Internal::style_color(context, separator, "gui.menu_separator.color",
                Float4U(0.24f, 0.30f, 0.38f, 1.0f));
            context->draw(line);
            context->end_element();
            return separator;
        }
    }
}
