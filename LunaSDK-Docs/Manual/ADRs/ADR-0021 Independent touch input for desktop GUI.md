## Status
Approved.

## Last updated
2026/10/20

## Background
Window declares basic touch notifications, but Windows does not produce them. GUIWindow converts touch contacts into the shared mouse pointer stream. GUI has one mouse capture and button state, and EditorGUI scroll views only interpret wheels and scrollbar manipulation. This prevents direct touch scrolling and makes contact movement conflict with buttons, mouse input and other contacts.

## Decision
1. Window exposes separate touch-down, move, up and cancel events with a device identity, contact identity and client coordinates. Cancellation ends a contact without activation. Existing touch event identities remain stable.
2. Windows consumes native PT_TOUCH WM_POINTER messages, tracks active contacts, converts physical screen coordinates into client coordinates, and cancels contacts on capture loss, focus loss or window destruction. Touch compatibility mouse messages are discarded. Pen and physical mouse handling retain their own paths.
3. GUIWindow preserves touches as independent GUI touch events. It never translates a touch into a pointer-button or pointer-wheel event.
4. GUI maintains contact-specific positions, movement, targets and captures, keyed by device and touch identity. Touch routing never modifies mouse position, hover, buttons, capture or click history. Shared semantic interaction flags may report activation from either input source; touch-specific flags identify its source.
5. Generic directional touch gesture capabilities on interactables arbitrate competing descendants and ancestors after a movement threshold. A winning drag receives movement, losing targets receive touch cancellation, and a moved or canceled contact cannot click. Scroll views declare their supported pan axes; sliders and other drag controls declare their manipulation axes. These capabilities describe routing, not widget-specific actions.
6. EditorGUI components interpret native touch events alongside mouse events. Buttons and choices consume shared semantic activation; controls needing coordinates consume the appropriate raw input stream. A control manipulation is owned by its initiating input source/contact, so concurrent input cannot take over an in-progress operation.
7. Window-backed application adapters convert both touch and mouse positions into GUI logical coordinates. Per-frame mouse sampling remains independent of active touches.

## Impact
Touch scrolling, tapping and direct manipulation work without synthesizing mouse input. Multiple contacts can interact with independent controls while the mouse remains usable. Hosts must rebuild against the new GUI interface and should submit touch cancellation whenever a contact is interrupted. Touch interaction requires deterministic coverage of cross-frame routing, cancellation, movement thresholds, nested gestures and simultaneous mouse/contact input. Physical touchscreen verification is still necessary for platform delivery and device-specific behavior.

## Alternatives considered
* Promote the primary contact to a mouse: rejected because it merges state, loses contact identity and prevents independent concurrent interactions.
* Convert touch movement to wheel input in Windows: rejected because it embeds GUI gesture policy into Window and bypasses click/drag arbitration.
* Implement scrolling only in the application: rejected because all EditorGUI consumers need consistent touch behavior.

## Version history
* **2026/10/20** Proposed and approved as part of the requested independent touch input implementation.
