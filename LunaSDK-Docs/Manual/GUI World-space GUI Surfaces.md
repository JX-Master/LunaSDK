# GUI world-space GUI surfaces

## Functionality
GUI can render one logical two-dimensional context directly onto an arbitrary plane in clip space. The same
surface transform is applied to the SDF backend and the integrated VG backend, so analytic controls, text, images and
icons remain aligned under perspective projection.

The feature is intended for in-game displays, editor viewport overlays and other interfaces that should retain
analytic resolution without first rendering the complete GUI to an intermediate texture.

Screen-space GUI remains the default. Existing callers that do not provide a custom surface transform continue to
use an orthographic projection derived from `FrameDesc::logical_size`.

## Concepts
### Logical surface
A logical surface is the two-dimensional coordinate space owned by one GUI context. Layout, input, layers,
clips and draw commands use this coordinate space. The top-left origin and downward-positive Y convention do not
change in world-space mode.

### Surface-to-clip transform
The host composes the surface model, camera view and projection operations into one `surface_to_clip` matrix. GUI
does not own a camera or scene transform. The matrix is applied once per vertex by both rendering backends.

The input of this matrix always follows GUI's top-left origin and downward-positive Y convention. Hosts must not
add an extra clip-space Y flip for SDF. GUI internally normalizes VG's historical bottom-left draw coordinates
before applying the matrix.

### Perspective-correct local evaluation
SDF programs are still evaluated in logical surface coordinates. The SDF vertex shader sends the original surface
position to the pixel shader through perspective-correct interpolation. Pixel derivatives of signed distance provide
the local footprint of a framebuffer pixel and retain antialiased edges under projection.

### Depth mode
Depth testing is optional. The render target description may provide a depth-stencil texture; GUI derives its
format and owns the compatible pass. A typical world panel enables `less_equal` depth testing and disables depth
writes.

### Host input mapping
The host chooses the world surface hit by a pointer ray and maps the hit into surface-local coordinates. It then feeds
ordinary `InputEvent` records to that surface's context. One context per independently placed surface is recommended.

## Programming guide
### 1. Build the context in logical coordinates
Begin the frame exactly as for a window GUI. `logical_size` describes logical panel dimensions rather than physical
world units.

```cpp
GUI::FrameDesc frame;
frame.logical_size = Float2U(1024.0f, 576.0f);
frame.render_size = scene_render_size;
frame.delta_time = delta_time;
context->begin_frame(frame);
```

Build, lay out and route the UI using the normal GUI and EditorGUI APIs.

### 2. Compose the surface transform
Create a transform that maps local points `(x, y, 0, 1)` to clip space. Include any scale needed to convert logical
units into world units, followed by the surface pose, camera view and projection conventions used by the host.

```cpp
GUI::RenderSurfaceDesc surface;
surface.use_custom_transform = true;
surface.surface_to_clip = surface_to_clip;
```

### 3. Render

```cpp
lutry
{
    GUI::RenderTargetDesc target;
    target.color_texture = scene_color;
    target.color_load_op = RHI::LoadOp::load;
    target.color_final_state = RHI::TextureStateFlag::color_attachment_write;
    luexp(renderer->render(context, command_buffer, target, surface));
}
lucatchret;
```

The command buffer must be outside every pass before and after `render`. GUI records attachment barriers and all
required graphics/compute passes; the host still owns command-buffer submission and scene synchronization.

### 4. Enable scene depth when needed

```cpp
surface.depth_test_enable = true;
surface.depth_write_enable = false;
surface.depth_compare_function = RHI::CompareFunction::less_equal;

GUI::RenderTargetDesc target;
target.color_texture = scene_color;
target.depth_stencil_texture = scene_depth;
target.depth_load_op = RHI::LoadOp::load;
target.depth_store_op = RHI::StoreOp::store;
```

The current GUI renderer supports only single-sample render targets: the color texture and optional depth texture must
both have `sample_count` equal to `1`, and their extents must match. If backdrop capture interrupts a depth-enabled
pass, GUI preserves depth/stencil contents and loads them again before continuing.

### 5. Map pointer rays

```cpp
GUI::SurfaceRayHit hit;
if(GUI::ray_to_surface(
    ray_origin, ray_direction, world_to_surface, hit))
{
    GUI::InputEvent event;
    event.type = GUI::InputEventType::pointer_move;
    event.position = hit.position;
    context->add_input_event(event);
}
```

The helper returns positions outside the logical panel. The host may reject them for initial hover acquisition but
should continue delivering them while GUI owns pointer capture, so sliders and drag operations remain continuous.

### 6. Text input and IME
`TextInputState::rect` remains surface-local. A world-space host should project the rectangle corners to its window
when positioning a platform IME candidate window. Clipboard callbacks are unchanged.

## Limitations
The initial feature treats one context as one plane. Per-element three-dimensional transforms, curved surfaces and
several independently transformed layers in one context are outside the first implementation. A host may still use
several contexts and render each with a different surface transform.

Painter-ordered backdrop capture is initially screen-space only. A command stream containing a live capture returns
`not_supported` when `use_custom_transform` is enabled because a perspective-projected logical rectangle does not
have one uniform screen-space blur radius or a simple axis-aligned capture region.
