GUI Core drawing records primitive GUI-level commands and compiles them to VG. Higher-level packages decide which commands represent a widget.

## Designed functionality
The drawing system provides a narrow bridge between GUI data and VG rendering:

1. Store draw commands on elements and layers.
2. Support element-relative and layer-coordinate rectangles.
3. Draw rectangles, gradients, rounded rectangles, lines, text, images and VG shape ranges.
4. Preserve layer Z order during compilation.
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

### Rectangle reference
`DrawCommandRectReference::layer` means `DrawCommand::rect` is already in layer coordinates.

`DrawCommandRectReference::element` means the rectangle is resolved from the owning element's layout rectangle. This is the common mode for widget chrome.

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

```cpp
luexp(context->register_font(Name("default"), Font::get_default_font()));
```

## Programming guide
### Record a command during element build
`draw` appends to the current layer and current element.

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

High-level scroll views and table views typically emit clip commands around content.

### Compile to VG
```cpp
luexp(context->compile_draw_commands(shape_draw_list));
luexp(shape_draw_list->compile());
```

The destination draw list is reset before GUI Core emits commands. Commands are emitted in layer Z order.

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
### Button chrome from primitive commands
```cpp
GUICore::InteractionState state = context->get_interaction_state(button.id);

GUICore::DrawCommand bg;
bg.type = GUICore::DrawCommandType::rounded_rect;
bg.rect_reference = GUICore::DrawCommandRectReference::element;
bg.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
bg.radius = 5.0f;
bg.color = state.hovered ? hovered_color : normal_color;
context->draw_for_element(button, bg);

GUICore::DrawCommand label;
label.type = GUICore::DrawCommandType::text;
label.rect_reference = GUICore::DrawCommandRectReference::element;
label.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
label.font = Name("default");
label.font_size = 16.0f;
label.color = text_color;
label.text = "Apply";
context->draw_for_element(button, label);
```

This is the kind of rendering logic a higher-level immediate API package owns.
