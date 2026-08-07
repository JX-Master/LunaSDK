## Status
Approved.

## Last updated
2026/7/30

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

The original GUICore rendering boundary assumed that every generated draw command would be compiled into one VG shape draw list and rasterized exclusively by VG. This is sufficient for ordinary vector shapes, text, and images, but it is too restrictive for procedural shadows, temporary render targets, offscreen composition, and future multi-pass effects. Some of these operations do not consume a VG shape buffer and are more naturally implemented by dedicated RHI pipelines. Requiring VG to own every such pipeline would make VG absorb GUI-specific render orchestration that is unrelated to vector path rasterization.

The later application-owned render-pass boundary is also too restrictive for painter-ordered effects. A backdrop blur
must finish all earlier GUI draws, make their color result readable, run one or more filtering passes, then resume
ordinary GUI drawing with the previous color contents loaded. A renderer that is invoked only inside one
application-owned render pass cannot perform those transitions or decide how many render passes the final command
stream requires. Replaying every prefix into a second private target would preserve the old public interface but
would duplicate GUI work and would not necessarily capture the real target contents.

GUICore layout, drawing and input use a two-dimensional logical coordinate system. This is appropriate for desktop
windows and render-to-texture interfaces, but in-game GUI also needs to render directly on an arbitrary plane in a
three-dimensional scene. The standalone VG renderer already accepts a 4x4 projection transform and evaluates contour
antialiasing from perspective-correct interpolated shape coordinates, while the initial GUICore SDF path mapped
instance rectangles directly from logical screen coordinates to clip space. A complete core rendering contract must
therefore support window and world-space surfaces without coupling GUICore to a game camera, physics world or scene
graph, enlarging every SDF instance, or changing the logical input model.

World-space rendering must preserve existing screen-space output, keep SDF and VG content aligned under one surface
transform, preserve analytic antialiasing under perspective, optionally use scene depth, and continue routing input as
host-independent two-dimensional events.

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
GUICore logical layer
        |
GUICore rendering layer
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

### Two-layer GUICore architecture
`GUICore` is internally divided into a logical layer and a rendering layer. These are architectural subsystems of the same module rather than two competing GUI contexts or element trees.

1. **Logical layer**
   - Owns the context, typeless element tree, layers, state, style, layout, input routing, interaction state, and draw-command generation.
   - Produces one ordered stream of renderer-facing primitive draw commands after layout and interaction resolution.
   - Defines painter order, clipping intent, element ownership, and the resources referenced by each command.
   - Does not create RHI pipelines, own render passes, or decide how a primitive is rasterized.

2. **Rendering layer**
   - Consumes the generated draw-command stream and compiles it into an ordered render plan and concrete RHI command-buffer operations.
   - Owns pipeline selection, descriptor binding, temporary render resources, target and sampled-resource transitions, all GUI render/compute pass boundaries, and preservation of painter order across different rasterization paths.
   - Receives a caller-provided final color texture and an optional depth-stencil texture. It derives attachment formats from those resources, begins and ends every GUI pass, and leaves the attachments in documented final states.
   - Uses VG as the primary backend for vector paths, rounded rectangles, text, images, and other suitable vector primitives.
   - May use GUICore-owned RHI pipelines directly for primitives or effects that do not naturally use VG shape buffers, such as analytic shadows, backdrop filtering and offscreen composition.
   - Does not interpret widget semantics or high-level style policy. Immediate API packages still decide which primitives and effects to emit.

Direct RHI use is an internal rendering-layer capability, not a general escape hatch that lets immediate API packages inject arbitrary command-buffer callbacks. Public draw commands must remain inspectable data with deterministic ordering and declared resource usage. New RHI-backed primitives should be represented by explicit draw-command types and implemented by rendering-layer command compilers.

The renderer exposes one recording entry point. `IRenderer::render` receives the context, a recording graphics command
buffer, one render-target description and one surface description. The command buffer must not be inside any pass
when the call begins or ends. The renderer generates commands when necessary, compiles the ordered plan, uploads
resources, records every required barrier and graphics/compute pass, and transitions the supplied attachments to
their requested final states. It does not submit, wait, reset the command buffer, present a swapchain, or provide
cross-queue synchronization; those remain application responsibilities.

The required color texture is the final GUI target. The target description supplies the first-pass color load
operation, clear value and requested final state; color storage is always preserved. An optional depth-stencil
texture supplies its own load/store/clear values and final state. `RenderSurfaceDesc` keeps projection, depth
test/write, comparison and culling policy, but no longer duplicates the depth format. When depth testing is enabled,
the target description must provide a compatible depth-stencil texture.

### Painter-ordered backdrop blur
Backdrop blur is split into capture and drawing responsibilities.

1. A `BackdropBlurCaptureDesc` is attached sparsely to an element. At that element's painter entry, before its
   `before_children` callback, command generation inserts an inspectable `backdrop_blur_capture` marker. The marker
   draws no pixels but forms an ordered render-plan barrier.
2. A `backdrop_blur` draw command resolves the nearest capture attachment in the current element's
   self-or-ancestor chain. Parent links never cross layers. The compiler converts this relationship into a frame-local
   capture resource reference; missing or forward references fail closed.
3. Reaching a live capture marker flushes pending VG, SDF and backdrop batches, stores and ends the current color
   pass, makes the accumulated target readable, filters the required region into an immutable renderer-owned texture,
   then resumes GUI drawing in a new load/store pass.
