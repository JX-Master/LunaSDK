/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Description.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Layout.hpp"
#include "RenderProxy.hpp"

namespace Luna
{
    namespace GUI
    {
        struct IDrawList;
        struct Node;

        struct NodeInputContext;
        struct NodeMeasureContext;
        struct NodeRenderContext;

        enum class ColorValueType : u8
        {
            f32,
            u8,
            rgba8
        };

        enum class ColorChannelPart : u8
        {
            none,
            rgb,
            hsv
        };

        enum class ImageFlag : u32
        {
            none = 0x00,
            flip_y = 0x01
        };

        enum class NodeLayerRole : u8
        {
            normal,
            root,
            popup,
            tooltip
        };

        enum class NodeLayoutBehavior : u8
        {
            linear,
            scroll,
            table,
            grid,
            canvas,
            dock_space,
            tab_bar,
            tab_item
        };

        enum class NodeLayoutFlow : u8
        {
            vertical,
            horizontal
        };

        struct NodeRenderState
        {
            bool hovered = false;
            bool active = false;
            bool focused = false;
            Float2U surface_size = Float2U(0.0f);
            Float2U pointer_position = Float2U(0.0f);
            f32 delta_time = 1.0f / 60.0f;
            f64 time = 0.0;
        };

        struct NodeRenderLayout
        {
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);

            Vector<f32> table_column_offsets;
            Vector<f32> table_column_widths;
            Vector<f32> table_row_offsets;
            Vector<f32> table_row_heights;
            u32 table_columns = 0;
            u32 table_rows = 0;

            RectF tab_header_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_header_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool tab_scrollable = false;
            f32 tab_scroll_max = 0.0f;
            bool tab_content_visible = true;

            bool dock_panel_child = false;
            bool dock_panel_visible = true;
            bool dock_panel_floating = false;
            id_t dock_space_id = 0;
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            DockPanelStyle dock_panel_style;
            u32 dock_leaf_index = U32_MAX;

            Float2U scroll_content_size = Float2U(0.0f);
            Float2U scroll_viewport_size = Float2U(0.0f);
            bool scroll_has_vertical = false;
            bool scroll_has_horizontal = false;
        };

