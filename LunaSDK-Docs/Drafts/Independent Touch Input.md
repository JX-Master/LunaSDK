## Designed functionality

Touch input travels from Window through GUIWindow into GUI and EditorGUI as its own event stream. A touch does not create a mouse button, mouse position update, mouse hover, mouse capture or wheel event. The mouse can operate while multiple contacts interact with other controls.

The implementation supports contact lifetime and cancellation, tapping and double tapping, content panning, scrollbars, direct manipulation and directional gesture arbitration. Buttons, checkboxes, switches, radio buttons, selectable items, button groups, tabs, disclosure controls, combo boxes and menus use semantic activation from either input source. Sliders, scalar/vector value drags, text selection, color pickers, table splitters and dock manipulation interpret independent contact coordinates.

## Concepts

### Window contact

`WindowTouchEvent` contains the source `window`, `device_id`, contact `id` and client-relative `x`/`y`. The pair `(device_id, id)` identifies a live contact. IDs can be reused after a contact ends. A device identity of zero means the backend does not expose one.

`WindowTouchDownEvent` starts a contact. `WindowTouchMoveEvent` updates it, including positions outside the client bounds while captured. `WindowTouchUpEvent` ends it normally. `WindowTouchCancelEvent` interrupts it without activation. Hosts must not substitute an up event for cancellation.

The Windows backend consumes `PT_TOUCH` input from `WM_POINTER*`, uses `GetPointerInfo` and `ScreenToClient`, and cancels active contacts on pointer capture loss, native cancellation, input focus loss or window destruction. It filters touch compatibility mouse messages so a physical touch is not also processed as a click. Physical mouse and pen behavior keep their existing paths. No `RegisterTouchWindow` or application-level touch-to-mouse translation is required. UIKit cancellation likewise produces a cancel event.

### GUI contact and state

`InputEventType::touch_down`, `touch_move`, `touch_up` and `touch_cancel` use `InputEvent::device_id`, `touch_id` and `position`. They do not use `pointer_id`, `button` or `wheel_delta`.

`IContext::get_touch_state(device_id, touch_id, state)` returns position, initial position, accumulated frame movement and the contact-specific capture owner. It returns false after up/cancel. `capture_touch` transfers only that contact and cancels losing targets. `release_touch_capture` cancels its targets and prevents a later release from activating them. These operations do not affect `capture_pointer`.

Touch hit testing shares element geometry, clipping, layer ordering and `PointerHitBehavior` policies with the mouse, while routing state remains separate. `InteractionState::hovered` and its pointer-position fields remain mouse-only. `touch_active`, `touch_clicked`, `touch_double_clicked` and touch-position fields report touch state. Semantic `active`, `clicked`, `double_clicked` and subtree flags combine supported input sources so ordinary activation controls do not need duplicate action code. Click coordinates describe the activation that produced them.

### Gesture ownership

`Interactable::touch_gestures` declares `TouchGestureFlag::drag_x`, `drag_y`, both, or neither. After a contact moves at least eight logical units, GUI selects the nearest enabled, writable ancestor that supports the predominant movement axis. It sends `touch_cancel` to losing targets and routes further movement to the winner. A moved or canceled contact cannot produce a tap, even if it returns to its starting position.

Delivered movement includes `touch_delta` and `touch_dragging`. The first claimed movement includes displacement from the starting point; subsequent movement contains per-event displacement. A viewport applies these deltas only after it wins the gesture. EditorGUI scroll views declare their scrollable axes; horizontal sliders and value drags claim horizontal movement while allowing vertical movement to reach an enclosing vertical scroll view.

Each direct manipulation is owned by its initiating mouse stream or touch contact. A different contact or mouse movement cannot take over an ongoing slider, text-selection or value-drag operation. Independent controls can be manipulated concurrently.

## Programming guide

### Window-backed hosts

Install `GUIWindowInputAdapter` normally. It translates each Window touch notification into the corresponding GUI touch type, preserving device and contact identity. Call `update_input` after `begin_frame` to flush queued events. Per-frame mouse sampling can continue because touch routing does not read or overwrite the mouse stream.

Applications with a custom DPI bridge must transform the positions of touch events as well as mouse events from client units into `FrameDesc::logical_size` units. Do not scale contact IDs or turn displacement into wheel units. GUI computes logical contact deltas after the transformed events are submitted.

### Custom GUI hosts

Submit contacts after `begin_frame` and before `route_input`:

```cpp
GUI::InputEvent event;
event.type = GUI::InputEventType::touch_down;
event.device_id = source_device;
event.touch_id = source_contact;
event.position = logical_position;
context->add_input_event(event);
```

Use the same identity for movement and the terminal up/cancel event. Submit `touch_cancel` if the device, remote connection or contact is interrupted. Submit `blur` on input focus loss to clear all input state. Do not emit an additional pointer-down/up or wheel event for a touch.

### Custom interactive controls

Use the normal `Interactable` flags and hit policy. Read semantic `clicked` for a tap/click action, or `touch_clicked` when an action must distinguish its source. Declare manipulation axes when a control consumes contact movement:

```cpp
GUI::Interactable interactive;
interactive.pointer_hit_behavior = GUI::PointerHitBehavior::target;
set_flags(interactive.flags, GUI::InteractableFlag::activatable);
interactive.touch_gestures = GUI::TouchGestureFlag::drag_x;
context->set_interactable(element, interactive);
```

For coordinate-based manipulation, inspect `get_routed_input_events`. Handle touch down, move, up and cancel separately from pointer events; preserve the initiating `(device_id, touch_id)` until it ends. Use `RoutedInputEvent::element_position` for local positions. Release temporary drag/selection state on cancellation and do not commit a click or drop from that event.

### EditorGUI hosts

Existing EditorGUI controls accept both paths without an opt-in input mode. `DefaultStyleDesc::input_mode` controls visual density and target sizes; it does not select or disable an input source. `ScrollViewDesc` continues to configure enabled axes, wheel scale and scrollbar behavior. Native touch movement is measured in logical units and does not use `wheel_scale`.

## Validation and current scope

Run `TouchInputTest` through LunaBuild. From the SDK root:

```powershell
dotnet run --project LunaBuild.csproj -- run --target TouchInputTest --platform Windows --arch x64 --mode Release --rhi D3D12
```

An importing project can select `LunaSDK.TouchInputTest`. Tests inject GUI contacts and verify Window event translation, multiple device/contact identities, simultaneous mouse input, capture, cancellation, ID reuse, activation thresholds, scrolling and manipulation ownership. Native physical delivery still needs a touchscreen check, including capture loss and monitor DPI changes. This feature provides direct panning; it does not add kinetic scrolling, pinch/rotate gestures, pressure or pen-specific editing semantics.

Windows protocol references: [WM_POINTERDOWN](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [WM_POINTERCAPTURECHANGED](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointercapturechanged).
