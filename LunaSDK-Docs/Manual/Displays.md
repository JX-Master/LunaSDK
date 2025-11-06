Display APIs allows the application to query informations of the display screens of the platform. One platform may have multiple displays, for example, a PC with multiple monitors connected, or a smartphone with two or more screens. Every display may have different physical properties like resolution, color depth, pixel density, etc. Displays can also be added or removed at runtime, for example, by connecting and disconnecting monitors from or to one PC. When writing window management codes, the application should take care of such features in order to prevent display-related bugs.

The application can get a list of displays of the platform by calling `get_displays`, and get the primary display of the platform by calling  `get_primary_display`. One physical display is represented by ` display_t `, which is simply an opaque pointer to an internal display information entry. The pointer is valid until the display is disconnect or the window module is closed.

## Display and Video Mode

A **video mode** is one set of parameters that describes one particular display settings. One display may support multiple display modes, which can be queried by `get_display_supported_video_modes`.  At any given time, only one display mode is active for one display, which can be fetched by calling `get_display_video_mode`. Also, the native resolution of one display can be fetched by calling `get_display_native_resolution`. When creating or setting a fullscreen window, the application should always set the window's video mode as one of video modes supported by the target screen to prevent potential display issues.

## DPI Scaling

DPI (dots per inch) describes the pixel density of one display. Two displays with the same physical size may have different resolutions, thus have different DPI values. Since most UI elements uses dots (or pixels) to represent their sizes, this can cause such elements look different on displays with different DPI values. **DPI scaling** applies one scale factor to the original layout coordinate to change the visual size of UI elements, so that elements can have the same visual size on displays with different DPI values. The DPI scale value of one monitor can be fetched by calling `get_display_dpi_scale` function.

## Display Events

Display events can be monitored by setting callbacks to events fetched from `get_display_events`. Note that setting/triggering such events are not thread-safe, always handle such events on main thread (the thread that initializes LunaSDK).