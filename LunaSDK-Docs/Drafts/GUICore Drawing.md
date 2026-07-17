GUI Core drawing records primitive GUI-level commands and preserves their painter order across two rendering
backends. Analytic geometry, gradients and shadows use the GUICore SDF renderer; text, images and arbitrary vector
paths use VG. Higher-level packages decide which commands represent a widget.

## Designed functionality
The drawing system provides a narrow bridge between GUI data and rendering:

1. Record static draw commands and attach optional delayed draw callbacks to elements.
2. Support element-relative and layer-coordinate rectangles.
3. Draw analytic SDF shapes, CSS-like gradients, strokes and shadows alongside text, images and VG shape ranges.
4. Generate commands after layout and input routing while preserving element painter order and layer Z order.
5. Keep rendering policy outside GUI Core widgets, because GUI Core has no widgets.

GUI Core does not begin RHI render passes or present swapchains. The application owns RHI command buffers and render targets.

## Concepts
### Draw command
`GUICore::DrawCommand` describes one primitive command. Its `type` selects the payload fields used by the command.

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

`rect`, `gradient_rect`, `rounded_rect` and `shadow` are convenient commands for common GUI chrome. The renderer
translates them to SDF shape and color programs before submission. Use `sdf` directly for circles, ellipses,
capsules, four-corner rounded rectangles, strokes, composed shapes, and multi-stop gradients.

### Draw callback
`DrawConfig` attaches an optional callback to one element. Generation can invoke the callback in
`DrawPhase::before_children`, `DrawPhase::after_children`, or both phases.

The callback runs after layout and input routing, so it can read final layout rectangles, interaction state,
state objects and style values. It emits commands by calling `IContext::draw`. It must not mutate the element
tree, layout, interaction state, or application data.

`DrawConfig::userdata` is owned by the caller and must remain valid until command generation finishes. GUI Core
stores draw configurations sparsely, so elements without callbacks do not carry callback pointers.

### Painter order
For each layer, GUI Core replays element construction as a painter-order operation stream:

1. Invoke the element's `before_children` callback.
2. Replay static commands recorded at that position.
3. Generate child elements in construction order.
4. Invoke the element's `after_children` callback.

This lets a container draw its background below children and its border or feedback above children without
requiring the final layout rectangle during element construction.

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
2. A color program contains one solid, linear, radial, conic, four-corner bilinear, or unified shadow instruction.
   Gradients support up to 16 ordered stops, optional interpolation midpoints, and pad or repeat spreading.

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

Build one color program per ordered effect pass. The same validated shape program can be reused by shadow, fill,
border, and highlight commands. Consecutive commands that reference the same buffer pages remain separate logical
passes but are submitted as one instanced draw call.

### Font
Text draw commands reference fonts by `Name`. Register fonts on the context before using them.

Text draw commands do not participate in layout measurement. If an element should use text-driven `fit` sizing,
the higher-level package must install a `LayoutCallbackConfig::measure_callback` that measures the text with its own
text, font and font-size data. See [[GUICore Layout]] for the measure callback contract.

```cpp
luexp(context->register_font(Name("default"), Font::get_default_font()));
```

## Programming guide
### Record a static command during element build
Outside draw generation, `draw` records a command at the current layer, element, and painter-order position.

```cpp
GUICore::DrawCommand bg;
bg.type = GUICore::DrawCommandType::rounded_rect;
bg.rect_reference = GUICore::DrawCommandRectReference::element;
bg.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
bg.color = Float4U(0.12f, 0.16f, 0.22f, 1.0f);
bg.radius = 6.0f;
context->draw(bg);
```

### Record a command for a specific element
Use `draw_for_element` when a command is emitted after the element build scope has ended.

```cpp
context->draw_for_element(element, bg);
```

Invalid handles are ignored.

### Attach delayed drawing
Use a draw callback when rendering depends on final layout, input, state, or style data.

```cpp
RV draw_button(GUICore::IContext* context, const GUICore::ElementHandle& element,
    GUICore::DrawPhase phase, void* userdata)
{
    if(phase == GUICore::DrawPhase::before_children)
    {
        GUICore::InteractionState interaction = context->get_interaction_state(element.id);
        GUICore::DrawCommand background;
        background.type = GUICore::DrawCommandType::rounded_rect;
        background.rect_reference = GUICore::DrawCommandRectReference::element;
        background.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        background.color = interaction.hovered ? Float4U(0.20f, 0.40f, 0.70f, 1.0f) :
            Float4U(0.12f, 0.16f, 0.22f, 1.0f);
        background.radius = 6.0f;
        context->draw(background);
    }
    return ok;
}

GUICore::DrawConfig draw_config;
draw_config.name = Name("editor.button");
draw_config.callback = draw_button;
draw_config.phases = GUICore::DrawPhaseFlag::before_children;
context->set_draw_config(button, draw_config);
```

Use both phases when one element needs a background below its children and feedback above them:

```cpp
draw_config.phases = GUICore::DrawPhaseFlag::before_children |
    GUICore::DrawPhaseFlag::after_children;
```

