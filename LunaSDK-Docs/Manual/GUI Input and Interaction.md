GUI input is host-independent. Window events, in-game raycast results, remote UI streams or tests all feed the same `GUI::InputEvent` records into a context.

## Designed functionality
The input and interaction system provides reusable primitives for higher-level GUI packages:

1. Queue input events in screen logical coordinates.
2. Route pointer and keyboard events through layers and hit-testable elements.
3. Maintain hover, active, focus, capture and click state.
4. Deliver raw and element-local input events to target elements.
5. Keep platform text input and clipboard integration separate from Window.

GUI routes input. It does not interpret a routed event as a specific widget action such as slider drag, text editing or menu navigation. That interpretation belongs to the immediate API package.

Composite protocols such as drag-drop also belong to high-level packages. They can be implemented by combining generic hit testing, pointer capture, routed input events, state objects, layers and draw commands without adding payload or source/target concepts to GUI.

## Concepts
### Input event
`GUI::InputEvent` describes pointer, keyboard, UTF-8 text, semantic navigation and focus events. Pointer positions are in screen logical coordinates with a top-left origin.

Keyboard keys use `Luna::KeyCode` from Runtime. GUI does not define another key enum.

`device_id` and `pointer_id` are preserved in submitted and delivered events, but the current context maintains one shared pointer stream and one shared keyboard state. Simultaneous multi-pointer routing is not implemented: separate pointer IDs do not receive independent hover, button or capture state.

### Interactable
`GUI::Interactable` is optional element data that enables input behavior. Most boolean capabilities are packed in `InteractableFlag`:

1. `hoverable`: produce hover state.
2. `activatable`: produce active/capture/click state.
3. `focusable`: receive keyboard focus.
4. `scrollable`: receive wheel events routed from descendant hit targets.
5. `disabled`: ignore interactive behavior.
6. `read_only`: allow focus and raw event delivery, but suppress automatic active, pointer-capture and click state.

`pointer_hit_behavior` and `focus_scope` remain separate fields because they are not simple capability bits.
`PointerHitBehavior` controls pointer hit routing:

1. `none`: do not participate in pointer hit testing.
2. `pass_through`: receive pointer events and interaction state, then continue to lower elements.
3. `target`: receive pointer events and stop routing.
4. `block`: stop routing without receiving pointer events.

### Interaction state
`GUI::InteractionState` is produced by `IContext::route_input`. It reports state for an element and its subtree, including hover, active, focus, click, double-click and pointer positions.

Multiple `pass_through` elements can be hovered or active at the same time. `active` means an element participates in the current primary pointer interaction; use `IContext::captured_element` to identify the single representative pointer-capture owner.

### Semantic navigation
Navigation is represented by intent-level input events instead of hard-wired keyboard behavior:

1. `navigation_dpad`: directional focus movement.
2. `navigation_move`: sequential forward or backward focus movement.
3. `navigation_confirm`: activate or confirm the focused element.
4. `navigation_back`: request back or cancel behavior from the focused element.

GUI does not translate `KeyCode` values into these events. The host adapter decides the mapping. The default `GUIWindow` adapter maps arrow keys to D-pad, Tab and Shift+Tab to sequential movement, Enter to confirm, and Escape to back; it also forwards the original key event.

Elements use automatic navigation without storing extra data. Calling `set_navigation_config` adds a sparse `NavigationConfig` record for that element. Every direction and action can use `automatic`, `none`, or `callback` behavior. Callback mode receives a `NavigationRequest` and can call `focus_element` for custom navigation or `navigate_default` to explicitly request the automatic fallback.

### Delivered events
`get_delivered_input_events` returns input events delivered to one element. `get_routed_input_events` additionally includes element-local pointer positions.

### Text input and clipboard
Text editing controls request platform text input with `request_text_input`. The host reads `get_text_input_state` and activates IME or virtual keyboard behavior.

Clipboard access is provided by `ClipboardIO` callbacks. Window-backed adapters and in-game hosts install these callbacks; GUI itself does not depend on Window.

## Programming guide
### Queue input
```cpp
GUI::InputEvent event;
event.type = GUI::InputEventType::pointer_move;
event.position = pointer_screen_pos;
context->add_input_event(event);
```

For a batch:

```cpp
context->add_input_events(Span<const GUI::InputEvent>(events.data(), events.size()));
```

Call these after `begin_frame` and before `route_input`.

### Attach interactable data
```cpp
GUI::Interactable interactable;
interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
set_flags(interactable.flags, GUI::InteractableFlag::activatable);
set_flags(interactable.flags, GUI::InteractableFlag::focusable);
context->set_interactable(element, interactable);
```

### Route input
After the element tree has been built and laid out, call:

```cpp
context->route_input();
```

Input routing depends on layout results. Route after layout, not before it.

### Input routing algorithm
`route_input` consumes the queued input events in submission order and updates the context interaction state for the current frame. The current implementation follows this outline:

