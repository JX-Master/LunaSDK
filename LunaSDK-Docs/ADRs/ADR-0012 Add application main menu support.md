## Status
Approved.

## Last updated
2026/8/26

## Background
macOS applications conventionally expose an application-wide menu bar through `NSApplication.mainMenu`. The first
top-level menu is the application menu shown next to the Apple menu; other top-level menus commonly include File,
Edit, View, Window and Help. LunaSDK currently creates `NSApplication` and pumps AppKit events through the Window
module, but it does not create or expose an application main menu. Applications such as Studio therefore draw a menu
bar inside the client area, which does not participate in AppKit menu roles, responder routing, Services, Window and
Help integration, or native keyboard-equivalent handling.

An application main menu is not a GUI element and is not owned by one window. Putting it in GUI or EditorGUI would
couple a system-level application service to a rendered GUI surface, while putting it in `IWindow` would give an
application-global object ambiguous ownership in multi-window programs. The Window module already owns native
application bootstrap, the main-thread event pump, application events, clipboard access and native dialogs, so it is
the existing application/platform boundary that can host the feature without introducing a new module dependency.

The usual AppKit Quit action calls `NSApplication::terminate:`. Once AppKit accepts termination, cleanup at the end of
`main` is not executed. LunaSDK applications instead own their update loop and rely on returning through `luna_main`
to release Runtime objects and call `Luna::close`. Studio also needs a cancelable unsaved-change check. Native Quit
therefore cannot be allowed to terminate the process directly.

## Decision
### Public application-menu model
On macOS, the Window module provides an application-main-menu API in `ApplicationMenu.hpp`. The complete public API,
including related menu events and the accepted-Quit query, is enclosed by `LUNA_PLATFORM_MACOS`. Other platforms do
not declare or export these symbols. Cross-platform applications select their menu implementation at compile time;
Studio continues to use its EditorGUI client-area menu outside macOS.

The menu is an application-global tree described by value descriptors. Nodes are commands, submenus or separators.
Command nodes have a non-zero application-defined `u64` identifier. Descriptors contain UTF-8 titles, enabled and
check states, an optional keyboard shortcut, an optional standard platform role and child nodes for submenus.

`set_application_menu` validates and deep-copies the complete descriptor tree before installing it. The caller does
not retain descriptor strings or spans. Replacing a menu is transactional: an invalid replacement leaves the current
menu unchanged. Item identifiers are unique among all non-zero identifiers in one tree and provide stable handles for
incremental title, enabled, check-state and visibility updates without exposing native menu objects.

The initial public operations are:

1. Replace the application main menu from a descriptor tree.
2. Restore the platform default application menu.
3. Update the cached presentation state or title of an identified item.

The API is macOS-only because its application-global semantics model `NSApplication.mainMenu`. A future Windows menu
API may attach a menu to a specific `IWindow`; it will not pretend that Win32 window menus have macOS
application-global semantics.

### Commands, standard roles and native validation
Selecting an application-defined command by pointer or keyboard dispatches one synchronous
`ApplicationMenuItemInvokedEvent` through the existing application-wide Window event handler. The event carries the
command identifier and no `IWindow`, because the menu remains valid with no open window. The backend retains no
application callback or userdata beyond the existing Window event handler.

Standard roles let a backend preserve native behavior without exposing Objective-C selectors. The initial macOS roles
cover About, Settings, Services, Hide, Hide Others, Show All, Quit, the Window menu and the Help menu. AppKit-owned
roles use native target/action or designated `NSApplication` menu properties. Application-defined commands use one
backend-owned action target and carry their identifier as native represented data.

Settings is a conventional placement and title rather than an application-independent AppKit action. A Settings-role
item therefore requires a non-zero command identifier and dispatches the same command event as an
application-defined item; the application opens its own settings interface and explicitly assigns Command+Comma when
it wants the conventional shortcut.

Menu-item validation reads the state cached by the Window API and remains fast and main-thread-only. That cached
state is authoritative for descriptor-owned items, including commands that forward to standard AppKit actions.
AppKit retains automatic validation inside its designated Services, Window and Help submenus. The application
updates command state when its model changes rather than rebuilding the menu or running application code from an
AppKit validation callback.

Keyboard shortcuts use Luna `KeyCode` plus Window-owned modifier flags. The Cocoa backend maps these to AppKit key
equivalents and modifier masks. A main-menu key equivalent has priority over dispatching the same physical key as a
Luna window key event. When AppKit handles a shortcut, the backend suppresses the corresponding Luna key-down and
key-up pair so one gesture cannot invoke the command twice.

