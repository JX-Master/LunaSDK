GUI drawing generates primitive GUI-level commands and preserves their declared Paint Order across two rendering
backends. Analytic geometry, gradients and shadows use the GUI SDF renderer; text, images and arbitrary vector
paths use VG. Higher-level packages decide which commands represent a widget.

## Designed functionality
The drawing system provides a narrow bridge between GUI data and rendering:

1. Attach generation-time draw callbacks to elements and submit every command with an explicit Paint Order ID.
2. Support element-relative and layer-coordinate rectangles.
3. Draw analytic SDF shapes, CSS-like gradients, strokes and shadows alongside text, images and VG shape ranges.
4. Generate commands after layout and input routing while preserving required element and GUI Layer order.
5. Keep rendering policy outside GUI widgets, because GUI has no widgets.

GUI owns every render/compute pass and target transition needed to execute its final command stream. The
application owns the command buffer, final color and optional depth-stencil textures, submission, synchronization
and presentation.

## Concepts
### Draw command
`GUI::DrawCommand` describes one primitive command. Its `type` selects the payload fields used by the command.

Supported command types are:

1. `rect`
2. `gradient_rect`
3. `rounded_rect`
4. `line`
5. `text`
6. `image`
7. `shape`
8. `shadow`
9. `sdf`
10. `push_clip`
11. `pop_clip`
12. `backdrop_blur_capture`
13. `backdrop_blur`

`rect`, `gradient_rect`, `rounded_rect` and `shadow` are convenient commands for common GUI chrome. The renderer
translates them to SDF shape and color programs before submission. Use `sdf` directly for circles, ellipses,
capsules, four-corner rounded rectangles, strokes, composed shapes, and multi-stop gradients.

### Draw callback
`DrawConfig` attaches an optional callback to one element. Generation can invoke the callback in
`DrawPhase::before_children`, `DrawPhase::after_children`, or both phases.

The callback runs after layout and input routing, so it can read final layout rectangles, interaction state,
state objects and style values. It emits commands by calling
`IContext::draw(const DrawCommand&, paint_order_id_t)`. Commands cannot be submitted during element construction.
The callback must not mutate the element tree, layout, interaction state, or application data.

`DrawConfig::userdata` is owned by the caller and must remain valid until command generation finishes. GUI
stores draw configurations sparsely, so elements without callbacks do not carry callback pointers.

### Paint Order
`paint_order_id_t` is an unsigned, dense, frame-local ordering coordinate. A lower ID renders before a higher ID.
Giving commands the same ID explicitly permits the renderer to reorder those commands across complete backend batch
keys; submission order remains deterministic within one batch key. Use distinct IDs for overlapping primitives
whose composition order matters.

A draw callback receives the first Paint Order ID available to its phase and returns the maximum ID it emitted or
reserved. Every `draw` call must pass an ID no lower than the callback input, and the returned maximum must cover all
IDs emitted by the callback. A callback that emits nothing and reserves no IDs returns its input unchanged.

Each non-empty GUI Layer receives a non-overlapping Paint Order range in bottom-to-top layer order. IDs are
regenerated with draw commands and have no stable identity across frames. `GUI::Layer` remains a structural object
for element ownership, coordinates and input priority; it is not a Paint Order ID.

### Element paint traversal
For each layer, GUI traverses elements in painter order:

1. Invoke the element's `before_children` callback.
2. Generate child elements according to the element's child Paint Order mode.
3. Invoke the element's `after_children` callback with the first free ID after the child subtrees.

This lets a container draw its background below children and its border or feedback above children without
requiring the final layout rectangle during element construction.

`ChildPaintOrderMode::sequential` is the safe default and gives each direct child a non-overlapping range in
submission order. `ChildPaintOrderMode::shared` gives independent direct child subtrees the same initial ID so their
compatible commands can batch. Select `shared` only when the complete painted extents of those children cannot
interfere, including shadows, negative margins, visual overflow, explicit clips and custom drawing. A backdrop
capture or another Paint Order barrier ends the current shared run.

