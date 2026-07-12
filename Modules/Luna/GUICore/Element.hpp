/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Element.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Input.hpp"

namespace Luna
{
    namespace GUICore
    {
        struct IContext;
        struct ElementHandle;

        //! Specifies how a size value should be interpreted by layout algorithms.
        enum class SizeKind : u8
        {
            //! Use content-driven size.
            fit,
            //! Use an absolute logical-coordinate size.
            fixed,
            //! Use a percentage of the parent content size.
            percent
        };

        //! Describes one axis size request.
        struct SizeValue
        {
            //! The size interpretation mode.
            SizeKind kind = SizeKind::fit;
            //! The numeric value used by @ref SizeKind::fixed and @ref SizeKind::percent.
            f32 value = 0.0f;
            //! Minimum resolved size.
            f32 min = 0.0f;
            //! Maximum resolved size. Values less than zero mean no maximum.
            f32 max = -1.0f;
        };

        //! Describes the minimum, desired and maximum content size returned by a measure callback.
        //! @remark These sizes describe the element content box before padding is added. Layout algorithms add
        //! padding and apply @ref SizeValue constraints when resolving the final element box size.
        struct MeasureResult
        {
            //! Minimum content size that can still present the element meaningfully.
            Float2U minimum = Float2U(0.0f);
            //! Preferred content size when neither flex grow nor flex shrink is applied.
            Float2U desired = Float2U(0.0f);
            //! Maximum content size. Use `F32_MAX` on an axis to mark it as unbounded.
            Float2U maximum = Float2U(F32_MAX, F32_MAX);
        };

        //! Called by layout measurement to compute package-defined content size for one element.
        //! @param[in] context The context that owns @p element.
        //! @param[in] element The element being measured.
        //! @param[in] available_content_size Available content-box size after margin and padding are removed.
        //! @param[in] userdata User data stored in @ref LayoutCallbackConfig.
        //! @return Returns content-box minimum, desired and maximum sizes.
        using MeasureCallback = MeasureResult(*)(IContext* context, const ElementHandle& element,
            const Float2U& available_content_size, void* userdata);

        //! Called by @ref IContext::apply_layout to arrange or finalize one element subtree.
        //! @param[in] context The context that owns @p element.
        //! @param[in] element The element being arranged.
        //! @param[in] rect The element rectangle in layer coordinates.
        //! @param[in] userdata User data stored in @ref LayoutCallbackConfig.
        //! @return Returns success or failure code.
        using LayoutCallback = RV(*)(IContext* context, const ElementHandle& element, const RectF& rect, void* userdata);

        //! Describes frequently accessed layout input attached directly to one typeless element.
        struct LayoutConfig
        {
            //! Width request.
            SizeValue width;
            //! Height request.
            SizeValue height;
            //! Margin in left, top, right, bottom order.
            Float4U margin = Float4U(0.0f);
            //! Padding in left, top, right, bottom order.
            Float4U padding = Float4U(0.0f);
            //! Flex grow factor used by flex layout on the main axis. Zero disables growing.
            f32 flex_grow = 0.0f;
            //! Flex shrink factor used by flex layout on the main axis. Zero disables shrinking.
            f32 flex_shrink = 1.0f;
        };

        //! Describes optional layout callbacks stored in the context sparse callback array.
        struct LayoutCallbackConfig
        {
            //! Optional semantic identifier for the package-defined or core-provided layout algorithm.
            //! @remark GUI Core does not dispatch callbacks by this value. It is intended for diagnostics and
            //! higher-level capability recognition.
            Name algorithm;
            //! Optional content measure callback. If this is `nullptr`, content minimum and desired sizes are zero.
            MeasureCallback measure_callback = nullptr;
            //! Arrange callback. If this is `nullptr`, the element does not arrange its children.
            LayoutCallback callback = nullptr;
            //! Optional post-order callback called after children are arranged.
            LayoutCallback finalize_callback = nullptr;
            //! User data passed to layout callbacks. The owner that installs the callback owns this memory.
            void* userdata = nullptr;
        };

        //! Identifies one linear layout axis.
        enum class LayoutAxis : u8
        {
            //! Lays out children from left to right.
            x,
            //! Lays out children from top to bottom.
            y
        };

        //! Identifies how flex layout wraps children into multiple lines.
        enum class FlexWrap : u8
        {
            //! Places all children on one line.
            none,
            //! Wraps children to additional lines in the cross-axis positive direction.
            wrap,
            //! Wraps children to additional lines in the cross-axis negative direction.
            wrap_reverse
        };

