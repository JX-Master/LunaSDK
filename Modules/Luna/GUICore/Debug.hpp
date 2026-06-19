/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Debug.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "DrawCommand.hpp"
#include "Style.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Runtime counters collected by one GUI Core context.
        struct PerformanceCounters
        {
            //! Context generation of the frame represented by these counters.
            u64 frame_generation = 0;
            //! Number of layers in the frame.
            u32 layer_count = 0;
            //! Number of elements in the frame.
            u32 element_count = 0;
            //! Number of input events queued for the frame.
            u32 input_event_count = 0;
            //! Number of input events delivered to routed elements.
            u32 delivered_input_event_count = 0;
            //! Number of elements that participate in hit testing.
            u32 interactable_count = 0;
            //! Number of draw commands recorded for the frame.
            u32 draw_command_count = 0;
            //! Number of states stored in the context after cleanup.
            u32 state_count = 0;
            //! Number of styles stored in the context.
            u32 style_count = 0;
            //! Number of style entry schemas registered in the context.
            u32 style_schema_count = 0;
            //! Number of debug issues logged for the current frame.
            u32 debug_issue_count = 0;
            //! Number of debug pass records logged for the current frame.
            u32 debug_pass_count = 0;
            //! Time spent clearing expired state objects during the latest @ref IContext::begin_frame call, in milliseconds.
            f64 state_gc_ms = 0.0;
            //! Time spent routing input events during the latest @ref IContext::route_input call, in milliseconds.
            f64 input_route_ms = 0.0;
            //! Time spent compiling GUI Core draw commands to a VG draw list during the latest
            //! @ref IContext::compile_draw_commands call, in milliseconds.
            f64 draw_compile_ms = 0.0;
            //! Time spent producing the latest debug snapshot through @ref IContext::dump_debug_info, in milliseconds.
            f64 debug_dump_ms = 0.0;
        };

        //! Identifies the subsystem that produced one debug pass record.
        enum class DebugPassKind : u8
        {
            //! Frame lifecycle pass.
            frame,
            //! Layout algorithm pass.
            layout,
            //! Input routing pass.
            input,
            //! Draw command compilation pass.
            render,
            //! State store maintenance pass.
            state,
            //! Package-defined pass.
            custom
        };

        //! Serializable debug pass record for diagnostics and external tooling.
        struct DebugPassInfo
        {
            //! Subsystem that produced this pass.
            DebugPassKind kind = DebugPassKind::custom;
            //! Stable pass name.
            Name name;
            //! Stable reason identifier.
            Name reason;
            //! Related element ID, or zero when the pass is not tied to one element.
            id_t element = 0;
            //! Context generation when the pass was logged.
            u64 frame_generation = 0;
            //! Optional elapsed time in milliseconds.
            f64 duration_ms = 0.0;
            //! Optional human-readable details.
            String detail;
        };

        //! Severity of one debug issue logged by GUI Core or a higher-level immediate API package.
        enum class DebugIssueSeverity : u8
        {
            //! Informational note.
            info = 0,
            //! Non-fatal suspicious condition.
            warning = 1,
            //! Error condition that likely affects GUI behavior or rendering.
            error = 2
        };

        //! Serializable debug issue record for one GUI Core frame.
        struct DebugIssueInfo
        {
            //! Issue severity.
            DebugIssueSeverity severity = DebugIssueSeverity::info;
            //! Subsystem or feature that reported the issue.
            Name category;
            //! Related element ID, or zero when the issue is not tied to one element.
            id_t element = 0;
            //! Human-readable diagnostic message.
            String message;
        };

        //! Debug snapshot for one layer.
        struct DebugLayerInfo
        {
            //! Stable layer ID.
            id_t id = 0;
            //! Layer root element index.
            u32 root = INVALID_ELEMENT;
            //! First draw command emitted by this layer.
            u32 first_draw_command = U32_MAX;
            //! Number of draw commands emitted by this layer.
            u32 draw_command_count = 0;
            //! Draw command indexes emitted by this layer.
            Vector<u32> draw_command_indices;
            //! Layer top-left position in screen coordinates.
            Float2U screen_position = Float2U(0.0f);
            //! Human-readable debug name.
            Name debug_name;
        };

        //! Debug snapshot for one local style entry.
        struct DebugStyleEntryInfo
        {
            //! Style entry name.
            Name entry;
            //! Local entry inheritance mode.
            StyleEntryMode mode = StyleEntryMode::inherit;
            //! Local entry value used when @ref mode is @ref StyleEntryMode::set.
            StyleValue value;
        };

        //! Debug snapshot for one style schema value resolved for an element.
        struct DebugResolvedStyleEntryInfo
        {
            //! Package or style family that declared this entry.
            Name owner;
            //! Style entry name.
            Name entry;
            //! Resolved value, or the schema default value when no style value is available.
            StyleValue value;
            //! Whether @ref value came from the schema default value.
            bool defaulted = true;
        };

        //! Debug snapshot for one element.
        struct DebugElementInfo
        {
            //! Stable element ID.
            id_t id = 0;
            //! Element index.
            u32 index = INVALID_ELEMENT;
            //! Owning layer index.
            u32 layer = INVALID_LAYER;
            //! Parent element index.
            u32 parent = INVALID_ELEMENT;
            //! First child index.
            u32 first_child = INVALID_ELEMENT;
            //! Last child index.
            u32 last_child = INVALID_ELEMENT;
            //! Next sibling index.
            u32 next_sibling = INVALID_ELEMENT;
            //! Previous sibling index.
            u32 prev_sibling = INVALID_ELEMENT;
            //! Element depth.
            u32 depth = 0;
            //! Bound style name.
            Name style;
            //! Debug name.
            Name debug_name;
            //! Layout input attached to this element.
            LayoutInput layout;
            //! Layout rectangle.
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Clip rectangle.
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Measured content size.
            Float2U content_size = Float2U(0.0f);
            //! Whether this element participates in hit testing.
            bool hit_test = false;
            //! Whether this element blocks pointer hit testing from reaching content behind it.
            bool blocks_pointer_input = false;
            //! Pointer routing propagation behavior used when this element is hit.
            PointerInputPropagation pointer_input_propagation = PointerInputPropagation::stop;
            //! Whether this element can become hovered.
            bool hoverable = false;
            //! Whether this element can become active through pointer interaction.
            bool activatable = false;
            //! Whether this element can receive keyboard focus.
            bool focusable = false;
            //! Whether this element can receive routed pointer wheel events.
            bool scrollable = false;
            //! Whether this element is disabled for interaction.
            bool disabled = false;
            //! Whether this element is readonly.
            bool readonly_ = false;
            //! Focus scope attached to this element.
            id_t focus_scope = 0;
            //! Payload types this element can provide as a drag-drop source.
            Vector<Name> drag_source_types;
            //! Payload types this element can accept as a drag-drop target.
            Vector<Name> drag_target_types;
            //! Style entries resolved for this element from registered style schemas.
            Vector<DebugResolvedStyleEntryInfo> resolved_style;
            //! Whether this element was hovered after the latest input routing pass.
            bool hovered = false;
            //! Whether this element was active after the latest input routing pass.
            bool active = false;
            //! Whether this element currently owns pointer capture.
            bool captured = false;
            //! Whether this element was focused after the latest input routing pass.
            bool focused = false;
            //! Whether this element was clicked during the latest input routing pass.
            bool clicked = false;
            //! Whether this element was double-clicked during the latest input routing pass.
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
            Float2U clicked_element_position = Float2U(0.0f);
            //! Element rectangle used when computing @ref clicked_element_position.
            RectF clicked_element_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether this element or any descendant was hovered after the latest input routing pass.
            bool subtree_hovered = false;
            //! Whether this element or any descendant was active after the latest input routing pass.
            bool subtree_active = false;
            //! Whether this element or any descendant was focused after the latest input routing pass.
            bool subtree_focused = false;
            //! Whether this element or any descendant was clicked during the latest input routing pass.
            bool subtree_clicked = false;
            //! Whether this element or any descendant was double-clicked during the latest input routing pass.
            bool subtree_double_clicked = false;
            //! First draw command emitted by this element.
            u32 first_draw_command = U32_MAX;
            //! Number of draw commands emitted by this element.
            u32 draw_command_count = 0;
        };

        //! Debug snapshot for one style.
        struct DebugStyleInfo
        {
            //! Style name.
            Name name;
            //! Parent style name.
            Name parent;
            //! Number of local entries in this style.
            u32 entry_count = 0;
            //! Local style entries defined by this style.
            Vector<DebugStyleEntryInfo> entries;
        };

        //! Debug snapshot for input events routed to one element.
        struct DebugInputDeliveryInfo
        {
            //! Target element ID.
            id_t element_id = 0;
            //! Events delivered to this element.
            Vector<InputEvent> events;
            //! Events delivered to this element with routed target-local pointer positions.
            Vector<RoutedInputEvent> routed_events;
        };

        //! Serializable debug snapshot produced by a GUI Core context.
        struct DebugInfo
        {
            //! Frame counters.
            PerformanceCounters counters;
            //! Layer snapshots in bottom-to-top order.
            Vector<DebugLayerInfo> layers;
            //! Element snapshots in dense storage order.
            Vector<DebugElementInfo> elements;
            //! Style snapshots.
            Vector<DebugStyleInfo> styles;
            //! Style entry schemas declared by high-level immediate API packages.
            Vector<StyleEntrySchema> style_schemas;
            //! Input events queued for the frame.
            Vector<InputEvent> input_events;
            //! Input events routed to elements.
            Vector<DebugInputDeliveryInfo> input_deliveries;
            //! Issues logged for this frame.
            Vector<DebugIssueInfo> issues;
            //! Pass records logged for this frame.
            Vector<DebugPassInfo> passes;
            //! Active data scope stack when the snapshot is produced.
            Vector<id_t> data_scope_stack;
            //! Hit-test result for the current pointer position, or zero when no element was hit.
            id_t hovered_element = 0;
            //! Current active element, or zero when no element is active.
            id_t active_element = 0;
            //! Current pointer capture element, or zero when no element captures pointer input.
            id_t captured_element = 0;
            //! Current focused element, or zero when no element is focused.
            id_t focused_element = 0;
            //! Current focus scope ID, or zero when the focus is in the root scope.
            id_t focused_scope = 0;
            //! Whether a drag-drop operation is active.
            bool drag_drop_active = false;
            //! Active drag-drop source element ID, or zero when no drag-drop operation is active.
            id_t drag_drop_source = 0;
            //! Active drag-drop payload type.
            Name drag_drop_type;
            //! Draw commands recorded for the frame.
            Vector<DrawCommand> draw_commands;
        };

        //! In-memory debug frame timeline used by debug panels and external tools to step through captured frames.
        //! @remark This is a plain data container. GUI Core does not control the host application's main loop; callers
        //! decide when to push snapshots produced by @ref IContext::dump_debug_info.
        struct DebugFrameTimeline
        {
            //! Captured frames in chronological order.
            Vector<DebugInfo> frames;
            //! Current cursor into @ref frames.
            usize cursor = 0;
        };
    }
}
