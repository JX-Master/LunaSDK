## Status

Approved.

## Last updated

2026/9/3

## Background

GUI elements currently draw through delayed `DrawCallback` functions. Delayed callbacks are necessary when final drawing depends on routed input, post-layout state, custom SDF construction, or other procedural decisions. They are unnecessarily indirect for static visuals such as a panel background, text, an image, or an ordered stack of shadows and strokes whose complete description is already known while the element tree is built.

Attaching a generic draw callback to every potentially styled layout element is not acceptable. It would make non-visual elements pay callback and style-resolution costs, and generic style names could accidentally enable painting on elements that were intended to remain layout-only. Static painting must therefore be explicit and sparse.

ADR-0016 assigns painter ordering and batching through Paint Order IDs. Element Visual Effects must participate in that same ordering model without introducing a second ordering mechanism.

## Decision

GUI Core provides Element Visual Effects as an optional, context-owned paint configuration on every typeless `Element`.

- `ElementVisualEffect` stores one concrete, drawable `DrawCommand` template.
- `ElementVisualConfig` stores ordered `before_children` and `after_children` effect spans.
- `IContext::set_element_visual_config` copies both spans into frame-local storage. Callers may therefore pass temporary arrays and strings owned by the command value are copied with it.
- The configuration is sparse. Elements without a draw callback or Visual Effects do not allocate a paint record and do not perform style lookup.
- Visual Effects are explicit values. GUI Core does not inspect styles or infer effects from an element type. EditorGUI, GameGUI, or business code resolves its own styles and attaches only the effects it intends to draw.
- During draw-command generation, GUI Core directly copies each effect command into the generated command stream, overwrites its element, layer, and Paint Order metadata, and assigns consecutive Paint Order IDs in list order.
- Generation order for one element is:
  1. backdrop capture;
  2. `before_children` Visual Effects;
  3. the `before_children` draw callback;
  4. child subtrees;
  5. `after_children` Visual Effects;
  6. the `after_children` draw callback.
- Each effect reserves one Paint Order ID. Multiple effects may be attached in any order, so two differently colored strokes are represented by two stroke effects with independently configured rectangles, offsets, radii, colors, and widths.
- `rounded_rect_stroke` is added as a drawable GUI primitive. It represents a centered analytic rounded-rectangle stroke and uses `DrawCommand::line_width` as its total width.
- Element Visual Effects accept drawable commands only. Structural commands such as clip pushes and pops and backdrop-capture markers remain procedural generator responsibilities and are rejected by the setter. Backdrop capture continues to use its dedicated attachment API.
- Draw callbacks remain the escape hatch for visuals that depend on `route_input`, require procedural post-layout work, emit structural commands, or need custom Paint Order range management.

The existing sparse element draw-configuration index is generalized internally into a sparse paint record that stores both callback and Visual Effect configuration. This avoids adding a per-element allocation or a separate hot-path lookup.

## Migration policy

This decision does not require every existing callback to migrate.

Implementations are migrated when their final visual description does not depend on `route_input` and can be expressed by the available drawable commands. Interaction-dependent components keep callbacks. Static components that currently require procedural post-layout SDF construction may remain callbacks until an equivalent native effect exists.

When a container uses shared child Paint Order ranges, its complete painted extent includes all of its Visual Effects, including shadow and stroke outsets. The caller remains responsible for choosing shared mode only when sibling painted extents do not overlap.

## Impact

### Positive

- Static elements avoid callback invocation and callback userdata lifetime management.
- Layout-only elements remain paint-free unless a higher-level package explicitly attaches effects.
- Ordered fill, stroke, shadow, text, image, shape, backdrop-blur, and SDF commands use the same Paint Order and batching path as callback output.
- Multiple visual layers are data rather than bespoke callback code.
- GUI Core remains widget-free and does not interpret EditorGUI or GameGUI styles.

### Negative

- `IContext` changes ABI and receives a new interface ID.
- Effect command lists are copied into frame-local storage.
- One effect consumes one Paint Order ID even when the command later produces no visible pixels.
- Complex procedural visuals still need callbacks, so both mechanisms remain part of the GUI API.

## Alternatives considered

### Give every layout element a style-aware callback

Rejected because it adds work to non-visual elements and allows broad style definitions to cause accidental rendering.

### Reintroduce unrestricted `record_draw`

Rejected because a general recording API obscures the distinction between static element visuals and procedural drawing, and structural commands make ownership and ordering across children ambiguous. The Visual Effects API deliberately records only explicit drawable commands in two well-defined element phases.

### Translate effects in an element callback

Rejected because it retains callback overhead and callback lifetime concerns. GUI Core already owns command generation and can translate static effects directly.

### Reorder callback output by geometric overlap analysis

Rejected as the primary solution because it is more expensive and less predictable than explicit Paint Order allocation. Geometry-based optimization may still be explored independently.

## Version history

* **2026/9/3** Proposed and approved the sparse GUI Core Element Visual Effects model, static-only migration policy, and analytic rounded-rectangle stroke primitive.
