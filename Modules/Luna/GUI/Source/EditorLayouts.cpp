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
        static bool valid_core_element(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            if(!context || !element.id || element.generation != context->generation())
            {
                return false;
            }
            const GUICore::Element* core_element = context->get_element(element.index);
            return core_element && core_element->id == element.id;
        }

        static RV set_element_layout_config(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::LayoutConfig& config)
        {
            if(!valid_core_element(context, layout))
            {
                return BasicError::bad_arguments();
            }
            context->set_layout_config(layout, config);
            return ok;
        }

        static Ref<CoreLayoutUserdataArenaState> layout_userdata_arena_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreLayoutUserdataArenaState>(0);
            Ref<CoreLayoutUserdataArenaState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreLayoutUserdataArenaState>();
            }
            if(state->generation != context->generation())
            {
                state->generation = context->generation();
                state->block_index = 0;
                state->offset = 0;
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static void* allocate_layout_userdata_raw(GUICore::IContext* context, usize size, usize alignment)
        {
            luassert(context);
            Ref<CoreLayoutUserdataArenaState> state = layout_userdata_arena_state(context);
            size = max(size, (usize)1);
            alignment = max(alignment, (usize)1);
            constexpr usize block_alignment = 16;
            constexpr usize default_block_size = 4096;
            usize required_block_size = max(default_block_size, align_upper(size, block_alignment));
            for(;;)
            {
                if(state->block_index >= state->blocks.size())
                {
                    state->blocks.push_back(Blob(required_block_size, block_alignment));
                }
                Blob& block = state->blocks[state->block_index];
                if(block.size() < size)
                {
                    if(state->offset == 0)
                    {
                        block.resize(required_block_size, false);
                    }
                    else
                    {
                        ++state->block_index;
                        state->offset = 0;
                        continue;
                    }
                }
                usize offset = align_upper(state->offset, alignment);
                if(offset + size <= block.size())
                {
                    state->offset = offset + size;
                    return (void*)((byte_t*)block.data() + offset);
                }
                ++state->block_index;
                state->offset = 0;
            }
        }

        template <typename _Ty>
        static _Ty* allocate_layout_userdata(GUICore::IContext* context, const _Ty& value)
        {
            _Ty* ret = (_Ty*)allocate_layout_userdata_raw(context, sizeof(_Ty), alignof(_Ty));
            *ret = value;
            return ret;
        }

        template <typename _Ty>
        static Span<const _Ty> allocate_layout_userdata_span(GUICore::IContext* context, Span<const _Ty> values)
        {
            if(values.empty())
            {
                return Span<const _Ty>();
            }
            _Ty* ret = (_Ty*)allocate_layout_userdata_raw(context, sizeof(_Ty) * values.size(), alignof(_Ty));
            memcpy(ret, values.data(), sizeof(_Ty) * values.size());
            return Span<const _Ty>(ret, values.size());
        }

        static void fill_linear_layout_config(GUICore::IContext* context, GUICore::LayoutConfig& config,
            const GUICore::LinearLayoutDesc& desc)
        {
            config.name = Name("gui.core.linear");
            config.callback = GUICore::layout_linear;
            config.userdata = allocate_layout_userdata(context, desc);
        }

        LUNA_GUI_API RV set_editor_linear_layout_config(GUICore::IContext* context,
            const GUICore::ElementHandle& layout, const GUICore::LinearLayoutDesc& desc)
        {
            GUICore::LayoutConfig config;
            fill_linear_layout_config(context, config, desc);
            return set_element_layout_config(context, layout, config);
        }

        static void set_grid_layout_config(GUICore::IContext* context, GUICore::LayoutConfig& config,
            const GUICore::GridLayoutDesc& desc)
        {
            config.name = Name("gui.core.grid");
            config.callback = GUICore::layout_grid;
            config.userdata = allocate_layout_userdata(context, desc);
        }

        static void set_stack_layout_config(GUICore::IContext* context, GUICore::LayoutConfig& config,
            const GUICore::StackLayoutDesc& desc)
        {
            config.name = Name("gui.core.stack");
            config.callback = GUICore::layout_stack;
            config.userdata = allocate_layout_userdata(context, desc);
        }

        static void set_canvas_layout_config(GUICore::IContext* context, GUICore::LayoutConfig& config,
            const GUICore::CanvasLayoutDesc& desc)
        {
            config.name = Name("gui.core.canvas");
            config.callback = GUICore::layout_canvas;
            GUICore::CanvasLayoutDesc packed;
            packed.default_item = desc.default_item;
            packed.items = allocate_layout_userdata_span(context, desc.items);
            packed.clip_children = desc.clip_children;
            config.userdata = allocate_layout_userdata(context, packed);
        }

        static const GUICore::ScrollViewportLayoutDesc* scroll_viewport_layout_desc(const GUICore::Element& element)
        {
            return (const GUICore::ScrollViewportLayoutDesc*)element.layout_config.userdata;
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

        static void set_table_config_from_desc(GUICore::IContext* context, GUICore::LayoutConfig& config,
            const GUICore::TableLayoutDesc& desc)
        {
            config.name = Name("gui.core.table");
            config.callback = GUICore::layout_table;
            GUICore::TableLayoutDesc packed;
            packed.columns = allocate_layout_userdata_span(context, desc.columns);
            packed.rows = allocate_layout_userdata_span(context, desc.rows);
            packed.cells = allocate_layout_userdata_span(context, desc.cells);
            packed.gap = desc.gap;
            packed.clip_children = desc.clip_children;
            config.userdata = allocate_layout_userdata(context, packed);
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

        static GUICore::LayoutConfig table_config_from_scope(GUICore::IContext* context, const CoreTableBuildScope& scope)
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
            GUICore::LayoutConfig config;
            set_table_config_from_desc(context, config, desc);
            return config;
        }

        static RV apply_scroll_view_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::ScrollViewportLayoutDesc desc);

        static Float2U clamp_scroll_offset(Float2U scroll, const Float2U& content_size, const Float2U& viewport_size)
        {
            Float2U max_scroll(
                max(content_size.x - viewport_size.x, 0.0f),
                max(content_size.y - viewport_size.y, 0.0f));
            scroll.x = clamp(scroll.x, 0.0f, max_scroll.x);
            scroll.y = clamp(scroll.y, 0.0f, max_scroll.y);
            return scroll;
        }

        static RectF scroll_viewport_content_rect(const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return RectF(
                rect.offset_x + layout.padding.x,
                rect.offset_y + layout.padding.y,
                max(rect.width - layout.padding.x - layout.padding.z, 0.0f),
                max(rect.height - layout.padding.y - layout.padding.w, 0.0f));
        }

        static Float2U scroll_viewport_content_size(const RectF& rect, const GUICore::LayoutInput& layout)
        {
            RectF content_rect = scroll_viewport_content_rect(rect, layout);
            return Float2U(content_rect.width, content_rect.height);
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
            if(!valid_core_element(context, layout))
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
            state->scroll = desc.scroll_offset;
            set_scroll_view_state(context, layout.id, state);
            return GUICore::layout_scroll_viewport(context, layout, rect, &desc);
        }

        static bool is_scroll_layout_element(const GUICore::Element& element)
        {
            if(element.layout_config.callback == GUICore::layout_scroll_viewport)
            {
                return true;
            }
            return element.layout_config.name == Name("gui.editor.scroll_view") ||
                element.layout_config.name == Name("gui.editor.scroll_viewport") ||
                element.layout_config.name == Name("gui.core.scroll_viewport");
        }

        static void accumulate_scroll_content_bounds(GUICore::IContext* context,
            u32 element_index, const RectF& content_rect, const Float2U& scroll_offset, Float2U& content_size)
        {
            const GUICore::Element* element = context->get_element(element_index);
            if(!element)
            {
                return;
            }
            const GUICore::LayoutResult& layout = element->layout_result;
            bool scroll_layout = is_scroll_layout_element(*element);
            f32 left = layout.rect.offset_x - content_rect.offset_x + scroll_offset.x;
            f32 top = layout.rect.offset_y - content_rect.offset_y + scroll_offset.y;
            f32 right = left + (scroll_layout ? layout.rect.width : max(layout.rect.width, layout.content_size.x));
            f32 bottom = top + (scroll_layout ? layout.rect.height : max(layout.rect.height, layout.content_size.y));
            content_size.x = max(content_size.x, right);
            content_size.y = max(content_size.y, bottom);

            if(scroll_layout)
            {
                return;
            }
            for(u32 child = element->first_child; child != GUICore::INVALID_ELEMENT;)
            {
                const GUICore::Element* child_element = context->get_element(child);
                if(!child_element)
                {
                    break;
                }
                u32 next = child_element->next_sibling;
                accumulate_scroll_content_bounds(context, child, content_rect, scroll_offset, content_size);
                child = next;
            }
        }

        static Float2U resolved_scroll_view_content_size(GUICore::IContext* context,
            const GUICore::Element& element, const GUICore::ScrollViewportLayoutDesc& desc)
        {
            RectF content_rect = scroll_viewport_content_rect(element.layout_result.rect, element.layout);
            Float2U content_size(0.0f, 0.0f);
            Vector<u32> children = collect_children(context, element);
            for(u32 child_index : children)
            {
                accumulate_scroll_content_bounds(context, child_index, content_rect, desc.scroll_offset, content_size);
            }
            return content_size;
        }

        static RV finalize_scroll_view_layout(GUICore::IContext* context,
            const GUICore::ElementHandle& layout, const GUICore::ScrollViewportLayoutDesc& desc, bool update_state)
        {
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            GUICore::ScrollViewportLayoutDesc measure_desc = desc;
            Ref<CoreScrollViewState> state;
            if(update_state)
            {
                state = scroll_view_state(context, layout.id);
                measure_desc.scroll_offset = state->scroll;
            }
            Float2U content_size = resolved_scroll_view_content_size(context, *element, measure_desc);
            GUICore::LayoutResult result = element->layout_result;
            result.content_size = content_size;
            context->set_layout_result(layout, result);
            if(update_state)
            {
                state->viewport_size = scroll_viewport_content_size(result.rect, element->layout);
                state->content_size = content_size;
                state->scroll = clamp_scroll_offset(state->scroll, state->content_size, state->viewport_size);
                set_scroll_view_state(context, layout.id, state);
            }
            return ok;
        }

        static RV scroll_view_layout_callback(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, void*)
        {
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            const GUICore::ScrollViewportLayoutDesc* desc = scroll_viewport_layout_desc(*element);
            if(!desc)
            {
                return BasicError::bad_arguments();
            }
            return apply_scroll_view_layout(context, layout, rect, *desc);
        }

        static RV scroll_view_finalize_callback(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF&, void*)
        {
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            const GUICore::ScrollViewportLayoutDesc* desc = scroll_viewport_layout_desc(*element);
            if(!desc)
            {
                return BasicError::bad_arguments();
            }
            return finalize_scroll_view_layout(context, layout, *desc, true);
        }

        static RV scroll_viewport_layout_callback(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, void*)
        {
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            return GUICore::layout_scroll_viewport(context, layout, rect, element->layout_config.userdata);
        }

        static RV scroll_viewport_finalize_callback(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF&, void*)
        {
            const GUICore::Element* element = context->get_element(layout.index);
            if(!element || element->id != layout.id)
            {
                return BasicError::bad_arguments();
            }
            const GUICore::ScrollViewportLayoutDesc* desc = scroll_viewport_layout_desc(*element);
            if(!desc)
            {
                return BasicError::bad_arguments();
            }
            return finalize_scroll_view_layout(context, layout, *desc, false);
        }

        static GUICore::ElementHandle begin_core_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const c8* default_label, const GUICore::LayoutInput& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, label ? Name(label) : Name(default_label));
            context->set_layout(element, layout);
            return element;
        }

        static void push_element_clip(GUICore::IContext* context, const Float4U& inset)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::push_clip;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(inset.x, inset.y, -inset.z, -inset.w);
            context->draw(command);
        }

        static void pop_element_clip(GUICore::IContext* context)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::pop_clip;
            context->draw(command);
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
            GUICore::LayoutConfig config;
            fill_linear_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::x;
            GUICore::LayoutConfig config;
            fill_linear_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
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
            GUICore::LayoutConfig config;
            fill_linear_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::LinearLayoutDesc desc)
        {
            luassert(context);
            desc.axis = GUICore::LayoutAxis::y;
            GUICore::LayoutConfig config;
            fill_linear_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
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
            GUICore::LayoutConfig config;
            set_grid_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::GridLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            set_grid_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
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
            GUICore::LayoutConfig config;
            set_stack_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_stack_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::StackLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            set_stack_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
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
            GUICore::LayoutConfig config;
            set_canvas_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::CanvasLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            set_canvas_layout_config(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_scroll_viewport(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "scroll_viewport", layout);
            push_element_clip(context, layout.padding);
            return element;
        }

        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            config.name = Name("gui.editor.scroll_viewport");
            config.callback = scroll_viewport_layout_callback;
            config.finalize_callback = scroll_viewport_finalize_callback;
            config.userdata = allocate_layout_userdata(context, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_element_clip(context);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            config.name = Name("gui.editor.scroll_viewport");
            config.callback = scroll_viewport_layout_callback;
            config.finalize_callback = scroll_viewport_finalize_callback;
            config.userdata = allocate_layout_userdata(context, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_element_clip(context);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
            return r;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_scroll_view(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "scroll_view", layout);
            push_element_clip(context, layout.padding);
            Ref<CoreScrollViewState> state = scroll_view_state(context, id);
            apply_routed_scroll_input(context, id, *state);
            set_scroll_view_state(context, id, state);
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::scrollable);
            context->set_interactable(element, interactable);
            return element;
        }

        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            config.name = Name("gui.editor.scroll_view");
            config.callback = scroll_view_layout_callback;
            config.finalize_callback = scroll_view_finalize_callback;
            config.userdata = allocate_layout_userdata(context, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_element_clip(context);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            config.name = Name("gui.editor.scroll_view");
            config.callback = scroll_view_layout_callback;
            config.finalize_callback = scroll_view_finalize_callback;
            config.userdata = allocate_layout_userdata(context, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_element_clip(context);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
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
            GUICore::LayoutConfig config = table_config_from_scope(context, *scope);
            RV r = set_element_layout_config(context, layout, config);
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
            GUICore::LayoutConfig config = table_config_from_scope(context, *scope);
            RV r = set_element_layout_config(context, layout, config);
            pop_table_scope(context, layout.id);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
            return r;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::TableLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            set_table_config_from_desc(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_table_scope(context, layout.id);
            context->end_element();
            return r;
        }

        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::TableLayoutDesc& desc)
        {
            luassert(context);
            GUICore::LayoutConfig config;
            set_table_config_from_desc(context, config, desc);
            RV r = set_element_layout_config(context, layout, config);
            pop_table_scope(context, layout.id);
            context->end_element();
            if(succeeded(r))
            {
                r = context->apply_layout(layout, rect);
            }
            return r;
        }

        LUNA_GUI_API RV layout_editor_tree(GUICore::IContext* context, const GUICore::ElementHandle& root,
            const RectF& rect)
        {
            if(!valid_core_element(context, root))
            {
                return BasicError::bad_arguments();
            }
            const GUICore::Element* root_element = context->get_element(root.index);
            if(!root_element || root_element->id != root.id)
            {
                return BasicError::bad_arguments();
            }
            return context->apply_layout(root, rect);
        }
    }
}
