GUI Core input is host-independent. Window events, in-game raycast results, remote UI streams or tests all feed the same `GUICore::InputEvent` records into a context.

## Designed functionality
The input and interaction system provides reusable primitives for higher-level GUI packages:

1. Queue input events in screen logical coordinates.
2. Route pointer and keyboard events through layers and hit-testable elements.
3. Maintain hover, active, focus, capture and click state.
4. Deliver raw and element-local input events to target elements.
5. Provide drag-drop source, target and payload routing.
6. Keep platform text input and clipboard integration separate from Window.

GUI Core routes input. It does not interpret a routed event as a specific widget action such as slider drag, text editing or menu navigation. That interpretation belongs to the immediate API package.

## Concepts
### Input event
`GUICore::InputEvent` describes pointer, keyboard, UTF-8 text and focus events. Pointer positions are in screen logical coordinates with a top-left origin.

Keyboard keys use `Luna::KeyCode` from Runtime. GUI Core does not define another key enum.

### Interactable
`GUICore::Interactable` is optional element data that enables input behavior. Most boolean capabilities are packed in `InteractableFlag`:

1. `hit_test`: participate in hit testing.
2. `blocks_pointer_input`: block input from reaching lower layers or elements behind this one.
3. `hoverable`: produce hover state.
4. `activatable`: produce active/capture/click state.
5. `focusable`: receive keyboard focus.
6. `scrollable`: receive wheel events routed from descendant hit targets.
7. `disabled`: ignore interactive behavior.
8. `read_only`: allow focus but prevent active editing behavior.

`pointer_input_propagation`, `focus_scope` and drag-drop source/target payload type lists remain separate fields because they are not simple capability bits.

### Interaction state
`GUICore::InteractionState` is produced by `IContext::route_input`. It reports state for an element and its subtree, including hover, active, focus, click, double-click and pointer positions.

### Delivered events
`get_delivered_input_events` returns input events delivered to one element. `get_routed_input_events` additionally includes element-local pointer positions.

### Text input and clipboard
Text editing controls request platform text input with `request_text_input`. The host reads `get_text_input_state` and activates IME or virtual keyboard behavior.

Clipboard access is provided by `ClipboardIO` callbacks. Window-backed adapters and in-game hosts install these callbacks; GUI Core itself does not depend on Window.

### Drag-drop
Drag-drop source and target support is explicit. A source must declare provided payload types, and a target must declare accepted payload types.

## Programming guide
### Queue input
```cpp
GUICore::InputEvent event;
event.type = GUICore::InputEventType::pointer_move;
event.position = pointer_screen_pos;
context->add_input_event(event);
```

For a batch:

```cpp
context->add_input_events(Span<const GUICore::InputEvent>(events.data(), events.size()));
```

Call these after `begin_frame` and before `route_input`.

