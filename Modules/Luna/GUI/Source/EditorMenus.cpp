/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorMenus.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "EditorInternal.hpp"
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static bool valid_core_element(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            if(!context || !element.id || element.generation != context->generation())
            {
                return false;
            }
            const GUICore::Element* core_element = context->get_element(element.index);
            return core_element && core_element->id == element.id;
        }

        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_relative_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
            const Float4U& color, f32 width, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.rect_layout_scale = scale;
            command.color = color;
            command.line_width = width;
            context->draw(command);
        }

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = horizontal_alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static GUICore::LayoutConfig fixed_layout(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            if(width > 0.0f)
            {
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = width;
            }
            if(height > 0.0f)
            {
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
            }
            return layout;
        }

        static u64 hash_u64_local(u64 value, u64 h = 14695981039346656037ull)
        {
            const byte_t* p = (const byte_t*)&value;
            for(usize i = 0; i < sizeof(value); ++i)
            {
                h ^= (u64)p[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        static u64 hash_cstr_local(const c8* str, u64 h)
        {
            if(str)
            {
                while(*str)
                {
                    h ^= (u64)(byte_t)*str;
                    h *= 1099511628211ull;
                    ++str;
                }
            }
            return h;
        }

        static GUICore::id_t derived_id(GUICore::id_t id, const c8* salt)
        {
            return hash_cstr_local(salt, hash_u64_local(id));
        }

        static Ref<CoreMenuPopupState> menu_popup_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<CoreMenuPopupState>(id);
            Ref<CoreMenuPopupState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreMenuPopupState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static Ref<CoreMenuBuildState> menu_build_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreMenuBuildState>(0);
            Ref<CoreMenuBuildState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreMenuBuildState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        LUNA_GUI_API RV layout_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar,
            const RectF& rect);

        static RV menu_bar_layout_callback(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar,
            const RectF& rect, void*)
        {
            return layout_menu_bar(context, menu_bar, rect);
        }

        static RV defer_menu_bar_layout(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar)
        {
            if(!valid_core_element(context, menu_bar))
            {
                return BasicError::bad_arguments();
            }
            GUICore::LayoutCallbackConfig config;
            config.algorithm = Name("gui.editor.menu_bar");
            config.callback = menu_bar_layout_callback;
            config.finalize_callback = nullptr;
            config.userdata = nullptr;
            context->set_layout_callback_config(menu_bar, config);
            return ok;
        }

        static GUICore::id_t menu_popup_id(GUICore::id_t id)
        {
            return derived_id(id, "menu_popup");
        }

        static f32 menu_label_width(GUICore::IContext* context, const c8* label, bool top_level, bool has_submenu,
            bool has_shortcut)
        {
            f32 font_size = style_value(context, Name("gui.editor.menu_item.font_size"),
                GUICore::style_f32(15.0f)).number.x;
            f32 check_width = top_level ? 0.0f : style_value(context, Name("gui.editor.menu_item.check_width"),
                GUICore::style_f32(30.0f)).number.x;
            f32 right_width = top_level ? 20.0f : (has_submenu ? 36.0f : (has_shortcut ? 84.0f : 16.0f));
            return max((f32)(label ? strlen(label) : 0) * font_size * 0.54f + check_width + right_width, top_level ? 42.0f : 120.0f);
        }

        static void set_menu_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element, bool enabled)
        {
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            set_flags(interactable.flags, GUICore::InteractableFlag::disabled, !enabled);
            context->set_interactable(element, interactable);
        }

        static void draw_menu_item_chrome(GUICore::IContext* context, const c8* label, const c8* shortcut,
            bool selected, bool enabled, bool top_level, bool submenu, bool open, const GUICore::InteractionState& interaction)
        {
            bool active = enabled && (open || interaction.active);
            bool hovered = enabled && interaction.hovered;
            if(active || hovered)
            {
                Float4U background = style_value(context, active ? Name("gui.editor.menu_item.background_active") :
                    Name("gui.editor.menu_item.background_hovered"), active ?
                    GUICore::style_f32x4(Float4U(0.18f, 0.36f, 0.62f, 1.0f)) :
                    GUICore::style_f32x4(Float4U(0.14f, 0.22f, 0.32f, 1.0f))).number;
                f32 radius = style_value(context, top_level ? Name("gui.editor.menu_item.top_level_radius") :
                    Name("gui.editor.menu_item.radius"), GUICore::style_f32(top_level ? 4.0f : 3.0f)).number.x;
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                    background, radius);
            }
            Float4U text_color = style_value(context, enabled ? Name("gui.editor.menu_item.text_color") :
                Name("gui.editor.menu_item.text_disabled"), enabled ?
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.50f, 0.56f, 0.64f, 1.0f))).number;
            f32 check_width = style_value(context, Name("gui.editor.menu_item.check_width"), GUICore::style_f32(30.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.menu_item.font_size"), GUICore::style_f32(15.0f)).number.x;
            f32 text_x = top_level ? 10.0f : check_width;
            f32 text_pad_right = top_level ? 20.0f : (submenu ? 46.0f : 74.0f);
            draw_relative_text(context, RectF(text_x, 0.0f, -text_x - text_pad_right, 0.0f), label, text_color, font_size);
            if(selected && !top_level)
            {
                draw_relative_line(context, Float2U(9.0f, 0.0f), Float2U(14.0f, 0.0f), text_color, 2.0f,
                    Float4U(0.0f, 0.56f, 0.0f, 0.72f));
                draw_relative_line(context, Float2U(14.0f, 0.0f), Float2U(24.0f, 0.0f), text_color, 2.0f,
                    Float4U(0.0f, 0.72f, 0.0f, 0.32f));
            }
            if(submenu && !top_level)
            {
                draw_relative_text(context, RectF(-24.0f, 0.0f, 18.0f, 0.0f), ">",
                    text_color, font_size, VG::TextAlignment::center);
            }
            else if(shortcut && shortcut[0] && !top_level)
            {
                f32 shortcut_size = style_value(context, Name("gui.editor.menu_item.shortcut_font_size"),
                    GUICore::style_f32(14.0f)).number.x;
                f32 shortcut_alpha = style_value(context, Name("gui.editor.menu_item.shortcut_alpha"),
                    GUICore::style_f32(0.72f)).number.x;
                Float4U shortcut_color = Float4U(text_color.x, text_color.y, text_color.z, text_color.w * shortcut_alpha);
                draw_relative_text(context, RectF(-88.0f, 0.0f, 80.0f, 0.0f), shortcut,
                    shortcut_color, shortcut_size, VG::TextAlignment::end);
            }
        }

        static void close_open_menus(GUICore::IContext* context)
        {
            Ref<CoreMenuBuildState> build_state = menu_build_state(context);
            for(const CoreMenuBuildScope& scope : build_state->menu_stack)
            {
                close_popup(context, scope.popup_id);
            }
        }

        LUNA_GUI_API GUICore::ElementHandle begin_menu_bar(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle menu_bar = Internal::begin_element(context, id, label ? label : "menu_bar");
            context->set_layout_config(menu_bar, layout);
            Float4U background = style_value(context, Name("gui.editor.menu_bar.background"),
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.75f))).number;
            Float4U border = style_value(context, Name("gui.editor.menu_bar.border"),
                GUICore::style_f32x4(Float4U(0.20f, 0.27f, 0.36f, 1.0f))).number;
            f32 border_width = style_value(context, Name("gui.editor.menu_bar.border_width"),
                GUICore::style_f32(1.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rect, RectF(0.0f, 0.0f, 0.0f, 0.0f), background);
            if(border_width > 0.0f)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rect, RectF(0.0f, -border_width, 0.0f, border_width),
                    border, 0.0f, Float4U(0.0f, 1.0f, 1.0f, 0.0f));
            }
            menu_build_state(context)->menu_bar_stack.push_back(menu_bar);
            return menu_bar;
        }

        LUNA_GUI_API RV end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar, const RectF& rect)
        {
            luassert(context);
            Ref<CoreMenuBuildState> build_state = menu_build_state(context);
            luassert(!build_state->menu_bar_stack.empty());
            build_state->menu_bar_stack.pop_back();
            RV r = defer_menu_bar_layout(context, menu_bar);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(menu_bar, rect);
            }
            return r;
        }

        LUNA_GUI_API RV end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar)
        {
            luassert(context);
            Ref<CoreMenuBuildState> build_state = menu_build_state(context);
            luassert(!build_state->menu_bar_stack.empty());
            build_state->menu_bar_stack.pop_back();
            RV r = defer_menu_bar_layout(context, menu_bar);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV layout_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar, const RectF& rect)
        {
            if(!valid_core_element(context, menu_bar))
            {
                return BasicError::bad_arguments();
            }
            GUICore::LayoutResult result;
            result.rect = rect;
            result.clip_rect = rect;
            result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(menu_bar, result);
            GUICore::FlexLayoutDesc layout_desc;
            layout_desc.axis = GUICore::LayoutAxis::x;
            layout_desc.main_axis_gap = style_value(context, Name("gui.editor.menu_bar.gap"), GUICore::style_f32(4.0f)).number.x;
            return GUICore::layout_flex(context, menu_bar, rect, &layout_desc);
        }

        LUNA_GUI_API bool begin_menu(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool enabled,
            GUICore::ElementHandle* out_handle, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            Ref<CoreMenuBuildState> build_state = menu_build_state(context);
            bool top_level = !build_state->menu_bar_stack.empty() && build_state->menu_stack.empty();
            GUICore::id_t popup_id = menu_popup_id(id);
            Ref<CoreMenuPopupState> state = menu_popup_state(context, popup_id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(enabled && interaction.clicked)
            {
                if(is_popup_open(context, popup_id))
                {
                    close_popup(context, popup_id);
                }
                else
                {
                    f32 item_x = interaction.clicked_screen_position.x - interaction.clicked_element_position.x;
                    f32 item_y = interaction.clicked_screen_position.y - interaction.clicked_element_position.y;
                    state->popup_position = top_level ? Float2U(item_x, item_y + interaction.clicked_element_rect.height) :
                        Float2U(item_x + interaction.clicked_element_rect.width, item_y);
                    open_popup(context, popup_id);
                }
            }

            bool open = is_popup_open(context, popup_id);
            GUICore::LayoutConfig item_layout = layout;
            if(item_layout.width.kind == GUICore::SizeKind::fit)
            {
                item_layout.width.kind = top_level ? GUICore::SizeKind::fixed : GUICore::SizeKind::percent;
                item_layout.width.value = top_level ? menu_label_width(context, label, true, true, false) : 1.0f;
            }
            if(item_layout.height.kind == GUICore::SizeKind::fit)
            {
                item_layout.height.kind = GUICore::SizeKind::fixed;
                item_layout.height.value = top_level ? 24.0f : 26.0f;
            }

            GUICore::ElementHandle item = Internal::begin_element(context, id, label ? label : "menu");
            if(out_handle)
            {
                *out_handle = item;
            }
            context->set_layout_config(item, item_layout);
            set_menu_interactable(context, item, enabled);
            draw_menu_item_chrome(context, label, nullptr, false, enabled, top_level, true, open, interaction);
            context->end_element();

            if(!open)
            {
                return false;
            }
            PopupDesc popup_desc;
            popup_desc.position = state->popup_position;
            popup_desc.layout = fixed_layout(190.0f, 240.0f);
            GUICore::ElementHandle popup;
            if(!begin_popup(context, popup_id, popup_desc, &popup))
            {
                return false;
            }
            CoreMenuBuildScope scope;
            scope.popup_id = popup_id;
            scope.popup_root = popup;
            build_state->menu_stack.push_back(scope);
            return true;
        }

        LUNA_GUI_API RV end_menu(GUICore::IContext* context, const RectF& rect)
        {
            luassert(context);
            Ref<CoreMenuBuildState> build_state = menu_build_state(context);
            luassert(!build_state->menu_stack.empty());
            CoreMenuBuildScope scope = build_state->menu_stack.back();
            build_state->menu_stack.pop_back();
            return end_popup(context, scope.popup_root, rect);
        }

        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const c8* shortcut, bool selected, bool enabled, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(enabled && interaction.clicked)
            {
                close_open_menus(context);
            }
            GUICore::LayoutConfig item_layout = layout;
            if(item_layout.width.kind == GUICore::SizeKind::fit)
            {
                item_layout.width.kind = GUICore::SizeKind::percent;
                item_layout.width.value = 1.0f;
            }
            if(item_layout.height.kind == GUICore::SizeKind::fit)
            {
                item_layout.height.kind = GUICore::SizeKind::fixed;
                item_layout.height.value = 26.0f;
            }
            GUICore::ElementHandle item = Internal::begin_element(context, id, label ? label : "menu_item");
            context->set_layout_config(item, item_layout);
            set_menu_interactable(context, item, enabled);
            draw_menu_item_chrome(context, label, shortcut, selected, enabled, false, false, false, interaction);
            context->end_element();
            return item;
        }

        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const c8* shortcut, bool* selected, bool enabled, const GUICore::LayoutConfig& layout)
        {
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(enabled && selected && interaction.clicked)
            {
                *selected = !*selected;
            }
            return menu_item(context, id, label, shortcut, selected && *selected, enabled, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle menu_separator(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::LayoutConfig separator_layout = layout;
            if(separator_layout.width.kind == GUICore::SizeKind::fit)
            {
                separator_layout.width.kind = GUICore::SizeKind::percent;
                separator_layout.width.value = 1.0f;
            }
            if(separator_layout.height.kind == GUICore::SizeKind::fit)
            {
                separator_layout.height.kind = GUICore::SizeKind::fixed;
                separator_layout.height.value = 7.0f;
            }
            GUICore::ElementHandle separator = Internal::begin_element(context, id, "menu_separator");
            context->set_layout_config(separator, separator_layout);
            f32 padding = style_value(context, Name("gui.editor.menu_separator.padding"),
                GUICore::style_f32(8.0f)).number.x;
            Float4U color = style_value(context, Name("gui.editor.menu_separator.color"),
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f))).number;
            f32 width = style_value(context, Name("gui.editor.menu_separator.width"), GUICore::style_f32(1.0f)).number.x;
            draw_relative_line(context, Float2U(padding, 0.0f), Float2U(-padding, 0.0f), color, width,
                Float4U(0.0f, 0.5f, 1.0f, 0.0f));
            context->end_element();
            return separator;
        }
    }
}
