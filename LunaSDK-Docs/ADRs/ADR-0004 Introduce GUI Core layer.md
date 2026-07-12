## Status
Proposed.

## Last updated
2026/7/12

## Background
LunaSDK currently has a `GUI` module that has already moved away from the old ImGui backend. The current implementation uses explicit GUI contexts, per-frame descriptions, layers, state objects, style entries, render proxies, VG-backed drawing, and GUI debugging support. This direction has proven useful for Studio, GUIEditor, in-game GUI rendering, and custom vector rendering.

However, the current `GUI` module still mixes several responsibilities in one layer:

1. It stores a runtime node tree.
2. It owns input routing, focus, hover, active, popup, scroll, table, tab, and dock interaction rules.
3. It provides built-in widgets and views.
4. It translates widget state and style into draw commands.
5. It exposes debug information and editor metadata.

This makes the module powerful, but it also creates architectural pressure. Even after the node-kind enum was removed and concrete nodes became RTTI objects, the context and input code still contain widget-specific branches for cases such as scroll bars, table separators, tab headers, numeric controls, dock panels, and popup handling. As more widgets, visual effects, editor features, and in-game GUI use cases are added, this coupling will keep increasing unless the lower-level GUI primitives are separated from the widget layer.

At the same time, GUIEditor and GUIAsset need stable design-time and runtime metadata. Visual editing should not depend on private widget implementation details, and custom user widgets should not require modifying the core GUI module. A lower-level GUI core layer can provide a small set of orthogonal primitives that are easier to inspect, test, record, replay, and compose.

Since common GUI concepts will be moved into `GUICore`, the high-level immediate API does not need to remain a fully generic widget framework. Instead, LunaSDK should support multiple immediate API packages for different application domains. Each package can make stronger visual and interaction assumptions, keep its implementation smaller, and still interoperate with other packages because they all build the same `GUICore` element tree.

## Decision
Introduce a new `GUICore` layer between `VG` and high-level immediate GUI packages.

`GUICore` is a low-level, data-oriented GUI foundation. It must not provide high-level widgets such as buttons, checkboxes, combo boxes, menu items, color editors, or dock panels. Instead, it provides orthogonal primitives that higher-level modules use to build widgets, views, tools, and editors.

The current `GUI` module becomes one immediate API package built on top of `GUICore`. This package is primarily designed for DCC software, editor tools, Studio, and GUIEditor. It should favor concise, efficient, editor-style controls over fully customizable game UI visuals. Other packages may be introduced later for game HUDs, console UI, mobile UI, or other visual styles.

The new layering is:

```text
Applications, Studio, GUIEditor, game code
        |
Immediate API packages, GUIAsset generators, editor adapters
        |
GUICore
        |
VG, Font, RHI, Runtime
```

This decision removes the following concepts from the long-term high-level `GUI` architecture:

1. `GUI::Node`
   - The runtime should maintain only one tree: the typeless `GUICore` element tree.
   - High-level immediate APIs submit data into this tree directly.

2. `GUI::RenderProxy`
   - High-level immediate APIs attach package-owned GUICore draw callbacks or submit static commands directly.
   - Rendering policy is part of the immediate API package implementation, not a fully replaceable per-widget runtime callback.

3. `GUI::IContext`
   - Context ownership, frame state, input, layout, state, style, and rendering are `GUICore` responsibilities.
   - High-level GUI APIs take a `GUICore` context pointer or wrapper handle and operate on the core element tree.

### GUICore responsibilities
`GUICore` owns the following systems:

1. **Element tree**
   - Stores per-frame GUI elements in dense arrays.
   - Each element has a stable ID, layer ID, parent/child/sibling topology, layout input, layout result, style binding, interaction binding, optional draw callback binding, draw-command ownership metadata, and debug metadata.
   - The element tree is the core-level ground truth for the submitted frame.
   - The tree is typeless. Every element has the same concrete storage type.
   - Elements do not use inheritance, virtual methods, or per-widget subclasses to define behavior.
   - Element behavior is defined only by the data attached to the element, such as layout records, interactable records, draw callback records, draw command ownership metadata, clip records, and debug tags. State IDs are derived by callers from stable owner IDs and boxed state types rather than stored by elements.
   - An element may represent a text label, a button chrome, a shape, a layout container, a hit-test region, or any other primitive submitted by higher-level code, but the core element itself does not know that semantic widget type.

