The Window module provides native application entry adapters, system windows, event dispatch, desktop display queries, text input, clipboard access, and system dialogs.

Start with these pages:

* [[Application Main Function and Event Loop]] describes `AppMain.hpp`, `luna_main`, the application-owned event loop, and the global event handler.
* [[Manipulating Windows]] describes desktop-created windows, mobile system windows, state, coordinates, DPI scaling, and window events.
* [[Displays]] describes the desktop display query API.
* [[Input Method Editor (IME)]] describes per-window text input and on-screen keyboards.

Additional public APIs that do not yet have dedicated Manual pages are declared in:

* `<Luna/Window/Clipboard.hpp>` for system clipboard text;
* `<Luna/Window/FileDialog.hpp>` for native file and directory dialogs;
* `<Luna/Window/MessageBox.hpp>` for native message boxes.

Their current backend coverage is:

| API | Windows | macOS | iOS | Android |
| --- | --- | --- | --- | --- |
| Clipboard text | Supported | Supported | Supported | No implementation |
| File and directory dialogs | Supported | Supported | No implementation | No implementation |
| Message boxes | Supported | Supported | No implementation | No implementation |

The headers declare these functions for every platform, so a platform marked "No implementation" will fail at link time if the function is called. Guard such calls by target platform or provide an application-level fallback.

Register `module_window()` and call `init_modules()` before using these APIs. Window creation, manipulation, event polling, text input, and supported native dialogs belong on the main thread.
