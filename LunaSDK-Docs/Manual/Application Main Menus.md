An application main menu is native, application-wide system UI. On macOS it is the menu bar shown to the right of
the Apple menu and remains available even when the application has no open windows. It is distinct from an
EditorGUI menu bar, which is drawn inside one GUI surface. **Application main menu API only works on macOS.**

The Window module exposes application main menus as a platform-independent descriptor tree. Applications do not
create or retain native menu objects, and ordinary application code does not need to compile as Objective-C++.
The initial backend is macOS. Other platforms report that the capability is unsupported.

## Concepts

### Menu tree

`ApplicationMenuDesc` contains the top-level `ApplicationMenuItemDesc` nodes. Every top-level node is a submenu. On
macOS, the first top-level submenu is the application menu; subsequent submenus commonly represent File, Edit, View,
Window and Help.

Each node has one `ApplicationMenuItemType`:

* `command` identifies an invocable item;
* `submenu` contains child items;
* `separator` provides visual grouping and has no title or children.

`set_application_menu` validates and deep-copies the complete tree. Descriptor arrays and title strings therefore
need to remain valid only until the function returns. Replacing the menu is transactional: a rejected descriptor
does not remove the currently installed menu.

### Item identifiers and state

An application-defined command has a non-zero `application_menu_item_id_t`. Every non-zero identifier must be unique
within the tree. The identifier is delivered by `ApplicationMenuItemInvokedEvent` and can later be passed to
`set_application_menu_item_state` or `set_application_menu_item_title`.

`ApplicationMenuItemState` controls whether an item is enabled and visible and whether its check state is clear,
checked or mixed. State changes update the installed native item without rebuilding the menu.

### Standard roles

`ApplicationMenuItemRole` lets the platform backend preserve native behavior without exposing platform selectors.
The macOS backend recognizes About, Settings, Services, Hide, Hide Others, Show All, Quit, Window-menu and Help-menu
roles. A role may supply its platform-default title when `ApplicationMenuItemDesc::title` is `nullptr`. Settings has
no application-independent native action, so a Settings item requires a non-zero identifier and dispatches
`ApplicationMenuItemInvokedEvent`; use Command+Comma when the conventional shortcut is wanted.

Application-defined items use the `none` role and dispatch `ApplicationMenuItemInvokedEvent`. Standard About, Hide,
Services, Window and Help behavior is owned by AppKit. Quit uses Luna's cancelable quit request described below.

### Keyboard shortcuts

A shortcut combines one `KeyCode` with `Window::KeyModifierFlag`. On macOS the `system` modifier represents Command
and `alt` represents Option. Native menu shortcuts take priority over a matching raw window key event, so one key
gesture does not execute both a menu command and an application key handler.

### Quit requests

Selecting the standard Quit item or receiving a supported native termination request dispatches
`ApplicationRequestQuitEvent`. Its `do_quit` member is initially `true`. An event handler may set it to `false`, for
example when the user cancels an unsaved-work prompt.

When accepted, `is_application_quit_requested` remains `true`. The application should observe this state in its main
loop, release its objects and return normally through `luna_main`. This is intentionally different from immediately
terminating the process, because LunaSDK cleanup and `Luna::close` must still run.

## Programming guide

### Initialize the Window module

Set the application name after registering the Window module and before initializing modules. The macOS backend uses
this name in its default application menu. If it is absent, the backend falls back to application-bundle metadata and
then the process name.

```cpp
Window::StartupParams startup_params;
startup_params.name = "Example";
Window::set_startup_params(startup_params);
lupanic_if_failed(init_modules());
```

Use `supports_application_menu` before installing an application menu. This lets the same application retain an
in-window menu on platforms that do not provide an application-global menu.

### Install a menu

The following example installs a conventional macOS application menu and a File menu. The descriptor storage is
local because `set_application_menu` copies it.

```cpp
#include <Luna/Window/ApplicationMenu.hpp>
#include <Luna/Window/Event.hpp>

constexpr Window::application_menu_item_id_t MENU_SAVE = 1;

RV install_menu()
{
    Window::ApplicationMenuItemDesc app_items[4];
    app_items[0].role = Window::ApplicationMenuItemRole::about;
    app_items[1].type = Window::ApplicationMenuItemType::submenu;
    app_items[1].role = Window::ApplicationMenuItemRole::services;
    app_items[2].role = Window::ApplicationMenuItemRole::hide;
    app_items[2].shortcut_key = KeyCode::h;
    app_items[2].shortcut_modifiers = Window::KeyModifierFlag::system;
    app_items[3].role = Window::ApplicationMenuItemRole::quit;
    app_items[3].shortcut_key = KeyCode::q;
    app_items[3].shortcut_modifiers = Window::KeyModifierFlag::system;

    Window::ApplicationMenuItemDesc file_items[1];
    file_items[0].title = "Save";
    file_items[0].id = MENU_SAVE;
    file_items[0].shortcut_key = KeyCode::s;
    file_items[0].shortcut_modifiers = Window::KeyModifierFlag::system;

    Window::ApplicationMenuItemDesc top_level[2];
    top_level[0].type = Window::ApplicationMenuItemType::submenu;
    top_level[0].title = "Example";
    top_level[0].children = Span<const Window::ApplicationMenuItemDesc>(app_items, 4);
    top_level[1].type = Window::ApplicationMenuItemType::submenu;
    top_level[1].title = "File";
    top_level[1].children = Span<const Window::ApplicationMenuItemDesc>(file_items, 1);

    Window::ApplicationMenuDesc menu;
    menu.items = Span<const Window::ApplicationMenuItemDesc>(top_level, 2);
    return Window::set_application_menu(menu);
}
```

Applications normally include the full standard application menu with Services, Hide Others and Show All in
addition to this shortened example. Call `reset_application_menu` to restore the backend-provided default menu.

### Handle commands and Quit

Menu commands use the same application-wide handler as other Window events:

```cpp
void handle_event(object_t event, void*)
{
    if(auto command = cast_object<Window::ApplicationMenuItemInvokedEvent>(event))
    {
        if(command->item_id == MENU_SAVE)
        {
            save_document();
        }
    }
    else if(auto quit = cast_object<Window::ApplicationRequestQuitEvent>(event))
    {
        quit->do_quit = confirm_unsaved_work();
    }
}
```

Include the accepted quit state in the application loop:

```cpp
while(!Window::is_application_quit_requested() && !window->is_closed())
{
    Window::poll_events();
    if(Window::is_application_quit_requested() || window->is_closed()) break;
    // Update and render one frame.
}
```

### Update command state

Update menu state when the corresponding application model changes. For example, the Save command can mirror the
document's dirty state without rebuilding the menu:

```cpp
Window::ApplicationMenuItemState state;
state.enabled = has_unsaved_changes();
lupanic_if_failed(Window::set_application_menu_item_state(MENU_SAVE, state));
```

All application-menu functions and related event handling run on the main thread. Do not wait for worker threads or
perform long-running work while AppKit is opening or tracking a menu.

Before shutting down, clear the Window event handler and release application objects as usual. The Window module
detaches its native menu targets during module shutdown; applications do not release native menu resources.