2. **Layer system**
   - A layer owns one root element tree and an ordered generated draw-command index list. GUI Core generates and compiles all layers into one destination VG draw list in painter order.
   - Layers are stored in bottom-to-top Z order and are rendered with painter's algorithm.
   - Input is routed from top layers to bottom layers.
   - Popups, tooltips, drag previews, modal panels, debug overlays, and normal content are represented as layers instead of special widget cases.

3. **State store**
   - Keeps the existing typed state object model: state ID, boxed data object, and lifetime.
   - Supports current-frame, next-frame, process, and future persistent lifetimes.
   - Does not know widget-specific state shapes.

4. **Style system**
   - Keeps named styles, parent inheritance, local overrides, inherited entries, and explicit unsets.
   - Keeps scalar, vector, color, and name-valued entries.
   - Provides style schema metadata for tools and debug output.
   - Does not hard-code widget style keys.

5. **Layout engine**
   - Provides reusable layout primitives rather than widget-specific layout code.
   - Initial primitives should include flex layout, canvas placement, grid layout, table tracks, scroll viewport layout, and clipping.
   - The element axis size model should stay small: `fixed`, `percent`, and `fit` with explicit min/max constraints. Content-driven `fit` sizing is supplied by measure callbacks owned by higher-level packages instead of by scanning draw commands or hard-coding text knowledge into GUICore.
   - Specialized sizing such as ratio table tracks belongs to the specific parent layout primitive that owns that placement policy.
   - Layout should be axis-agnostic where practical so that width and height logic do not diverge.
   - Layouting is implemented by independent functions operating on element data, not as element methods.

6. **Interactable graph**
   - Provides reusable input primitives for hover, active, focus, capture, disabled, readonly, focus scopes, keyboard navigation, and hit testing.
   - Supports parent-child interactable relationships so compound widgets can query whether any child is hovered, active, or focused.
   - This system is the long-term replacement for widget-specific input branches in the current GUI context.

7. **Input router**
   - Consumes GUI input events.
   - Routes pointer, keyboard, text, scroll, focus, blur, and capture events through layers and interactables.
   - Supports focus scopes and keyboard navigation.
   - Supports scroll routing and future scroll-to-rect propagation.

8. **Draw command generation**
   - Records static GUI-level draw commands during tree construction and supports an optional delayed draw callback on each element.
   - Generates the final command stream after layout, input routing, and higher-level package state resolution.
   - Invokes callbacks before children, after children, or at both traversal phases so parent backgrounds and overlays have deterministic painter order.
   - Provides primitive commands for rectangles, rounded rectangles, gradients, text, images, shapes, lines, and clipping.
   - Does not know widget rendering policies.
   - High-level immediate API packages install package-owned callbacks or submit static commands directly.

9. **Debug and instrumentation**
   - Provides an in-memory debug snapshot for element trees, layers, layout inputs/results, interactables, input routing, style resolution, draw commands, and performance counters. A portable external-tool representation is a separate projection because runtime callbacks, userdata, and resources are pointer-valued.
   - Should eventually support input recording, input replay, frame stepping, issue logging, and layout/input pass reason logging.

### Data and algorithm separation
`GUICore` follows a data-oriented model. The element tree is a data container. Core features are algorithms that operate on this data.

Core features should be implemented as independent functions or systems:

1. Layout functions operate on layout data and write layout results.
2. Input routing functions operate on layers, elements, and interactable records.
3. Draw generation functions traverse element data and produce draw commands; rendering functions compile those commands into VG draw lists.
4. Style functions operate on style records and requested style keys.
5. Debug functions capture the data needed to inspect the frame in-process.

These features must remain orthogonal where possible. A caller should be able to use layout without using input routing, use draw commands without using high-level widgets, or use interactables without using a particular widget package.

Drag-drop is a composite interaction protocol rather than a GUICore primitive. GUICore does not store payloads, source or target type lists, deliveries, or drag-drop state, and its input router does not special-case pointer release for dropping. A high-level package may implement drag-drop by composing generic hit testing, pointer capture, routed input events, context state, layers, and draw commands. Different packages may choose different payload protocols, activation thresholds, previews, target feedback, and delivery lifetimes.