4. One capture result may serve multiple descendant draw commands. A later capture sees every command completed
   before its own marker, including earlier backdrop draws. Capture contents are regenerated each frame; only
   allocation capacity may be pooled across frames.

The semantic output region is bounded by the capture element and its consuming backdrop draws. Filtering reads an
expanded source region that includes the algorithm's kernel halo. Rounded masks and GUI clips apply when the filtered
result is drawn, not while its source is captured. Screen-space target coordinates map filtered pixels back to draw
commands without stretching a cropped texture.

Softness is a Gaussian standard deviation in logical surface units. Filtering first performs the requested
power-of-two downsample chain using an energy-preserving center-and-diagonal tent kernel derived from Studio Bloom,
then performs normalized horizontal and vertical Gaussian convolution at the working resolution. Linear sampling
combines adjacent Gaussian tap pairs. The requested downsample level is a minimum: the renderer adds levels until
the largest framebuffer-space working standard deviation is at most 10 pixels, or until the bounded chain reaches
its limit or the captured extent. If the limit is reached first, the working kernel clamps to 10 pixels. Gaussian
support is truncated at 2.5 standard deviations. The capture halo covers that texel-rounded support plus the
footprint contributed by the downsample tent chain.
Working standard deviations are quantized to half-pixel steps, and normalized paired sample offsets and weights are
precomputed on the CPU. A quantized working standard deviation at or below 0.5 pixels skips Gaussian convolution
and retains only the requested snapshot or downsample result.

Backdrop filtering uses a storage-writable intermediate format selected from the caller's color-target format
rather than a fixed format. Ordinary RGBA/BGRA 8-bit targets use RGBA8 UNORM storage, floating-point targets preserve
their floating-point precision when the RHI supports the corresponding storage format, and unsupported integer,
compressed or depth formats fail explicitly. GUICore filters the numeric values supplied by the texture format and
does not explicitly decode or encode a transfer function. This accepts the small luminance difference between
encoded-space and linear-light blur in exchange for a simpler and less expensive filter.

The initial backdrop implementation supports single-sample, two-dimensional screen-space targets. A target used by
a live capture must support both `color_attachment` and `read_texture`. Non-sampleable swapchain images remain valid
when the command stream has no live captures; a capture on such a target returns `not_supported` instead of silently
changing semantics. Perspective/world-space backdrop filtering and MSAA capture are deferred.

VG remains a raster backend rather than a render-pass owner. Its integrated shape renderer receives target format,
extent and fixed-function compatibility data, exposes the resource usages needed by recorded draw ranges, and can
submit selected ranges into GUICore-owned passes. GUICore combines those usages with SDF and backdrop resources and
records the actual barriers.

### SDF shape and paint programs
GUICore owns a signed-distance-field backend for analytic GUI geometry and effects. VG remains the backend for text,
images, icons and arbitrary vector contours, while SDF handles rectangles, rounded rectangles, circles, ellipses,
capsules, constructive geometry, gradients, borders, highlights and soft inset or outset effects.

The SDF backend consumes two private transient scalar-float streams:

1. A **shape buffer** containing analytic primitives and prefix constructive-solid-geometry expressions.
2. A **color buffer** containing one or more ordered concrete paint or effect instructions for a reusable shape.

Both resources use a four-byte structured-buffer stride, matching the scalar storage model of `VG::IShapeBuffer` but
remaining a separate GUICore ABI. Widget semantics, Style names and arbitrary shader callbacks do not enter either
stream. Immediate API packages resolve Style values into concrete color instructions before submission.

#### Scalar instruction format
Every instruction begins with an integer-valued float32 opcode. Opcodes remain below `2^24`, so conversion between
the float representation and `u32` is exact. There is no universal record-length, arity or format-version header.
Fixed-length instructions derive their layout from the opcode; variable instructions carry only their own required
count, such as `num_stops` or a future `num_points`.

CPU handles retain offset and allocation length for validation, page packing and diagnostics. These lengths are not
repeated in every instruction. The GPU instance stores the color span length, from which the shader reaches the end
by parsing each instruction's known layout. Streams are transient rather than persistent serialized ABIs; any future
persistent cache version belongs to resource-level metadata.

The initial limits are:

1. 64 shape instructions, 512 shape floats and an evaluation stack depth of 16.
2. 8 ordered color instructions, 1024 color floats and 16 stops per gradient instruction.
3. 16 MiB per scalar program page; no individual shape or color program may cross a page boundary.

#### Shape instructions
The initial shape instruction set contains:

1. Axis-aligned rectangle.
2. Axis-aligned rounded rectangle with four independent circular corner radii.
3. Circle.
4. Axis-aligned ellipse.
5. Capsule defined by a line segment and radius.
6. Prefix union, intersection, difference and exclusive-or operations.

Shape coordinates use top-left GUI logical coordinates with Y increasing downward. A primitive returns a signed
distance with negative values inside. Binary operations preserve operand order and use prefix notation:

```text
union(A, B)        = min(A, B)
intersection(A, B) = max(A, B)
difference(A, B)   = max(A, -B)
xor(A, B)          = max(min(A, B), -max(A, B))
```

The CPU validator derives expression length, operand count, maximum stack depth, instruction count and conservative
bounds before any program is uploaded.

#### Color instructions and distance clipping
The low eight bits of a color opcode select the base algorithm. Two additional bits select independent signed-distance
limits:

```text
bits 0..7: base color opcode
bit 8:     inner clip
bit 9:     outer clip
```

