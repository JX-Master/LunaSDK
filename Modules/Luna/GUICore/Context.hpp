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
#include "DrawCommand.hpp"
#include "Performance.hpp"
#include "Style.hpp"
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

            //! Gets the latest pointer position seen by the shared GUI Core pointer routing stream.
            //! @return Returns the pointer position in screen logical coordinates.
            virtual Float2U get_pointer_position() const = 0;

            //! Gets the pointer movement accumulated while routing the latest input batch.
            //! @return Returns the pointer delta in screen logical coordinates.
            virtual Float2U get_pointer_delta() const = 0;

            //! Checks whether the latest pointer position is inside the GUI screen.
            //! @return Returns `true` if the pointer is inside @ref FrameDesc::screen_size.
            virtual bool is_pointer_inside() const = 0;

            //! Checks whether one pointer button is currently held down on the shared GUI Core pointer routing stream.
            //! @param[in] button The pointer button to query.
            //! @return Returns `true` if the button has received a pointer-down event without a matching pointer-up or blur event.
            virtual bool is_pointer_button_down(PointerButton button) const = 0;

            //! Checks whether one key is currently held down in the shared GUI Core keyboard state.
            //! @param[in] key The key to query.
            //! @return Returns `true` if the key has received a key-down event without a matching key-up or blur event.
            virtual bool is_key_down(KeyCode key) const = 0;

            //! Gets the latest keyboard modifier state reported by the shared input stream.
            //! @return Returns the current modifier flags.
            virtual KeyModifierFlag get_key_modifiers() const = 0;

            //! Queues one input event for the current frame.
            //! @param[in] event The input event in screen logical coordinates.
            virtual void add_input_event(const InputEvent& event) = 0;

            //! Queues multiple input events for the current frame.
            //! @param[in] events The events in the order they should be processed.
            virtual void add_input_events(Span<const InputEvent> events) = 0;

            //! Gets input events queued for the current frame.
            //! @return Returns a read-only span in submission order.
            //! @remark The returned span is invalidated by the next input event insertion or @ref begin_frame call.
            virtual Span<const InputEvent> get_input_events() const = 0;

            //! Begins building one layer.
            //! @param[in] id The stable layer ID.
            //! @param[in] screen_position The layer top-left position in screen coordinates.
            //! @remark The first element created after this call becomes this layer's root element.
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f)) = 0;

            //! Ends the current layer.
            virtual void pop_layer() = 0;

            //! Gets all layers submitted for the current frame in bottom-to-top order.
            //! @return Returns a read-only span of the dense layer array.
            //! @remark The returned span is invalidated by the next layer insertion or @ref begin_frame call.
            virtual Span<const Layer> get_layers() const = 0;

            //! Sets the human-readable debug name of one layer.
            //! @param[in] id Stable layer ID.
            //! @param[in] name Human-readable debug name.
            //! @remark Debug names are observational metadata and must not affect GUI behavior.
            virtual void set_layer_debug_name(id_t id, const Name& name) = 0;

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

            //! Gets the complete data scope stack from the default scope to the current scope.
            //! @return Returns a read-only span of scoped IDs.
            //! @remark The returned span is invalidated by the next data scope mutation or @ref begin_frame call.
            virtual Span<const id_t> get_data_scope_stack() const = 0;

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
            //! @return Returns the created element handle.
            virtual ElementHandle begin_element(id_t id) = 0;

            //! Ends the current element.
            virtual void end_element() = 0;

            //! Sets layout configuration for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] config The layout configuration copied into the element.
            virtual void set_layout_config(const ElementHandle& element, const LayoutConfig& config) = 0;

            //! Sets optional layout callbacks for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] config The callback configuration copied into context-owned sparse storage.
            //! @remark Callback and userdata values must remain valid until layout finishes for this frame.
            virtual void set_layout_callback_config(const ElementHandle& element,
                const LayoutCallbackConfig& config) = 0;

            //! Gets optional layout callbacks attached to an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @return Returns the attached callback configuration, or a default configuration for invalid handles and
            //! elements without layout callbacks.
            virtual LayoutCallbackConfig get_layout_callback_config(const ElementHandle& element) const = 0;

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
            //! attached @ref LayoutCallbackConfig callback is applied top-down with optional finalize callbacks.
            virtual RV apply_layout(const ElementHandle& root, const RectF& rect) = 0;

            //! Measures one element using its layout configuration and optional measure callback.
            //! @param[in] element The element to measure.
            //! @param[in] available_size Available element-box size before margin is applied.
            //! @return Returns resolved element-box minimum, desired and maximum sizes, excluding margin.
            //! @remark If the element does not provide a measure callback, `fit` axes use zero content size and
            //! resolve to padding plus explicit @ref SizeValue constraints.
            virtual MeasureResult measure_element(const ElementHandle& element, const Float2U& available_size) = 0;

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

            //! Binds one named style to an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] style The style name to bind. Passing an empty name clears the binding.
            virtual void bind_style(const ElementHandle& element, const Name& style) = 0;

            //! Gets one element by dense index.
            //! @param[in] index The element index.
            //! @return Returns the element, or `nullptr` if @p index is invalid.
            virtual const Element* get_element(u32 index) const = 0;

            //! Gets all elements submitted for the current frame in dense storage order.
            //! @return Returns a read-only span of the dense element array.
            //! @remark The returned span is invalidated by the next element insertion or @ref begin_frame call.
            virtual Span<const Element> get_elements() const = 0;

            //! Finds one element by stable ID.
            //! @param[in] id The stable element ID.
            //! @return Returns the element, or `nullptr` if no element with this ID exists.
            virtual const Element* find_element(id_t id) const = 0;

            //! Finds one element handle by stable ID.
            //! @param[in] id The stable element ID.
            //! @return Returns the current-frame element handle, or an invalid handle if no element with this ID exists.
            virtual ElementHandle find_element_handle(id_t id) const = 0;

            //! Sets the human-readable debug name of one element.
            //! @param[in] element The element to name.
            //! @param[in] name Human-readable debug name.
            //! @remark Debug names are observational metadata and must not affect GUI behavior.
            virtual void set_element_debug_name(const ElementHandle& element, const Name& name) = 0;

            //! Sets delayed draw behavior for an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @param[in] config The draw callback configuration to attach.
            //! @remark The callback and userdata must remain valid until @ref generate_draw_commands or
            //! @ref compile_draw_commands finishes for this frame. Invalid handles are ignored.
            virtual void set_draw_config(const ElementHandle& element, const DrawConfig& config) = 0;

            //! Gets delayed draw behavior attached to an element.
            //! @param[in] element The element handle returned by @ref begin_element.
            //! @return Returns the attached configuration, or a default configuration for invalid handles and
            //! elements without delayed draw behavior.
            virtual DrawConfig get_draw_config(const ElementHandle& element) const = 0;

            //! Generates the final draw command stream for the current element trees.
            //! @return Returns success or failure code.
            //! @remark Generation traverses layers from bottom to top and elements in painter order. Element draw
            //! callbacks run at their configured phases, while commands recorded through @ref draw and
            //! @ref draw_for_element retain their build-time positions. Call this after layout, input routing, and
            //! higher-level package state resolution when complete commands must be inspected before compilation.
            virtual RV generate_draw_commands() = 0;

            //! Gets the latest generated draw commands in frame submission order.
            //! @return Returns a read-only span of the latest draw command stream.
            //! @remark This is useful for diagnostics, tooling, and custom rendering inspection. Content-driven
            //! measurement should use @ref LayoutCallbackConfig::measure_callback rather than scanning draw commands.
            //! Call @ref generate_draw_commands first when the frame uses draw callbacks. Callers can filter by
            //! @ref DrawCommand::element when they need element-local commands.
            virtual Span<const DrawCommand> get_draw_commands() const = 0;

            //! Records or emits one primitive draw command.
            //! @param[in] command The command to append to the current layer and current element.
            //! @remark During element construction this records a static command at the current painter-order
            //! position. During a draw callback this emits a generated command for the callback's element.
            virtual void draw(const DrawCommand& command) = 0;

            //! Records one primitive draw command for a specific element.
            //! @param[in] element The element that owns the draw command.
            //! @param[in] command The command to append to the element's layer.
            //! @remark This is useful for layout-dependent commands that are emitted after the element build scope
            //! has ended. Invalid handles are ignored.
            virtual void draw_for_element(const ElementHandle& element, const DrawCommand& command) = 0;

            //! Compiles generated draw commands into one VG shape draw list.
            //! @param[in] draw_list The destination VG draw list.
            //! @return Returns success or failure code.
            //! @remark The destination draw list is reset before commands are emitted. If draw commands have not
            //! been generated explicitly, this call generates them first. Commands are emitted in layer Z order.
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
            //! @ref PointerHitBehavior::pass_through elements are reported as event targets, then traversal continues
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

            //! Captures subsequent pointer movement and primary pointer release events to one element.
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

            //! Gets one style record by name.
            //! @param[in] name The style name.
            //! @return Returns the style record, or `nullptr` if the style is not defined.
            //! @remark The returned pointer is invalidated when the style store is modified.
            virtual const Style* get_style(const Name& name) const = 0;

            //! Gets all styles defined in this context.
            //! @return Returns a read-only reference to the named style store.
            //! @remark References and iterators are invalidated when the style store is modified.
            virtual const HashMap<Name, Style>& get_styles() const = 0;

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
        };

        //! Creates a new GUI Core context.
        //! @return Returns the created context.
        LUNA_GUICORE_API Ref<IContext> new_context();

        //! Gets the GUI Core module object.
        //! @return Returns the GUI Core module object.
        LUNA_GUICORE_API Module* module_gui_core();
    }
}
