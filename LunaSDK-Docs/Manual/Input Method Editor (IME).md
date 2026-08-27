An input method editor lets users enter text that is not mapped directly to physical keys, including Chinese, Japanese, and Korean text. The same Window text-input API can ask a mobile platform to show its on-screen keyboard.

Text input state belongs to one `IWindow`. Different windows can activate text input and place their input areas independently. Call these APIs on the main thread.

## Starting and stopping text input

Text input is inactive when a window is created. Call `IWindow::begin_text_input` when a text field gains editing focus, and call `IWindow::end_text_input` when editing ends. Both functions return `RV`. Use `IWindow::is_text_input_active` to query the current state.

```cpp
RV set_editing_active(Window::IWindow* window, bool active)
{
    return active ? window->begin_text_input() : window->end_text_input();
}
```

On platforms with an on-screen keyboard, beginning text input may show that keyboard and ending text input may hide it.

## Setting the text input area

An IME may draw a candidate or composition overlay near the edited text. Call `IWindow::set_text_input_area` after activating text input and whenever the field or cursor moves:

```cpp
RV update_text_input_area(Window::IWindow* window,
    i32 x, i32 y, i32 width, i32 height, i32 caret_x)
{
    RectI field_rect(x, y, width, height);
    i32 cursor_x = caret_x - x;
    return window->set_text_input_area(field_rect, cursor_x);
}
```

`field_rect` is measured in window coordinates. The second argument is the cursor's X offset relative to `field_rect.offset_x`.

The current Windows backend uses both the rectangle and cursor offset. The macOS and iOS backends use the rectangle but currently ignore the separate cursor offset. Android currently treats the entire call as a no-op.

## Receiving text

Install the application-wide Window event handler with `Window::set_event_handler`. On Windows, macOS, and iOS, committed UTF-8 text is delivered in `WindowInputTextEvent::text` while text input is active. The current Android backend can request that the software keyboard be shown or hidden, but it does not yet route committed text into `WindowInputTextEvent`; applications targeting Android need a platform input integration until that backend is completed.

```cpp
void handle_text_event(object_t event, void*)
{
    if(auto input = cast_object<Window::WindowInputTextEvent>(event))
    {
        // Insert input->text into the application's text model.
    }
}
```

Key-down events describe physical or logical keys and are not a substitute for `WindowInputTextEvent` when entering user-visible text. `ScreenKeyboardShownEvent` and `ScreenKeyboardHiddenEvent` report on-screen keyboard visibility changes on platforms that generate those events.
