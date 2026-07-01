## Status
Approved.

## Last updated
2026/6/28

## Background
ADR-0004 introduces `GUICore` as a low-level, data-oriented GUI foundation. `GUICore` owns the typeless element tree, input routing, focus state, and reusable interactable primitives, while higher-level immediate API packages and application views own widget behavior and business-specific semantics.

GUICore now receives semantic navigation input events instead of hard-wiring keyboard keys to navigation behavior. Host adapters translate platform input into:

1. Directional navigation, such as keyboard arrow keys or gamepad D-pad.
2. Sequential forward/backward navigation, such as Tab and Shift+Tab.
3. Confirm and back navigation, such as gamepad A/B or keyboard Enter/Esc.

The remaining question is how explicit navigation relationships should be represented. Some interfaces need navigation behavior that cannot be inferred reliably from element tree order or screen-space geometry. Examples include game menus, custom inspectors, non-rectangular tool palettes, virtualized lists, canvas overlays, and views where the next focus target depends on application state.

One possible design is to store a navigation graph in `GUICore::IContext`. That would make `GUICore` own edges between focusable elements. However, explicit navigation graphs are often tightly coupled to view structure and business rules. Keeping them in the context would make the core layer responsible for semantic relationships that belong to higher-level view code.

## Decision
GUICore will not store a context-level navigation graph.

Instead, each element may carry a small per-element navigation configuration as part of its input configuration. This configuration controls how the focused element responds to navigation requests:

1. `NavigationMode::automatic`
   - Use GUICore's built-in automatic navigation.
   - Directional navigation uses the current spatial focus algorithm.
   - Forward/backward navigation uses the current focusable element order within the active focus scope.
   - Confirm and back use GUICore's default event delivery/action behavior.

2. `NavigationMode::none`
   - Consume the navigation request without moving focus and without invoking callbacks.
   - This is used when a direction or action should be explicitly blocked.

3. `NavigationMode::callback`
   - Invoke a user-supplied callback attached to the focused element.
   - The callback may call `IContext::focus_element` to choose the next target manually.
   - The callback may call `IContext::navigate_default` to explicitly fall back to automatic behavior.

The callback return value means whether the callback handled the request. Returning `false` consumes the request as a no-op; it does not automatically fall back. This avoids hidden behavior and lets the callback author decide exactly when automatic behavior should run.

This design keeps explicit navigation knowledge in the view or application layer. GUICore only provides:

1. Semantic navigation input events.
2. Default automatic navigation behavior.
3. Per-element opt-out and callback hooks.
4. Focus mutation APIs that callbacks can use.

The per-element navigation configuration is frame-local element data, just like layout input, interactable data, style binding, and draw command ranges. It is rebuilt by the immediate API package or application view each frame.

## Impact
This decision keeps GUICore small and orthogonal:

1. GUICore does not need to maintain graph lifecycle, graph validation, cross-frame edge cleanup, or graph serialization.
2. Navigation behavior remains close to the view code that understands the business rules.
3. Views can implement dynamic navigation, conditional focus movement, wrapping, skip lists, modal behavior, or virtualized item navigation without teaching GUICore about those concepts.
4. Automatic navigation remains available for simple controls and as a fallback path for callbacks.
5. `NavigationMode::none` provides a cheap way to block accidental focus movement in a specific direction or action.

The trade-off is that GUICore cannot inspect or visualize a complete navigation graph by itself, because that graph may exist only inside user callback code. Debug tools can show the per-element navigation modes and callback presence, but they cannot know every possible runtime target chosen by application logic.

Another consequence is that serialized GUI assets should not store raw callback pointers. If GUIAsset needs to expose this feature, it should serialize higher-level navigation properties or handler IDs, then bind runtime callbacks during view generation.

## Alternatives considered
### Store a navigation graph in `GUICore::IContext`
This would allow GUICore to own explicit edges such as left/right/up/down/forward/backward targets. It would make graph visualization and validation easier.

This option was rejected because explicit navigation relationships are frequently business-specific. A context-level graph would add ownership, lifetime, validation, and debug complexity to the core layer, and it would encourage applications to put view semantics into GUICore.

### Store direct target IDs in every element
This would keep navigation data on the element rather than the context, but each element would still need several explicit target fields.

This option was rejected for the initial design because direct target IDs are too rigid for dynamic views. Callback mode can implement direct target jumps, wrapping, conditional skipping, or virtualized item navigation without expanding the core element record with many relationship fields.

### Keep only automatic navigation
This would preserve the smallest API surface.

This option was rejected because automatic tree-order and geometry-based navigation cannot handle all editor, in-game, and custom view navigation requirements reliably.

## Version history
* **2026/6/28** Proposed and approved.
