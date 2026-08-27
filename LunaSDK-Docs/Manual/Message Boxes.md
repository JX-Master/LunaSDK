A message box presents a short native, modal message and asks the user to choose one of the application's
buttons. Button text is supplied by the application, so it can describe the action precisely and be localized without
depending on platform-provided OK, Yes or No labels. The call blocks until the dialog closes and returns the selected
button's index.

Message boxes are provided by the Window module on Windows and macOS. The operating system controls their exact size,
button placement and visual style.

## Concepts

### Message text and title

The message text explains the condition or question. The title identifies the application or operation. Both are
null-terminated UTF-8 strings and are copied into platform-native storage for the duration of the call.

Keep the text concise. A message box is not a replacement for a document window, scrolling report or multi-step
workflow.

### Buttons and selection

The `buttons` span contains null-terminated UTF-8 button titles in semantic priority order. The function returns the
zero-based index of the button selected by the user. Duplicate titles are allowed because selection is identified by
position in the input span.

The operating system decides the buttons' physical layout. In particular, macOS may place the first button on the
right or arrange buttons vertically. Do not reverse the array or derive an index from screen position.

### Default and cancel buttons

`default_button_index` identifies the button activated by Return and defaults to the first button.

`cancel_button_index` identifies the button activated by Escape and, on platforms that provide one for the message
box, the native window-close action. Specify `USIZE_MAX` when the user must select an explicit button and the dialog
must not otherwise close. The default and cancel indices must not identify the same button.

Cancellation is defined by the index, not by visible text. A localized title such as `"取消"` therefore has the same
behavior as `"Cancel"` when it is designated as the cancel button.

### Icons

`MessageBoxIcon` requests a platform-appropriate informational, warning, question or error presentation. The mapping
is best effort: a platform may render multiple values identically, omit an icon without a native equivalent or show
application identity artwork when `none` is requested. Use an icon only when it helps communicate the kind of
condition; the text and button actions remain authoritative.

## Programming guide

### Initialize the Window module

Use message boxes on the Luna main thread after the Window module has been initialized. Applications that use the
standard entry adapter should initialize Runtime and the Window module before showing a dialog:

```cpp
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Window/Window.hpp>

lupanic_if_failed(Luna::init());
lupanic_if_failed(add_modules({module_window()}));
lupanic_if_failed(init_modules());
```

The function runs a native modal event loop while the calling thread blocks. It does not associate the dialog with a
particular Luna window, so modality outside the calling thread is platform-specific. Do not wait for a worker
operation from a message box or show it from a worker thread.

### Show an acknowledgement

A single button is suitable for reporting an error the user can only acknowledge:

```cpp
auto result = Window::message_box(
    "The project could not be opened.",
    "Open Project",
    {"OK"},
    Window::MessageBoxIcon::error);

if(failed(result))
{
    log_error("Example", "Failed to show the message box: %s", explain(result.errcode()));
}
```

The first and only button is the default. No cancel index is needed.

### Ask the user to choose an action

Use action-specific text and compare the returned index with named local constants:

```cpp
constexpr usize SAVE = 0;
constexpr usize DISCARD = 1;
constexpr usize CANCEL = 2;

auto result = Window::message_box(
    "Save changes before closing the project?",
    "Close Project",
    {"Save", "Don't Save", "Cancel"},
    Window::MessageBoxIcon::warning,
    SAVE,
    CANCEL);

if(failed(result) || result.get() == CANCEL)
{
    return;
}
if(result.get() == SAVE)
{
    save_project();
}
close_project();
```

Clicking Cancel or pressing Escape returns `CANCEL`. On Windows, using the title-bar close action also returns
`CANCEL`.

### Handle failures

`message_box` returns `E_BAD_ARGUMENTS` for an empty button span, null or empty button text, invalid button indices or
invalid UTF-8. It returns `E_DATA_TOO_BIG` if the platform cannot represent the button count. A detectable native
dialog allocation failure returns `E_OUT_OF_MEMORY`. Native dialog failures return `E_BAD_PLATFORM_CALL`, and a
native interruption that did not select a button returns `E_INTERRUPTED`.

On Windows, the function also fails when the Luna main thread was previously initialized with an incompatible COM
apartment model. Keep native UI and other apartment-threaded Windows UI services on the main STA thread.

### Shutdown

The function retains no strings or native objects after it returns. Release application objects and close the Window
module through the normal module shutdown path; message boxes require no separate shutdown call.