        struct NodeRenderContext
        {
            virtual ~NodeRenderContext() = default;
            virtual u32 current_node_index() const = 0;
            virtual const Node* get_node(u32 node_index) const = 0;
            virtual const Node* find_node(id_t node_id) const = 0;
            virtual bool get_node_render_layout(u32 node_index, NodeRenderLayout& out_layout) const = 0;
            virtual IDrawList* draw_list() = 0;
            virtual object_t get_state(id_t id) const = 0;
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;
            virtual void clear_state(id_t id) = 0;
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) const = 0;
            template <typename T>
            T* get_widget_state(id_t owner_id) const
            {
                object_t obj = get_state(make_state_id<T>(owner_id));
                return obj ? cast_object<T>(obj) : nullptr;
            }
            template <typename T>
            Ref<T> get_or_create_widget_state(id_t owner_id, StateLifetime lifetime = StateLifetime::next_frame)
            {
                id_t state_id = make_state_id<T>(owner_id);
                object_t existing = get_state(state_id);
                if(existing)
                {
                    Ref<T> state;
                    object_retain(existing);
                    state.attach(existing);
                    RV r = set_state(state_id, state.object(), lifetime);
                    luassert_always(succeeded(r));
                    return state;
                }
                Ref<T> state = new_object<T>();
                RV r = set_state(state_id, state.object(), lifetime);
                luassert_always(succeeded(r));
                return state;
            }
            virtual bool is_popup_open(id_t popup_id) const = 0;
            virtual void draw_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius = 0.0f,
                RHI::ITexture* texture = nullptr, ImageFlag image_flags = ImageFlag::none) = 0;
            virtual void draw_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                bool top_left, bool top_right, bool bottom_right, bool bottom_left) = 0;
            virtual void draw_gradient_rect(const RectF& rect, const RectF& clip_rect,
                const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left) = 0;
            virtual void draw_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color) = 0;
            virtual void draw_line(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width = 1.0f) = 0;
            virtual void draw_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color,
                TextAlignment horizontal_alignment = TextAlignment::begin,
                TextAlignment vertical_alignment = TextAlignment::center) = 0;
        };

        struct NodeMeasureContext
        {
            virtual ~NodeMeasureContext() = default;
            virtual const Node* parent() const = 0;
            virtual Float2U surface_size() const = 0;
            virtual LayoutMetrics measure_text(const c8* text, usize text_size, f32 font_size, f32 max_width) const = 0;
        };

        struct NodeInputContext
        {
            virtual ~NodeInputContext() = default;
            virtual Float2U pointer_position() const = 0;
            virtual RectF rect() const = 0;
            virtual object_t get_state(id_t id) const = 0;
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;
            virtual void clear_state(id_t id) = 0;
            virtual void set_state(const Name& key, const Any& value) = 0;
            template <typename T>
            T* get_widget_state(id_t owner_id) const
            {
                object_t obj = get_state(make_state_id<T>(owner_id));
                return obj ? cast_object<T>(obj) : nullptr;
            }
            template <typename T>
            Ref<T> get_or_create_widget_state(id_t owner_id, StateLifetime lifetime = StateLifetime::next_frame)
            {
                id_t state_id = make_state_id<T>(owner_id);
                object_t existing = get_state(state_id);
                if(existing)
                {
                    Ref<T> state;
                    object_retain(existing);
                    state.attach(existing);
                    RV r = set_state(state_id, state.object(), lifetime);
                    luassert_always(succeeded(r));
                    return state;
                }
                Ref<T> state = new_object<T>();
                RV r = set_state(state_id, state.object(), lifetime);
                luassert_always(succeeded(r));
                return state;
            }
            virtual bool is_popup_open(id_t popup_id) const = 0;
            virtual void open_menu_popup(id_t menu_id) = 0;
            virtual void close_popup(id_t popup_id) = 0;
            virtual void close_all_popups() = 0;
        };

        struct Node
        {
            lustruct("GUI::Node", "{AD82DACD-76EC-4EE5-8A82-0A6C4CC8BD5C}");

            virtual ~Node() = default;
            virtual Guid type_guid() const = 0;
            virtual Ref<Node> clone() const = 0;
            virtual LayoutMetrics measure() const
            {
                LayoutMetrics metrics;
                metrics.min_size = Float2U(1.0f);
                metrics.preferred_size = Float2U(1.0f);
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                return metrics;
            }
            virtual LayoutMetrics measure(NodeMeasureContext& ctx) const
            {
                return measure();
            }
            virtual NodeLayerRole layer_role() const
            {
                return NodeLayerRole::normal;
            }
            virtual NodeLayoutBehavior layout_behavior() const
            {
                return NodeLayoutBehavior::linear;
            }
            virtual NodeLayoutFlow layout_flow() const
            {
                return NodeLayoutFlow::vertical;
            }
            virtual bool default_interactive() const
            {
                return false;
            }
            virtual bool uses_node_measure() const
            {
                return true;
            }
            virtual void apply_container_defaults(LayoutDesc& desc) const {}
            virtual bool enabled_state() const
            {
                return true;
            }
            virtual bool hit_test(const RectF& rect, const RectF& clip_rect, const Float2U& pos) const
            {
                return interactive &&
                    pos.x >= rect.offset_x && pos.x <= rect.offset_x + rect.width &&
                    pos.y >= rect.offset_y && pos.y <= rect.offset_y + rect.height &&
                    pos.x >= clip_rect.offset_x && pos.x <= clip_rect.offset_x + clip_rect.width &&
                    pos.y >= clip_rect.offset_y && pos.y <= clip_rect.offset_y + clip_rect.height;
            }
            virtual void update_state(NodeInputContext& ctx) const {}
            virtual void on_click(NodeInputContext& ctx) {}

            id_t id = 0;
            u32 layer = U32_MAX;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            String text;
            Name style;
            RenderProxyDesc render_proxy;
            Size requested_size;
            LayoutStyle layout_style;
            LayoutDesc layout_desc;
            bool absolute_position = false;
            Float2U position = Float2U(0.0f);
            bool has_user_clip_rect = false;
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Vector<Name> drag_drop_source_types;
            Vector<Name> drag_drop_target_types;
            bool interactive = false;
        };

        struct NodeArray
        {
            Vector<Ref<Node>> data;

            struct Iterator
            {
                NodeArray* owner = nullptr;
                usize index = 0;

                Node& operator*() const { return owner->operator[](index); }
                Iterator& operator++()
                {
                    ++index;
                    return *this;
                }
                bool operator!=(const Iterator& rhs) const
                {
                    return owner != rhs.owner || index != rhs.index;
                }
            };

            struct ConstIterator
            {
                const NodeArray* owner = nullptr;
                usize index = 0;

                const Node& operator*() const { return owner->operator[](index); }
                ConstIterator& operator++()
                {
                    ++index;
                    return *this;
                }
                bool operator!=(const ConstIterator& rhs) const
                {
                    return owner != rhs.owner || index != rhs.index;
                }
            };

            NodeArray() = default;
            NodeArray(NodeArray&&) = default;
            NodeArray& operator=(NodeArray&&) = default;

            NodeArray(const NodeArray& rhs)
            {
                copy_from(rhs);
            }

            NodeArray& operator=(const NodeArray& rhs)
            {
                if(this != &rhs)
                {
                    clear();
                    copy_from(rhs);
                }
                return *this;
            }

            void copy_from(const NodeArray& rhs)
            {
                data.reserve(rhs.data.size());
                for(const Ref<Node>& node : rhs.data)
                {
                    data.push_back(node ? node->clone() : Ref<Node>());
                }
            }

            void clear()
            {
                data.clear();
            }

            bool empty() const
            {
                return data.empty();
            }

            usize size() const
            {
                return data.size();
            }

            void push_back(const Node& node)
            {
                data.push_back(node.clone());
            }

            void push_back(Ref<Node> node)
            {
                data.push_back(move(node));
            }

            Node& operator[](usize index)
            {
                return *data[index].get();
            }

            const Node& operator[](usize index) const
            {
                return *data[index].get();
            }

            Node& back()
            {
                return *data.back().get();
            }

            const Node& back() const
            {
                return *data.back().get();
            }

            Iterator begin()
            {
                return Iterator{this, 0};
            }

            Iterator end()
            {
                return Iterator{this, data.size()};
            }

            ConstIterator begin() const
            {
                return ConstIterator{this, 0};
            }

            ConstIterator end() const
            {
                return ConstIterator{this, data.size()};
            }
        };

        inline f32 style_f32(NodeRenderContext& ctx, const Node& node, const Name& entry, f32 default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_1(default_value));
            return value.value.x;
        }

        inline Float2U style_f32x2(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float2U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_2(default_value));
            return Float2U(value.value.x, value.value.y);
        }

        inline Float3U style_f32x3(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float3U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_3(default_value));
            return Float3U(value.value.x, value.value.y, value.value.z);
        }

        inline Float4U style_f32x4(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float4U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_4(default_value));
            return value.value;
        }

        struct Layer
        {
            id_t id = 0;
            u32 root = U32_MAX;
            Float2U screen_position = Float2U(0.0f);
        };

        struct Description
        {
            u64 generation = 0;
            Vector<Layer> layers;
            NodeArray nodes;
        };
    }
}
