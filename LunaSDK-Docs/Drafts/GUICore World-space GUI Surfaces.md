# GUICore world-space GUI surfaces

## Functionality
GUICore can render one logical two-dimensional context directly onto an arbitrary plane in clip space. The same
surface transform is applied to the SDF backend and the integrated VG backend, so analytic controls, text, images and
icons remain aligned under perspective projection.

The feature is intended for in-game displays, editor viewport overlays and other interfaces that should retain
analytic resolution without first rendering the complete GUI to an intermediate texture.

Screen-space GUI remains the default. Existing callers that do not provide a custom surface transform continue to
use an orthographic projection derived from `FrameDesc::screen_size`.

## Concepts
### Logical surface
A logical surface is the two-dimensional coordinate space owned by one GUICore context. Layout, input, layers,
clips and draw commands use this coordinate space. The top-left origin and downward-positive Y convention do not
change in world-space mode.

### Surface-to-clip transform
The host composes the surface model, camera view and projection operations into one `surface_to_clip` matrix. GUICore
does not own a camera or scene transform. The matrix is applied once per vertex by both rendering backends.

The input of this matrix always follows GUICore's top-left origin and downward-positive Y convention. Hosts must not
add an extra clip-space Y flip for SDF. GUICore internally normalizes VG's historical bottom-left draw coordinates
before applying the matrix.

### Perspective-correct local evaluation
SDF programs are still evaluated in logical surface coordinates. The SDF vertex shader sends the original surface
position to the pixel shader through perspective-correct interpolation. Pixel derivatives of signed distance provide
the local footprint of a framebuffer pixel and retain antialiased edges under projection.

### Depth mode
Depth testing is optional. If the caller declares a depth-stencil format, the active render pass must use a compatible
attachment. A typical world panel enables `less_equal` depth testing and disables depth writes.

### Host input mapping
The host chooses the world surface hit by a pointer ray and maps the hit into surface-local coordinates. It then feeds
ordinary `InputEvent` records to that surface's context. One context per independently placed surface is recommended.

## Programming guide
### 1. Build the context in logical coordinates
Begin the frame exactly as for a window GUI. `screen_size` describes logical panel dimensions rather than physical
world units.

```cpp
GUICore::FrameDesc frame;
frame.screen_size = Float2U(1024.0f, 576.0f);
frame.framebuffer_size = scene_framebuffer_size;
frame.dpi_scale = 1.0f;
frame.delta_time = delta_time;
context->begin_frame(frame);
```

Build, lay out and route the UI using the normal GUICore and GUI APIs.

### 2. Compose the surface transform
Create a transform that maps local points `(x, y, 0, 1)` to clip space. Include any scale needed to convert logical
units into world units, followed by the surface pose, camera view and projection conventions used by the host.

```cpp
GUICore::RenderSurfaceDesc surface;
surface.use_custom_transform = true;
surface.surface_to_clip = surface_to_clip;
```

### 3. Prepare and render

```cpp
lutry
{
    luexp(renderer->prepare(context, command_buffer, scene_color, surface));
    command_buffer->begin_render_pass(scene_pass);
    renderer->render(command_buffer);
    command_buffer->end_render_pass();
}
lucatchret;
```

The original three-argument `prepare` helper remains available for screen-space GUI.

### 4. Enable scene depth when needed

```cpp
surface.depth_stencil_format = scene_depth->get_desc().format;
surface.depth_test_enable = true;
surface.depth_write_enable = false;
surface.depth_compare_function = RHI::CompareFunction::less_equal;
```

The color and depth attachments in the active render pass must match the formats supplied during `prepare`.

### 5. Map pointer rays

```cpp
GUICore::SurfaceRayHit hit;
if(GUICore::ray_to_surface(
    ray_origin, ray_direction, world_to_surface, hit))
{
    GUICore::InputEvent event;
    event.type = GUICore::InputEventType::pointer_move;
    event.position = hit.position;
    context->add_input_event(event);
}
```

The helper returns positions outside the logical panel. The host may reject them for initial hover acquisition but
should continue delivering them while GUICore owns pointer capture, so sliders and drag operations remain continuous.

### 6. Text input and IME
`TextInputState::rect` remains surface-local. A world-space host should project the rectangle corners to its window
when positioning a platform IME candidate window. Clipboard callbacks are unchanged.

## Limitations
The initial feature treats one context as one plane. Per-element three-dimensional transforms, curved surfaces and
several independently transformed layers in one context are outside the first implementation. A host may still use
several contexts and render each with a different surface transform.