The initial algorithms are solid color, linear gradient, elliptical radial gradient, conic gradient, four-corner
bilinear gradient and analytic shadow. Colors are concrete non-premultiplied sRGB values. Gradient instructions store
ordered fixed-stride stops, support pad and repeat spread, midpoint-adjusted sRGB interpolation and at most 16 stops.

For signed distance `d`, an instruction may store `inner_distance`, `outer_distance`, or both. The shader forms:

```text
inner term   = -d - inner_distance
outer term   =  d - outer_distance
clip distance = maximum of the enabled terms
```

The resulting modes are:

1. `00b`: no SDF clipping inside the finite raster mesh.
2. `01b`: reject points deeper than `inner_distance` inside the contour.
3. `10b`: reject points farther than `outer_distance` outside the contour.
4. `11b`: keep an independently controlled boundary band on both sides.

Conventional fill is outer clip with `outer_distance = 0`. A centered stroke of width `w` uses both clips with each
distance set to `w / 2`. Different distances produce asymmetric inward and outward strokes without multiplying the
base opcode set. Clip coverage uses `fwidth` antialiasing, and shader discard may occur only after coverage is zero.

#### Ordered multi-effect programs and shape reuse
One SDF draw references one validated shape range and one contiguous color span. A color span may contain several
ordered instructions, such as:

```text
OuterShadow -> SolidColor -> InnerHighlight
```

The shader evaluates the base shape once, executes each color instruction in order and composes results with
premultiplied source-over. Shadows with different offsets still perform their required shifted shape evaluation.
Single-instruction programs keep a fast path. Several draws may also reference the same shape range with different
color spans when their raster domains, clip state or painter-order requirements prevent safe fusion.

This makes fill, border, highlight and shadow reuse explicit without expanding `SDFInstance`. The current instance is
40 bytes and stores the draw rectangle, evaluation origin, program offsets/span metadata and an index into a deduplicated
48-byte raster state containing rectangular and rounded GUI clips.

#### Unified analytic shadow
Inner and outer shadows use one Shadow opcode containing RGBA, offset, softness and spread. The shader compares the
original shape mask with a shifted softened mask to produce a two-sided signal. Generic clip flags choose the visible
part: inner clip at zero selects the outer side, outer clip at zero selects the inner side, no clip keeps both sides,
and both clips retain a bounded contour band. Shadow data does not live in `SDFDrawDesc` or per-instance state.

#### Raster domain, GUI clipping and blending
Every SDF draw uses a finite rectangle mesh as its shader invocation domain. NoClip and InnerClip mean only that the
shader does not impose an outward SDF limit; they do not create an infinite draw. The renderer derives conservative
bounds from the submitted draw rectangle, shape bounds, color clip distances, effect outsets and shadow falloff.

The responsibilities remain separate:

1. The rectangle mesh bounds shader invocation.
2. The shape buffer supplies signed distance.
3. Color clip bits restrict signed-distance coverage inside the mesh.
4. Color instructions compute paint or effects.
5. Deduplicated raster state and the GUI clip stack implement parent, rounded-container and scroll clipping.

The SDF pipeline emits premultiplied-alpha output and uses source-over blending. Invalid CPU programs fail closed;
unknown or malformed shader instructions output transparent black.

#### Ownership, paging and batch interleaving
The context owns transient shape/color streams. Regenerating draw commands restores the recorded stream prefix before
delayed callbacks run, so generation is deterministic. The renderer validates programs, packs them into reusable
structured-buffer pages and stores page-local offsets in instances.

Consecutive SDF instances using the same shape/color page pair are submitted with `draw_indexed_instanced` without
changing logical order. SDF instance state is not a batch key. VG, SDF and backdrop batches remain interleaved in
exact painter order, and capture markers prevent batching across a target-read boundary. Transitional rectangle,
rounded-rectangle, gradient-rectangle and shadow commands compile into the same SDF representation; text, images,
lines and arbitrary vector paths continue through VG.

### GUI surface rendering
One GUICore context describes one logical two-dimensional **GUI surface**. `FrameDesc::screen_size`, element layout
rectangles, layer positions, SDF programs, VG positions, GUI clips and input events remain in surface-local logical
coordinates with a top-left origin and downward-positive Y axis.

The host may supply a `surface_to_clip` matrix when rendering. It maps positions `(x, y, 0, 1)` from
logical surface coordinates directly to clip space. Without a custom transform, the renderer derives the existing
top-left orthographic projection from `FrameDesc::screen_size`. The matrix is stored once in frame data and does not
become part of `SDFInstance`, `SDFState` or the SDF batch key.

SDF draw rectangles remain axis-aligned quads in surface-local coordinates. The vertex shader transforms their four
vertices into clip space and passes the original surface position to the pixel shader using perspective-correct
interpolation. Shape evaluation, gradients, color effects, rectangular clips and rounded clips continue to use the
surface coordinate system. The integrated VG draw list historically generates bottom-left coordinates; the GUICore
renderer converts those positions to the public top-left convention before applying the same surface transform.

The first implementation supports one plane per context. Per-element transforms and several non-coplanar surfaces in
one context are deferred; a host that needs several independently placed panels uses one context and one renderer
preparation per surface.

#### Perspective antialiasing
Analytic coverage remains derivative based. The SDF pixel shader evaluates signed distance in perspective-correct
surface coordinates and uses `fwidth(distance)` to obtain the local-coordinate footprint of one framebuffer pixel.
VG retains its existing derivative-based contour coverage and receives the equivalent surface transform.

Soft-shadow parameters remain surface-local logical distances so a shadow scales and foreshortens with its surface.
The shader combines analytic softness with a minimum derivative footprint so minified or grazing-angle shadows do not
alias.

