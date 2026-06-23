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
#include "Base.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Specifies how a size value should be interpreted by layout algorithms.
        enum class SizeKind : u8
        {
            //! Use content-driven size.
            fit,
            //! Use the largest measured content size among sibling elements on the same axis.
            fit_largest,
            //! Use an absolute pixel size.
            pixels,
            //! Use a percentage of the parent size.
            percent,
            //! Consume remaining space using a weighted ratio.
            ratio,
            //! Expand to the available size.
            expand
        };

        //! Describes one axis size request.
        struct SizeValue
        {
            //! The size interpretation mode.
            SizeKind kind = SizeKind::fit;
            //! The numeric value used by @ref SizeKind::pixels, @ref SizeKind::percent, @ref SizeKind::ratio
            //! and @ref SizeKind::expand.
            f32 value = 0.0f;
            //! Minimum resolved size.
            f32 min = 0.0f;
            //! Maximum resolved size. Values less than zero mean no maximum.
            f32 max = -1.0f;
        };

        //! Describes layout input attached to one typeless element.
        struct LayoutInput
        {
            //! Width request.
            SizeValue width;
            //! Height request.
            SizeValue height;
            //! Margin in left, top, right, bottom order.
            Float4U margin = Float4U(0.0f);
            //! Padding in left, top, right, bottom order.
            Float4U padding = Float4U(0.0f);
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

        //! Controls how one hit element affects pointer routing to elements behind it.
        enum class PointerInputPropagation : u8
        {
            //! Pointer input stops at this element when it is hit.
            //! @remark This preserves the traditional topmost GUI hit-test behavior.
            stop,
            //! Pointer input can continue to elements below this one.
            //! @remark The element is still visible to @ref IContext::hit_test and debug tooling, but internal
            //! input routing skips it when selecting the event target.
            pass_through
        };

        struct Interactable;

        //! Bit flags describing reusable input behavior attached to one element.
        enum class InteractableFlag : u16
        {
            //! No interactable behavior.
            none = 0x0000,
            //! This element participates in hit testing.
            hit_test = 0x0001,
            //! This element blocks pointer hit testing from reaching lower layers or elements behind it.
            //! @remark A blocking element does not receive input events unless @ref hit_test is also enabled.
            blocks_pointer_input = 0x0002,
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
            //! Controls whether pointer input can continue to lower elements after this element is hit.
            PointerInputPropagation pointer_input_propagation = PointerInputPropagation::stop;
            //! Optional focus scope ID.
            id_t focus_scope = 0;
            //! Payload types this element can provide as a drag-drop source.
            Vector<Name> drag_source_types;
            //! Payload types this element can accept as a drag-drop target.
            Vector<Name> drag_target_types;
        };

        inline bool has_flags(const Interactable& interactable, InteractableFlag flags)
        {
            return test_flags(interactable.flags, flags);
        }

        //! Per-frame and cross-frame interaction state produced by GUI Core input routing.
        struct InteractionState
        {
            //! Whether the element is currently hovered.
            bool hovered = false;
            //! Whether the element is currently active through pointer capture.
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
            //! Whether this element or any of its descendants is currently active through pointer capture.
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
            //! Layout input for this element.
            LayoutInput layout;
            //! Layout result for this element.
            LayoutResult layout_result;
            //! Optional interaction behavior.
            Interactable interactable;
            //! First draw command emitted by this element.
            u32 first_draw_command = U32_MAX;
            //! Number of draw commands emitted by this element.
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
            //! First draw command emitted by this layer.
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
