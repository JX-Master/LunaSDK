GUI Core drawing records primitive GUI-level commands and compiles them to VG. Higher-level packages decide which commands represent a widget.

## Designed functionality
The drawing system provides a narrow bridge between GUI data and VG rendering:

1. Record static draw commands and attach optional delayed draw callbacks to elements.
2. Support element-relative and layer-coordinate rectangles.
3. Draw rectangles, gradients, rounded rectangles, lines, text, images and VG shape ranges.
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
8. `push_clip`
9. `pop_clip`

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

### Compile to VG
```cpp
luexp(context->generate_draw_commands());
luexp(context->compile_draw_commands(shape_draw_list));
luexp(shape_draw_list->compile());
```

Explicit generation is useful when tooling needs to inspect the final command stream. Compilation automatically
generates commands when the caller has not done so. The destination draw list is reset before GUI Core emits
commands, and commands are emitted in layer Z order.

### Render the VG draw list
The host renders the compiled VG draw calls with its RHI target.

```cpp
Span<const VG::ShapeDrawCall> draw_calls = shape_draw_list->get_draw_calls();
if(!draw_calls.empty())
{
    Float4x4U transform = ProjectionMatrix::make_orthographic_off_center(
        0.0f, frame.screen_size.x,
        0.0f, frame.screen_size.y,
        0.0f, 1.0f);

    luexp(renderer->begin(render_target));
    renderer->draw(shape_draw_list->get_vertex_buffer(),
        shape_draw_list->get_index_buffer(), draw_calls, &transform);
    luexp(renderer->end());
    renderer->submit(command_buffer);
}
```

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