        //! Identifies how flex layout distributes free space.
        enum class FlexAlignment : u8
        {
            //! Aligns items to the start of the axis.
            start,
            //! Aligns items to the center of the axis.
            center,
            //! Aligns items to the end of the axis.
            end,
            //! Stretches items or lines to consume the available cross-axis space.
            stretch,
            //! Places equal space between adjacent items or lines.
            space_between,
            //! Places equal space around every item or line.
            space_around,
            //! Places equal space before, between and after every item or line.
            space_evenly
        };

        //! Describes one flex layout pass.
        struct FlexLayoutDesc
        {
            //! Child placement axis.
            LayoutAxis axis = LayoutAxis::y;
            //! Whether the main-axis placement direction is reversed.
            bool reverse = false;
            //! Wrapping mode.
            FlexWrap wrap = FlexWrap::none;
            //! Main-axis alignment, equivalent to CSS `justify-content`.
            FlexAlignment main_alignment = FlexAlignment::start;
            //! Cross-axis item alignment, equivalent to CSS `align-items`.
            FlexAlignment cross_alignment = FlexAlignment::stretch;
            //! Cross-axis line alignment, equivalent to CSS `align-content`.
            FlexAlignment line_alignment = FlexAlignment::start;
            //! Gap between adjacent children on the main axis.
            f32 main_axis_gap = 0.0f;
            //! Gap between adjacent flex lines on the cross axis.
            f32 cross_axis_gap = 0.0f;
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Identifies how grid layout derives its column count and cell size.
        enum class GridLayoutMode : u8
        {
            //! Uses @ref GridLayoutDesc::cell_size as an absolute cell size and derives the column count from available width.
            fixed_cell_size,
            //! Uses @ref GridLayoutDesc::column_count and derives the cell width from available width.
            fixed_column_count
        };

        //! Describes one row-major grid layout pass.
        struct GridLayoutDesc
        {
            //! Grid sizing mode.
            GridLayoutMode mode = GridLayoutMode::fixed_cell_size;
            //! Cell size in layer logical coordinates.
            //! @remark In @ref GridLayoutMode::fixed_column_count mode, `x` is ignored and `y` is used as row height.
            Float2U cell_size = Float2U(64.0f, 64.0f);
            //! Number of columns used by @ref GridLayoutMode::fixed_column_count mode.
            u32 column_count = 1;
            //! Gap between adjacent cells.
            Float2U gap = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Describes one child placement rule for canvas layout.
        struct CanvasLayoutItem
        {
            //! Child element ID this rule applies to. Zero means the rule is ignored for ID matching.
            id_t element_id = 0;
            //! Minimum anchor in parent content rectangle normalized coordinates.
            Float2U anchor_min = Float2U(0.0f);
            //! Maximum anchor in parent content rectangle normalized coordinates.
            //! @remark If one axis has equal min/max anchors, that axis uses the child layout size and pivot.
            //! If one axis has different anchors, that axis stretches between the two anchored edges.
            Float2U anchor_max = Float2U(0.0f);
            //! Offset in left, top, right, bottom order.
            //! @remark For non-stretched axes, only the left/top component for that axis is used as anchored position offset.
            Float4U offset = Float4U(0.0f);
            //! Pivot used when an axis is not stretched. `(0, 0)` means top-left and `(0.5, 0.5)` means center.
            Float2U pivot = Float2U(0.0f);
        };

        //! Describes one canvas layout pass.
        struct CanvasLayoutDesc
        {
            //! Placement records matched by child element ID.
            Span<const CanvasLayoutItem> items;
            //! Fallback placement used when no item matches a child.
            CanvasLayoutItem default_item;
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Describes one scroll viewport layout pass.
        struct ScrollViewportLayoutDesc
        {
            //! Current content scroll offset in layer logical coordinates.
            Float2U scroll_offset = Float2U(0.0f);
            //! Maximum absolute scroll displacement expected between two consecutive frames.
            //! @remark Higher-level code can expand the previous-frame visible rectangle by this amount before
            //! deciding which children to submit. GUI Core does not apply overscan or implement virtualization
            //! itself. Values should be non-negative.
            Float2U max_scroll_delta = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the viewport content rectangle.
            bool clip_children = true;
        };

