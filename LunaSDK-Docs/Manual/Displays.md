The display API in `<Luna/Window/Display.hpp>` queries desktop display screens. It is available when `LUNA_PLATFORM_DESKTOP` is defined; the current Window backends implement these queries on Windows and macOS. Mobile applications use their system-created window rather than this desktop display API.

A display is represented by the opaque `Window::display_t` handle. Treat the value only as an argument to Window display functions. The operating system can add, remove, or reconfigure displays, so do not assume that a previously queried list remains current indefinitely.

## Enumerating displays

Use `get_primary_display` to obtain the platform's primary display and `get_displays` to append all online displays to a vector. `get_displays` does not clear existing elements in the destination vector.

```cpp
#include <Luna/Window/Display.hpp>

using namespace Luna;

void query_displays(Vector<Window::display_t>& displays,
    Window::display_t& primary)
{
    Window::get_displays(displays);
    primary = Window::get_primary_display();
}
```

The current API does not expose display connection or disconnection events. Re-query the display list before an operation that depends on the current desktop layout.

## Display properties

The following functions query one display:

* `get_display_position` returns the display origin in virtual screen coordinates.
* `get_display_working_area` returns the region not occupied by system UI such as a taskbar or menu bar.
* `get_display_name` returns the platform display name.

These functions return `R<T>` because a stale or invalid display handle can cause the platform query to fail.

```cpp
RV inspect_display(Window::display_t display)
{
    lutry
    {
        lulet(position, Window::get_display_position(display));
        lulet(working_area, Window::get_display_working_area(display));
        lulet(name, Window::get_display_name(display));
        // Use position, working_area, and name here.
    }
    lucatchret;
    return ok;
}
```

## Video modes

`Window::VideoMode` describes a display width, height, bits per pixel, and refresh rate. Use `get_display_supported_video_modes` to append the modes reported for a display, and `get_display_video_mode` to query its current mode.

```cpp
RV inspect_video_modes(Window::display_t display)
{
    lutry
    {
        Vector<Window::VideoMode> modes;
        luexp(Window::get_display_supported_video_modes(display, modes));
        lulet(current_mode, Window::get_display_video_mode(display));
        // Compare current_mode with the entries in modes here.
    }
    lucatchret;
    return ok;
}
```

These APIs are queries only. The current Window public API does not change a display's video mode and does not expose exclusive fullscreen window settings.

On macOS, the current backend reports `bits_per_pixel` as 32 and converts the Core Graphics refresh-rate value to an integer. Treat these fields as normalized selection metadata rather than a lossless representation of the native mode.

## DPI scaling

The current API exposes DPI scaling per window rather than per display. Use `IWindow::get_dpi_scale_factor` for the window being laid out, and handle `WindowDPIScaleChangedEvent` through the application-wide Window event handler when that factor may have changed. See [[Manipulating Windows#DPI scaling]].
