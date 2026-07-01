/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Debug.hpp"
#include "DragDrop.hpp"
#include "Input.hpp"
#include "Context.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Describes one element visited by @ref IContext::hit_test.
        struct HitTestVisit
        {
            //! Visited element handle.
            ElementHandle element;
            //! Visited element data.
            const Element* element_data = nullptr;
            //! Whether this element can receive pointer events for this hit-test traversal.
            bool event_target = false;
            //! Whether hit-test traversal stops at this element.
            bool routing_stop = false;
            //! Pointer hit behavior that caused this visit.
            PointerHitBehavior pointer_hit_behavior = PointerHitBehavior::none;
        };

        //! Called for every element visited by @ref IContext::hit_test.
        //! @param[in] visit The visited element information.
        //! @param[in] userdata User data passed to @ref IContext::hit_test.
        using HitTestCallback = void(const HitTestVisit& visit, void* userdata);

        //! @interface IContext
        //! Owns GUI Core frame data, layers, elements, input events, state objects, styles and draw commands.
        //! @remark GUI Core contexts are widget-free. High-level immediate API packages operate on this interface to
        //! submit typeless elements and primitive draw commands.
        struct [[Luna::interface("{B5B4D4D6-EFAB-4EB8-9D6B-DA10722CB5FD}")]] IContext : virtual Interface
        {
            //! Begins a new frame and resets per-frame element, layer, input and draw command storage.
            //! @param[in] desc The frame description supplied by the host screen.
            virtual void begin_frame(const FrameDesc& desc) = 0;

            //! Gets the current context generation.
            //! @return Returns the generation value used to validate frame-local handles.
            virtual u32 generation() const = 0;

            //! Gets the frame description supplied to the latest @ref begin_frame call.
            //! @return Returns the current frame description.
            virtual FrameDesc get_frame_desc() const = 0;

            //! Gets the latest pointer position seen by the GUI Core input router.
            //! @return Returns the pointer position in screen logical coordinates.
            virtual Float2U get_pointer_position() const = 0;

            //! Gets the pointer movement accumulated while routing the latest input batch.
            //! @return Returns the pointer delta in screen logical coordinates.
            virtual Float2U get_pointer_delta() const = 0;

            //! Checks whether the latest pointer position is inside the GUI screen.
            //! @return Returns `true` if the pointer is inside @ref FrameDesc::screen_size.
            virtual bool is_pointer_inside() const = 0;

            //! Checks whether one pointer button is currently held down.
            //! @param[in] button The pointer button to query.
            //! @return Returns `true` if the button has received a pointer-down event without a matching pointer-up or blur event.
            virtual bool is_pointer_button_down(PointerButton button) const = 0;

            //! Checks whether one key is currently held down.
            //! @param[in] key The key to query.
            //! @return Returns `true` if the key has received a key-down event without a matching key-up or blur event.
            virtual bool is_key_down(KeyCode key) const = 0;

            //! Gets the latest keyboard modifier state reported by input events.
            //! @return Returns the current modifier flags.
            virtual KeyModifierFlag get_key_modifiers() const = 0;

            //! Queues one input event for the current frame.
            //! @param[in] event The input event in screen logical coordinates.
            virtual void add_input_event(const InputEvent& event) = 0;

            //! Queues multiple input events for the current frame.
            //! @param[in] events The events in the order they should be processed.
            virtual void add_input_events(Span<const InputEvent> events) = 0;

            //! Begins building one layer.
            //! @param[in] id The stable layer ID.
            //! @param[in] screen_position The layer top-left position in screen coordinates.
            //! @param[in] debug_name Optional human-readable debug name.
            //! @remark The first element created after this call becomes this layer's root element.
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f), const Name& debug_name = Name()) = 0;

            //! Ends the current layer.
            virtual void pop_layer() = 0;

            //! Pushes one data scope used by @ref make_id.
            //! @param[in] id Stable scope ID.
            //! @remark Data scopes isolate generated IDs for high-level immediate API packages. They do not change
            //! the element tree topology by themselves.
            virtual void push_data_scope(id_t id) = 0;

            //! Pops the current data scope.
            virtual void pop_data_scope() = 0;

            //! Gets the current data scope.
            //! @return Returns the active data scope, or @ref DEFAULT_DATA_SCOPE when no explicit scope is active.
            virtual id_t current_data_scope() const = 0;

            //! Creates one scoped ID from a numeric local ID.
            //! @param[in] local_id Local ID inside the current data scope.
            //! @return Returns the generated stable ID.
            virtual id_t make_id(id_t local_id) const = 0;

            //! Creates one scoped ID from a string local ID.
            //! @param[in] local_name Local string ID inside the current data scope.
            //! @return Returns the generated stable ID.
            virtual id_t make_id(const c8* local_name) const = 0;

            //! Begins one typeless element.
            //! @param[in] id Stable element ID.
            //! @param[in] debug_name Optional human-readable debug name.
            //! @return Returns the created element handle.
            virtual ElementHandle begin_element(id_t id, const Name& debug_name = Name()) = 0;

            //! Ends the current element.
            virtual void end_element() = 0;

            //! Sets layout configuration for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] config The layout configuration copied into the element.
            virtual void set_layout_config(const ElementHandle& element, const LayoutConfig& config) = 0;

            //! Sets layout result for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] result The layout result to attach.
            //! @remark This is primarily useful while layout algorithms are being migrated into GUI Core.
            virtual void set_layout_result(const ElementHandle& element, const LayoutResult& result) = 0;

            //! Applies the layout callback tree rooted at one element.
            //! @param[in] root The root element to arrange.
            //! @param[in] rect The root rectangle in layer coordinates.
            //! @return Returns success or failure code.
            //! @remark Layout is a context-owned pass. The root rectangle is written first, then each element's
            //! attached @ref LayoutConfig callback is applied top-down with optional finalize callbacks.
            virtual RV apply_layout(const ElementHandle& root, const RectF& rect) = 0;

            //! Sets interactable data for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] interactable The interactable data to attach.
            virtual void set_interactable(const ElementHandle& element, const Interactable& interactable) = 0;

            //! Sets navigation input behavior for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] navigation The navigation behavior to attach.
            virtual void set_navigation_config(const ElementHandle& element, const NavigationConfig& navigation) = 0;

            //! Gets navigation input behavior for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @return Returns the element navigation behavior. Invalid handles return default automatic behavior.
            virtual NavigationConfig get_navigation_config(const ElementHandle& element) const = 0;

            //! Sets custom pointer hit-test behavior for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] hit_test The hit-test behavior to attach.
            //! @remark The default behavior is rectangle hit testing. Custom callbacks refine hits inside the element
            //! layout rectangle and clip rectangle.
            virtual void set_hit_test_config(const ElementHandle& element, const ElementHitTestConfig& hit_test) = 0;

            //! Gets custom pointer hit-test behavior for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @return Returns the element hit-test behavior. Invalid handles return default rectangle behavior.
            virtual ElementHitTestConfig get_hit_test_config(const ElementHandle& element) const = 0;

            //! Sets drag-drop source payload types for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] types Payload types this source explicitly provides.
            virtual void set_drag_drop_source_types(const ElementHandle& element, Span<const Name> types) = 0;

            //! Sets drag-drop target payload types for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] types Payload types this target explicitly accepts.
            virtual void set_drag_drop_target_types(const ElementHandle& element, Span<const Name> types) = 0;

            //! Binds one named style to an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] style The style name to bind. Passing an empty name clears the binding.
            virtual void bind_style(const ElementHandle& element, const Name& style) = 0;

            //! Gets one element by dense index.
            //! @param[in] index The element index.
            //! @return Returns the element, or `nullptr` if @p index is invalid.
            virtual const Element* get_element(u32 index) const = 0;

            //! Finds one element by stable ID.
            //! @param[in] id The stable element ID.
            //! @return Returns the element, or `nullptr` if no element with this ID exists.
            virtual const Element* find_element(id_t id) const = 0;

            //! Finds one element handle by stable ID.
            //! @param[in] id The stable element ID.
            //! @return Returns the current-frame element handle, or an invalid handle if no element with this ID exists.
            virtual ElementHandle find_element_handle(id_t id) const = 0;

            //! Gets all recorded draw commands in frame submission order.
            //! @return Returns a read-only span of draw commands recorded in the current frame.
            //! @remark Layout algorithms can inspect commands owned by one element to derive content-driven
            //! sizes. Callers should filter by @ref DrawCommand::element when they need element-local commands.
            virtual Span<const DrawCommand> get_draw_commands() const = 0;

            //! Records one primitive draw command.
            //! @param[in] command The command to append to the current layer and current element.
            virtual void draw(const DrawCommand& command) = 0;

            //! Records one primitive draw command for a specific element.
            //! @param[in] element The element that owns the draw command.
            //! @param[in] command The command to append to the element's layer.
            //! @remark This is useful for layout-dependent commands that are emitted after the element build scope
            //! has ended. Invalid handles are ignored.
            virtual void draw_for_element(const ElementHandle& element, const DrawCommand& command) = 0;

            //! Compiles recorded draw commands into one VG shape draw list.
            //! @param[in] draw_list The destination VG draw list.
            //! @return Returns success or failure code.
            //! @remark The destination draw list is reset before commands are emitted. Commands are emitted in layer Z order.
            virtual RV compile_draw_commands(VG::IShapeDrawList* draw_list) = 0;

            //! Registers one font that can be referenced by text draw commands.
            //! @param[in] id The stable font ID used by @ref DrawCommand::font.
            //! @param[in] font The font file object.
            //! @param[in] font_index The font face index inside @p font.
            //! @return Returns success or failure code.
            virtual RV register_font(const Name& id, Font::IFontFile* font, u32 font_index = 0) = 0;

            //! Gets one registered font.
            //! @param[in] id The stable font ID.
            //! @return Returns the registered font, or a null font when @p id is not registered.
            virtual FontDesc get_font(const Name& id) = 0;

            //! Sets clipboard callbacks used by higher-level text editing controls.
            //! @param[in] io Clipboard callback table.
            virtual void set_clipboard_io(const ClipboardIO& io) = 0;

            //! Gets clipboard callbacks currently installed in this context.
            //! @return Returns the clipboard callback table.
            virtual ClipboardIO get_clipboard_io() = 0;

            //! Requests platform text input for one element.
            //! @param[in] element The element that owns text input.
            //! @param[in] cursor Current UTF-8 byte cursor offset in the element text.
            //! @remark Higher-level text controls should call this every frame while focused. The final text input
            //! rectangle is resolved from the element layout result when @ref get_text_input_state is called.
            virtual void request_text_input(const ElementHandle& element, i32 cursor) = 0;

            //! Gets the active text input state for host IME and virtual keyboard integration.
            //! @return Returns the current text input state.
            virtual TextInputState get_text_input_state() = 0;

            //! Routes queued input events through layers and interactable elements.
            //! @remark Elements in upper layers receive input before elements in lower layers. This updates interaction
            //! states returned by @ref get_interaction_state.
            virtual void route_input() = 0;

            //! Hit-tests one screen position through layers and interactable elements.
            //! @param[in] screen_position The position in screen logical coordinates.
            //! @param[in] callback Optional callback invoked for every element that passes hit testing before routing stops.
            //! @param[in] userdata Opaque user data passed to @p callback.
            //! @return Returns the final pointer routing stop, or an invalid handle if nothing was hit.
            //! @remark The traversal checks upper layers first and newer elements first.
            //! @ref PointerHitBehavior::pass_through elements are reported to @p callback, then traversal continues
            //! to lower elements. Traversal stops at @ref PointerHitBehavior::target and @ref PointerHitBehavior::block.
            virtual ElementHandle hit_test(const Float2U& screen_position, HitTestCallback* callback = nullptr,
                void* userdata = nullptr) const = 0;

            //! Hit-tests one screen position and reports every visited element through one function object.
            //! @param[in] screen_position The position in screen logical coordinates.
            //! @param[in] callback The callback object invoked for every visited element.
            //! @return Returns the final pointer routing stop, or an invalid handle if nothing was hit.
            template <typename _Callback>
            ElementHandle hit_test(const Float2U& screen_position, _Callback&& callback) const
            {
                using Callback = remove_reference_t<_Callback>;
                struct CallbackContext
                {
                    Callback* callback;
                };
                CallbackContext callback_context { &callback };
                return hit_test(screen_position, [](const HitTestVisit& visit, void* userdata) {
                    CallbackContext* callback_context = (CallbackContext*)userdata;
                    (*callback_context->callback)(visit);
                }, &callback_context);
            }

            //! Gets the latest routed interaction state for one element.
            //! @param[in] id The stable element ID.
            //! @return Returns the interaction state. Missing elements return the default state.
            virtual InteractionState get_interaction_state(id_t id) const = 0;

            //! Gets input events delivered to one element during the latest input routing pass.
            //! @param[in] id The stable element ID.
            //! @return Returns the delivered input events. Missing elements return an empty span.
            //! @remark GUI Core only routes events to elements. High-level immediate API packages decide how to
            //! interpret key, text, wheel and pointer events for their own controls.
            virtual Span<const InputEvent> get_delivered_input_events(id_t id) = 0;

            //! Gets input events delivered to one element with target-local pointer coordinates.
            //! @param[in] id The stable element ID.
            //! @return Returns the routed input events. Missing elements return an empty span.
            //! @remark Raw @ref InputEvent positions remain in screen logical coordinates. Use this API when a
            //! control needs element-local pointer coordinates without reimplementing layer and layout transforms.
            virtual Span<const RoutedInputEvent> get_routed_input_events(id_t id) = 0;

            //! Sets keyboard focus to one focusable element.
            //! @param[in] id The stable element ID. Passing zero clears focus.
            //! @remark The target element must exist, be focusable and not disabled. Read-only elements can still receive focus.
            virtual void focus_element(id_t id) = 0;

            //! Gets the currently focused element ID.
            //! @return Returns the focused element ID, or zero when no element is focused.
            virtual id_t focused_element() const = 0;

            //! Runs the default automatic navigation behavior for one request.
            //! @param[in] request The navigation request to process.
            //! @return Returns `true` if the request changed focus or produced the default action.
            //! @remark Navigation callbacks can call this to explicitly fall back to GUI Core automatic behavior.
            virtual bool navigate_default(const NavigationRequest& request) = 0;

            //! Captures subsequent pointer movement and release events to one element.
            //! @param[in] id The element ID that should capture pointer input. Passing zero clears pointer capture.
            //! @remark The target element must exist, be activatable, not disabled, and not read-only. Pointer capture
            //! is reported as the element's active interaction state during input routing.
            virtual void capture_pointer(id_t id) = 0;

            //! Releases pointer capture.
            //! @param[in] id Optional element ID that must match the current capture owner. Passing zero releases any owner.
            virtual void release_pointer_capture(id_t id = 0) = 0;

            //! Gets the current pointer capture owner.
            //! @return Returns the captured element ID, or zero when no element captures pointer input.
            virtual id_t captured_element() const = 0;

            //! Starts one drag-drop operation from a source element.
            //! @param[in] source The drag-drop source element.
            //! @param[in] payload_type The payload type to provide.
            //! @param[in] data Payload data to copy into the context. May be `nullptr` when @p data_size is zero.
            //! @param[in] data_size Payload data size in bytes.
            //! @return Returns success or failure code.
            //! @remark The source element must explicitly provide @p payload_type through @ref set_drag_drop_source_types.
            virtual RV start_drag_drop(const ElementHandle& source, const Name& payload_type, const void* data, usize data_size) = 0;

            //! Clears the active drag-drop operation.
            virtual void clear_drag_drop() = 0;

            //! Checks whether a drag-drop operation is active.
            //! @return Returns `true` if a payload is currently being dragged.
            virtual bool is_drag_drop_active() const = 0;

            //! Gets the active drag-drop payload.
            //! @return Returns the active payload, or `nullptr` if no drag-drop operation is active.
            virtual const DragDropPayload* get_drag_drop_payload() = 0;

            //! Hit-tests one drag-drop target compatible with a payload type.
            //! @param[in] payload_type The payload type to accept.
            //! @param[in] screen_position The position in screen logical coordinates.
            //! @return Returns the topmost compatible target element, or an invalid handle if no compatible target is hit.
            virtual ElementHandle hit_test_drag_drop_target(const Name& payload_type, const Float2U& screen_position) const = 0;

            //! Gets one delivered drag-drop payload for a target.
            //! @param[in] target The target element.
            //! @param[in] payload_type The expected payload type.
            //! @return Returns the delivered payload, or `nullptr` if no compatible delivery exists.
            virtual const DragDropPayload* get_drag_drop_delivery(const ElementHandle& target, const Name& payload_type) = 0;

            //! Gets a state object by ID.
            //! @param[in] id The state identifier.
            //! @return Returns the boxed state object, or `nullptr` when no state exists.
            virtual object_t get_state(id_t id) = 0;

            //! Sets or refreshes a state object.
            //! @param[in] id The state identifier.
            //! @param[in] data The boxed state object.
            //! @param[in] lifetime The lifetime used for automatic cleanup.
            //! @return Returns success or failure code.
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;

            //! Clears one state object immediately.
            //! @param[in] id The state identifier.
            virtual void clear_state(id_t id) = 0;

            //! Defines a style if it does not already exist.
            //! @param[in] name The style name.
            //! @param[in] parent Optional parent style name.
            virtual void define_style(const Name& name, const Name& parent = Name()) = 0;

            //! Changes the parent style of an existing style.
            //! @param[in] name The style name.
            //! @param[in] parent The new parent style name, or an empty name for no parent.
            virtual void set_style_parent(const Name& name, const Name& parent) = 0;

            //! Sets or overrides one style entry.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            //! @param[in] value The entry value.
            virtual void set_style_value(const Name& style, const Name& entry, const StyleValue& value) = 0;

            //! Removes a local style entry so that the value is inherited from the parent style.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            virtual void inherit_style_entry(const Name& style, const Name& entry) = 0;

            //! Marks a style entry as explicitly unset, hiding an inherited value.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            virtual void unset_style_entry(const Name& style, const Name& entry) = 0;

            //! Pushes a style name onto the build style stack.
            //! @param[in] style The style name to bind to subsequently created elements.
            virtual void push_style(const Name& style) = 0;

            //! Pops the latest style from the build style stack.
            virtual void pop_style() = 0;

            //! Gets the currently active build style.
            //! @return Returns the style at the top of the style stack, or an empty name if the stack is empty.
            virtual Name current_style() const = 0;

            //! Resolves one style entry through the style inheritance chain.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            //! @param[in] default_value The fallback value if the entry is not found or is unset.
            //! @return Returns the resolved style value.
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) = 0;

            //! Registers or replaces one style entry schema.
            //! @param[in] schema Style entry metadata declared by a high-level immediate API package.
            //! @remark GUI Core stores this metadata for editors and debug tooling. It does not interpret the entry
            //! as widget behavior.
            virtual void register_style_entry_schema(const StyleEntrySchema& schema) = 0;

            //! Gets all registered style entry schemas.
            //! @return Returns the style entry schema records registered in this context.
            virtual Span<const StyleEntrySchema> get_style_entry_schemas() = 0;

            //! Gets performance counters collected for the current GUI Core frame.
            //! @return Returns a copy of the current performance counters.
            virtual PerformanceCounters get_performance_counters() = 0;

            //! Dumps a serializable debug snapshot for the current GUI Core frame.
            //! @return Returns the debug information snapshot.
            virtual DebugInfo dump_debug_info() = 0;

            //! Logs one debug issue for the current frame.
            //! @param[in] severity Issue severity.
            //! @param[in] category Subsystem or feature that reports the issue.
            //! @param[in] message Human-readable diagnostic message.
            //! @param[in] element Related element ID, or zero if the issue is not tied to one element.
            //! @remark Issues are frame-local and are serialized by @ref dump_debug_info.
            virtual void log_debug_issue(DebugIssueSeverity severity, const Name& category, const c8* message,
                id_t element = 0) = 0;

            //! Logs one debug pass reason for the current frame.
            //! @param[in] kind Subsystem that produced the pass.
            //! @param[in] name Stable pass name.
            //! @param[in] reason Stable reason identifier.
            //! @param[in] element Related element ID, or zero when the pass is not tied to one element.
            //! @param[in] detail Optional human-readable detail text.
            //! @param[in] duration_ms Optional elapsed time in milliseconds.
            //! @remark Pass records are frame-local and are serialized by @ref dump_debug_info. They are intended
            //! for debug tooling and do not affect layout, input or rendering behavior.
            virtual void log_debug_pass(DebugPassKind kind, const Name& name, const Name& reason, id_t element = 0,
                const c8* detail = nullptr, f64 duration_ms = 0.0) = 0;
        };

        //! Creates a new GUI Core context.
        //! @return Returns the created context.
        LUNA_GUICORE_API Ref<IContext> new_context();

        //! Queues the input events stored in one debug snapshot into a GUI Core context.
        //! @param[in] context The context that will receive the recorded input events.
        //! @param[in] info The debug snapshot whose input events should be replayed.
        //! @return Returns success or failure code.
        //! @remark This helper does not call @ref IContext::begin_frame or @ref IContext::route_input. Callers
        //! should rebuild the desired element tree for the replay frame, call this function, then route input.
        LUNA_GUICORE_API RV replay_input_events(IContext* context, const DebugInfo& info);

        //! Appends one debug frame to a timeline and moves the cursor to the appended frame.
        //! @param[in,out] timeline The timeline that receives the frame.
        //! @param[in] info The debug snapshot to append.
        //! @param[in] max_frames Optional maximum number of frames to keep. Passing zero keeps all frames.
        //! @remark When @p max_frames is non-zero and the timeline grows past the limit, the oldest frames are removed.
        LUNA_GUICORE_API void push_debug_frame(DebugFrameTimeline& timeline, const DebugInfo& info, usize max_frames = 0);

        //! Gets the current debug frame from a timeline.
        //! @param[in] timeline The timeline to read.
        //! @return Returns the current frame, or `nullptr` when the timeline is empty.
        LUNA_GUICORE_API const DebugInfo* current_debug_frame(const DebugFrameTimeline& timeline);

        //! Seeks to a specific debug frame index.
        //! @param[in,out] timeline The timeline to edit.
        //! @param[in] index The desired frame index.
        //! @return Returns the selected frame, or `nullptr` when the timeline is empty.
        //! @remark Out-of-range indexes are clamped to the last captured frame.
        LUNA_GUICORE_API const DebugInfo* seek_debug_frame(DebugFrameTimeline& timeline, usize index);

        //! Moves the debug frame cursor by a signed delta.
        //! @param[in,out] timeline The timeline to edit.
        //! @param[in] delta The signed frame offset to apply.
        //! @return Returns the selected frame, or `nullptr` when the timeline is empty.
        //! @remark Stepping is clamped to the captured frame range.
        LUNA_GUICORE_API const DebugInfo* step_debug_frame(DebugFrameTimeline& timeline, isize delta);

        //! Clears all frames from a debug timeline.
        //! @param[in,out] timeline The timeline to clear.
        LUNA_GUICORE_API void clear_debug_frames(DebugFrameTimeline& timeline);

        //! Gets the GUI Core module object.
        //! @return Returns the GUI Core module object.
        LUNA_GUICORE_API Module* module_gui_core();
    }
}
