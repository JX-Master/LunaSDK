## Status
Approved.

## Last updated
2026/8/26

## Background
The Window module exposes synchronous native message boxes on desktop platforms. The public API models the available
buttons with `MessageBoxType`, whose values mirror the fixed Win32 `MessageBoxW` combinations, and reports the
selection with the semantic `MessageBoxButton` enumeration. Applications therefore cannot use action-specific or
localized button text, even though the macOS backend already constructs an `NSAlert` and could accept arbitrary
titles.

The fixed semantic result also couples application behavior to English concepts such as OK, Yes, No and Retry.
Replacing those concepts with application-defined text requires a stable way to identify the selected button that
does not depend on its title or its platform-determined visual position.

Default and cancellation behavior cannot be inferred portably from text. AppKit recognizes certain English button
titles for keyboard equivalents, while Windows task dialogs identify cancellation through a button identifier.
Localized or action-specific text must retain the same Return and Escape behavior on both platforms and native
window-close behavior where the platform exposes it.

## Decision
### Public API
The Window module replaces the preset message-box API with one function that accepts application-defined UTF-8
button text and returns the zero-based index of the selected button:

```cpp
R<usize> message_box(
    const c8* text,
    const c8* title,
    Span<const c8*> buttons,
    MessageBoxIcon icon = MessageBoxIcon::none,
    usize default_button_index = 0,
    usize cancel_button_index = USIZE_MAX);
```

`buttons` must contain at least one non-empty, null-terminated, valid UTF-8 string. Duplicate titles are allowed
because the return value identifies the input position rather than the text. The default index must identify one
button. The cancel index is either `USIZE_MAX` or identifies a button different from the default button.

The default button is activated by Return. When a cancel button is specified, clicking that button or pressing Escape
returns the cancel button's index. A platform-provided window-close action also returns that index. When no cancel
button is specified, the dialog cannot be dismissed without selecting one of its buttons. A platform-driven
interruption that does not select a button is reported as `E_INTERRUPTED`.

The function remains synchronous and modal on the calling thread. It does not associate the dialog with a particular
Luna window, so modality outside that thread is platform-specific. Button text and the span need to remain valid only
until the function returns. The platform decides physical button layout; callers order buttons by semantic priority
and never interpret screen position.

`MessageBoxType` and `MessageBoxButton` are removed without a compatibility overload. This is an intentional source
and binary break. `MessageBoxIcon` remains a best-effort platform abstraction; a platform may render multiple values
identically or omit an icon that has no native equivalent.

### Threading and lifetime
Message boxes are Window UI and must be shown on the Luna main thread after the Window module is initialized. The
function may run a nested native modal event loop while it blocks. It does not retain caller strings after returning
and does not expose native dialog or button objects.

### Windows backend
Windows uses `TaskDialogIndirect` and supplies one `TASKDIALOG_BUTTON` for every public button. Internal numeric button
identifiers are unique and are mapped back to input indices. The designated cancel button uses the native cancel
identifier so Escape and the title-bar close action produce the same public index. The configured default button uses
the corresponding internal identifier.

The backend converts all strings to UTF-16 and keeps them stable for the complete native call. Literal ampersands in
application button text are escaped so Win32 does not reinterpret them as mnemonic markup. The Window binary declares
its dependency on Common Controls version 6 and links Comctl32. LunaBuild embeds linker-generated manifests in
Windows DLL and executable outputs so the dependency is present for both shared and static Window builds. The calling
thread is initialized as a single-threaded COM apartment for the duration of the call when necessary; an existing
incompatible apartment is reported as an invalid calling context rather than silently dispatching to another thread.

The message text is task-dialog content rather than a large main instruction. No callback, command links, radio
buttons, verification checkbox or parent window are introduced by this change.

### macOS backend
macOS continues to use `NSAlert`. It adds buttons in input order with `addButtonWithTitle:` and maps the contiguous
`NSAlertFirstButtonReturn` response range back to input indices. The backend assigns Return and Escape key equivalents
explicitly from the public indices instead of relying on English button titles. AppKit remains responsible for native
horizontal or vertical layout.

The implementation remains Objective-C++, uses an autorelease pool and calls `runModal` on the main thread. It does
not introduce a parent-window sheet or asynchronous completion API.

## Impact
Applications can provide localized and action-specific labels such as Save, Don't Save and Cancel while handling the
result with stable indices. Windows and macOS share explicit default and cancellation behavior even when the visible
text differs.

Every caller of the old API must migrate. Calls that only acknowledge an error pass a single-element button span and
may continue ignoring the returned value. Calls that branch on `MessageBoxButton` must branch on documented local
indices instead.

Windows gains a dependency on Common Controls version 6 and the task-dialog STA requirement. Shared and static builds
must both preserve the manifest dependency. Message-box calls fail when the Luna main thread was previously placed in
an incompatible COM apartment.

The API remains intentionally small. Parent windows, document-modal sheets, asynchronous presentation, destructive
button roles and command links may be designed separately without changing the meaning of this synchronous function.

## Alternatives considered
### Keep `MessageBoxType` and add custom text overrides
This retains a fixed number and semantic meaning for each button, makes custom labels positional exceptions to an
otherwise semantic enumeration, and cannot express arbitrary button counts cleanly.

### Return the selected text or an application-provided identifier
Returning text makes duplicate labels ambiguous and ties control flow to localization. Per-button identifiers require
a descriptor structure for a synchronous API whose caller already has stable array indices. Returning the input index
is simpler and matches both native backends.

### Infer default and cancel buttons from their titles
This fails for localization and action-specific wording and gives different keyboard behavior on Windows and macOS.
Explicit indices make the behavior independent of visible text.

### Keep using `MessageBoxW` on Windows and rename its controls
Changing message-box controls after creation relies on implementation details and still cannot represent the new
contract reliably. `TaskDialogIndirect` is the native API designed for application-defined buttons and result IDs.

### Add a parent window or asynchronous API in the same change
Windows owner-modal dialogs and macOS sheets introduce ownership, lifetime and callback semantics unrelated to custom
button text. Keeping the existing synchronous, calling-thread-modal boundary avoids expanding this API redesign.

## Version history
* **2026/8/26** Proposed and approved.