### Immediate API package responsibilities
The existing `GUI` module becomes one high-level immediate API package built on top of `GUICore`.

An immediate API package owns:

1. User-facing APIs such as `text_button`, `checkbox`, `input_text`, `slider`, `combo`, `menu`, `tab_bar`, `color_edit`, and higher-level views.
2. The algorithms that translate those APIs into `GUICore` element data and package-owned draw callbacks.
3. The visual style and interaction behavior of those controls.
4. The small set of user-facing customization points intentionally supported by that package.
5. GUIAsset runtime generation helpers for widgets provided by that package.
6. Optional composite interaction protocols such as drag-drop when required by that package.

An immediate API package does not need to be universally customizable. The default `GUI` package should expose simple customization points such as margins, spacing, colors, font choices, and selected behavior flags. It should not promise that users can completely replace the rendering logic of every control.

Different immediate API packages can coexist and nest because they all submit elements into the same `GUICore` context. For example, an editor-style package and an in-game HUD package may both build into one frame as long as they follow the same core layer, layout, input, and draw command rules.

### Rendering policy and style schema
`GUI::RenderProxy` is removed from the target architecture.

In the new model, a high-level immediate API installs a package-owned draw callback or emits static commands for each element it creates. The callback runs only during GUICore's fixed painter-order traversal and can read final layout, interaction state, state objects, and style values before emitting primitive commands. It has no object identity, registration, inheritance, RTTI type, or lifecycle of its own.

This callback is not a revival of `GUI::RenderProxy`. It is a low-level, typeless GUICore command-generation hook. The default editor-style GUI package still owns its rendering policy and does not promise that users can replace every widget renderer.

Style usage schema remains useful, but it is no longer attached to `GUI::RenderProxy`. Instead:

1. `GUICore` provides style records, style resolution, and in-memory debug snapshots.
2. Immediate API packages declare the style keys that their APIs read.
3. GUIEditor and debug tools consume these declarations to display editable style properties.
4. Core draw primitives remain widget-independent.

### GUIAsset and GUIEditor
`GUIAsset` remains a design-time representation, not a direct serialization of the GUICore element tree. GUIAsset nodes describe editable widgets/views and their properties. At runtime, GUIAsset generation calls high-level `GUI` APIs, which then build GUICore elements.

GUIEditor should eventually consume three kinds of schema:

1. Widget/view property schema from GUIAsset node types.
2. Style usage schema from the immediate API package that implements the widget.
3. Parent layout attachment schema from GUICore layout primitives and high-level layout APIs.

This lets GUIEditor present meaningful inspector controls without depending on private runtime element records.

### Module boundary
Prefer a separate module:

```text
Modules/Luna/GUICore
Modules/Luna/GUI
Modules/Luna/GUIWindow
```

`GUICore` may depend on `Runtime`, `RHI`, `VG`, and `Font` only when required for draw command and font resource integration. It must not depend on `Window`, `HID` platform event types, Studio, GUIEditor, or high-level GUI widgets.

`GUI` depends on `GUICore` and provides the default editor-style immediate API package. It must not define its own runtime context, runtime node tree, or render proxy system.

`GUIWindow` adapts Window/HID input and clipboard APIs to GUICore/GUI input interfaces.

### Migration plan
The migration should be staged to avoid keeping two competing GUI runtimes alive for too long.

#### Phase 1: GUICore skeleton
Create the `GUICore` module and add:

1. Frame description.
2. Input event types.
3. `GUICore` context.
4. Typeless element records and dense element storage.
5. Layer records.
6. State store.
7. Style store.
8. Draw command list.
9. Basic debug dump.

The current `GUI` module should still build and run while this layer is introduced.

#### Phase 2: Data-oriented element tree
Move the runtime tree source of truth from `GUI::Node` to the `GUICore` element tree.

This phase should:

1. Add APIs for creating elements, setting parent-child topology, and attaching layout/interactable/draw data.
2. Keep all element storage typeless.
3. Avoid virtual methods and per-widget element subclasses.
4. Add element debug output and ID lookup.
5. Add adapter code so old GUI widgets can submit core elements during migration.