#### Optional scene depth
The render-surface description declares depth test/write state, comparison function and cull mode. The render-target
description optionally supplies the real depth-stencil texture and its load/store/clear contract. Screen-space
rendering leaves depth disabled. A normal in-game panel uses `less_equal` depth testing with depth writes disabled so
scene geometry can occlude the GUI without transparent GUI fragments becoming occluders.

SDF and integrated VG pipelines derive compatible color and depth formats from the supplied attachments. GUICore
owns their barriers and pass creation. If a backdrop capture interrupts a depth-enabled GUI pass, intermediate
depth/stencil contents are preserved and the resumed pass loads them before drawing continues.

#### Surface input mapping
GUICore does not accept camera rays or query a physics world. The host selects a surface, intersects the pointer ray
with its plane, transforms the hit into logical surface coordinates and submits an ordinary `InputEvent` to that
surface's context.

GUICore provides a math helper that maps a world-space ray through a host-provided `world_to_surface` affine matrix.
It intentionally returns positions outside `FrameDesc::screen_size`, allowing captured sliders and drags to continue
after a pointer leaves the panel. Surface selection, occlusion and nearest-hit ordering remain host responsibilities.

### GUICore responsibilities
`GUICore` owns the following systems:

1. **Element tree**
   - Stores per-frame GUI elements in dense arrays.
   - Each element has a stable ID, layer ID, parent/child/sibling topology, hot layout input, layout result, style binding, interaction binding, sparse optional callback bindings, and draw-command ownership metadata.
   - The element tree is the core-level ground truth for the submitted frame.
   - The tree is typeless. Every element has the same concrete storage type.
   - Elements do not use inheritance, virtual methods, or per-widget subclasses to define behavior.
   - Element behavior is defined only by the data attached to the element, such as layout records, interactable records, sparse callback records, draw command ownership metadata, and clip records. State IDs are derived by callers from stable owner IDs and boxed state types rather than stored by elements.
   - Human-readable element and layer debug names are resident observational metadata. Debug names must never affect layout, input, rendering, identity, or other GUI behavior.
   - An element may represent a text label, a button chrome, a shape, a layout container, a hit-test region, or any other primitive submitted by higher-level code, but the core element itself does not know that semantic widget type.

2. **Layer system**
   - A layer owns one root element tree and an ordered generated draw-command index list. The logical layer generates all layer commands in painter order, and the rendering layer compiles them into one ordered render plan.
   - Layers are stored in bottom-to-top Z order and are rendered with painter's algorithm.
   - Input is routed from top layers to bottom layers.
   - Popups, tooltips, drag previews, modal panels, debug overlays, and normal content are represented as layers instead of special widget cases.

3. **State store**
   - Keeps the existing typed state object model: state ID, boxed data object, and lifetime.
   - Supports current-frame, next-frame, context, and future persistent lifetimes.
   - Does not know widget-specific state shapes.

4. **Style system**
   - Keeps named styles, parent inheritance, local overrides, inherited entries, and explicit unsets.
   - Keeps scalar, vector, color, and name-valued entries.
   - Provides style schema metadata for tools and inspectors.
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
   - Provides primitive commands for rectangles, rounded rectangles, gradients, text, images, shapes, lines, clipping, and explicitly modeled rendering effects such as shadows.
   - Does not know widget rendering policies.
   - High-level immediate API packages install package-owned callbacks or submit static commands directly.

9. **Inspection and performance instrumentation**
   - Exposes read-only spans for the current frame's dense element and layer arrays. Tools inspect element topology, layout input and results, interactables, sparse callback bindings, routed interaction state, styles, and draw commands through ordinary public APIs instead of requesting a duplicated debug snapshot.
   - Keeps `PerformanceCounters`, human-readable debug names, and high-level debug views available in all builds.
   - Does not use a dedicated debug compilation option. The remaining inspection features have small runtime cost, while one stable API and ABI avoids conditional build complexity.
   - Does not own issue/pass trace logs, input recording, input replay, frame timelines, or a cross-process debug transport. Such tools may be implemented outside GUICore when a concrete use case requires them.

### Data and algorithm separation
`GUICore` follows a data-oriented model. The element tree is a data container. Core features are algorithms that operate on this data.

Core features should be implemented as independent functions or systems:

1. Layout functions operate on layout data and write layout results.
2. Input routing functions operate on layers, elements, and interactable records.
3. Draw generation functions traverse element data and produce draw commands; rendering-layer functions compile those commands into an ordered render plan and concrete RHI operations, using VG where appropriate.
4. Style functions operate on style records and requested style keys.
5. Inspection tools read the current frame through read-only element, layer, style, interaction, callback configuration, and draw-command APIs.

These features must remain orthogonal where possible. A caller should be able to use layout without using input routing, use draw commands without using high-level widgets, or use interactables without using a particular widget package.

### Hot element data and sparse callback bindings
Frequently accessed data stays directly in the dense element array. This includes element topology, `LayoutConfig`, `LayoutResult`, `Interactable`, style binding and draw-command summary fields.

Optional callback-bearing records are stored in separate per-context sparse arrays:

1. `LayoutCallbackConfig` for measure, arrange and finalize callbacks.
2. `NavigationConfig` for per-action navigation policy and callbacks.
3. `ElementHitTestConfig` for custom shape hit testing.
4. `DrawConfig` for delayed command generation.

