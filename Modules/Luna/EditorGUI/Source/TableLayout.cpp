/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TableLayout.cpp
* @author JXMaster
* @date 2026/7/16
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
            struct TableSplitterDrawData
            {
                u32 column = 0;
            };

            static RectF inset_rect(const RectF& rect, const Float4U& inset)
            {
                return RectF(rect.offset_x + inset.x, rect.offset_y + inset.y,
                    max(rect.width - inset.x - inset.z, 0.0f),
                    max(rect.height - inset.y - inset.w, 0.0f));
            }

            static RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 left = max(a.offset_x, b.offset_x);
                f32 top = max(a.offset_y, b.offset_y);
                f32 right = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 bottom = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(left, top, max(right - left, 0.0f), max(bottom - top, 0.0f));
            }

            static R<GUI::paint_order_id_t> draw_table_splitter(GUI::IContext* context,
                const GUI::ElementHandle& element, GUI::DrawPhase,
                GUI::paint_order_id_t paint_order_id, void*)
            {
                GUI::InteractionState interaction = context->get_interaction_state(element.id);
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = RectF(-0.5f, 0.0f, 1.0f, 0.0f);
                command.rect_layout_scale = Float4U(0.5f, 0.0f, 0.0f, 1.0f);
                command.color = style_color(context, element,
                    interaction.hovered || interaction.active ? "gui.table.separator_hovered" :
                    "gui.table.separator", interaction.hovered || interaction.active ?
                    Float4U(0.10f, 0.55f, 0.90f, 1.0f) : Float4U(0.28f, 0.34f, 0.40f, 0.8f));
                context->draw(command, paint_order_id);
                return paint_order_id;
            }

            static RV layout_table(GUI::IContext* context, const GUI::ElementHandle& element,
                const RectF& rect, void* userdata)
            {
                TableAction* action = (TableAction*)userdata;
                if(!action || !action->layout_desc)
                {
                    return E_BAD_ARGUMENTS;
                }
                lutry(GUI::layout_table(context, element, rect, action->layout_desc));
                if(!action->splitter_count)
                {
                    return ok;
                }
                const GUI::Element* table = context->get_element(element.index);
                if(!table)
                {
                    return E_BAD_ARGUMENTS;
                }
                RectF content_rect = inset_rect(rect, table->layout.padding);
                f32 cursor = content_rect.offset_x;
                for(usize i = 0; i < action->splitter_count; ++i)
                {
                    cursor += action->layout_desc->columns[i].value;
                    f32 center = cursor + action->layout_desc->gap.x * 0.5f;
                    const GUI::Element* splitter = context->find_element(action->splitter_ids[i]);
                    if(splitter)
                    {
                        GUI::LayoutResult result;
                        result.rect = RectF(center - action->desc.resize_handle_width * 0.5f,
                            content_rect.offset_y, action->desc.resize_handle_width, content_rect.height);
                        result.clip_rect = intersect_rect(result.rect, table->layout_result.clip_rect);
                        result.content_size = Float2U(result.rect.width, result.rect.height);
                        context->set_layout_result(GUI::ElementHandle { splitter->id,
                            context->find_element_handle(splitter->id).index, element.generation }, result);
                    }
                    cursor += action->layout_desc->gap.x;
                }
                return ok;
            }

            static GUI::MeasureResult measure_table(GUI::IContext* context,
                const GUI::ElementHandle& element, const Float2U& available_content_size, void* userdata)
            {
                TableAction* action = (TableAction*)userdata;
                if(!action || !action->layout_desc)
                {
                    return GUI::MeasureResult();
                }
                return GUI::measure_table(context, element, available_content_size, action->layout_desc);
            }

            static RV finalize_table(GUI::IContext* context, const GUI::ElementHandle& element,
                const RectF& rect, void* userdata)
            {
                TableAction* action = (TableAction*)userdata;
                const GUI::Element* table = context->get_element(element.index);
                if(!action || !action->state || !table)
                {
                    return ok;
                }
                f32 content_top = rect.offset_y + table->layout.padding.y;
                action->state->visible_min_y = max(table->layout_result.clip_rect.offset_y - content_top, 0.0f);
                action->state->visible_max_y = max(table->layout_result.clip_rect.offset_y +
                    table->layout_result.clip_rect.height - content_top, action->state->visible_min_y);
                action->state->layout_generation = context->generation();
                return ok;
            }

            static void apply_persistent_column_widths(TableBuildScope& scope)
            {
                TableState& state = *scope.state;
                if(state.column_widths.size() != scope.columns.size())
                {
                    state.column_widths.resize(scope.columns.size());
                    state.column_source_widths.resize(scope.columns.size());
                    for(usize i = 0; i < scope.columns.size(); ++i)
                    {
                        state.column_widths[i] = scope.columns[i].value;
                        state.column_source_widths[i] = scope.columns[i].value;
                    }
                }
                for(usize i = 0; i < scope.columns.size(); ++i)
                {
                    GUI::TableTrackDesc& column = scope.columns[i];
                    if(column.kind != GUI::TableTrackSizeKind::pixels)
                    {
                        continue;
                    }
                    if(state.resizing_column != (i32)i &&
                        state.column_source_widths[i] != column.value)
                    {
                        state.column_widths[i] = column.value;
                    }
                    state.column_source_widths[i] = column.value;
                    column.value = clamp(state.column_widths[i], column.min,
                        column.max >= 0.0f ? max(column.max, column.min) : F32_MAX);
                }
            }

            bool resolve_table_action(GUI::IContext* context, TableAction& action)
            {
                if(!action.state || !action.layout_desc || !action.splitter_count)
                {
                    return false;
                }
                TableState& state = *action.state;
                bool changed = false;
                for(usize i = 0; i < action.splitter_count; ++i)
                {
                    id_t splitter_id = action.splitter_ids[i];
                    for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(splitter_id))
                    {
                        if(routed.event.type == GUI::InputEventType::pointer_down &&
                            routed.event.button == GUI::PointerButton::left)
                        {
                            state.resizing_column = (i32)i;
                            state.resize_start_pointer_x = routed.event.position.x;
                            state.resize_start_width = action.layout_desc->columns[i].value;
                        }
                    }
                }
                if(state.resizing_column >= 0 && (usize)state.resizing_column < action.splitter_count)
                {
                    usize column_index = (usize)state.resizing_column;
                    if(context->is_pointer_button_down(GUI::PointerButton::left))
                    {
                        const GUI::TableTrackDesc& column = action.layout_desc->columns[column_index];
                        f32 width = state.resize_start_width + context->get_pointer_position().x -
                            state.resize_start_pointer_x;
                        width = clamp(width, column.min,
                            column.max >= 0.0f ? max(column.max, column.min) : F32_MAX);
                        if(width != action.mutable_columns[column_index].value)
                        {
                            action.mutable_columns[column_index].value = width;
                            state.column_widths[column_index] = width;
                            changed = true;
                        }
                    }
                    else
                    {
                        state.resizing_column = -1;
                    }
                }
                return changed;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_table_layout(GUI::IContext* context, id_t id,
            const c8* label, const GUI::LayoutConfig& layout, const TableDesc& desc)
        {
            luassert(context && id);
            GUI::ElementHandle element = Internal::begin_element(context, id,
                label ? label : "Table Layout", layout);
            context->set_child_paint_order_mode(element, GUI::ChildPaintOrderMode::shared);
            Ref<Internal::TableState> state = Internal::widget_state<Internal::TableState>(context, id);
            Internal::TableBuildScope scope;
            scope.table = element;
            scope.desc = desc;
            scope.state = state.get();
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            if(!frame->scroll_stack.empty())
            {
                scope.overscan_y = frame->scroll_stack.back().data->desc.max_scroll_delta.y;
            }
            frame->table_stack.push_back(move(scope));
            return element;
        }

        LUNA_EDITOR_GUI_API void set_table_columns(GUI::IContext* context,
            Span<const GUI::TableTrackDesc> columns)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->table_stack.empty());
            Internal::TableBuildScope& scope = frame->table_stack.back();
            scope.columns.assign(columns.begin(), columns.end());
            Internal::apply_persistent_column_widths(scope);
            scope.column_widths_applied = !scope.columns.empty();
        }

        LUNA_EDITOR_GUI_API bool begin_table_row(GUI::IContext* context,
            const GUI::TableTrackDesc& row)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->table_stack.empty());
            Internal::TableBuildScope& scope = frame->table_stack.back();
            luassert(!scope.row_open);
            scope.current_row = (u32)scope.rows.size();
            GUI::TableTrackDesc resolved_row = row;
            if(scope.desc.fixed_row_height_mode)
            {
                resolved_row.kind = GUI::TableTrackSizeKind::pixels;
                resolved_row.value = max(scope.desc.fixed_row_height, 0.0f);
                resolved_row.min = resolved_row.value;
                resolved_row.max = resolved_row.value;
            }
            scope.rows.push_back(resolved_row);
            const GUI::Element* table = context->get_element(scope.table.index);
            scope.row_previous_last_child = table ? table->last_child : GUI::INVALID_ELEMENT;
            scope.row_open = true;
            scope.row_visible = true;
            if(scope.desc.fixed_row_height_mode && scope.desc.virtualize_fixed_rows &&
                scope.state->layout_generation + 1 == context->generation())
            {
                f32 row_top = (f32)scope.current_row *
                    (max(scope.desc.fixed_row_height, 0.0f) + scope.desc.gap.y);
                f32 row_bottom = row_top + max(scope.desc.fixed_row_height, 0.0f);
                scope.row_visible = row_bottom >= scope.state->visible_min_y - scope.overscan_y &&
                    row_top <= scope.state->visible_max_y + scope.overscan_y;
            }
            return scope.row_visible;
        }

        LUNA_EDITOR_GUI_API void end_table_row(GUI::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->table_stack.empty());
            Internal::TableBuildScope& scope = frame->table_stack.back();
            luassert(scope.row_open);
            const GUI::Element* table = context->get_element(scope.table.index);
            u32 child_index = table ? table->first_child : GUI::INVALID_ELEMENT;
            if(scope.row_previous_last_child != GUI::INVALID_ELEMENT)
            {
                const GUI::Element* previous = context->get_element(scope.row_previous_last_child);
                child_index = previous ? previous->next_sibling : GUI::INVALID_ELEMENT;
            }
            u32 column = 0;
            while(child_index != GUI::INVALID_ELEMENT)
            {
                const GUI::Element* child = context->get_element(child_index);
                if(!child)
                {
                    break;
                }
                GUI::TableLayoutCell cell;
                cell.element_id = child->id;
                cell.row = scope.current_row;
                cell.column = column++;
                cell.padding = scope.desc.cell_padding;
                scope.cells.push_back(cell);
                child_index = child->next_sibling;
            }
            scope.max_columns = max(scope.max_columns, column);
            scope.state->inferred_column_count = max(scope.state->inferred_column_count, column);
            scope.row_open = false;
        }

        LUNA_EDITOR_GUI_API void end_table_layout(GUI::IContext* context,
            const GUI::ElementHandle& element)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->table_stack.empty());
            Internal::TableBuildScope& scope = frame->table_stack.back();
            luassert(scope.table.id == element.id && !scope.row_open);
            if(scope.columns.empty())
            {
                u32 column_count = max(scope.max_columns, scope.state->inferred_column_count);
                scope.columns.resize(column_count);
            }
            if(!scope.column_widths_applied)
            {
                Internal::apply_persistent_column_widths(scope);
            }

            Internal::TableAction* action = Internal::allocate_frame<Internal::TableAction>(context);
            action->id = element.id;
            action->desc = scope.desc;
            action->state = scope.state;
            action->layout_desc = Internal::allocate_frame<GUI::TableLayoutDesc>(context);
            action->layout_desc->gap = scope.desc.gap;
            action->layout_desc->clip_children = scope.desc.clip_children;

            GUI::TableTrackDesc* columns = Internal::allocate_frame_array<GUI::TableTrackDesc>(context,
                scope.columns.size());
            for(usize i = 0; i < scope.columns.size(); ++i) columns[i] = scope.columns[i];
            action->mutable_columns = columns;
            action->layout_desc->columns = Span<const GUI::TableTrackDesc>(columns, scope.columns.size());
            GUI::TableTrackDesc* rows = Internal::allocate_frame_array<GUI::TableTrackDesc>(context,
                scope.rows.size());
            for(usize i = 0; i < scope.rows.size(); ++i) rows[i] = scope.rows[i];
            action->layout_desc->rows = Span<const GUI::TableTrackDesc>(rows, scope.rows.size());
            GUI::TableLayoutCell* cells = Internal::allocate_frame_array<GUI::TableLayoutCell>(context,
                scope.cells.size());
            for(usize i = 0; i < scope.cells.size(); ++i) cells[i] = scope.cells[i];
            action->layout_desc->cells = Span<const GUI::TableLayoutCell>(cells, scope.cells.size());

            bool absolute_columns = scope.columns.size() > 1;
            for(const GUI::TableTrackDesc& column : scope.columns)
            {
                absolute_columns &= column.kind == GUI::TableTrackSizeKind::pixels;
            }
            if(scope.desc.resizable_columns && absolute_columns)
            {
                action->splitter_count = scope.columns.size() - 1;
                action->splitter_ids = Internal::allocate_frame_array<id_t>(context, action->splitter_count);
                id_t splitter_scope = Internal::derived_id(element.id, "table_splitter");
                for(usize i = 0; i < action->splitter_count; ++i)
                {
                    id_t splitter_id = GUI::make_scoped_id(splitter_scope, (u64)i + 1);
                    action->splitter_ids[i] = splitter_id;
                    GUI::LayoutConfig splitter_layout;
                    splitter_layout.width.kind = GUI::SizeKind::fixed;
                    splitter_layout.width.value = scope.desc.resize_handle_width;
                    splitter_layout.height.kind = GUI::SizeKind::fixed;
                    splitter_layout.height.value = 1.0f;
                    GUI::ElementHandle splitter = Internal::begin_element(context, splitter_id,
                        "Table Column Splitter", splitter_layout);
                    GUI::Interactable interactable;
                    interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                    set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
                    set_flags(interactable.flags, GUI::InteractableFlag::activatable);
                    context->set_interactable(splitter, interactable);
                    Internal::TableSplitterDrawData* draw_data =
                        Internal::allocate_frame<Internal::TableSplitterDrawData>(context);
                    draw_data->column = (u32)i;
                    GUI::DrawConfig draw;
                    draw.name = Name("gui.table.splitter");
                    draw.callback = Internal::draw_table_splitter;
                    draw.userdata = draw_data;
                    context->set_draw_config(splitter, draw);
                    context->end_element();
                }
            }

            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.table_layout");
            callbacks.measure_callback = Internal::measure_table;
            callbacks.callback = Internal::layout_table;
            callbacks.finalize_callback = Internal::finalize_table;
            callbacks.userdata = action;
            context->set_layout_callback_config(element, callbacks);
            context->end_element();
            Internal::add_action(context, Internal::ActionType::table, element.id, action);
            frame->table_stack.pop_back();
        }
    }
}
