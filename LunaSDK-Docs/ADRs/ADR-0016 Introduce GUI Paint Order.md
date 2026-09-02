## Status
Approved.

## Last updated
2026/9/2

## Background
GUI currently preserves painter order by replaying each GUI Layer's `begin_element`, `static_command`, and
`end_element` operations in submission order. The renderer consumes that order immediately while maintaining lexical
clip state and emitting work to the VG and SDF backends. This guarantees correct composition, but it also prevents
commands from otherwise independent widget subtrees from being grouped by compatible rendering state.

A common editor interface sequence is:

```text
component 1 background
component 1 text
component 2 background
component 2 text
```

When background and text use different rendering backends or pipelines, this sequence repeatedly switches backend
state and produces separate draw calls. If the complete painted regions of the two components do not overlap, the
equivalent order is:

```text
component 1 background
component 2 background
component 1 text
component 2 text
```

GUI construction and layout already know where sibling subtrees are independent. Recovering that information later
by comparing command bounds in the renderer would duplicate layout knowledge, would need to account for clips,
shadows, visual overflow, backdrop dependencies, and custom primitives, and may require quadratic comparisons.

GUI also already defines `GUI::Layer`. A GUI Layer is a stable structural object used for root ownership, local
coordinate origins, top-level input priority, popup and floating-window ordering, and `bring_layer_to_front`.
Naming the new ordering coordinate as another kind of Layer would make these distinct responsibilities ambiguous.

The required solution must:

1. Preserve exact rendering for overlapping or otherwise order-dependent content.
2. Let explicitly independent sibling subtrees share ordering positions so compatible commands can batch.
3. Express top-level GUI Layer ordering through the same rendering-order mechanism.
4. Preserve lexical clip semantics and ordered framebuffer dependencies such as backdrop capture.
5. Keep GUI's typeless element tree and Context-controlled traversal.
6. Allow existing static commands and callbacks to migrate incrementally without visual changes.
7. Avoid release-time geometric overlap analysis.

## Decision
Introduce **Paint Order** as the single frame-local ordering coordinate for GUI rendering.

### Paint Order identity and ordering contract
`paint_order_id_t` is an unsigned, frame-local, densely allocated integer. It is regenerated with draw commands and
has no stable identity across frames.

Every drawable command carries a `paint_order_id`. Commands with a lower Paint Order ID must execute before commands
with a higher Paint Order ID. Commands with the same Paint Order ID explicitly grant the renderer permission to
reorder them across complete batch keys. Submission order remains deterministic within one complete batch key.

The same Paint Order ID is therefore stronger than an equal Z value: it declares that cross-key reordering cannot
change the required visual result. Callers must use distinct Paint Order IDs for overlapping primitives whose
composition order matters, unless those primitives are represented by one atomic command with internally defined
ordering.

Paint Order count is independent of draw-call count. Adjacent Paint Order ranges may still be emitted in one draw
call when their complete batch key is compatible and preserving instance order also preserves Paint Order.

### Existing GUI Layer responsibilities
`GUI::Layer`, `Layer::id`, and `DrawCommand::layer` retain their existing structural meanings. In particular,
`DrawCommand::layer` remains the owning GUI Layer index used to resolve local coordinates and element ownership.

`DrawCommand` gains a separate `paint_order_id`. Each GUI Layer also records the first and maximum Paint Order IDs
allocated to its root subtree for inspection and validation.

GUI Layers are visited in their current bottom-to-top order. The first non-empty GUI Layer root starts at Paint Order
ID zero. Every later non-empty GUI Layer root starts at one greater than the previous non-empty GUI Layer's maximum
Paint Order ID. An empty GUI Layer has no Paint Order range and does not consume an ID. Reordering GUI Layers with
`bring_layer_to_front` therefore regenerates non-overlapping Paint Order ranges on the next command generation pass.
The renderer does not use GUI Layer boundaries as an additional ordering rule after Paint Order has been assigned.

GUI Layer order remains authoritative for input routing. Input is routed before delayed draw generation, and one
element may occupy a Paint Order range rather than one scalar order, so Paint Order does not replace structural input
topology.