An element stores only a `u32` index for each optional record, with `U32_MAX` meaning the default behavior. The sparse arrays are logically cleared and rebuilt by `begin_frame`, while their allocated capacity can be reused. Callback userdata is frame-scoped unless the installing package deliberately provides longer-lived storage. Cross-frame widget data belongs in the state store, not callback configuration.

`LayoutCallbackConfig::algorithm` is semantic metadata for diagnostics and higher-level capability recognition. GUI Core dispatches through callback pointers and never uses the name as a callback registry key.

### Per-element navigation policy
GUICore consumes semantic navigation input rather than hard-wiring platform keys. Host adapters translate keyboard,
gamepad or other input into directional navigation, sequential forward/backward navigation, confirm and back actions.

GUICore does not store a context-level navigation graph. Explicit relationships are usually view- and
business-specific, especially for game menus, virtualized lists, canvas overlays, custom inspectors and interfaces
whose next target depends on application state. Each element instead carries a small frame-local navigation
configuration for each action:

1. `NavigationMode::automatic`
   - Use GUICore's built-in navigation.
   - Directional actions use the spatial focus algorithm.
   - Forward/backward actions use focusable order within the active focus scope.
   - Confirm and back use default event delivery and action behavior.

2. `NavigationMode::none`
   - Consume the request without moving focus or invoking a callback.
   - This explicitly blocks one direction or action.

3. `NavigationMode::callback`
   - Invoke the callback attached to the focused element.
   - The callback may call `IContext::focus_element` to select a target.
   - The callback may call `IContext::navigate_default` when it deliberately wants automatic behavior.

A callback return value reports whether it handled the request. Returning `false` consumes the request as a no-op; it
does not silently fall back to automatic navigation. This keeps fallback explicit and lets view code implement direct
jumps, wrapping, conditional skipping, modal rules and virtualized navigation without expanding the core element with
target fields.

`NavigationConfig` is rebuilt each frame like layout, interactable and draw configuration. GUICore owns semantic
events, default algorithms, opt-out modes, callback dispatch and focus mutation APIs, but not application navigation
edges. Inspection tools can report automatic, disabled and callback modes from the current element array, but cannot
predict all targets selected inside business callbacks. GUIAsset must serialize higher-level navigation properties or
handler identifiers rather than raw callback pointers.

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

### Built-in vector icons and composition
The default `GUI` package provides a curated core icon set for common navigation, editing, file, status and
interaction actions. The checked-in icon data is generated offline from a pinned revision of the MIT-licensed
Phosphor icon source. The generator converts supported SVG paths into native VG scalar shape commands, so deployed
applications do not parse SVG, depend on an SVG runtime, or compile one C++ function per icon. GUI initializes one
shared immutable VG shape buffer and each icon variant references a compact range of that buffer. Missing optional
weights fall back to the regular variant.

An icon is an ordinary measured and drawn child element. Containers such as buttons, menu items, tabs and rows do
not expose icon slots, inspect their children for icon types, or alter behavior based on icon presence. Applications
compose an icon and text with the same flex, grid and other layout primitives used for arbitrary children. The
container's normal gap, alignment and interaction rules determine the result. This preserves package layering and
allows application-defined icon elements to participate without widget-specific integration.

The built-in core set remains intentionally curated to bound binary size. Editor- or DCC-specific icon packs may be
generated and linked by higher-level packages without expanding the default GUI binary. Icon provenance, source
revision and selected variants remain in a machine-readable manifest beside the generated data, and the upstream
license is distributed with that data.

### Rendering policy and style schema
`GUI::RenderProxy` is removed from the target architecture.

In the new model, a high-level immediate API installs a package-owned draw callback or emits static commands for each element it creates. The callback runs only during GUICore's fixed painter-order traversal and can read final layout, interaction state, state objects, and style values before emitting primitive commands. It has no object identity, registration, inheritance, RTTI type, or lifecycle of its own.

This callback is not a revival of `GUI::RenderProxy`. It is a low-level, typeless GUICore command-generation hook. The default editor-style GUI package still owns its rendering policy and does not promise that users can replace every widget renderer.

The GUICore rendering layer rasterizes the primitive commands produced by these callbacks, but it does not choose a widget's colors, spacing, shadow recipe, animation, or visual composition. For example, a GUI package may emit two shadow commands and one rounded-rectangle command for a neumorphic button; the rendering layer only executes those commands using its VG and RHI-backed primitive implementations.

Style usage schema remains useful, but it is no longer attached to `GUI::RenderProxy`. Instead:

1. `GUICore` provides style records, style resolution, and read-only access to style records referenced by elements.
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

Within `GUICore`, logical data and algorithms should remain separable from rendering implementation code. RHI
pipeline objects, descriptor sets, backdrop capture textures and other temporary GPU resources belong to the
rendering layer and must not be stored in elements, context state objects or unrelated layout/input APIs. Applications
create the final color and optional depth-stencil textures and own command-buffer submission, synchronization and
presentation. GUICore owns attachment transitions and every render/compute pass recorded by its renderer.

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
9. Performance counters and read-only element/layer inspection.

The current `GUI` module should still build and run while this layer is introduced.

#### Phase 2: Data-oriented element tree
Move the runtime tree source of truth from `GUI::Node` to the `GUICore` element tree.

This phase should:

