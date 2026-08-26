A window is a system-managed surface represented by `Window::IWindow`. It can receive input and can be used as the presentation surface for an RHI swap chain. Window objects are reference-counted and are normally stored in `Ref<Window::IWindow>`.

Use Window APIs on the main thread. The platform backends dispatch events and perform native window operations on that thread.

## Desktop and system-created windows

On desktop platforms, create windows with `Window::new_window` after the Window module has been initialized:

```cpp
RV create_desktop_window(Ref<Window::IWindow>& window)
{
    lutry
    {
        luset(window, Window::new_window(
            "Example",
            Window::DEFAULT_POS,
            Window::DEFAULT_POS,
            1280,
            720));
    }
    lucatchret;
    return ok;
}
```

`DEFAULT_POS` lets the windowing system select an initial position. A width or height of zero lets it select a suitable size. Desktop applications may create multiple windows.

On mobile and console platforms, the operating system creates the application window. Obtain it with `Window::get_system_window`; `new_window` is not available on those platforms. The system window cannot be moved, resized, retitled, or restyled by the application.

```cpp
RV acquire_application_window(Ref<Window::IWindow>& window)
{
    lutry
    {
    #if defined(LUNA_PLATFORM_DESKTOP)
        luset(window, Window::new_window("Example"));
    #else
        window = Window::get_system_window();
    #endif
    }
    lucatchret;
    return ok;
}
```

## Window state and style

The cross-platform queries include `is_closed`, `has_input_focus`, `has_mouse_focus`, `is_minimized`, `get_position`, `get_size`, `get_framebuffer_size`, and `get_dpi_scale_factor`.

Desktop windows additionally support:

* closing, foreground activation, minimization, maximization, and restoration;
* visibility changes;
* changing the client-area position and size;
* changing the title;
* changing `WindowStyleFlag::resizable` and `WindowStyleFlag::borderless`.

`WindowCreationFlag::hidden` creates a desktop window without initially showing it. A borderless window has no system decoration; it is not an exclusive fullscreen mode. The current public Window API does not change display video modes or expose exclusive fullscreen settings.

## Window coordinates and framebuffer size

`IWindow::get_position` and `IWindow::get_size` use platform screen coordinates. Screen coordinates are not necessarily physical pixels. Use `IWindow::get_framebuffer_size` when allocating render targets or resetting a swap chain.

Use `screen_to_client` and `client_to_screen` to convert points between the virtual screen coordinate space and a window's client coordinate space.

## DPI scaling

`IWindow::get_dpi_scale_factor` returns the ratio between the window's current DPI and the platform's default DPI. The value can change, for example when a desktop window moves to a display with a different scale.

Handle `WindowDPIScaleChangedEvent` through the application-wide event handler, then query `get_dpi_scale_factor` again and update the layout and pixel resources that depend on it.

## Window events

Install one application-wide handler with `Window::set_event_handler`, and call `Window::poll_events` from the main loop. There is no per-window `get_events` API. Every event derived from `WindowEvent` contains a retained `window` member identifying its target.

```cpp
#include <Luna/Runtime/Log.hpp>

void handle_window_event(object_t event, void*)
{
    if(auto moved = cast_object<Window::WindowMoveEvent>(event))
    {
        log_info("Example", "Window moved to %d, %d", moved->x, moved->y);
    }
}

void install_window_handler()
{
    Window::set_event_handler(handle_window_event, nullptr);
}
```

A `WindowRequestCloseEvent` is closed by default. Set its `do_close` member to `false` in the handler only when the application must reject that request. A `WindowClosedEvent` reports that the native window has already closed and that attached resources should be released.
