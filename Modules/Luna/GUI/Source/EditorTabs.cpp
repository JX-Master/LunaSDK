/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorTabs.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            GUICore::Interactable interactable;
            interactable.hit_test = true;
            interactable.hoverable = true;
            interactable.activatable = true;
            interactable.focusable = true;
            context->set_interactable(element, interactable);
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
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

        static RectF intersect_rect(const RectF& a, const RectF& b)
        {
            f32 min_x = max(a.offset_x, b.offset_x);
            f32 min_y = max(a.offset_y, b.offset_y);
            f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
            f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
            return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
        }

        static Ref<TabBarState> tab_bar_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<TabBarState>(id);
            Ref<TabBarState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<TabBarState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static Ref<CoreTabBuildState> tab_build_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreTabBuildState>(0);
            Ref<CoreTabBuildState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreTabBuildState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        static Ref<EditorLayoutPassState> layout_pass_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<EditorLayoutPassState>(0);
            Ref<EditorLayoutPassState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<EditorLayoutPassState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        static RV defer_tab_bar_layout(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar)
        {
            if(!context || !tab_bar.id || tab_bar.context != context->get_object() || tab_bar.generation != context->generation())
            {
                return BasicError::bad_arguments();
            }
            Ref<EditorLayoutPassState> state = layout_pass_state(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::tab_bar;
            state->requests.insert_or_assign(tab_bar.id, move(request));
            lupanic_if_failed(context->set_state(GUICore::make_state_id<EditorLayoutPassState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
            return ok;
        }

        static bool tab_header_contains(const TabBarState& state, GUICore::id_t id)
        {
            return tab_order_contains(state, id);
        }

        static f32 tab_header_width(GUICore::IContext* context, const c8* label, bool close_button)
        {
            f32 padding_x = style_value(context, Name("gui.editor.tab.padding_x"), GUICore::style_f32(14.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.tab.font_size"), GUICore::style_f32(15.0f)).number.x;
            f32 close_width = close_button ? 20.0f : 0.0f;
            usize len = label ? strlen(label) : 0;
            return max((f32)len * font_size * 0.52f + padding_x * 2.0f + close_width, 48.0f);
        }

        LUNA_GUI_API GUICore::ElementHandle begin_tab_bar(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, TabBarFlag flags, const GUICore::LayoutInput& layout)
        {
            luassert(context && id);
            (void)flags;
            GUICore::ElementHandle element = context->begin_element(id, label ? Name(label) : Name("tab_bar"));
            context->set_layout(element, layout);

            Float4U background = style_value(context, Name("gui.editor.tab_bar.background"),
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.75f))).number;
            f32 radius = style_value(context, Name("gui.editor.tab_bar.radius"), GUICore::style_f32(4.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                background, radius);

            Ref<TabBarState> state = tab_bar_state(context, id);
            for(GUICore::id_t tab_id : state->tab_order)
            {
                if(context->get_interaction_state(tab_id).clicked)
                {
                    state->tab_selected_id = tab_id;
                    break;
                }
            }
            CoreTabBuildScope scope;
            scope.tab_bar_id = id;
            scope.selected_id = state->tab_selected_id;
            tab_build_state(context)->stack.push_back(scope);
            return element;
        }

        LUNA_GUI_API bool begin_tab_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool* open, TabItemFlag flags, GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            Ref<CoreTabBuildState> build_state = tab_build_state(context);
            luassert(!build_state->stack.empty());
            CoreTabBuildScope& scope = build_state->stack.back();
            bool item_open = !open || *open;
            if(!item_open)
            {
                if(out_handle)
                {
                    *out_handle = GUICore::ElementHandle();
                }
                return false;
            }
            if(!scope.first_open_id && !test_flags(flags, TabItemFlag::button))
            {
                scope.first_open_id = id;
            }

            Ref<TabBarState> state = tab_bar_state(context, scope.tab_bar_id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(test_flags(flags, TabItemFlag::selected) || (interaction.clicked && !test_flags(flags, TabItemFlag::button)))
            {
                scope.selected_id = id;
                state->tab_selected_id = id;
            }
            bool visible = !test_flags(flags, TabItemFlag::button) &&
                ((scope.selected_id && scope.selected_id == id) || (!scope.selected_id && !scope.visible_tab_chosen));
            if(visible)
            {
                scope.visible_tab_chosen = true;
            }
            scope.header_ids.push_back(id);

            GUICore::ElementHandle header = context->begin_element(id, label ? Name(label) : Name("tab_item"));
            if(out_handle)
            {
                *out_handle = header;
            }
            set_basic_interactable(context, header);
            bool selected = visible || (state->tab_selected_id == id);
            Float4U background = style_value(context, selected ? Name("gui.editor.tab.background_selected") :
                (interaction.hovered ? Name("gui.editor.tab.background_hovered") : Name("gui.editor.tab.background")),
                selected ? GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)) :
                (interaction.hovered ? GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.25f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.10f, 0.12f, 0.16f, 1.0f)))).number;
            Float4U text_color = style_value(context, selected ? Name("gui.editor.tab.text_selected") :
                Name("gui.editor.tab.text"), selected ?
                GUICore::style_f32x4(Float4U(1.0f)) :
                GUICore::style_f32x4(Float4U(0.72f, 0.78f, 0.86f, 1.0f))).number;
            f32 radius = style_value(context, Name("gui.editor.tab.radius"), GUICore::style_f32(4.0f)).number.x;
            f32 padding_x = style_value(context, Name("gui.editor.tab.padding_x"), GUICore::style_f32(14.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.tab.font_size"), GUICore::style_f32(15.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(0.0f, 0.0f, 0.0f, 0.0f), background, radius);
            draw_relative_text(context, RectF(padding_x, 0.0f, -padding_x * 2.0f, 0.0f),
                label, text_color, font_size, VG::TextAlignment::center);
            context->end_element();
            return visible;
        }

        LUNA_GUI_API void end_tab_item(GUICore::IContext* context)
        {
            luassert(context);
        }

        static RV finish_tab_bar_build(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar)
        {
            if(!context || !tab_bar.id || tab_bar.context != context->get_object() || tab_bar.generation != context->generation())
            {
                return BasicError::bad_arguments();
            }
            Ref<CoreTabBuildState> build_state = tab_build_state(context);
            luassert(!build_state->stack.empty());
            CoreTabBuildScope scope = build_state->stack.back();
            build_state->stack.pop_back();
            Ref<TabBarState> state = tab_bar_state(context, tab_bar.id);
            if(!scope.visible_tab_chosen && scope.first_open_id)
            {
                state->tab_selected_id = scope.first_open_id;
            }
            else if(scope.selected_id)
            {
                state->tab_selected_id = scope.selected_id;
            }
            state->tab_order = scope.header_ids;
            lupanic_if_failed(context->set_state(GUICore::make_state_id<TabBarState>(tab_bar.id), state.object(),
                GUICore::StateLifetime::next_frame));
            return ok;
        }

        LUNA_GUI_API RV layout_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar, const RectF& rect)
        {
            luassert(context);
            Ref<TabBarState> state = tab_bar_state(context, tab_bar.id);
            f32 header_height = style_value(context, Name("gui.editor.tab_bar.header_height"), GUICore::style_f32(30.0f)).number.x;
            f32 header_x = rect.offset_x;
            const GUICore::Element* bar = context->get_element(tab_bar.index);
            if(bar)
            {
                f32 content_y = rect.offset_y + header_height;
                f32 content_gap = style_value(context, Name("gui.editor.tab_bar.content_gap"), GUICore::style_f32(4.0f)).number.x;
                for(u32 child = bar->first_child; child != GUICore::INVALID_ELEMENT;)
                {
                    const GUICore::Element* child_element = context->get_element(child);
                    if(!child_element)
                    {
                        break;
                    }
                    u32 next = child_element->next_sibling;
                    GUICore::ElementHandle child_handle;
                    child_handle.context = context;
                    child_handle.id = child_element->id;
                    child_handle.index = child;
                    child_handle.generation = context->generation();
                    GUICore::LayoutResult child_layout;
                    if(tab_header_contains(*state.get(), child_element->id))
                    {
                        f32 width = tab_header_width(context, child_element->debug_name.c_str(), false);
                        child_layout.rect = RectF(header_x, rect.offset_y, min(width, max(rect.offset_x + rect.width - header_x, 1.0f)), header_height);
                        child_layout.clip_rect = intersect_rect(child_layout.rect, rect);
                        header_x += width;
                    }
                    else
                    {
                        f32 content_height = 24.0f;
                        if(child_element->layout.height.kind == GUICore::SizeKind::pixels)
                        {
                            content_height = child_element->layout.height.value;
                        }
                        else if(child_element->layout_result.content_size.y > 0.0f)
                        {
                            content_height = child_element->layout_result.content_size.y;
                        }
                        content_height = min(content_height, max(rect.offset_y + rect.height - content_y, 1.0f));
                        child_layout.rect = RectF(rect.offset_x, content_y, rect.width, content_height);
                        child_layout.clip_rect = intersect_rect(child_layout.rect, rect);
                        content_y += content_height + content_gap;
                    }
                    context->set_layout_result(child_handle, child_layout);
                    child = next;
                }
            }

            GUICore::LayoutResult bar_layout;
            bar_layout.rect = rect;
            bar_layout.clip_rect = rect;
            bar_layout.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(tab_bar, bar_layout);
            return ok;
        }

        LUNA_GUI_API RV end_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar)
        {
            luassert(context);
            RV r = finish_tab_bar_build(context, tab_bar);
            if(succeeded(r))
            {
                r = defer_tab_bar_layout(context, tab_bar);
            }
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar, const RectF& rect)
        {
            luassert(context);
            RV r = finish_tab_bar_build(context, tab_bar);
            if(succeeded(r))
            {
                r = layout_tab_bar(context, tab_bar, rect);
            }
            context->end_element();
            return r;
        }
    }
}