### Backdrop capture and drawing
`BackdropBlurCaptureDesc` is a sparse element attachment. When it is enabled, command generation inserts a
`backdrop_blur_capture` marker immediately after entering the element and before its `before_children` callback. The
marker has no raster output; it tells the renderer to materialize all earlier target color into an immutable filtered
texture.

A `backdrop_blur` draw command resolves the nearest capture attachment in its owning element's self-or-ancestor
chain. Parent links remain layer-local. The relationship is resolved while the final command stream is compiled and
is valid only for that render recording. Missing captures fail instead of reusing stale frame data.

Capture softness is expressed in logical surface units. One capture may serve several backdrop draw commands. A
later nested capture includes all commands completed before its own marker, including earlier backdrop draws.
Rounded masks and GUI clips affect backdrop drawing; they do not truncate the source neighborhood required by the
filter kernel.

The renderer treats softness as the Gaussian standard deviation. Each requested downsample level is a separate
2x center-and-diagonal tent pass, followed by normalized horizontal and vertical Gaussian passes at the resulting
resolution. The requested level is a minimum; the renderer adds levels until the largest framebuffer-space working
standard deviation is at most 10 pixels, subject to the bounded chain and captured extent, then clamps the working
kernel to that limit if no more levels are available. Gaussian support is truncated at 2.5 standard deviations, and
the capture halo covers its texel-rounded radius plus the footprint of the downsample tent chain. Working standard
deviations are quantized to half-pixel steps, and normalized adjacent-tap offsets and weights are precomputed on the
CPU before linear sampling. A quantized working standard deviation at or below 0.5 pixels skips the Gaussian passes.

Intermediate texture precision follows the target format family instead of using a fixed format. RGBA/BGRA 8-bit
targets use storage-writable RGBA8 UNORM intermediates, while supported floating-point targets retain matching
floating-point precision. The filter operates on the numeric values supplied by the texture format and performs no
explicit sRGB/linear transfer-function conversion.

### Rectangle reference
`DrawCommandRectReference::layer` means `DrawCommand::rect` is already in layer coordinates.

`DrawCommandRectReference::element` means the rectangle is resolved from the owning element's layout rectangle. This is the common mode for widget chrome.

Every non-clip command is automatically intersected with its owning element's `LayoutResult::clip_rect` when compiled. Explicit clip commands can further restrict that region for a command sequence.

### Element-relative trailing inset
When `rect_reference` is `element`, `rect_layout_scale` can scale from the element size.

```cpp
command.rect = RectF(4.0f, 4.0f, -8.0f, -8.0f);
command.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
```

This describes an inset rectangle: left/top offset by 4 pixels, width/height equal to element size minus 8 pixels.

### Shape command
`ShapeDesc` references a command range inside a `VG::IShapeBuffer`. Shape coordinates are interpreted as GUI shape coordinates where Y increases downward. If `ShapeDesc::texture` is not null, the shape acts as a texture mask.

### SDF shape and color programs
An SDF draw references two context-owned scalar `float32` program ranges:

1. A shape program contains rectangle, four-corner rounded rectangle, circle, ellipse, capsule, union,
   intersection, difference, and XOR instructions. Boolean operations use prefix order.
2. A color program contains one or more ordered solid, linear, radial, conic, four-corner bilinear, or unified shadow
   instructions. One program accepts at most `SDF_MAX_COLOR_INSTRUCTIONS` (currently 8) instructions. Gradients
   support up to 16 ordered stops, optional interpolation midpoints, and pad or repeat spreading.

Programs use local GUI coordinates. Build the complete scalar sequence, append it through
`append_sdf_shape_program` or `append_sdf_color_program`, and store the returned validated metadata in
`DrawCommand::sdf`. Programs appended during element construction are copied into generated frame storage.
Programs appended by a draw callback are regenerated with that callback. Do not retain their ranges across
frames.

The low eight bits of the first color float select the base algorithm. Bit 8 enables an inner-distance limit and
bit 9 enables an outer-distance limit. Enabled distances immediately follow the encoded opcode. This yields four
generic clip modes:

1. `SDFClipDesc::no_clip()` lets the finite command rectangle provide the raster domain.
2. `SDFClipDesc::inner(distance)` removes samples deeper than `distance` inside the shape.
3. `SDFClipDesc::outer(distance)` removes samples farther than `distance` outside the shape.
4. `SDFClipDesc::inner_outer(inner, outer)` keeps an asymmetric band around the contour.