1. Add APIs for creating elements, setting parent-child topology, and attaching layout/interactable/draw data.
2. Keep all element storage typeless.
3. Avoid virtual methods and per-widget element subclasses.
4. Add element ID lookup, dense element/layer scanning, and human-readable debug names.
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
3. **Better tooling**: GUIEditor, debug panels, and external inspection tools can read a smaller and more regular runtime model without duplicating it into a second snapshot representation.
4. **Better performance potential**: dense data arrays and data-oriented traversal are easier to optimize than many concrete widget node types.
5. **Better in-game GUI support**: layers, draw primitives, interactables, and input routing are host-independent and can be used for window GUI and in-game surfaces.
6. **Specialized UI packages**: different domains can provide different immediate API packages without duplicating core layout, input, state, and draw infrastructure.
7. **Simpler default GUI package**: the default editor-style package can stop pretending to be fully generic and can optimize for DCC/editor workflows.
8. **Extensible rendering orchestration**: GUICore can preserve one painter-ordered command stream while selecting
   one or many passes and combining VG vector rasterization with dedicated RHI pipelines and temporary targets.
9. **Direct world-space GUI**: SDF geometry, VG text, images and icons can render on an arbitrary scene plane without an intermediate GUI texture.
10. **Stable hot-path data**: world-space projection does not enlarge the 40-byte SDF instance, duplicate raster states or change existing batch keys.
11. **Resolution-independent perspective output**: perspective-correct local coordinates retain analytic gradients, clipping and antialiasing at the final scene sampling rate.
12. **Host-independent world input**: ray-to-surface mapping reuses the existing logical input router without adding camera, scene or physics dependencies to GUICore.
13. **Reusable analytic effects**: one validated SDF shape can drive ordered fill, asymmetric stroke, gradient, highlight and shadow instructions without duplicating geometry.
14. **Compact programmable data**: scalar opcode streams avoid universal headers, while multi-effect color spans and deduplicated raster states reduce instance and upload cost.
15. **View-owned explicit navigation**: applications can block or override individual navigation actions without making GUICore own a business-specific graph.

Risks:

1. **Large migration cost**: current widgets, layout code, input code, debug tools, and GUIEditor integration must be migrated gradually.
2. **Temporary duplication**: old node runtime and new core runtime may coexist during migration. This must be minimized.
3. **Over-generalization risk**: GUICore may become too abstract if it tries to solve every future UI problem up front.
4. **Boundary erosion risk**: high-level widget behavior may leak into GUICore if module boundaries are not enforced.
5. **Performance regression risk**: a data-oriented design still needs benchmarks; new abstractions should not assume they are faster without measurement.
6. **Customization regression risk**: removing render proxies reduces per-widget render customization in the default GUI package. This is intentional, but it must be documented and offset by supporting additional immediate API packages.
7. **Rendering-layer scope risk**: the new rendering layer may duplicate VG or grow into a general-purpose render graph if its responsibilities are not kept to GUI command execution.
8. **Backend interleaving risk**: VG-backed and direct-RHI commands must preserve exact painter order, clipping and
   resource states across GUICore-owned pass boundaries.
9. **Surface-transform integration risk**: custom transforms must follow GUICore's top-left coordinate convention
   and use attachments compatible with the renderer-owned pass plan.
10. **Grazing-angle risk**: extremely minified surfaces and geometry crossing the near plane require explicit visual regression coverage.
11. **World-input adapter risk**: platform IME rectangles must be projected back to screen space, while surface selection and occlusion remain host responsibilities.
12. **Initial plane-count limitation**: one context cannot initially contain several independently transformed planes.
13. **SDF validation dependency**: scalar instructions omit universal record lengths and versions, so CPU validation must reject malformed programs before upload.
14. **SDF shader-cost risk**: deep CSG, many gradient stops, multi-effect spans and shifted shadow evaluation can increase fragment cost.
15. **Approximate-distance risk**: CSG min/max preserves contours but not exact Euclidean distance near every seam.
16. **Navigation inspection limitation**: tools can see callback presence and mode but cannot infer every runtime target selected by view code.
17. **Backdrop pass-break risk**: each live capture may store and reload attachments and dispatch filtering work;
    tile-based GPUs can make pass breaks more expensive than the backdrop draw itself.
18. **Attachment capability risk**: a target that is valid for ordinary GUI drawing may not be sampleable for
    backdrop capture. Swapchain, MSAA and perspective paths require explicit validation rather than implicit fallback.

Mitigations:

1. Add benchmarks and performance counters before migrating heavy systems.
2. Keep GUICore primitive-only.
3. Migrate one widget group at a time.
4. Preserve existing GUI tests and add new stress tests for each migrated subsystem.
5. Do not migrate DockSpace first.
6. Keep GUIAsset as a design-time representation rather than serializing GUICore internals.
7. Document each immediate API package's intended domain and supported customization surface.
8. Keep vector path rasterization in VG and add direct RHI pipelines only for explicit GUICore primitives that do not naturally use VG shape buffers.
9. Keep RHI execution internal to the rendering layer; do not expose arbitrary command-buffer callbacks as ordinary draw commands.
10. Expose one renderer entry point that starts and ends outside every pass. Let GUICore compile the complete plan,
    transition supplied attachments and record all required graphics/compute passes; keep submit, synchronization and
    presentation application-owned.