### Element paint protocol
Draw callbacks receive the first Paint Order ID available to their phase and return the maximum Paint Order ID they
used or reserved.

The `before_children` phase emits an element's background or other underlay. Context then allocates Paint Order for
the element's children. The `after_children` phase receives the first free Paint Order ID after the child subtrees and
emits overlays such as focus feedback, splitters, and docking indicators.

Context continues to own element traversal. Draw callbacks do not recursively paint children. This preserves the
typeless element model, prevents missing or duplicate child traversal, and keeps static operations, phases, clips,
and backdrop markers under one generation algorithm.

Context validates that a callback does not emit below its input Paint Order ID and that its returned maximum is not
below any Paint Order ID it emitted. A callback that emits nothing does not advance the enclosing subtree merely by
returning its unchanged input value. Checked increment is used when allocating the next ID.

### Child Paint Order modes
Each element declares one `ChildPaintOrderMode` for its direct child subtrees:

1. `sequential` gives each child a non-overlapping Paint Order range in submission order. It is the safe default.
2. `shared` gives every child in the current shared run the same initial Paint Order ID and takes the maximum of the
   ranges returned by those children.

Only a container that guarantees independence of the complete painted extents of its relevant children may select
`shared`. This guarantee includes shadows, negative margins, visual overflow, explicit clips, and custom drawing; it
must not be inferred solely from the selected layout algorithm. Debug tooling may diagnose suspicious intersections,
but release command generation performs no geometric overlap analysis.

Containers that require several ordering groups express them through typeless grouping elements. For example, a
scroll view uses a sequential content subtree followed by a shared scrollbar group. Dock panels may share one group,
while splitters are emitted by an `after_children` callback at a later Paint Order.

A Paint Order barrier ends the current shared run. Backdrop capture and future custom operations with framebuffer or
external ordering dependencies are barriers and own exclusive Paint Order IDs. Normal children following a barrier
start at a greater Paint Order ID.

### Compatibility for existing operations
Existing static commands and legacy callback draws without an explicit Paint Order ID remain supported during
migration. The compatibility path assigns each such operation the next available Paint Order ID in original painter
order. A legacy operation between shared children closes the current shared run.

This path intentionally preserves exact output while providing no new cross-operation batching permission. Hot
widgets migrate to explicit `draw(command, paint_order_id)` calls and maximum-order returns before their parent
containers opt into shared Paint Order.

### Clip and backdrop dependencies
`push_clip` and `pop_clip` are lexical operations, not sortable drawables. The renderer first replays the original
submission stream and resolves the final effective clip into immutable per-command state. It then excludes the clip
stack operations from Paint Order sorting. Different immutable clip states may share a batch when the backend stores
clip selection per instance.

A backdrop capture is an execution event with an exclusive Paint Order ID. All captured content has a lower Paint
Order ID, and every consumer of that capture has a higher Paint Order ID. The capture still ends the active color
pass, filters the accumulated target, and resumes rendering, but its position in the final plan is determined only by
Paint Order. No separate render epoch is introduced.

### Renderer compilation
GUI rendering compilation is divided into four stages:

1. Replay the original lexical stream to resolve element ownership, coordinate origins, clips, and backdrop capture
   relationships into immutable resolved draw records.
2. Stable-sort resolved events by Paint Order ID.
3. Within each equal Paint Order range, group commands by their complete backend batch key in deterministic
   first-seen-key order while preserving submission order inside each key.
4. Emit the grouped SDF, VG, and capture batches and coalesce adjacent compatible batches when doing so preserves
   Paint Order.

The initial SDF batch key contains the shape-program page and color-program page. The initial VG key contains the
effective shape buffer, texture, and complete sampler state. Per-instance transform, rectangle, color, and resolved
clip selection are not batch-key fields. A future material or custom pipeline must add all state that changes the
draw call to its complete key.