`SDFClipDesc::fill()` is outer clip at distance zero. `SDFClipDesc::stroke(width)` is a centered band. The same
clip descriptor can be applied to any color opcode. An outer shadow uses inner clip at zero, while an inner shadow
uses outer clip at zero.

One SDF draw evaluates one shape and one contiguous color-program range. Its color instructions are applied in append
order, and each newer result is composited over the accumulated result using premultiplied source-over blending. In a
multi-instruction color program, every non-shadow instruction must have an outer clip; the default
`SDFClipDesc::fill()` satisfies this requirement. Use separate draw commands when effects require different raster
domains or clip state, or when painter order requires other content between them. The same validated shape program can
also be reused by separate shadow, fill, border, or highlight commands when those cases apply.

### Font
Text draw commands reference fonts by `Name`. Register fonts on the context before using them.

Text draw commands do not participate in layout measurement. If an element should use text-driven `fit` sizing,
the higher-level package must install a `LayoutCallbackConfig::measure_callback` that measures the text with its own
text, font and font-size data. See [[GUI Layout]] for the measure callback contract.

```cpp
luexp(context->register_font(Name("default"), Font::get_default_font()));
```

## Programming guide
### Attach drawing to an element
All primitive commands are emitted from a `DrawConfig` callback. Store any command inputs in caller-owned frame data,
attach that data through `DrawConfig::userdata`, and keep it valid until command generation finishes.

```cpp
R<GUI::paint_order_id_t> draw_button(GUI::IContext* context,
    const GUI::ElementHandle& element, GUI::DrawPhase phase,
    GUI::paint_order_id_t paint_order_id, void* userdata)
{
    if(phase == GUI::DrawPhase::before_children)
    {
        GUI::InteractionState interaction = context->get_interaction_state(element.id);
        GUI::DrawCommand background;
        background.type = GUI::DrawCommandType::rounded_rect;
        background.rect_reference = GUI::DrawCommandRectReference::element;
        background.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        background.color = interaction.hovered ? Float4U(0.20f, 0.40f, 0.70f, 1.0f) :
            Float4U(0.12f, 0.16f, 0.22f, 1.0f);
        background.radius = 6.0f;
        context->draw(background, paint_order_id);
    }
    return paint_order_id;
}

GUI::DrawConfig draw_config;
draw_config.name = Name("editor.button");
draw_config.callback = draw_button;
draw_config.phases = GUI::DrawPhaseFlag::before_children;
context->set_draw_config(button, draw_config);
```

If one callback emits overlapping commands that require order, assign increasing IDs and return the last one:

```cpp
context->draw(background, paint_order_id);
context->draw(border, paint_order_id + 1);
return paint_order_id + 1;
```

Commands that are safe to regroup across backend batch keys may use the same ID. Returning an ID greater than every
emitted command reserves the intervening range for the enclosing traversal.

Use both phases when one element needs a background below its children and feedback above them:

```cpp
draw_config.phases = GUI::DrawPhaseFlag::before_children |
    GUI::DrawPhaseFlag::after_children;
```

The remaining primitive examples assume they run inside a draw callback and use its `paint_order_id` parameter.

### Draw text
```cpp
GUI::DrawCommand text;
text.type = GUI::DrawCommandType::text;
text.rect_reference = GUI::DrawCommandRectReference::element;
text.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
text.rect = RectF(8.0f, 0.0f, -16.0f, 0.0f);
text.font = Name("default");
text.font_size = 16.0f;
text.color = Float4U(0.92f, 0.95f, 1.0f, 1.0f);
text.text = "Build";
context->draw(text, paint_order_id);
```

### Draw an image
```cpp
GUI::DrawCommand image;
image.type = GUI::DrawCommandType::image;
image.rect_reference = GUI::DrawCommandRectReference::element;
image.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
image.texture = texture;
image.min_texcoord = Float2U(0.0f, 0.0f);
image.max_texcoord = Float2U(1.0f, 1.0f);
context->draw(image, paint_order_id);
```