The goal of this phase is to make `GUICore` element data the only long-term runtime tree representation.

#### Phase 3: Interactable graph
Implement core interactables before migrating most widgets. This phase should include:

1. Hover, active, focus, capture, disabled, and readonly state.
2. Parent-child interactable hierarchy.
3. Focus scopes.
4. Keyboard navigation basics.
5. Pointer hit testing through layers.

The goal of this phase is to replace the most fragile widget-specific input code with reusable primitives.

#### Phase 4: Immediate API package migration
Migrate the simplest APIs in the default editor-style `GUI` package:

1. Text.
2. Image.
3. Shape.
4. Button container.
5. Text button.
6. Checkbox.
7. Radio button.
8. Selectable.

These APIs should submit GUICore element data, interactable data, and layout data, then attach package-owned GUICore draw callbacks or static commands. They should not create `GUI::Node` objects or assign `GUI::RenderProxy` objects.

#### Phase 5: Layout primitives
Migrate layout containers:

1. Horizontal and vertical layouts.
2. Canvas layout.
3. Grid layout.
4. Table layout.
5. Scroll view.

Add layout benchmarks and a large scroll/table stress test for every migration step.

#### Phase 6: Complex APIs and views
Migrate complex views:

1. Combo.
2. Menu bar and menu items.
3. Popup and tooltip helpers.
4. Tab bar.
5. Input text.
6. Numeric slider/drag/input helpers.
7. Color edit and color picker.

These should use interactable hierarchy, focus scopes, layers, and draw primitives instead of widget-specific context branches.

#### Phase 7: Docking and Studio/GUIEditor integration
DockSpace can remain in the high-level `GUI` layer until core layout and interactable primitives are stable. It should be migrated only after the simpler systems are proven.

Studio and GUIEditor should be updated after the corresponding widget groups are migrated. GUIEditor must remain schema-driven and should not edit GUICore internal element records directly.

#### Phase 8: Remove legacy GUI runtime concepts
After all high-level APIs build through GUICore, remove:

1. `GUI::Node`.
2. `GUI::RenderProxy`.
3. `GUI::IContext`.
4. Compatibility shims that expose old node behavior.

## Impact
This change is large and intentionally architectural. It moves LunaSDK GUI from a widget-node-centered runtime to a core-primitive-centered runtime.

Expected benefits:

1. **Cleaner extension model**: users can build custom widgets by composing core primitives instead of modifying the GUI module.
2. **Less context coupling**: widget-specific input and rendering branches can be removed from the context over time.
3. **Better tooling**: GUIEditor, debug panels, replay tools, and external MCP/debug clients can inspect a smaller and more regular runtime model.
4. **Better performance potential**: dense data arrays and data-oriented traversal are easier to optimize than many concrete widget node types.
5. **Better in-game GUI support**: layers, draw primitives, interactables, and input routing are host-independent and can be used for window GUI and in-game surfaces.
6. **Specialized UI packages**: different domains can provide different immediate API packages without duplicating core layout, input, state, and draw infrastructure.
7. **Simpler default GUI package**: the default editor-style package can stop pretending to be fully generic and can optimize for DCC/editor workflows.

Risks:

1. **Large migration cost**: current widgets, layout code, input code, debug tools, and GUIEditor integration must be migrated gradually.
2. **Temporary duplication**: old node runtime and new core runtime may coexist during migration. This must be minimized.
3. **Over-generalization risk**: GUICore may become too abstract if it tries to solve every future UI problem up front.
4. **Boundary erosion risk**: high-level widget behavior may leak into GUICore if module boundaries are not enforced.
5. **Performance regression risk**: a data-oriented design still needs benchmarks; new abstractions should not assume they are faster without measurement.
6. **Customization regression risk**: removing render proxies reduces per-widget render customization in the default GUI package. This is intentional, but it must be documented and offset by supporting additional immediate API packages.

Mitigations:

1. Add benchmarks and debug counters before migrating heavy systems.
2. Keep GUICore primitive-only.
3. Migrate one widget group at a time.
4. Preserve existing GUI tests and add new stress tests for each migrated subsystem.
5. Do not migrate DockSpace first.
6. Keep GUIAsset as a design-time representation rather than serializing GUICore internals.
7. Document each immediate API package's intended domain and supported customization surface.