        //! Identifies how one table track size is resolved.
        enum class TableTrackSizeKind : u8
        {
            //! Uses the largest measured cell content on this track.
            fit,
            //! Uses an absolute pixel size.
            pixels,
            //! Uses a percentage of the table content size.
            percent,
            //! Consumes remaining space using a weighted ratio.
            ratio
        };

        //! Describes one table row or column track.
        struct TableTrackDesc
        {
            //! Track sizing mode.
            TableTrackSizeKind kind = TableTrackSizeKind::fit;
            //! Numeric value used by @ref TableTrackSizeKind::pixels, @ref TableTrackSizeKind::percent and
            //! @ref TableTrackSizeKind::ratio.
            f32 value = 0.0f;
            //! Minimum resolved track size.
            f32 min = 0.0f;
            //! Maximum resolved track size. Values less than zero mean no maximum.
            f32 max = -1.0f;
        };

        //! Describes one child-to-cell attachment for table layout.
        struct TableLayoutCell
        {
            //! Child element ID this cell applies to.
            id_t element_id = 0;
            //! Zero-based row index.
            u32 row = 0;
            //! Zero-based column index.
            u32 column = 0;
            //! Number of rows occupied by the cell. Zero is treated as one.
            u32 row_span = 1;
            //! Number of columns occupied by the cell. Zero is treated as one.
            u32 column_span = 1;
            //! Cell padding in left, top, right, bottom order.
            Float4U padding = Float4U(0.0f);
        };

        //! Describes one table track layout pass.
        struct TableLayoutDesc
        {
            //! Column tracks.
            Span<const TableTrackDesc> columns;
            //! Row tracks.
            Span<const TableTrackDesc> rows;
            //! Child-to-cell attachments.
            Span<const TableLayoutCell> cells;
            //! Gap between adjacent columns and rows.
            Float2U gap = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the table content rectangle.
            bool clip_children = true;
        };

        //! Describes the layout result of one element.
        struct LayoutResult
        {
            //! The element rectangle in layer coordinates.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! The element clip rectangle in layer coordinates.
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! The measured content size.
            Float2U content_size = Float2U(0.0f);
        };

        //! Controls how one element participates in pointer hit testing.
        enum class PointerHitBehavior : u8
        {
            //! This element does not participate in pointer hit testing.
            none,
            //! This element receives pointer events, then pointer routing continues to lower elements.
            pass_through,
            //! This element can receive pointer events and stops pointer routing.
            target,
            //! This element stops pointer routing without receiving pointer events.
            block
        };

        struct Interactable;

        //! Bit flags describing reusable input behavior attached to one element.
        enum class InteractableFlag : u16
        {
            //! No interactable behavior.
            none = 0x0000,
            //! This element can become hovered.
            hoverable = 0x0004,
            //! This element can become active through pointer interaction.
            activatable = 0x0008,
            //! This element can receive keyboard focus.
            focusable = 0x0010,
            //! This element can receive pointer wheel events routed from descendant hit targets.
            scrollable = 0x0020,
            //! This element is disabled for interaction.
            disabled = 0x0040,
            //! This element is read-only.
            read_only = 0x0080
        };

        //! Checks whether one or more interactable flags are set.
        //! @param[in] interactable The interactable data to test.
        //! @param[in] flags The flags to test.
        //! @return Returns `true` if all flags in @p flags are set.
        inline bool has_flags(const Interactable& interactable, InteractableFlag flags);

        //! Describes reusable input behavior attached to one element.
        struct Interactable
        {
            //! Interactable behavior flags.
            InteractableFlag flags = InteractableFlag::none;
            //! Pointer hit-test behavior.
            PointerHitBehavior pointer_hit_behavior = PointerHitBehavior::none;
            //! Optional focus scope ID.
            id_t focus_scope = 0;
        };

        inline bool has_flags(const Interactable& interactable, InteractableFlag flags)
        {
            return test_flags(interactable.flags, flags);
        }

        //! Controls how one element handles navigation input.
        enum class NavigationMode : u8
        {
            //! Let GUI Core run its automatic navigation behavior.
            automatic,
            //! Consume the navigation input without moving focus or invoking callbacks.
            none,
            //! Invoke the element navigation callback.
            callback
        };

        //! Describes one navigation request passed to a navigation callback.
        struct NavigationRequest
        {
            //! Element that receives the navigation request.
            id_t source = 0;
            //! Original navigation input event kind.
            InputEventType event_type = InputEventType::navigation_dpad;
            //! Direction payload for @ref InputEventType::navigation_dpad requests.
            NavigationDirection direction = NavigationDirection::right;
            //! Move payload for @ref InputEventType::navigation_move requests.
            NavigationMove move = NavigationMove::forward;
            //! Original input event.
            InputEvent event;
        };