11. Keep the public surface coordinate convention fixed at top-left/Y-down and adapt backend-specific coordinate conventions inside the renderer.
12. Validate both default orthographic and custom perspective paths with the same mixed SDF/VG sample content.
13. Keep scene selection, ray ordering, camera composition and IME screen projection in host adapters rather than GUICore.
14. Validate every SDF opcode, count, scalar parameter, prefix expression and program boundary before upload.
15. Keep strict shape-instruction, stack-depth, color-instruction and gradient-stop limits and prevent programs from crossing pages.
16. Maintain CPU reference evaluation, cross-backend shader compilation, visual SDF samples and counters for scalar data, upload bytes, instances, pages and backend switches.
17. Treat fragment discard and multi-effect fusion as measured optimizations rather than unconditional assumptions.
18. Keep navigation callbacks frame-local and require explicit calls to `navigate_default` when view code wants automatic fallback.
19. Track capture count, render-pass count, filtered pixels, blur dispatches and temporary texture bytes. Eliminate
    unused captures, share one immutable result across multiple draws, and reject unsupported target capabilities.

## Alternatives considered

### Require VG to remain the exclusive GUICore rasterization backend
* Status: rejected.
* Pros:
    1. Keeps one simple rendering dependency and one existing draw-list format.
    2. Reuses the current VG shape renderer without introducing a new rendering subsystem.
    3. Makes all GUI rendering appear uniform to hosts.
* Cons:
    1. Procedural shadows and other effects that do not consume shape buffers would still need to be forced into VG-specific APIs.
    2. Offscreen and multi-pass composition require render-target and pass orchestration above individual VG shape draws.
    3. VG would accumulate GUI-specific pipelines and scheduling responsibilities unrelated to vector rasterization.
    4. A single VG draw-list contract cannot naturally describe arbitrary ordered RHI-backed primitives without becoming a general GUI renderer itself.

### Keep application-owned render passes and replay painter prefixes for backdrop capture
* Status: rejected.
* Pros:
    1. Preserves the existing public `prepare`/`render` split.
    2. Does not require GUICore to begin or end the application's target pass.
* Cons:
    1. Every capture requires replaying earlier GUI ranges into a private target, duplicating raster work.
    2. The replayed prefix is not guaranteed to match color already present in the real target.
    3. Nested captures multiply work and make target/depth equivalence difficult to validate.
    4. Pass count and attachment lifetimes remain split across two owners.

### Give each immediate API package direct RHI rendering callbacks
* Status: rejected.
* Pros:
    1. Maximum rendering freedom for custom packages.
    2. Package authors can introduce effects without changing GUICore command types.
* Cons:
    1. Arbitrary callbacks are not inspectable or serializable command data.
    2. Resource usage, barriers, clipping, and painter order become difficult for GUICore to validate and schedule.
    3. It recreates an unrestricted render-proxy model at a lower layer.
    4. Package-specific RHI code would leak backend and lifetime details into logical GUI construction.

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

### Store an explicit navigation graph in GUICore::IContext
* Status: rejected.
* Pros:
    1. Makes explicit edges easy for GUICore tools to visualize and validate.
* Cons:
    1. Adds graph ownership, lifetime, validation and cross-frame cleanup to the core.
    2. Encourages business-specific relationships to leak into GUICore.
    3. Handles dynamic, conditional and virtualized targets less naturally than view callbacks.

### Store direct navigation target IDs in every element
* Status: rejected for the initial design.
* Pros:
    1. Keeps explicit relationships beside element input data.
* Cons:
    1. Requires several permanent target fields on every element.
    2. Is too rigid for wrapping, conditional skipping, virtualized views and state-dependent targets.

### Support automatic navigation only
* Status: rejected.
* Pros:
    1. Preserves the smallest navigation API.
* Cons:
    1. Tree order and geometry cannot reliably represent every editor, game-menu and custom-view navigation rule.

### Keep a common float4 SDF instruction header
* Status: rejected.
* Pros:
    1. Gives every instruction an explicit record length, arity and version.
* Cons:
    1. Repeats metadata already implied by fixed opcodes.
    2. Wastes space and diverges from VG's scalar structured-buffer convention.
    3. Variable instructions can carry their own count without a universal header.

### Keep fill, stroke and shadow fields in SDF instance state
* Status: rejected.
* Pros:
    1. Makes simple passes easy to configure without parsing a color instruction.
* Cons:
    1. Mixes geometry references with concrete paint algorithms.
    2. Duplicates color and effect data and enlarges the hot instance stream.
    3. Prevents one reusable shape from selecting independent ordered color programs cleanly.

### Define separate fill and stroke opcodes for every paint algorithm
* Status: rejected.
* Pros:
    1. Gives every combination a direct shader branch.
* Cons:
    1. Multiplies the opcode set as new paints and effects are added.
    2. Generic inner/outer distance clip bits express fill, symmetric and asymmetric strokes for every algorithm.

### Require one fixed allocation size for every SDF instruction
* Status: rejected.
* Pros:
    1. Makes random access and bounds validation trivial.
* Cons:
    1. Wastes substantial space for gradients and future point lists.
    2. Opcode-specific count fields preserve compactness while keeping parsing bounded.

### Treat NoClip and InnerClip as infinite SDF draws
* Status: rejected.
* Pros:
    1. Avoids deriving effect-specific bounds.
* Cons:
    1. The finite rectangle mesh already defines shader invocation.
    2. Treating clip semantics as raster bounds would conflate two independent responsibilities.

### Extend the VG contour command buffer with SDF operations
* Status: rejected.
* Pros:
    1. Uses one public vector command stream and renderer.
* Cons:
    1. VG evaluates contour coverage rather than signed distance.
    2. Adding GUI CSG, gradients and analytic shadows would join unrelated raster models and expand VG into a GUI-specific backend.

### Render every world-space GUI to a texture and map the texture to a mesh
* Status: rejected as the only world-space path; remains supported as an application choice.
* Pros:
    1. Naturally supports arbitrary mesh placement and material composition.
    2. Can cache interfaces that update infrequently.