1. Clear delivered-event buffers and transient interaction flags such as `hovered`, `clicked`, `double_clicked` and subtree flags.
2. Remove interaction records whose elements no longer exist in the current element tree.
3. Clear active or focused IDs when the corresponding element was not rebuilt, became disabled or can no longer receive the requested state.
4. Process every queued input event in order. Pointer events update pointer position, pointer delta, modifier keys and the inside-screen flag before routing.
5. Hit testing walks layers from top to bottom, then elements from newest to oldest inside each layer. An element can be hit only when its layout rectangle contains the pointer and its non-empty `clip_rect` also contains the pointer.
6. Pointer target selection uses `hit_test`. Both `PointerHitBehavior::pass_through` and `PointerHitBehavior::target` are event targets. Pass-through targets receive events and let traversal continue; target and block stop traversal, while block receives no events.
7. Pointer movement is delivered to every active element during a primary pointer interaction. Otherwise it is delivered to every current event target, including pass-through targets.
8. Pointer down is delivered to every current event target. A left-button down can activate every compatible activatable target, choose the first such target as the representative capture owner, and focus the first compatible focusable target.
9. A left-button up is delivered to every active target. Each active target still under the pointer can produce `clicked`, and may produce `double_clicked` when it is close enough in time and screen distance to its previous click. Non-primary releases follow the current hit target list and do not release primary capture.
10. Wheel events are delivered to the nearest scrollable ancestor of the highest routed event target that has one. If none has a scrollable ancestor, they are delivered to the routed event targets.
11. Raw keyboard and UTF-8 text events are delivered to the focused element. Navigation occurs only when the host submits one of the semantic `navigation_*` events.
12. `blur` clears pointer-inside, hover, active, focus, last-click state and key/button states.
13. After all events are processed, the context writes final `hovered`, `active` and `focused` flags, then propagates hover/active/focus/click/double-click state to ancestors as subtree flags.

The router records two event streams. `get_delivered_input_events` returns the original events delivered to an element. `get_routed_input_events` also includes element-local pointer coordinates computed from the element layout rectangle and its layer screen position.

`LayoutResult::clip_rect` participates in hit testing and is automatically intersected with every element-owned draw command during compilation. `DrawCommandType::push_clip` and `DrawCommandType::pop_clip` add optional nested clipping to that automatic element clip.

### Query interaction state
```cpp
GUI::InteractionState state = context->get_interaction_state(button.id);
if(state.clicked)
{
    // The high-level package can toggle state, open a popup, submit an action, etc.
}
```

`subtree_hovered`, `subtree_active` and related subtree flags are useful for compound controls whose visual state depends on children.

### Use hit testing
```cpp
GUI::ElementHandle routing_stop = context->hit_test(context->get_pointer_position());
if(routing_stop.index != GUI::INVALID_ELEMENT)
{
    // Inspect routing_stop.id or use it for debug highlighting.
}
```

`hit_test` returns the final routing stop: a `target` or `block` element. A position that hits only pass-through elements returns an invalid handle even though those elements are reported through the callback and receive pointer events. Hit testing checks upper layers before lower layers. Pass a callback to observe every visited element before routing stops:

```cpp
context->hit_test(context->get_pointer_position(), [](const GUI::HitTestVisit& visit) {
    // Inspect visit.element, visit.pointer_hit_behavior, visit.event_target and visit.routing_stop.
});
```

### Use pass-through hit boxes
Set `pointer_hit_behavior` to `pass_through` when an element should receive pointer events without preventing lower elements from receiving them. Add or omit `hoverable`, `activatable`, and `focusable` flags to choose which interaction states it should produce.

```cpp
GUI::Interactable overlay;
overlay.pointer_hit_behavior = GUI::PointerHitBehavior::pass_through;
set_flags(overlay.flags, GUI::InteractableFlag::hoverable);
context->set_interactable(overlay_element, overlay);
```

### Custom hit test
Rectangle hit testing is the default and requires no per-element callback record. Install sparse `ElementHitTestConfig` with `ElementHitTestMode::callback` when an element needs an arbitrary hit shape such as a circle or pie-menu sector. GUI first checks the layout rectangle and clip rectangle, then calls the callback with element-local and screen-space coordinates.

```cpp
GUI::ElementHitTestConfig hit_test;
hit_test.mode = GUI::ElementHitTestMode::callback;
hit_test.callback = hit_test_circle;
hit_test.userdata = &circle_data;
context->set_hit_test_config(round_button, hit_test);
```

### Focus and pointer capture
```cpp
context->focus_element(text_field.id);
context->capture_pointer(slider_thumb.id);
context->release_pointer_capture(slider_thumb.id);
```

The focus target must exist, be focusable, and not disabled. A capture target must exist, be activatable, and be neither disabled nor read-only. Capture applies to the shared pointer stream. Explicit `capture_pointer` resets the active target set to the captured element; automatic primary-pointer capture may leave multiple active pass-through targets, so subsequent pointer moves and primary-button releases are delivered to every active target until release, blur, or a new primary interaction.

### Request text input
```cpp
if(context->get_interaction_state(input_element.id).focused)
{
    context->request_text_input(input_element, cursor_byte_offset);
}

GUI::TextInputState text_input = context->get_text_input_state();
```

The host adapter should use `text_input.rect` in screen coordinates for IME candidate window placement.

### Install clipboard callbacks
```cpp
GUI::ClipboardIO clipboard;
clipboard.userdata = host;
clipboard.get_text = get_clipboard_text;
clipboard.set_text = set_clipboard_text;
context->set_clipboard_io(clipboard);
```

## Examples
### A clickable raw element
```cpp
GUI::ElementHandle item = context->begin_element(context->make_id("raw-button"));
context->set_layout_config(item, button_layout);

GUI::Interactable interactable;
interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
set_flags(interactable.flags, GUI::InteractableFlag::activatable);
set_flags(interactable.flags, GUI::InteractableFlag::focusable);
context->set_interactable(item, interactable);

context->end_element();

// After layout and route_input:
GUI::InteractionState state = context->get_interaction_state(item.id);
if(state.clicked)
{
    do_action();
}
```

### Routed pointer coordinates
```cpp
for(const GUI::RoutedInputEvent& event : context->get_routed_input_events(canvas.id))
{
    if(event.event.type == GUI::InputEventType::pointer_move && event.has_element_position)
    {
        Float2U local = event.element_position;
        update_handle_hover(local);
    }
}
```

Use routed coordinates for controls such as sliders, canvas editors and color pickers.