        //! Called when one element receives a navigation request in @ref NavigationMode::callback mode.
        //! @param[in] context The context that owns the source element.
        //! @param[in] request The navigation request.
        //! @param[in] userdata User data stored in @ref NavigationConfig.
        //! @return Returns `true` if the callback handled the request. Returns `false` to consume it as a no-op.
        //! @remark Call @ref IContext::navigate_default inside the callback to explicitly fall back to automatic behavior.
        using NavigationCallback = bool(*)(IContext* context, const NavigationRequest& request, void* userdata);

        //! Describes navigation input behavior attached to one element.
        struct NavigationConfig
        {
            //! Behavior for @ref NavigationDirection::left.
            NavigationMode left = NavigationMode::automatic;
            //! Behavior for @ref NavigationDirection::right.
            NavigationMode right = NavigationMode::automatic;
            //! Behavior for @ref NavigationDirection::up.
            NavigationMode up = NavigationMode::automatic;
            //! Behavior for @ref NavigationDirection::down.
            NavigationMode down = NavigationMode::automatic;
            //! Behavior for @ref NavigationMove::forward.
            NavigationMode forward = NavigationMode::automatic;
            //! Behavior for @ref NavigationMove::backward.
            NavigationMode backward = NavigationMode::automatic;
            //! Behavior for @ref InputEventType::navigation_confirm.
            NavigationMode confirm = NavigationMode::automatic;
            //! Behavior for @ref InputEventType::navigation_back.
            NavigationMode back = NavigationMode::automatic;
            //! Callback used when the selected navigation mode is @ref NavigationMode::callback.
            NavigationCallback callback = nullptr;
            //! User data passed to @ref callback.
            void* userdata = nullptr;
        };

        //! Controls how one element decides whether a pointer position is inside its hit region.
        enum class ElementHitTestMode : u8
        {
            //! Use the element layout rectangle.
            rect,
            //! Use @ref ElementHitTestConfig::callback after the default layout rectangle and clip checks pass.
            callback
        };

        //! Describes one custom element hit-test request.
        struct ElementHitTestRequest
        {
            //! Element being tested.
            id_t source = 0;
            //! Pointer position in screen coordinates.
            Float2U screen_position = Float2U(0.0f);
            //! Pointer position relative to the element layout rectangle's top-left corner.
            Float2U element_position = Float2U(0.0f);
            //! Element layout rectangle in layer coordinates.
            RectF element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Element clip rectangle in layer coordinates.
            RectF element_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Element layout rectangle in screen coordinates.
            RectF screen_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Element clip rectangle in screen coordinates.
            RectF screen_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        };

        //! Called when one element uses @ref ElementHitTestMode::callback.
        //! @param[in] context The context that owns the tested element.
        //! @param[in] request The custom hit-test request.
        //! @param[in] userdata User data stored in @ref ElementHitTestConfig.
        //! @return Returns `true` if the pointer hits the element's custom shape.
        //! @remark The callback is a shape refinement inside the element layout rectangle and clip rectangle.
        using ElementHitTestCallback = bool(*)(const IContext* context, const ElementHitTestRequest& request, void* userdata);

        //! Describes custom pointer hit-test behavior attached to one element.
        struct ElementHitTestConfig
        {
            //! Hit-test mode used by this element.
            ElementHitTestMode mode = ElementHitTestMode::rect;
            //! Callback used when @ref mode is @ref ElementHitTestMode::callback.
            ElementHitTestCallback callback = nullptr;
            //! User data passed to @ref callback.
            void* userdata = nullptr;
        };

