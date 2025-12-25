A window is a virtual surface that the application can draw user interface or game image on. One window is represented by `IWindow` interface, and can be created by calling  `new_window`. One window can be full-screen or windowed, a windowed window usually has a border and a title bar, which displays the window title and let the user to move, minimize, maximize and close the window.

## Multi-Window System and Single-Window System

Most desktop operating systems (Windows, macOS, Linux, etc.) are multi-window systems, while most mobile and gaming console operating systems (iOS, Android, PlayStation, XBOX, etc.) are single-window systems. Single window systems only support displaying one window at a time for one application (although some systems do support creating multiple windows for one application, only one of them can be set as visible), while multi-window systems support displaying multiple windows simultaneously. On single window systems, always create only one window for your application.

## Windowed Window and Full Screen Window

GUI-based operating systems will have some sort of window management subsystem (usually called windowing system) to manage all displayed windows. In such case, the windowing system controls the [[Displays#Display and Video Mode|video mode]] of all monitors, and all windows controlled by windowing system must use the same video mode of the windowing system. Meanwhile, some operating systems (like Windows and consoles) allows one window to bypass windowing system and take control of one display directly. In LunaSDK, we use **windowed window** to refer one window managed by the windowing system, and **full screen window** to refer one window that controls display directly.

> Not to be confused by a full screen window and a maximized window. A maximized window usually fills the whole user area of one display, but it is still managed by the windowing system, so it is still a windowed window.

One window can be set as windowed or full screen at creation time or later by calling `IWindow::set_display_settings`. For a windowed window, the application can set the window's position and size; for a full screen window, the application can set the target display and the video mode used by the target display.

## Window Coordinates and Pixel Size

Windows' positions and sizes are measured in window coordinates, which on some platforms are not represented using pixels. Use `IWindow::get_framebuffer_size` to fetch the window's pixel size if you need to create a frame buffer for rendering content to the window.

## DPI Scaling

The DPI scale value of one window is the same as the [[Displays#DPI Scaling|DPI scale value of the display]] that contains the window. The DPI scale value of one window can be fetched by calling `IWindow::get_dpi_scale_factor`. This value can change if the user moves the window from one display to another, in such case the system sends `WindowEvents::dpi_scale_changed` event to the application, so application can update its content to fit new DPI scale value.

## Window Events

Window events can be fetched by calling `IWindow::get_events`, then the application can register callbacks to the event that needs to be handled.  Note that window object is not thread safe, so that all window manipulation operations should only be done on main thread.