* Cons:
    1. Requires an offscreen target and additional texture memory.
    2. Adds filtering blur and update latency.
    3. Prevents SDF and VG analytic coverage from being evaluated directly at the final scene sampling rate.

### Store a 4x4 transform in every SDF instance
* Status: rejected for the initial surface model.
* Pros:
    1. Allows independently transformed elements inside one context.
* Cons:
    1. Enlarges the hot instance stream and duplicates a transform shared by most GUI geometry.
    2. Complicates clipping, CPU culling, batching and input semantics.
    3. A surface-level transform already covers ordinary in-game panels.

### Submit world-space rays directly to GUICore input routing
* Status: rejected.
* Pros:
    1. Lets GUICore perform surface intersection internally.
* Cons:
    1. Couples the input router to cameras, scene surfaces, occlusion and hit ordering.
    2. Duplicates responsibilities already owned by game and viewport hosts.
    3. Breaks the host-independent two-dimensional input contract.

### Add perspective projection without depth configuration
* Status: rejected.
* Pros:
    1. Requires fewer pipeline variants and a smaller preparation descriptor.
* Cons:
    1. Direct-rendered panels cannot be correctly occluded by scene geometry.
    2. Applications would need a separate ad-hoc rendering path for depth-tested GUI.

## Remarks
This ADR does not require an immediate full rewrite. It defines the target architecture and migration discipline. The project may temporarily build GUICore beside the current GUI runtime, but every migration phase should reduce the old widget-node-specific code surface.

The name `GUICore` is intentionally explicit. It should be understood as a lower-level primitive layer, not as a widget API. Normal application code should usually use one of the immediate API packages built on top of `GUICore`.

Until GUICore is formally released, architectural refinements to its logical layer, rendering layer, navigation,
SDF programs, surface model and related core contracts are recorded by revising this ADR rather than creating
additional GUICore-specific ADRs. The version history below preserves when each refinement was adopted.

## Version history
* **2026/7/30** Replaced the application-owned `prepare`/`render` split with one GUICore-owned rendering entry point.
  The host supplies a final color texture and optional depth-stencil texture while GUICore records all attachment
  transitions and graphics/compute passes. Added painter-ordered backdrop capture attachments, self-or-ancestor
  backdrop draws, immutable frame-local capture results and explicit target capability limits. Backdrop filtering
  uses Studio Bloom-derived tent downsampling followed by normalized horizontal and vertical Gaussian convolution.
  Revised the filter budget to performance-driven automatic downsampling, 2.5-sigma support, half-pixel
  precomputed kernels, target-derived intermediate precision, transfer-function-agnostic filtering and a sub-pixel
  fast path.
* **2026/7/22** Added the default GUI package's curated Phosphor-derived vector icon pack, offline SVG-to-VG generation contract and ordinary-child composition rule; widget containers remain icon-agnostic.
* **2026/7/22** Consolidated the previously separate per-element navigation and SDF shape/paint decisions into ADR-0004, and established this ADR as the single pre-release architecture record for GUICore.
* **2026/7/22** Integrated the world-space GUI surface decision into this ADR. GUICore now defines one top-left logical surface per context, a shared SDF/VG surface-to-clip transform, perspective derivative antialiasing, optional scene depth and host-side ray-to-surface input mapping.
* **2026/7/17** Adopted scalar SDF shape and color instruction streams, analytic primitives and prefix CSG, generic inner/outer distance clipping, unified shadows, bounded paging, deduplicated raster state and ordered multi-effect color spans.
* **2026/7/17** Defined application-owned render-pass boundaries. `IRenderer::prepare` compiles commands and prepares internal resources outside a pass, while `IRenderer::render` submits ordered VG and direct-RHI batches inside a compatible application-owned pass. VG shape renderers follow the same preparation/submission split.
* **2026/7/17** Split GUICore internally into a logical layer and a rendering layer. The logical layer generates ordered primitive draw commands, while the rendering layer owns command compilation and backend interleaving and may combine VG rasterization with explicit GUICore-owned RHI pipelines for shadows, offscreen composition, and future multi-pass effects.
* **2026/7/13** Made human-readable debug names and high-level debug views resident in all builds, and removed the dedicated GUI Core debug compilation option. The remaining memory and update cost is small compared with the API, ABI, build, and tooling complexity caused by conditional availability.
* **2026/7/12** Replaced the duplicated `DebugInfo` snapshot, issue/pass logs, input replay, and frame timeline with direct read-only inspection of the current element and layer data. Performance counters remain available in all builds, and debug names cannot affect GUI behavior.
* **2026/7/12** Reconciled the approved architecture with the implementation: layers keep ordered generated-command indexes and compile into one VG draw list; optional layout, navigation, hit-test and draw callback bindings use per-frame sparse arrays; Stack Layout is removed; and element state IDs are derived externally.
* **2026/7/4** Clarified the layout model: flex replaces the old linear-layout wording, element sizing is fixed/percent/fit with min/max constraints, and content-driven measurement is supplied by package-owned measure callbacks.
* **2026/6/28** Adopted semantic navigation input with automatic, disabled and callback policy stored per element instead of a context-owned navigation graph.
* **2026/6/17** Revised the decision to remove `GUI::Node`, `GUI::RenderProxy`, and `GUI::IContext` from the long-term architecture, define the element tree as typeless data, and narrow the default `GUI` module into one editor-style immediate API package.
* **2026/6/16** Proposed.