        //! Per-frame and cross-frame interaction state produced by GUI Core input routing.
        struct InteractionState
        {
            //! Whether the element is currently hovered.
            bool hovered = false;
            //! Whether the element is active for the current primary pointer interaction.
            //! @remark Multiple pass-through event targets can be active at once. Use @ref IContext::captured_element
            //! to identify the one representative pointer capture owner.
            bool active = false;
            //! Whether the element is currently focused.
            bool focused = false;
            //! Whether the element was clicked during the latest input routing pass.
            bool clicked = false;
            //! Whether the element was double-clicked during the latest input routing pass.
            bool double_clicked = false;
            //! Latest pointer position routed to this element in screen coordinates.
            Float2U pointer_screen_position = Float2U(0.0f);
            //! Latest pointer position routed to this element in element-local coordinates.
            Float2U pointer_element_position = Float2U(0.0f);
            //! Element rectangle used when computing @ref pointer_element_position.
            RectF pointer_element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Screen-space pointer position that produced the latest click.
            Float2U clicked_screen_position = Float2U(0.0f);
            //! Element-local pointer position that produced the latest click.
            //! @remark This is relative to the element layout rectangle's top-left corner.
            Float2U clicked_element_position = Float2U(0.0f);
            //! Element rectangle used when computing @ref clicked_element_position.
            RectF clicked_element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether this element or any of its descendants is currently hovered.
            bool subtree_hovered = false;
            //! Whether this element or any of its descendants is active for the current primary pointer interaction.
            bool subtree_active = false;
            //! Whether this element or any of its descendants is currently focused.
            bool subtree_focused = false;
            //! Whether this element or any of its descendants was clicked during the latest input routing pass.
            bool subtree_clicked = false;
            //! Whether this element or any of its descendants was double-clicked during the latest input routing pass.
            bool subtree_double_clicked = false;
        };

        //! One typeless GUI Core element record.
        //! @remark Element behavior is defined by attached data and external algorithms, not by inheritance or virtual methods.
        struct Element
        {
            //! Stable element ID.
            id_t id = 0;
            //! Owning layer index.
            u32 layer = INVALID_LAYER;
            //! Parent element index, or @ref INVALID_ELEMENT.
            u32 parent = INVALID_ELEMENT;
            //! First child element index, or @ref INVALID_ELEMENT.
            u32 first_child = INVALID_ELEMENT;
            //! Last child element index, or @ref INVALID_ELEMENT.
            u32 last_child = INVALID_ELEMENT;
            //! Next sibling element index, or @ref INVALID_ELEMENT.
            u32 next_sibling = INVALID_ELEMENT;
            //! Previous sibling element index, or @ref INVALID_ELEMENT.
            u32 prev_sibling = INVALID_ELEMENT;
            //! Element depth in its layer tree.
            u32 depth = 0;
            //! Style bound to this element.
            Name style;
            //! Human-readable debug name.
            Name debug_name;
            //! Layout configuration for this element.
            LayoutConfig layout;
            //! Sparse layout callback configuration index owned by the context, or `U32_MAX` when no callbacks are attached.
            u32 layout_callback_config = U32_MAX;
            //! Layout result for this element.
            LayoutResult layout_result;
            //! Optional interaction behavior.
            Interactable interactable;
            //! Sparse navigation configuration index owned by the context, or `U32_MAX` for automatic navigation.
            u32 navigation_config = U32_MAX;
            //! Sparse custom hit-test configuration index owned by the context, or `U32_MAX` for rectangle hit testing.
            u32 hit_test_config = U32_MAX;
            //! Sparse draw configuration index owned by the context, or `U32_MAX` when no callback is attached.
            u32 draw_config = U32_MAX;
            //! First draw command emission index for this element.
            //! @remark This is diagnostic summary data, not a contiguous command range. Commands can be interleaved
            //! when @ref IContext::draw_for_element is used after normal element construction.
            u32 first_draw_command = U32_MAX;
            //! Number of draw commands emitted by this element.
            //! @remark Filter @ref IContext::get_draw_commands by @ref DrawCommand::element to enumerate this
            //! element's actual commands.
            u32 draw_command_count = 0;
        };

        //! One GUI layer record.
        struct Layer
        {
            //! Stable layer ID.
            id_t id = 0;
            //! Layer top-left position in screen coordinates.
            Float2U screen_position = Float2U(0.0f);
            //! Root element index, or @ref INVALID_ELEMENT.
            u32 root = INVALID_ELEMENT;
            //! First draw command emission index for this layer.
            //! @remark This is diagnostic summary data. Use @ref draw_command_indices for the authoritative order.
            u32 first_draw_command = U32_MAX;
            //! Number of draw commands emitted by this layer.
            u32 draw_command_count = 0;
            //! Draw command indexes emitted by this layer.
            Vector<u32> draw_command_indices;
            //! Human-readable debug name.
            Name debug_name;
        };

        //! Handle returned by GUI Core element creation APIs.
        struct ElementHandle
        {
            //! Stable element ID.
            id_t id = 0;
            //! Element index in the current frame.
            u32 index = INVALID_ELEMENT;
            //! Context generation that produced this handle.
            u32 generation = 0;
        };
    }
}