### Attach interactable data
```cpp
GUICore::Interactable interactable;
set_flags(interactable.flags, GUICore::InteractableFlag::hit_test);
set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
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
6. Pointer target selection uses `hit_test_input_target`. A hit-testable element with `pointer_input_propagation == stop` becomes the target. An element with `blocks_pointer_input` can block lower elements even when it is not otherwise interactive. A pass-through hit-testable element can still be returned by `hit_test` for debug use, but it does not stop routing.
7. Pointer movement is delivered to the active captured element when one exists; otherwise it is delivered to the currently hovered element.
8. Pointer down is delivered to the hit target. A left-button down can set the active element when the target is activatable and not read-only, and can set keyboard focus when the target is focusable.
9. Pointer up is delivered to the active element when pointer capture is active. A left-button up over the same active element produces `clicked`, and may produce `double_clicked` when it is close enough in time and screen distance to the previous click.
10. Wheel events are delivered to the nearest scrollable ancestor of the hit target. If no scrollable ancestor exists, the event is delivered to the hit target.
11. Keyboard and UTF-8 text events are delivered to the focused element. `Tab` moves focus through focusable elements; arrow keys first try spatial focus movement and fall back to delivery when no spatial target is found.
12. Drag-drop release looks for the topmost hit target that explicitly accepts the active payload type, delivers the pointer-up event to that target, stores the delivery payload and clears the active drag-drop state.
13. `blur` clears pointer-inside, hover, active, focus, last-click state, key/button states and active drag-drop state.
14. After all events are processed, the context writes final `hovered`, `active` and `focused` flags, then propagates hover/active/focus/click/double-click state to ancestors as subtree flags.

The router records two event streams. `get_delivered_input_events` returns the original events delivered to an element. `get_routed_input_events` also includes element-local pointer coordinates computed from the element layout rectangle and its layer screen position.

`LayoutResult::clip_rect` participates in hit testing, but drawing is clipped only by explicit `DrawCommandType::push_clip` and `DrawCommandType::pop_clip` commands. Layout clipping and draw clipping are intentionally separate so a package can choose where clip-stack changes are worth the rendering cost.

### Query interaction state
```cpp
GUICore::InteractionState state = context->get_interaction_state(button.id);
if(state.clicked)
{
    // The high-level package can toggle state, open a popup, submit an action, etc.
}
```

`subtree_hovered`, `subtree_active` and related subtree flags are useful for compound controls whose visual state depends on children.

### Use hit testing
```cpp
GUICore::ElementHandle hit = context->hit_test(context->get_pointer_position());
if(hit.index != GUICore::INVALID_ELEMENT)
{
    // Inspect hit.id or use it for debug highlighting.
}
```

Hit testing checks upper layers before lower layers.

### Use pass-through hit boxes
Set `pointer_input_propagation` to `pass_through` when an element should be visible to hit testing and debug tools but should not stop pointer routing.

```cpp
GUICore::Interactable overlay;
set_flags(overlay.flags, GUICore::InteractableFlag::hit_test);
set_flags(overlay.flags, GUICore::InteractableFlag::hoverable);
overlay.pointer_input_propagation = GUICore::PointerInputPropagation::pass_through;
context->set_interactable(overlay_element, overlay);
```

### Focus and capture
```cpp
context->focus_element(text_field.id);
context->capture_pointer(slider_thumb.id);
context->release_pointer_capture(slider_thumb.id);
```

The target element must exist and be compatible with the requested operation.

### Request text input
```cpp
if(context->get_interaction_state(input_element.id).focused)
{
    context->request_text_input(input_element, cursor_byte_offset);
}

GUICore::TextInputState text_input = context->get_text_input_state();
```

The host adapter should use `text_input.rect` in screen coordinates for IME candidate window placement.

### Install clipboard callbacks
```cpp
GUICore::ClipboardIO clipboard;
clipboard.userdata = host;
clipboard.get_text = get_clipboard_text;
clipboard.set_text = set_clipboard_text;
context->set_clipboard_io(clipboard);
```

### Start drag-drop
```cpp
Name payload_type("asset.guid");
context->set_drag_drop_source_types(source, Span<const Name>(&payload_type, 1));

Guid asset_guid = selected_asset;
luexp(context->start_drag_drop(source, payload_type, &asset_guid, sizeof(asset_guid)));
```

### Accept drag-drop
```cpp
context->set_drag_drop_target_types(target, Span<const Name>(&payload_type, 1));

const GUICore::DragDropPayload* payload =
    context->get_drag_drop_delivery(target, payload_type);
if(payload)
{
    const Guid* guid = payload->data_as<Guid>();
}
```

## Examples
### A clickable raw element
```cpp
GUICore::ElementHandle item = context->begin_element(context->make_id("raw-button"), Name("Raw Button"));
context->set_layout(item, button_layout);

GUICore::Interactable interactable;
set_flags(interactable.flags, GUICore::InteractableFlag::hit_test);
set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
context->set_interactable(item, interactable);

context->end_element();

// After layout and route_input:
GUICore::InteractionState state = context->get_interaction_state(item.id);
if(state.clicked)
{
    do_action();
}
```

### Routed pointer coordinates
```cpp
for(const GUICore::RoutedInputEvent& event : context->get_routed_input_events(canvas.id))
{
    if(event.event.type == GUICore::InputEventType::pointer_move && event.has_element_position)
    {
        Float2U local = event.element_position;
        update_handle_hover(local);
    }
}
```

Use routed coordinates for controls such as sliders, canvas editors and color pickers.