### Draw text
```cpp
GUICore::DrawCommand text;
text.type = GUICore::DrawCommandType::text;
text.rect_reference = GUICore::DrawCommandRectReference::element;
text.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
text.rect = RectF(8.0f, 0.0f, -16.0f, 0.0f);
text.font = Name("default");
text.font_size = 16.0f;
text.color = Float4U(0.92f, 0.95f, 1.0f, 1.0f);
text.text = "Build";
context->draw(text);
```

### Draw an image
```cpp
GUICore::DrawCommand image;
image.type = GUICore::DrawCommandType::image;
image.rect_reference = GUICore::DrawCommandRectReference::element;
image.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
image.texture = texture;
image.min_texcoord = Float2U(0.0f, 0.0f);
image.max_texcoord = Float2U(1.0f, 1.0f);
context->draw(image);
```

Set `nearest_sampler` when the image should show exact texels, such as low-resolution test textures.

### Draw a shape
```cpp
GUICore::DrawCommand shape;
shape.type = GUICore::DrawCommandType::shape;
shape.rect_reference = GUICore::DrawCommandRectReference::element;
shape.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
shape.color = Float4U(1.0f);
shape.shape.buffer = icon_shape_buffer;
shape.shape.first_command = icon_first_command;
shape.shape.num_commands = icon_num_commands;
shape.shape.bounds = icon_bounds;
context->draw(shape);
```

### Draw a composed SDF gradient
The following command subtracts a smaller circle from a larger one, then paints the ring with a radial gradient.

```cpp
Vector<f32> shape_floats;
GUICore::sdf_shape_add_operation(shape_floats, GUICore::SDFShapeInstruction::difference_op);
GUICore::sdf_shape_add_circle(shape_floats, Float2U(32.0f), 30.0f);
GUICore::sdf_shape_add_circle(shape_floats, Float2U(32.0f), 18.0f);
lulet(shape_program, context->append_sdf_shape_program(shape_floats.cspan()));

GUICore::SDFGradientStop stops[] = {
    {0.0f, Float4U(1.0f, 0.25f, 0.30f, 1.0f), 0.5f},
    {1.0f, Float4U(0.35f, 0.02f, 0.06f, 1.0f), 0.5f}
};
Vector<f32> color_floats;
GUICore::sdf_color_add_radial_gradient(color_floats, Float2U(32.0f), Float2U(30.0f),
    Span<const GUICore::SDFGradientStop>(stops, 2));
lulet(color_program, context->append_sdf_color_program(color_floats.cspan()));

GUICore::DrawCommand ring;
ring.type = GUICore::DrawCommandType::sdf;
ring.rect = RectF(40.0f, 20.0f, 0.0f, 0.0f); // Places the local origin.
ring.sdf.shape = shape_program;
ring.sdf.color = color_program;
context->draw(ring);
```

### Reuse one shape for a shadow and fill
The shadow and fill are two color programs and two ordered commands. Both commands reference the same shape range.

```cpp
Vector<f32> shadow_floats;
GUICore::sdf_color_add_shadow(shadow_floats, Float4U(0.0f, 0.0f, 0.0f, 0.25f),
    Float2U(0.0f, 4.0f), 6.0f, 0.0f, GUICore::SDFClipDesc::inner(0.0f));
lulet(shadow_program, context->append_sdf_color_program(shadow_floats.cspan()));

GUICore::DrawCommand shadow = ring;
shadow.sdf.color = shadow_program;
context->draw(shadow);
context->draw(ring);
```

### Use clips
```cpp
GUICore::DrawCommand push;
push.type = GUICore::DrawCommandType::push_clip;
push.rect = clip_rect;
context->draw(push);

// Draw clipped content.

GUICore::DrawCommand pop;
pop.type = GUICore::DrawCommandType::pop_clip;
context->draw(pop);
```

Use explicit clips only when a command sequence needs a region stricter than the element layout clip. Scroll viewports and layout containers normally receive their base clipping from `LayoutResult::clip_rect` automatically.

### Prepare and render
Create one GUICore renderer for the RHI device and reuse it. `prepare` generates commands if necessary, validates
and uploads SDF programs, and prepares both SDF and VG batches. Call it outside a render pass. The application
retains ownership of target transitions and render-pass boundaries.

```cpp
Ref<GUICore::IRenderer> renderer;
luset(renderer, GUICore::new_renderer(device));

luexp(renderer->prepare(context, command_buffer, render_target));
RHI::RenderPassDesc render_pass;
render_pass.color_attachments[0] = RHI::ColorAttachment(
    render_target, RHI::LoadOp::load, RHI::StoreOp::store);
command_buffer->begin_render_pass(render_pass);
renderer->render(command_buffer);
command_buffer->end_render_pass();
```

`generate_draw_commands` remains useful when tooling needs to inspect the final ordered stream. It is idempotent
until a tree, layout, layer, draw configuration, or stored program mutation invalidates the stream.

## Examples
### Button chrome from delayed primitive commands
```cpp
GUICore::DrawConfig button_draw;
button_draw.name = Name("editor.button");
button_draw.callback = draw_button;
button_draw.userdata = &button_visuals;
context->set_draw_config(button, button_draw);
```

This is the kind of rendering logic a higher-level immediate API package owns. The callback is a core traversal
hook rather than a replaceable widget renderer object.
