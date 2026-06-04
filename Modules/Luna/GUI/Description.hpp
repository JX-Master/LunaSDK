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
        //! @addtogroup GUI GUI
        //! @{

        struct IDrawList;
        struct Node;

        struct NodeInputContext;
        struct NodeMeasureContext;
        struct NodeRenderContext;

        //! Identifies the storage representation used by color edit views.
        enum class ColorValueType : u8
        {
            //! Floating point channels in the 0-1 range.
            f32,
            //! 8-bit integer channels in the 0-255 range.
            u8,
            //! Packed RGBA8 value.
            rgba8
        };

        //! Identifies a color channel group.
        enum class ColorChannelPart : u8
        {
            //! No channel group.
            none,
            //! RGB color channels.
            rgb,
            //! HSV color channels.
            hsv
        };

        //! Bit flags controlling image rendering.
        enum class ImageFlag : u32
        {
            //! Default image rendering.
            none = 0x00,
            //! Flip image sampling vertically.
            flip_y = 0x01,
            //! Use nearest-neighbor texture sampling.
            nearest = 0x02
        };

        //! Describes the high-level layer role of a node.
        enum class NodeLayerRole : u8
        {
            //! A regular node inside its current layer.
            normal,
            //! The root node of a layer.
            root,
            //! A popup layer root.
            popup,
            //! A tooltip layer root.
            tooltip
        };

        //! Describes the layout behavior implemented by a node.
        enum class NodeLayoutBehavior : u8
        {
            //! Linear container or leaf behavior.
            linear,
            //! Scroll view container behavior.
            scroll,
            //! Table layout behavior.
            table,
            //! Grid layout behavior.
            grid,
            //! Canvas layout behavior.
            canvas,
            //! Dock space layout behavior.
            dock_space,
            //! Tab bar header/content behavior.
            tab_bar,
            //! Tab item behavior.
            tab_item
        };

        //! Describes the flow direction for linear layout behavior.
        enum class NodeLayoutFlow : u8
        {
            //! Lay out children from top to bottom.
            vertical,
            //! Lay out children from left to right.
            horizontal
        };

        //! Dynamic state passed to render proxy callbacks for one node.
        struct NodeRenderState
        {
            //! Whether the item is currently hovered.
            bool hovered = false;
            //! Whether the item is currently active.
            bool active = false;
            //! Whether the item is currently focused.
            bool focused = false;
            //! The current screen logical size.
            Float2U surface_size = Float2U(0.0f);
            //! The current pointer position in screen coordinates.
            Float2U pointer_position = Float2U(0.0f);
            //! The current frame delta time in seconds.
            f32 delta_time = 1.0f / 60.0f;
            //! The accumulated context time in seconds.
            f64 time = 0.0;
        };

        //! Layout data passed to render proxies for one node.
        struct NodeRenderLayout
        {
            //! The node rectangle in layer coordinates.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! The node clip rectangle in layer coordinates.
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);

            //! Table column offsets in the table node's coordinate space.
            Vector<f32> table_column_offsets;
            //! Table column widths.
            Vector<f32> table_column_widths;
            //! Table row offsets in the table node's coordinate space.
            Vector<f32> table_row_offsets;
            //! Table row heights.
            Vector<f32> table_row_heights;
            //! Number of table columns.
            u32 table_columns = 0;
            //! Number of table rows.
            u32 table_rows = 0;

            //! Tab item header rectangle.
            RectF tab_header_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Tab item header clip rectangle.
            RectF tab_header_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Tab close button rectangle.
            RectF tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Left tab scroll button rectangle.
            RectF tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Right tab scroll button rectangle.
            RectF tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether the tab bar can scroll.
            bool tab_scrollable = false;
            //! Maximum tab scroll offset.
            f32 tab_scroll_max = 0.0f;
            //! Whether this tab item's content is visible.
            bool tab_content_visible = true;

            //! Whether this node is represented as a dock panel child.
            bool dock_panel_child = false;
            //! Whether the dock panel is visible.
            bool dock_panel_visible = true;
            //! Whether the dock panel is floating.
            bool dock_panel_floating = false;
            //! The owning dock space ID.
            id_t dock_space_id = 0;
            //! Dock panel outer rectangle.
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel clip rectangle.
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel title bar rectangle.
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel close button rectangle.
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel resize hit rectangle.
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Resolved dock panel style.
            DockPanelStyle dock_panel_style;
            //! Dock leaf index for docked panels.
            u32 dock_leaf_index = U32_MAX;

            //! Total content size of a scroll view.
            Float2U scroll_content_size = Float2U(0.0f);
            //! Visible viewport size of a scroll view.
            Float2U scroll_viewport_size = Float2U(0.0f);
            //! Whether vertical scrolling is available.
            bool scroll_has_vertical = false;
            //! Whether horizontal scrolling is available.
            bool scroll_has_horizontal = false;
        };

        //! Services available to render proxy callbacks.
        //! @remark Coordinates passed to drawing helpers are layer-local unless the method explicitly says otherwise.
        struct NodeRenderContext
        {
            virtual ~NodeRenderContext() = default;
            //! Gets the index of the node currently being rendered.
            //! @return Returns the node index in the submitted description.
            virtual u32 current_node_index() const = 0;
            //! Gets a node by index.
            //! @param[in] node_index The node index.
            //! @return Returns the node pointer, or `nullptr` if the index is invalid.
            virtual const Node* get_node(u32 node_index) const = 0;
            //! Finds a node by ID.
            //! @param[in] node_id The node ID.
            //! @return Returns the node pointer, or `nullptr` if no node with this ID exists.
            virtual const Node* find_node(id_t node_id) const = 0;
            //! Gets render layout data for one node.
            //! @param[in] node_index The node index.
            //! @param[out] out_layout The returned layout data.
            //! @return Returns `true` if layout data exists.
            virtual bool get_node_render_layout(u32 node_index, NodeRenderLayout& out_layout) const = 0;
            //! Gets the draw list for the current layer.
            //! @return Returns the draw list used by this render pass.
            virtual IDrawList* draw_list() = 0;
            //! Gets a context state object by ID.
            //! @param[in] id The state ID.
            //! @return Returns the boxed state object, or `nullptr`.
            virtual object_t get_state(id_t id) const = 0;
            //! Sets or refreshes a context state object.
            //! @param[in] id The state ID.
            //! @param[in] data The boxed state object.
            //! @param[in] lifetime The state lifetime.
            //! @return Returns success or failure code.
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;
            //! Clears one context state object.
            //! @param[in] id The state ID.
            virtual void clear_state(id_t id) = 0;
            //! Resolves a style entry.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            //! @param[in] default_value The fallback value.
            //! @return Returns the resolved style value.
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) const = 0;
            //! Measures the cursor x offset for a UTF-8 text prefix using the current node's font style.
            //! @param[in] text The text to measure.
            //! @param[in] cursor The cursor byte offset.
            //! @param[in] font_size The font size in logical units.
            //! @return Returns the cursor x offset in logical units.
            virtual f32 text_cursor_x(const String& text, usize cursor, f32 font_size) const = 0;
            //! Converts an x offset to a UTF-8 cursor byte offset using the current node's font style.
            //! @param[in] text The text to inspect.
            //! @param[in] x The x offset in logical units.
            //! @param[in] font_size The font size in logical units.
            //! @return Returns the nearest cursor byte offset.
            virtual usize text_cursor_from_x(const String& text, f32 x, f32 font_size) const = 0;

            //! Gets a typed widget state object owned by the specified ID.
            //! @param[in] owner_id The owner widget or subsystem ID.
            //! @return Returns the typed state object, or `nullptr`.
            template <typename T>
            T* get_widget_state(id_t owner_id) const
            {
                object_t obj = get_state(make_state_id<T>(owner_id));
                return obj ? cast_object<T>(obj) : nullptr;
            }

            //! Gets or creates a typed widget state object owned by the specified ID.
            //! @param[in] owner_id The owner widget or subsystem ID.
            //! @param[in] lifetime The lifetime used when creating or refreshing the state.
            //! @return Returns the typed state object.
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
            //! Checks whether a popup is currently open.
            //! @param[in] popup_id The popup ID.
            //! @return Returns `true` if the popup is open.
            virtual bool is_popup_open(id_t popup_id) const = 0;
            //! Draws a filled rectangle, optionally textured.
            //! @param[in] rect The rectangle in layer coordinates.
            //! @param[in] clip_rect The clip rectangle in layer coordinates.
            //! @param[in] color The fill color.
            //! @param[in] radius The corner radius.
            //! @param[in] texture Optional texture.
            //! @param[in] image_flags Optional image rendering flags.
            virtual void draw_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius = 0.0f,
                RHI::ITexture* texture = nullptr, ImageFlag image_flags = ImageFlag::none) = 0;
            //! Draws a filled rectangle with individually enabled rounded corners.
            virtual void draw_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                bool top_left, bool top_right, bool bottom_right, bool bottom_left) = 0;
            //! Draws a rectangle with four-corner color interpolation.
            virtual void draw_gradient_rect(const RectF& rect, const RectF& clip_rect,
                const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left) = 0;
            //! Draws a filled ellipse that fits inside `rect`.
            virtual void draw_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color) = 0;
            //! Draws one line segment.
            virtual void draw_line(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width = 1.0f) = 0;
            //! Draws UTF-8 text inside `rect`.
            virtual void draw_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color,
                TextAlignment horizontal_alignment = TextAlignment::begin,
                TextAlignment vertical_alignment = TextAlignment::center) = 0;
        };

        //! Services available during node measurement.
        struct NodeMeasureContext
        {
            virtual ~NodeMeasureContext() = default;
            //! Gets the parent node being measured under.
            //! @return Returns the parent node, or `nullptr` for a layer root.
            virtual const Node* parent() const = 0;
            //! Gets the current GUI screen size.
            //! @return Returns the screen logical size.
            virtual Float2U surface_size() const = 0;
            //! Measures UTF-8 text using GUI text layout rules.
            //! @param[in] text The UTF-8 text pointer.
            //! @param[in] text_size The text size in bytes.
            //! @param[in] font_size The font size in logical units.
            //! @param[in] max_width The wrapping width, or `F32_MAX` for no wrapping.
            //! @return Returns text layout metrics.
            virtual LayoutMetrics measure_text(const c8* text, usize text_size, f32 font_size, f32 max_width) const = 0;
        };

        //! Services available while a node updates input-related item state.
        struct NodeInputContext
        {
            virtual ~NodeInputContext() = default;
            //! Gets the current pointer position in screen coordinates.
            //! @return Returns the pointer position.
            virtual Float2U pointer_position() const = 0;
            //! Gets the node rectangle in layer coordinates.
            //! @return Returns the node rectangle.
            virtual RectF rect() const = 0;
            //! Gets a context state object by ID.
            //! @param[in] id The state ID.
            //! @return Returns the boxed state object, or `nullptr`.
            virtual object_t get_state(id_t id) const = 0;
            //! Sets or refreshes a context state object.
            //! @param[in] id The state ID.
            //! @param[in] data The boxed state object.
            //! @param[in] lifetime The state lifetime.
            //! @return Returns success or failure code.
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;
            //! Clears one context state object.
            //! @param[in] id The state ID.
            virtual void clear_state(id_t id) = 0;
            //! Publishes an item query state value for the current node.
            //! @param[in] key The query key name.
            //! @param[in] value The query value.
            virtual void set_state(const Name& key, const Any& value) = 0;
            //! Gets a typed widget state object owned by the specified ID.
            //! @param[in] owner_id The owner widget or subsystem ID.
            //! @return Returns the typed state object, or `nullptr`.
            template <typename T>
            T* get_widget_state(id_t owner_id) const
            {
                object_t obj = get_state(make_state_id<T>(owner_id));
                return obj ? cast_object<T>(obj) : nullptr;
            }
            //! Gets or creates a typed widget state object owned by the specified ID.
            //! @param[in] owner_id The owner widget or subsystem ID.
            //! @param[in] lifetime The lifetime used when creating or refreshing the state.
            //! @return Returns the typed state object.
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
            //! Checks whether a popup is currently open.
            //! @param[in] popup_id The popup ID.
            //! @return Returns `true` if the popup is open.
            virtual bool is_popup_open(id_t popup_id) const = 0;
            //! Opens the popup used by one menu node.
            //! @param[in] menu_id The menu item ID.
            virtual void open_menu_popup(id_t menu_id) = 0;
            //! Closes one popup and its descendants.
            //! @param[in] popup_id The popup ID.
            virtual void close_popup(id_t popup_id) = 0;
            //! Closes all currently open popups.
            virtual void close_all_popups() = 0;
        };

        //! Base type for all GUI description nodes.
        //! @remark User modules can implement custom widgets by deriving from this type and passing the node to
        //! @ref custom_node or @ref IContext::add_node. The context calls virtual methods for measurement, layout
        //! classification, hit testing and input state updates.
        struct Node
        {
            lustruct("GUI::Node", "{AD82DACD-76EC-4EE5-8A82-0A6C4CC8BD5C}");

            virtual ~Node() = default;

            //! Gets the concrete node type GUID.
            //! @return Returns the GUID registered for the concrete node type.
            virtual Guid type_guid() const = 0;

            //! Clones this node into a new boxed object.
            //! @return Returns the cloned node.
            virtual Ref<Node> clone() const = 0;

            //! Measures intrinsic size without an external measure context.
            //! @return Returns intrinsic layout metrics.
            virtual LayoutMetrics measure() const
            {
                LayoutMetrics metrics;
                metrics.min_size = Float2U(1.0f);
                metrics.preferred_size = Float2U(1.0f);
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                return metrics;
            }

            //! Measures intrinsic size with a measure context.
            //! @param[in] ctx The measure context.
            //! @return Returns intrinsic layout metrics.
            virtual LayoutMetrics measure(NodeMeasureContext& ctx) const
            {
                return measure();
            }

            //! Gets the role this node plays in its layer.
            //! @return Returns the node layer role.
            virtual NodeLayerRole layer_role() const
            {
                return NodeLayerRole::normal;
            }

            //! Gets the layout behavior implemented by this node.
            //! @return Returns the layout behavior.
            virtual NodeLayoutBehavior layout_behavior() const
            {
                return NodeLayoutBehavior::linear;
            }

            //! Gets the flow direction used when this node behaves as a linear container.
            //! @return Returns the layout flow.
            virtual NodeLayoutFlow layout_flow() const
            {
                return NodeLayoutFlow::vertical;
            }

            //! Gets whether this node should be interactive by default when added through widget APIs.
            //! @return Returns `true` if the node is interactive by default.
            virtual bool default_interactive() const
            {
                return false;
            }

            //! Gets whether the layout pass should call @ref measure(NodeMeasureContext&) for this node.
            //! @return Returns `true` if node-specific measurement is required.
            virtual bool uses_node_measure() const
            {
                return true;
            }

            //! Applies container-specific default layout settings to child layout descriptors.
            //! @param[in,out] desc The layout descriptor to modify.
            virtual void apply_container_defaults(LayoutDesc& desc) const {}

            //! Gets whether this node is enabled for interaction.
            //! @return Returns `true` if this node accepts input.
            virtual bool enabled_state() const
            {
                return true;
            }

            //! Performs hit testing for this node.
            //! @param[in] rect The node rectangle in layer coordinates.
            //! @param[in] clip_rect The clip rectangle in layer coordinates.
            //! @param[in] pos The pointer position in layer coordinates.
            //! @return Returns `true` when the position hits the node.
            virtual bool hit_test(const RectF& rect, const RectF& clip_rect, const Float2U& pos) const
            {
                return interactive &&
                    pos.x >= rect.offset_x && pos.x <= rect.offset_x + rect.width &&
                    pos.y >= rect.offset_y && pos.y <= rect.offset_y + rect.height &&
                    pos.x >= clip_rect.offset_x && pos.x <= clip_rect.offset_x + clip_rect.width &&
                    pos.y >= clip_rect.offset_y && pos.y <= clip_rect.offset_y + clip_rect.height;
            }

            //! Updates query states or custom state after input routing reaches this node.
            //! @param[in] ctx The input context.
            virtual void update_state(NodeInputContext& ctx) const {}

            //! Handles a click routed to this node.
            //! @param[in] ctx The input context.
            virtual void on_click(NodeInputContext& ctx) {}

            //! Stable node ID generated by the build API.
            id_t id = 0;
            //! Index of the layer that owns this node.
            u32 layer = U32_MAX;
            //! Parent node index, or `U32_MAX` for a layer root.
            u32 parent = U32_MAX;
            //! First child node index.
            u32 first_child = U32_MAX;
            //! Last child node index.
            u32 last_child = U32_MAX;
            //! Next sibling node index.
            u32 next_sibling = U32_MAX;
            //! Depth in the node tree.
            u32 depth = 0;
            //! Optional node text or label.
            String text;
            //! Style bound to this node during build.
            Name style;
            //! Render proxy callbacks used to draw this node.
            RenderProxyDesc render_proxy;
            //! Requested size supplied by widget APIs.
            Size requested_size;
            //! Layout style assigned to this node.
            LayoutStyle layout_style;
            //! Layout descriptor used when this node is a container.
            LayoutDesc layout_desc;
            //! Whether this node is positioned absolutely in its parent.
            bool absolute_position = false;
            //! Absolute position used when @ref absolute_position is true.
            Float2U position = Float2U(0.0f);
            //! Whether a user-specified clip rectangle is assigned.
            bool has_user_clip_rect = false;
            //! User-specified clip rectangle in layer coordinates.
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Payload types this node can provide as a drag source.
            Vector<Name> drag_drop_source_types;
            //! Payload types this node can accept as a drag target.
            Vector<Name> drag_drop_target_types;
            //! Whether this node participates in item hit testing.
            bool interactive = false;
        };

        //! Copyable wrapper around the description node storage.
        //! @remark Copying a node array clones each node object so that a @ref Description can be safely copied.
        struct NodeArray
        {
            //! Node storage.
            Vector<Ref<Node>> data;

            //! Mutable iterator over node objects.
            struct Iterator
            {
                //! Owning node array.
                NodeArray* owner = nullptr;
                //! Current node index.
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

            //! Const iterator over node objects.
            struct ConstIterator
            {
                //! Owning node array.
                const NodeArray* owner = nullptr;
                //! Current node index.
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

            //! Clones nodes from another node array.
            //! @param[in] rhs The node array to copy.
            NodeArray(const NodeArray& rhs)
            {
                copy_from(rhs);
            }

            //! Clones nodes from another node array.
            //! @param[in] rhs The node array to copy.
            //! @return Returns this node array.
            NodeArray& operator=(const NodeArray& rhs)
            {
                if(this != &rhs)
                {
                    clear();
                    copy_from(rhs);
                }
                return *this;
            }

            //! Clones nodes from another node array into this array.
            //! @param[in] rhs The node array to copy.
            void copy_from(const NodeArray& rhs)
            {
                data.reserve(rhs.data.size());
                for(const Ref<Node>& node : rhs.data)
                {
                    data.push_back(node ? node->clone() : Ref<Node>());
                }
            }

            //! Removes all nodes.
            void clear()
            {
                data.clear();
            }

            //! Checks whether the array is empty.
            //! @return Returns `true` if no nodes are stored.
            bool empty() const
            {
                return data.empty();
            }

            //! Gets the number of nodes.
            //! @return Returns node count.
            usize size() const
            {
                return data.size();
            }

            //! Clones and appends a node.
            //! @param[in] node The node to clone.
            void push_back(const Node& node)
            {
                data.push_back(node.clone());
            }

            //! Appends a node reference.
            //! @param[in] node The node object to append.
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

        //! Reads a scalar f32 style entry for a node.
        inline f32 style_f32(NodeRenderContext& ctx, const Node& node, const Name& entry, f32 default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_1(default_value));
            return value.value.x;
        }

        //! Reads a f32x2 style entry for a node.
        inline Float2U style_f32x2(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float2U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_2(default_value));
            return Float2U(value.value.x, value.value.y);
        }

        //! Reads a f32x3 style entry for a node.
        inline Float3U style_f32x3(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float3U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_3(default_value));
            return Float3U(value.value.x, value.value.y, value.value.z);
        }

        //! Reads a f32x4 style entry for a node.
        inline Float4U style_f32x4(NodeRenderContext& ctx, const Node& node, const Name& entry, const Float4U& default_value)
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::f32_4(default_value));
            return value.value;
        }

        //! Reads a name style entry for a node.
        inline Name style_name(NodeRenderContext& ctx, const Node& node, const Name& entry, const Name& default_value = Name())
        {
            StyleValue value = ctx.get_style_value(node.style, entry, StyleValue::name(default_value));
            return value.type == StyleValueType::name ? value.name_value : default_value;
        }

        //! Describes one GUI layer in a frame description.
        struct Layer
        {
            //! Stable layer ID.
            id_t id = 0;
            //! Root node index for this layer.
            u32 root = U32_MAX;
            //! Layer top-left position in screen coordinates.
            Float2U screen_position = Float2U(0.0f);
        };

        //! Complete GUI description object built for one frame.
        //! @remark A description is the ground truth of the frame. Nodes absent from the description are considered absent
        //! from the submitted GUI, while their context state may survive according to state lifetime rules.
        struct Description
        {
            //! Build generation used by handles produced for this description.
            u64 generation = 0;
            //! Layers in painter order, from bottom to top.
            Vector<Layer> layers;
            //! Dense node array referenced by layer roots and node parent/child links.
            NodeArray nodes;
        };

        //! @}
    }
}