Set `nearest_sampler` when the image should show exact texels, such as low-resolution test textures.

### Draw a shape
```cpp
GUI::DrawCommand shape;
shape.type = GUI::DrawCommandType::shape;
shape.rect_reference = GUI::DrawCommandRectReference::element;
shape.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
shape.color = Float4U(1.0f);
shape.shape.buffer = icon_shape_buffer;
shape.shape.first_command = icon_first_command;
shape.shape.num_commands = icon_num_commands;
shape.shape.bounds = icon_bounds;
context->draw(shape, paint_order_id);
```

### Draw a composed SDF gradient
The following command subtracts a smaller circle from a larger one, then paints the ring with a radial gradient.

```cpp
Vector<f32> shape_floats;
GUI::sdf_shape_add_operation(shape_floats, GUI::SDFShapeInstruction::difference_op);
GUI::sdf_shape_add_circle(shape_floats, Float2U(32.0f), 30.0f);
GUI::sdf_shape_add_circle(shape_floats, Float2U(32.0f), 18.0f);
lulet(shape_program, context->append_sdf_shape_program(shape_floats.cspan()));

GUI::SDFGradientStop stops[] = {
    {0.0f, Float4U(1.0f, 0.25f, 0.30f, 1.0f), 0.5f},
    {1.0f, Float4U(0.35f, 0.02f, 0.06f, 1.0f), 0.5f}
};
Vector<f32> color_floats;
GUI::sdf_color_add_radial_gradient(color_floats, Float2U(32.0f), Float2U(30.0f),
    Span<const GUI::SDFGradientStop>(stops, 2));
lulet(color_program, context->append_sdf_color_program(color_floats.cspan()));

GUI::DrawCommand ring;
ring.type = GUI::DrawCommandType::sdf;
ring.rect = RectF(40.0f, 20.0f, 0.0f, 0.0f); // Places the local origin.
ring.sdf.shape = shape_program;
ring.sdf.color = color_program;
context->draw(ring, paint_order_id);
```

### Compose a shadow and fill in one color program
Append the effects in painter order. The fill has an outer clip, so it can share one validated color program and one
draw command with the shadow.

```cpp
Vector<f32> effect_floats;
GUI::sdf_color_add_shadow(effect_floats, Float4U(0.0f, 0.0f, 0.0f, 0.25f),
    Float2U(0.0f, 4.0f), 6.0f, 0.0f, GUI::SDFClipDesc::inner(0.0f));
GUI::sdf_color_add_radial_gradient(effect_floats, Float2U(32.0f), Float2U(30.0f),
    Span<const GUI::SDFGradientStop>(stops, 2), GUI::SDFGradientSpread::pad,
    GUI::SDFClipDesc::fill());
lulet(effect_program, context->append_sdf_color_program(effect_floats.cspan()));

GUI::DrawCommand shadowed_ring = ring;
shadowed_ring.sdf.color = effect_program;
context->draw(shadowed_ring, paint_order_id);
```

### Use clips
```cpp
GUI::DrawCommand push;
push.type = GUI::DrawCommandType::push_clip;
push.rect = clip_rect;
context->draw(push, paint_order_id);

// Draw clipped content.

GUI::DrawCommand pop;
pop.type = GUI::DrawCommandType::pop_clip;
context->draw(pop, paint_order_id);
```

Clip pushes and pops are resolved in lexical submission order before drawable commands are sorted, so they do not
consume separate Paint Order positions. Use explicit clips only when a command sequence needs a region stricter than
the element layout clip. Scroll viewports and layout containers normally receive their base clipping from
`LayoutResult::clip_rect` automatically.

### Draw a backdrop-filtered surface
Attach capture configuration to the surface root, then emit `backdrop_blur` before its tint and children:

```cpp
GUI::BackdropBlurCaptureDesc capture;
capture.softness = 12.0f;
capture.downsample_level = 1;
context->set_backdrop_blur_capture(panel, capture);

GUI::DrawConfig draw_config;
draw_config.name = Name("example.backdrop");
draw_config.callback = [](GUI::IContext* context,
    const GUI::ElementHandle&, GUI::DrawPhase,
    GUI::paint_order_id_t paint_order_id,
    void*) -> R<GUI::paint_order_id_t>
{
    GUI::DrawCommand backdrop;
    backdrop.type = GUI::DrawCommandType::backdrop_blur;
    backdrop.rect_reference = GUI::DrawCommandRectReference::element;
    backdrop.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
    backdrop.radius = 8.0f;
    context->draw(backdrop, paint_order_id);
    return paint_order_id;
};
context->set_draw_config(panel, draw_config);
```

Context inserts the capture marker before the element's `before_children` callback and gives the marker an exclusive
Paint Order ID. The callback receives the following ID, so its backdrop consumer is ordered after the capture. The
same element may own and consume the capture because lookup starts at self. A regular rounded rectangle drawn after
`backdrop_blur` at a greater Paint Order ID supplies the translucent tint; blur capture does not select widget colors.

The `EditorGUI` package wires this mechanism into popups, tooltips and floating dock panels. It exposes
`gui.popup.backdrop_softness`, `gui.tooltip.backdrop_softness` and
`gui.dock_panel.floating.backdrop_softness`, plus a corresponding `backdrop_downsample_level` entry for each surface.
The general editor style keeps softness at zero because some hosts render directly to a non-sampleable swap-chain
texture. A host with a color target that supports `read_texture` can opt in; the EditorGUITest reference theme uses
softness 16 for floating dock panels, with one requested downsample level. Popup and
tooltip backgrounds, plus `gui.dock_panel.floating.background`, provide a 96% surface tint over the filtered result.
EditorGUITest deliberately exaggerates its Popup and Tooltip samples to softness 30 and a 35% surface tint so the neutral
Overlay backdrop makes filtering and transmission immediately visible.
Popup panels place the inset backdrop result after their opaque border underlay and before the translucent surface
tint, so the border remains crisp without covering the filtered interior.

### Render
Create one GUI renderer for the RHI device and reuse it. `render` generates commands when necessary, compiles the
ordered plan, uploads resources, records every target transition, and begins and ends all required render/compute
passes. Call it while the graphics command buffer is recording and outside every pass. It returns outside every pass
and does not submit the command buffer.

Compilation first replays lexical commands to resolve clips and backdrop relationships, then stable-sorts drawable
events by Paint Order ID. Within one equal-order range it groups commands by complete backend batch key in
first-seen-key order while preserving submission order inside each key. Adjacent compatible batches may be
coalesced when doing so preserves Paint Order.

```cpp
Ref<GUI::IRenderer> renderer;
luset(renderer, GUI::new_renderer(device));

GUI::RenderTargetDesc target;
target.color_texture = render_target;
target.color_load_op = RHI::LoadOp::load;
target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
luexp(renderer->render(context, command_buffer, target));
```

The renderer owns upload buffers and descriptor sets referenced by the recorded commands and may replace them on its
next `render` call. Keep the renderer alive and do not reuse that renderer until the previous GPU work has completed.
For multiple frames in flight, use a separate renderer for each frame slot and reuse a slot only after its completion
fence has signaled.

The first internal color pass uses `color_load_op` and `color_clear_value`; every resumed pass uses load/store.
Color storage is always preserved. `depth_stencil_texture` is optional and required only when surface depth testing
is enabled.

Without live captures, the renderer records one color pass. Each live capture may end the current pass, filter the
accumulated target through compute passes, then resume color drawing in a load/store pass. A live capture requires a
single-sample `tex2d` color target with `color_attachment | read_texture` usage. A non-sampleable swapchain target
remains valid when no live capture is present.

`generate_draw_commands` remains useful when tooling needs to inspect the lexical generated stream and assigned Paint
Order IDs. It is idempotent until a tree, layout, layer, draw configuration, backdrop capture attachment, or stored
program mutation invalidates the stream.

## Examples
### Button chrome from a draw callback
```cpp
GUI::DrawConfig button_draw;
button_draw.name = Name("editor.button");
button_draw.callback = draw_button;
button_draw.userdata = &button_visuals;
context->set_draw_config(button, button_draw);
```

This is the kind of rendering logic a higher-level immediate API package owns. The callback is a core traversal
hook rather than a replaceable widget renderer object.