## Alternatives considered

### Continue evolving the current GUI module
* Status: rejected.
* Pros:
    1. Lowest short-term migration cost.
    2. Existing Studio and GUIEditor code keeps working with fewer changes.
    3. Current widget APIs already cover many practical needs.
* Cons:
    1. Context and input code will continue to accumulate widget-specific branches.
    2. Custom widgets still depend on implementation details.
    3. Debugging complex focus, popup, drag-drop, and scroll interactions remains harder.
    4. GUIEditor will need more ad-hoc knowledge about runtime behavior.

### Put GUI Core inside the existing GUI module only as an internal namespace
* Status: rejected for long-term architecture, acceptable only as a temporary migration step.
* Pros:
    1. Faster to start.
    2. Less build-system churn.
    3. Easier to reuse existing files during early prototyping.
* Cons:
    1. The primitive-only boundary is much harder to enforce.
    2. Widget-specific code can easily leak into the core layer.
    3. Other modules cannot depend on the core without also depending on high-level widgets.

### Make GUIAsset directly serialize GUICore element trees
* Status: rejected.
* Pros:
    1. One runtime representation could be reused for saving and editing.
    2. GUIEditor could inspect exactly what the runtime consumes.
* Cons:
    1. GUICore element records are frame-runtime data, not stable design data.
    2. GUI assets need semantic widgets/views, not low-level draw and interaction records.
    3. Runtime element internals will change more frequently than editable asset format.
    4. This conflicts with ADR-0003's decision that GUIAsset uses its own editable node types.

### Keep high-level widgets as concrete runtime node types
* Status: rejected as the long-term direction.
* Pros:
    1. Familiar implementation model.
    2. Easy to attach widget-specific data and virtual methods.
* Cons:
    1. Encourages widget-specific behavior in the runtime.
    2. Makes dense traversal and data-oriented optimization harder.
    3. Makes custom user widgets depend on extending the runtime node model.

### Keep GUI::RenderProxy as the primary customization mechanism
* Status: rejected.
* Pros:
    1. Users can completely replace the rendering logic of a widget.
    2. Existing style schema work can stay close to the current node/render-proxy model.
* Cons:
    1. It preserves a generic widget customization model that the new architecture intentionally avoids in the default package.
    2. It keeps rendering behavior attached to high-level widget runtime objects instead of direct draw command generation.
    3. It complicates data-oriented element storage.
    4. Different immediate API packages should define their own rendering policy instead of sharing one universal render proxy system.

### Keep GUI::IContext as the high-level context
* Status: rejected.
* Pros:
    1. Existing widget code can migrate more slowly.
    2. Applications continue to see a familiar context type.
* Cons:
    1. It creates two context concepts: one in GUI and one in GUICore.
    2. It makes it unclear which layer owns frame state, input routing, layout, and rendering.
    3. It weakens the rule that GUICore owns the only runtime element tree.

## Remarks
This ADR does not require an immediate full rewrite. It defines the target architecture and migration discipline. The project may temporarily build GUICore beside the current GUI runtime, but every migration phase should reduce the old widget-node-specific code surface.

The name `GUICore` is intentionally explicit. It should be understood as a lower-level primitive layer, not as a widget API. Normal application code should usually use one of the immediate API packages built on top of `GUICore`.

## Version history
* **2026/7/12** Reconciled the approved architecture with the implementation: layers keep ordered generated-command indexes and compile into one VG draw list; elements may attach sparse before/after-children draw callbacks; Stack Layout is removed; element state IDs are derived externally; and DebugInfo is an in-memory snapshot rather than a wire format.
* **2026/7/4** Clarified the layout model: flex replaces the old linear-layout wording, element sizing is fixed/percent/fit with min/max constraints, and content-driven measurement is supplied by package-owned measure callbacks.
* **2026/6/17** Revised the decision to remove `GUI::Node`, `GUI::RenderProxy`, and `GUI::IContext` from the long-term architecture, define the element tree as typeless data, and narrow the default `GUI` module into one editor-style immediate API package.
* **2026/6/16** Proposed.
