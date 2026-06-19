/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorLayouts.cpp
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

        static RV set_deferred_layout_request(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            EditorLayoutRequest&& request)
        {
            if(!context || !layout.id || layout.context != context->get_object() || layout.generation != context->generation())
            {
                return BasicError::bad_arguments();
            }
            Ref<EditorLayoutPassState> state = layout_pass_state(context);
            state->requests.insert_or_assign(layout.id, move(request));
            lupanic_if_failed(context->set_state(GUICore::make_state_id<EditorLayoutPassState>(0), state.object(),
                GUICore::StateLifetime::current_frame));
            return ok;
        }

        static Vector<u32> collect_children(GUICore::IContext* context, const GUICore::Element& parent)
        {
            Vector<u32> children;
            for(u32 child = parent.first_child; child != GUICore::INVALID_ELEMENT;)
            {
                const GUICore::Element* child_element = context->get_element(child);
                if(!child_element)
                {
                    break;
                }
                children.push_back(child);
                child = child_element->next_sibling;
            }
            return children;
        }

        static Ref<CoreTableBuildState> table_build_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreTableBuildState>(0);
            Ref<CoreTableBuildState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreTableBuildState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        static CoreTableBuildScope* current_table_scope(GUICore::IContext* context)
        {
            Ref<CoreTableBuildState> state = table_build_state(context);
            if(state->stack.empty())
            {
                return nullptr;
            }
            return &state->stack.back();
        }

        static void pop_table_scope(GUICore::IContext* context, GUICore::id_t table_id)
        {
            Ref<CoreTableBuildState> state = table_build_state(context);
            if(!state->stack.empty() && state->stack.back().table_id == table_id)
            {
                state->stack.pop_back();
            }
        }

        static void set_table_request_from_scope(EditorLayoutRequest& request, const CoreTableBuildScope& scope,
            Span<const GUICore::TableTrackDesc> columns)
        {
            request.kind = EditorLayoutRequestKind::table;
            request.table_columns.assign(columns.begin(), columns.end());
            request.table_rows.assign(scope.rows.begin(), scope.rows.end());
            request.table_cells.assign(scope.cells.begin(), scope.cells.end());
            request.table_gap = scope.gap;
            request.table_clip_children = scope.clip_children;
        }

        static Vector<GUICore::TableTrackDesc> default_table_columns(const CoreTableBuildScope& scope)
        {
            Vector<GUICore::TableTrackDesc> columns;
            u32 column_count = scope.columns.empty() ? scope.max_columns : (u32)scope.columns.size();
            columns.resize(column_count);
            for(GUICore::TableTrackDesc& column : columns)
            {
                column.kind = GUICore::TableTrackSizeKind::fit;
            }
            return columns;
        }

        static RV apply_table_scope_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const CoreTableBuildScope& scope)
        {
            Vector<GUICore::TableTrackDesc> default_columns;
            Span<const GUICore::TableTrackDesc> columns(scope.columns.data(), scope.columns.size());
            if(columns.empty())
            {
                default_columns = default_table_columns(scope);
                columns = Span<const GUICore::TableTrackDesc>(default_columns.data(), default_columns.size());
            }
            GUICore::TableLayoutDesc desc;
            desc.columns = columns;
            desc.rows = Span<const GUICore::TableTrackDesc>(scope.rows.data(), scope.rows.size());
            desc.cells = Span<const GUICore::TableLayoutCell>(scope.cells.data(), scope.cells.size());
            desc.gap = scope.gap;
            desc.clip_children = scope.clip_children;
            return GUICore::layout_table(context, layout, rect, desc);
        }

        static RV apply_scroll_view_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::ScrollViewportLayoutDesc desc);

        static RV apply_deferred_layout_request(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const EditorLayoutRequest& request)
        {
            switch(request.kind)
            {
            case EditorLayoutRequestKind::linear:
                return GUICore::layout_linear(context, layout, rect, request.linear);
            case EditorLayoutRequestKind::grid:
                return GUICore::layout_grid(context, layout, rect, request.grid);
            case EditorLayoutRequestKind::stack:
                return GUICore::layout_stack(context, layout, rect, request.stack);
            case EditorLayoutRequestKind::canvas:
            {
                GUICore::CanvasLayoutDesc desc;
                desc.default_item = request.canvas_default_item;
                desc.items = Span<const GUICore::CanvasLayoutItem>(request.canvas_items.data(), request.canvas_items.size());
                desc.clip_children = request.canvas_clip_children;
                return GUICore::layout_canvas(context, layout, rect, desc);
            }
            case EditorLayoutRequestKind::scroll_viewport:
                return GUICore::layout_scroll_viewport(context, layout, rect, request.scroll_viewport);
            case EditorLayoutRequestKind::scroll_view:
                return apply_scroll_view_layout(context, layout, rect, request.scroll_viewport);
            case EditorLayoutRequestKind::table:
            {
                GUICore::TableLayoutDesc desc;
                desc.columns = Span<const GUICore::TableTrackDesc>(request.table_columns.data(), request.table_columns.size());
                desc.rows = Span<const GUICore::TableTrackDesc>(request.table_rows.data(), request.table_rows.size());
                desc.cells = Span<const GUICore::TableLayoutCell>(request.table_cells.data(), request.table_cells.size());
                desc.gap = request.table_gap;
                desc.clip_children = request.table_clip_children;
                return GUICore::layout_table(context, layout, rect, desc);
            }
            case EditorLayoutRequestKind::tab_bar:
                return layout_tab_bar(context, layout, rect);
            case EditorLayoutRequestKind::menu_bar:
                return layout_menu_bar(context, layout, rect);
            default:
                return ok;
            }
        }

        static Float2U clamp_scroll_offset(Float2U scroll, const Float2U& content_size, const Float2U& viewport_size)
        {
            Float2U max_scroll(
                max(content_size.x - viewport_size.x, 0.0f),
                max(content_size.y - viewport_size.y, 0.0f));
            scroll.x = clamp(scroll.x, 0.0f, max_scroll.x);
            scroll.y = clamp(scroll.y, 0.0f, max_scroll.y);
            return scroll;
        }

        static Float2U scroll_viewport_content_size(const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return Float2U(
                max(rect.width - layout.padding.x - layout.padding.z, 0.0f),
                max(rect.height - layout.padding.y - layout.padding.w, 0.0f));
        }

        static Ref<CoreScrollViewState> scroll_view_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<CoreScrollViewState>(id);
            Ref<CoreScrollViewState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreScrollViewState>();
            }
            return state;
        }

        static void set_scroll_view_state(GUICore::IContext* context, GUICore::id_t id, const Ref<CoreScrollViewState>& state)
        {
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreScrollViewState>(id), state.object(),
                GUICore::StateLifetime::next_frame));
        }

        static void apply_routed_scroll_input(GUICore::IContext* context, GUICore::id_t id, CoreScrollViewState& state)
        {
            Span<const GUICore::RoutedInputEvent> events = context->get_routed_input_events(id);
            for(const GUICore::RoutedInputEvent& routed : events)
            {
                if(routed.event.type == GUICore::InputEventType::pointer_wheel)
                {
                    state.scroll.x -= routed.event.wheel_delta.x * 32.0f;
                    state.scroll.y -= routed.event.wheel_delta.y * 32.0f;
                }
            }
            state.scroll = clamp_scroll_offset(state.scroll, state.content_size, state.viewport_size);
        }

        static RV apply_scroll_view_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::ScrollViewportLayoutDesc desc)
        {
            if(!context || !layout.id || layout.context != context->get_object() || layout.generation != context->generation())
            {
                return BasicError::bad_arguments();
            }
            Ref<CoreScrollViewState> state = scroll_view_state(context, layout.id);
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            state->viewport_size = scroll_viewport_content_size(rect, element->layout);
            desc.scroll_offset = clamp_scroll_offset(state->scroll, state->content_size, state->viewport_size);
            RV r = GUICore::layout_scroll_viewport(context, layout, rect, desc);
            if(failed(r))
            {
                return r;
            }
            element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            state->content_size = element->layout_result.content_size;
            Float2U clamped_scroll = clamp_scroll_offset(desc.scroll_offset, state->content_size, state->viewport_size);
            if(clamped_scroll.x != desc.scroll_offset.x || clamped_scroll.y != desc.scroll_offset.y)
            {
                desc.scroll_offset = clamped_scroll;
                r = GUICore::layout_scroll_viewport(context, layout, rect, desc);
                if(failed(r))
                {
                    return r;
                }
            }
            state->scroll = clamped_scroll;
            set_scroll_view_state(context, layout.id, state);
            return ok;
        }

        static RV layout_editor_subtree(GUICore::IContext* context, const Ref<EditorLayoutPassState>& state,
            const GUICore::ElementHandle& element)
        {
            const GUICore::Element* core_element = context->get_element(element.index);
            if(!core_element || core_element->id != element.id)
            {
                return BasicError::bad_arguments();
            }
            auto request_iter = state->requests.find(element.id);
            if(request_iter != state->requests.end())
            {
                RV r = apply_deferred_layout_request(context, element, core_element->layout_result.rect, request_iter->second);
                if(failed(r))
                {
                    return r;
                }
                core_element = context->get_element(element.index);
                if(!core_element || core_element->id != element.id)
                {
                    return BasicError::bad_arguments();
                }
            }
            Vector<u32> children = collect_children(context, *core_element);
            for(u32 child_index : children)
            {
                const GUICore::Element* child = context->get_element(child_index);
                if(!child)
                {
                    continue;
                }
                GUICore::ElementHandle child_handle { element.context, child->id, child_index, element.generation };
                RV r = layout_editor_subtree(context, state, child_handle);
                if(failed(r))
                {
                    return r;
                }
            }
            return ok;
        }

        static GUICore::ElementHandle begin_core_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const c8* default_label, const GUICore::LayoutInput& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, label ? Name(label) : Name(default_label));
            context->set_layout(element, layout);
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_h_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "h_layout", layout);
        }

        LUNA_GUI_API RV end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::x;
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::linear;
            request.linear = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::x;
            RV r = GUICore::layout_linear(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_v_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "v_layout", layout);
        }

        LUNA_GUI_API RV end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::y;
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::linear;
            request.linear = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::y;
            RV r = GUICore::layout_linear(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_focus_scope(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "focus_scope", layout);
            GUICore::Interactable interactable;
            interactable.focus_scope = id;
            context->set_interactable(element, interactable);
            return element;
        }

        LUNA_GUI_API void end_focus_scope(GUICore::IContext* context)
        {
            luassert(context);
            context->end_element();
        }

        LUNA_GUI_API GUICore::ElementHandle begin_grid_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "grid_layout", layout);
        }

        LUNA_GUI_API RV end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::GridLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::grid;
            request.grid = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::GridLayoutDesc& desc)
        {
            luassert(context);
            RV r = GUICore::layout_grid(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_stack_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "stack_layout", layout);
        }

        LUNA_GUI_API RV end_stack_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::StackLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::stack;
            request.stack = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_stack_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::StackLayoutDesc& desc)
        {
            luassert(context);
            RV r = GUICore::layout_stack(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_canvas_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "canvas_layout", layout);
        }

        LUNA_GUI_API RV end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::CanvasLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::canvas;
            request.canvas_default_item = desc.default_item;
            request.canvas_clip_children = desc.clip_children;
            request.canvas_items.assign(desc.items.begin(), desc.items.end());
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::CanvasLayoutDesc& desc)
        {
            luassert(context);
            RV r = GUICore::layout_canvas(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_scroll_viewport(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            return begin_core_layout(context, id, label, "scroll_viewport", layout);
        }

        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::scroll_viewport;
            request.scroll_viewport = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            RV r = GUICore::layout_scroll_viewport(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_scroll_view(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "scroll_view", layout);
            Ref<CoreScrollViewState> state = scroll_view_state(context, id);
            apply_routed_scroll_input(context, id, *state);
            set_scroll_view_state(context, id, state);
            GUICore::Interactable interactable;
            interactable.hit_test = true;
            interactable.hoverable = true;
            interactable.scrollable = true;
            context->set_interactable(element, interactable);
            return element;
        }

        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::scroll_view;
            request.scroll_viewport = desc;
            RV r = set_deferred_layout_request(context, layout, move(request));
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            RV r = apply_scroll_view_layout(context, layout, rect, desc);
            context->end_element();
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_table_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "table_layout", layout);
            Ref<CoreTableBuildState> state = table_build_state(context);
            CoreTableBuildScope scope;
            scope.table_id = id;
            scope.table = element;
            state->stack.push_back(move(scope));
            return element;
        }

        LUNA_GUI_API void set_table_columns(GUICore::IContext* context, Span<const GUICore::TableTrackDesc> columns)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope);
            scope->columns.assign(columns.begin(), columns.end());
        }

        LUNA_GUI_API void set_table_gap(GUICore::IContext* context, const Float2U& gap)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope);
            scope->gap = gap;
        }

        LUNA_GUI_API void set_table_cell_padding(GUICore::IContext* context, const Float4U& padding)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope);
            scope->cell_padding = padding;
        }

        LUNA_GUI_API void set_table_clip_children(GUICore::IContext* context, bool clip_children)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope);
            scope->clip_children = clip_children;
        }

        LUNA_GUI_API bool begin_table_row(GUICore::IContext* context, const GUICore::TableTrackDesc& row)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope && !scope->row_open);
            const GUICore::Element* table = context->get_element(scope->table.index);
            luassert(table && table->id == scope->table_id);
            scope->row_child_begin = collect_children(context, *table).size();
            scope->current_row = (u32)scope->rows.size();
            scope->rows.push_back(row);
            scope->row_open = true;
            return true;
        }

        LUNA_GUI_API void end_table_row(GUICore::IContext* context)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            luassert(scope && scope->row_open);
            const GUICore::Element* table = context->get_element(scope->table.index);
            luassert(table && table->id == scope->table_id);
            Vector<u32> children = collect_children(context, *table);
            u32 column = 0;
            for(usize i = scope->row_child_begin; i < children.size(); ++i)
            {
                const GUICore::Element* child = context->get_element(children[i]);
                if(!child)
                {
                    continue;
                }
                GUICore::TableLayoutCell cell;
                cell.element_id = child->id;
                cell.row = scope->current_row;
                cell.column = column;
                cell.padding = scope->cell_padding;
                scope->cells.push_back(cell);
                ++column;
            }
            scope->max_columns = max(scope->max_columns, column);
            scope->row_open = false;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            if(!scope || scope->table_id != layout.id || scope->row_open)
            {
                return BasicError::bad_arguments();
            }
            EditorLayoutRequest request;
            Vector<GUICore::TableTrackDesc> default_columns;
            Span<const GUICore::TableTrackDesc> columns(scope->columns.data(), scope->columns.size());
            if(columns.empty())
            {
                default_columns = default_table_columns(*scope);
                columns = Span<const GUICore::TableTrackDesc>(default_columns.data(), default_columns.size());
            }
            set_table_request_from_scope(request, *scope, columns);
            RV r = set_deferred_layout_request(context, layout, move(request));
            pop_table_scope(context, layout.id);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect)
        {
            luassert(context);
            CoreTableBuildScope* scope = current_table_scope(context);
            if(!scope || scope->table_id != layout.id || scope->row_open)
            {
                return BasicError::bad_arguments();
            }
            RV r = apply_table_scope_layout(context, layout, rect, *scope);
            pop_table_scope(context, layout.id);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::TableLayoutDesc& desc)
        {
            luassert(context);
            EditorLayoutRequest request;
            request.kind = EditorLayoutRequestKind::table;
            request.table_columns.assign(desc.columns.begin(), desc.columns.end());
            request.table_rows.assign(desc.rows.begin(), desc.rows.end());
            request.table_cells.assign(desc.cells.begin(), desc.cells.end());
            request.table_gap = desc.gap;
            request.table_clip_children = desc.clip_children;
            RV r = set_deferred_layout_request(context, layout, move(request));
            pop_table_scope(context, layout.id);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::TableLayoutDesc& desc)
        {
            luassert(context);
            RV r = GUICore::layout_table(context, layout, rect, desc);
            pop_table_scope(context, layout.id);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV layout_editor_tree(GUICore::IContext* context, const GUICore::ElementHandle& root,
            const RectF& rect)
        {
            if(!context || !root.id || root.context != context->get_object() || root.generation != context->generation())
            {
                return BasicError::bad_arguments();
            }
            const GUICore::Element* root_element = context->get_element(root.index);
            if(!root_element || root_element->id != root.id)
            {
                return BasicError::bad_arguments();
            }
            GUICore::LayoutResult root_result;
            root_result.rect = rect;
            root_result.clip_rect = rect;
            root_result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(root, root_result);
            Ref<EditorLayoutPassState> state = layout_pass_state(context);
            return layout_editor_subtree(context, state, root);
        }
    }
}