### Default macOS menu and lifecycle
The Cocoa bootstrap owns strong references to its application delegate and menu action target. It creates a minimal
native application menu before finishing AppKit launch, using the application name from `StartupParams` when it is
available and otherwise falling back through `CFBundleDisplayName`, `CFBundleName` and the process name. Window module
initialization idempotently completes or refreshes that setup so applications that provide their own native entry
point can still use the menu API. Module shutdown detaches Luna-owned targets and menus before releasing their
storage.

The default macOS application menu contains About, Services, Hide, Hide Others, Show All and Quit with conventional
roles and shortcuts. Replacing the main menu may supply additional File, Edit, View, Window and Help menus and may
customize the first application menu, but the backend validates role/type combinations.

The Window module stores the configured application name rather than retaining `StartupParams::name`. This makes the
documented lifetime of `get_app_name` true and gives native application UI a stable name.

### Cancelable quit
Window adds `ApplicationRequestQuitEvent` with mutable `do_quit` initialized to `true`, plus a query for the accepted
quit-request state. The macOS Quit role and an SDK-owned `applicationShouldTerminate:` delegate path both dispatch
this request. If the handler leaves `do_quit` true, Window records an accepted quit request and the native delegate
still cancels immediate AppKit termination. The application update loop observes the accepted state, releases its
objects and returns through `luna_main`. If the handler sets `do_quit` false, the request is rejected and the loop
continues.

`ApplicationWillTerminateEvent` remains a non-cancelable late lifecycle notification and is not reused as a quit
request. Applications with a custom `NSApplicationDelegate` remain responsible for forwarding external native
termination requests; the menu action target does not replace a delegate that the application owns.

## Impact
LunaSDK applications can participate in the native macOS menu bar without compiling application code as
Objective-C++ or owning AppKit objects. Native menu commands share the existing Window event channel, and dynamic
command state remains under the application model rather than being inferred from EditorGUI widgets.

Applications that want standard Quit behavior must include the accepted quit-request state in their loop condition.
Code that uses the API must be compiled conditionally with `LUNA_PLATFORM_MACOS`. Studio routes native Save All, Undo,
Redo and View commands to the same operations used by its EditorGUI menu on macOS and compiles its prior EditorGUI
client-area menu on Windows and other platforms.

Giving native key equivalents priority changes Cocoa input delivery for handled shortcuts: content receives the menu
command event instead of a matching Window key pair. This matches macOS menu behavior and prevents duplicate command
execution, but applications that previously implemented Command shortcuts solely from raw Window key events should
migrate those commands to the application menu.

The descriptor API is intentionally less mutable than an object graph of menu and item interfaces. Structural plugin
changes replace the tree, while frequent state changes address items by identifier. If future context menus or very
large dynamic menus require retained menu objects, they can be introduced separately without exposing AppKit types in
this API.

## Alternatives considered
### Expose `NSApplication*`, `NSMenu*` or Objective-C selectors
This would be a small backend change but would require every application to compile Objective-C++, duplicate lifetime
and event bridging, and lose a portable command model. Native handles remain available only as an application-owned
escape hatch through a custom entry point.

### Implement the menu in EditorGUI
An EditorGUI menu is drawn into one GUI surface and is appropriate for in-window or in-game interfaces. It cannot be
the macOS application menu, remains unavailable when all windows are closed and cannot provide native Services,
Window, Help or keyboard-equivalent behavior.

### Attach the menu to `IWindow`
This matches Win32 menus but not AppKit, where one main menu belongs to `NSApplication` and changes with application
state. It was rejected for the initial feature; a separate window-menu API may be added for platforms that need one.

### Bind Quit directly to `terminate:`
This follows the standard AppKit template but can bypass `luna_main` cleanup and cannot express Studio's existing
unsaved-change cancellation through the Luna event model. A deferred, cancelable quit request preserves the
application-owned loop.

### Dispatch callbacks directly from menu items
Per-item callbacks would introduce additional userdata lifetime and reentrancy rules beside the Window module's
existing single event handler. Command events keep all native input and lifecycle messages on one established path.

### Export the API on every platform and report unsupported at runtime
This would allow cross-platform source code to compile while providing no working menu on some targets. It hides a
semantic platform difference until runtime and encourages applications to omit their real fallback UI. Compile-time
availability, matching the Window Display API pattern, makes the platform boundary explicit.

## Version history
* **2026/8/26** Proposed and approved.
* **2026/8/26** Restricted the API to macOS at compile time and retained EditorGUI menus on other platforms.