The first implementation uses stable sorting for `O(C log C)` ordering, where `C` is the number of resolved commands.
Because Paint Order IDs are dense, a later implementation may use vector buckets for `O(C + O)` collection, where
`O` is the number of occupied Paint Order IDs. Both approaches avoid geometric overlap analysis.

### Rollout and validation
The feature is introduced with every element using sequential child ordering and legacy operations using the strict
compatibility path. This state must reproduce existing output before shared child ordering is enabled.

Migration then proceeds through leaf primitives, common widgets, explicitly independent layout containers, and
special ordered containers. Tests cover shared button backgrounds and text, overlapping canvas children, text-input
selection/text/caret ordering, scroll content and scrollbar grouping, dock panels and splitters, independent GUI
Layer ranges, nested clips, backdrop capture barriers, and `bring_layer_to_front` range regeneration. RenderDoc
captures and renderer counters verify reductions in draw calls and backend switches without visual regressions.

## Impact
Expected benefits:

1. Independent widget subtrees can batch backgrounds, text, icons, and other compatible primitives without global
   overlap analysis.
2. Rendering order across normal content, floating windows, popups, and overlays uses one inspectable mechanism.
3. Existing before/children/after composition maps directly to Paint Order ranges.
4. Ordered effects remain expressible through distinct IDs and explicit barriers.
5. The common command-generation path stays linear apart from the initial stable sort.
6. Migration can be incremental and visually conservative.

Costs and risks:

1. `paint_order_id_t`, child-order policy, and range tracking enlarge GUI's public and internal draw model.
2. Selecting `shared` incorrectly can change pixels because same-order commands may be reordered.
3. Renderer compilation becomes multi-stage and temporarily retains resolved draw records.
4. Static compatibility commands serialize surrounding work until migrated.
5. Backdrop capture, custom operations, and future material systems must correctly declare barriers and complete
   batch keys.
6. Documentation and diagnostics must consistently distinguish structural GUI Layer from Paint Order.

## Alternatives considered

### Detect overlapping commands in the renderer
* Status: rejected.
* Pros:
    1. Requires no ordering declaration from containers.
* Cons:
    1. Duplicates layout knowledge after widget semantics have been erased.
    2. Must conservatively model shadows, clips, visual overflow, custom primitives, and backdrop footprints.
    3. Can require quadratic comparisons and still produce fragile results.

### Introduce paint-plane push and pop operations
* Status: rejected.
* Pros:
    1. Fits a stack-based immediate drawing API.
* Cons:
    1. Makes correct balancing a caller responsibility.
    2. Does not naturally expose a subtree's maximum order to its parent.
    3. Adds mutable implicit state where the element traversal already provides the required scopes.

### Let every draw callback recursively paint its children
* Status: rejected for GUI's typeless element model.
* Pros:
    1. Closely resembles Slate's widget-owned `OnPaint` traversal.
    2. Allows arbitrary per-widget child grouping.
* Cons:
    1. A callback may omit or paint a child more than once.
    2. Existing static operations and phase callbacks become substantially harder to preserve.
    3. It moves structural traversal from GUI Context into high-level package callbacks.
    4. Grouping elements and before/after phases express the required initial cases without transferring ownership.

### Reuse GUI Layer IDs as rendering order
* Status: rejected.
* Pros:
    1. Introduces no additional field named as an order coordinate.
* Cons:
    1. GUI Layer IDs are stable identities, while Paint Order IDs are dense and frame-local.
    2. One GUI Layer contains many independently ordered element subtrees.
    3. It conflates coordinates, input priority, ownership, and render batching.

### Preserve strict painter order for every command
* Status: rejected as the long-term design.
* Pros:
    1. Has the simplest correctness model.
* Cons:
    1. Repeated SDF/VG alternation and state changes produce avoidable draw calls.
    2. It ignores independence already known by containers.

## Remarks
This decision extends the GUI logical and rendering layers introduced by [[ADR-0004 Introduce GUI foundation layer]].
It changes only rendering-order generation and compilation. It does not introduce widget types into GUI, replace GUI
Layer input semantics, or change the application-owned submission and presentation boundary.

## Version history
* **2026/9/2** Proposed and approved.
