## Designed functionality
GUI solves the part of GUI work that is common to multiple GUI packages:

1. Store the submitted frame as a typeless element tree.
2. Route host-independent input through layers and interactable elements.
3. Store reusable state objects and named styles.
4. Run primitive layout algorithms on element data.
5. Generate GUI-level draw commands after layout and input, then render them through the integrated SDF and VG backends.
6. Expose performance counters and direct read-only frame inspection.

GUI does not provide high-level widgets. Controls such as buttons, checkboxes, menus, sliders, color editors, dock panels and inspectors belong to a higher-level package such as `Luna::EditorGUI`. Those packages build GUI elements, attach input data, run layout helpers and install package-owned draw callbacks or static draw commands.

This split keeps GUI small and orthogonal. Applications can use only the systems they need, and different immediate API packages can coexist because they submit into the same `GUI::IContext`.

## Concepts
### Context
`GUI::IContext` owns the per-frame element tree, layer list, sparse callback records, queued input events, routed interaction state, state store, style records and generated draw commands.

A context is explicit. There is no global current GUI context. Pass the `IContext*` to the higher-level API or directly to GUI functions.

### Frame
Each frame starts with `IContext::begin_frame`. The host provides a `GUI::FrameDesc` with logical screen size, framebuffer size, DPI scale and delta time.

Logical screen coordinates use a top-left origin, X to the right and Y downward. They are the coordinates used by input, layers and layout. The renderer later maps the ordered SDF and VG command batches to the supplied render target.

### Element tree
The element tree is typeless. Every node is a `GUI::Element` record with the same storage shape. Behavior is defined by data attached to the element:

1. `LayoutConfig`
2. `LayoutResult`
3. `Interactable`
4. Style binding
5. Sparse optional layout, navigation, custom hit-test and draw callback bindings
6. Draw-command ownership metadata
7. Human-readable debug names

Elements do not inherit from widget classes and do not have virtual behavior. Algorithms operate on this data.

### Layer
A layer owns one root element tree and an ordered set of generated draw command indexes. Layers are stored from bottom to top. Rendering uses painter's algorithm, while input routing checks top layers first.

Normal content, popups, tooltips, modal panels, drag previews and debug overlays should be represented as layers instead of special widget branches.

### Immediate API package
An immediate API package is a higher-level library that implements concrete controls by calling GUI APIs. The bundled `Luna::EditorGUI` package is tailored for data-editing applications such as DCC software, editors, office software, and enterprise administration software.

Such packages own visual policy and widget behavior. GUI owns shared storage, routing, layout, draw command compilation, style storage and direct read-only inspection.

## Programming guide
### Initialize modules
Applications normally initialize GUI together with Runtime, RHI, VG and Font. A window-backed application also initializes `Window`, `GUIWindow` and the chosen high-level GUI package.

```cpp
luexp(add_modules({
    module_rhi(),
    module_font(),
    module_vg(),
    GUI::module_gui(),
    EditorGUI::module_editor_gui()
}));
luexp(init_modules());
```

### Create a context
Create a context with `GUI::new_context`. Register fonts and any style schemas required by higher-level packages.

```cpp
Ref<GUI::IContext> context = GUI::new_context();
luexp(context->register_font(Name("default"), Font::get_default_font()));
EditorGUI::register_style_schemas(context);

Ref<GUI::IRenderer> renderer;
luset(renderer, GUI::new_renderer());
```

### Build one frame
The usual frame order is:

1. Begin the frame.
2. Feed input events.
3. Push one or more layers.
4. Build elements and attach layout, interaction and draw data.
5. Run layout.
6. Route input.
7. Resolve higher-level package interactions and update bound application values.
8. Reapply layout when the package reports that interaction changed layout-dependent state.
9. Generate draw commands explicitly only when tooling needs to inspect the final stream.
10. Ask the GUI renderer to compile and record the complete render plan.

```cpp
GUI::FrameDesc frame;
frame.screen_size = Float2U((f32)window_width, (f32)window_height);
frame.framebuffer_size = UInt2U(framebuffer_width, framebuffer_height);
frame.dpi_scale = dpi_scale;
frame.delta_time = delta_time;

context->begin_frame(frame);
context->add_input_events(input_events);

context->push_layer(1, Float2U(0.0f));
GUI::ElementHandle root = context->begin_element(1);

// Higher-level GUI package calls, or direct GUI element construction.

context->end_element();
context->pop_layer();

luexp(context->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
context->route_input();
EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(context);
if(resolved.relayout_requested)
{
    luexp(context->apply_layout(root,
        RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
}

GUI::RenderTargetDesc target;
target.color_texture = render_target;
target.color_load_op = RHI::LoadOp::load;
target.color_final_state = RHI::TextureStateFlag::present;
luexp(renderer->render(context, command_buffer, target));
```

`IRenderer::render` calls `generate_draw_commands` when the command stream is stale. Call
`IContext::generate_draw_commands` directly only when inspection or other tooling needs the final ordered commands
before rendering. Widget-bound values and package frame data must remain valid until interaction resolution and draw
command generation have both finished.

High-level packages attach dense `LayoutConfig` input and optional sparse `LayoutCallbackConfig` records while they build elements. `IContext::apply_layout` owns the top-down tree traversal and invokes those callbacks. GUI exposes the primitive helpers used by those callbacks in [[GUI Layout]].

### Render
GUI records static commands and delayed element draw callbacks. `IContext::generate_draw_commands` evaluates
callbacks against final layout and interaction data. `IRenderer::render` then compiles the ordered stream, interleaves
the SDF and VG backends, performs required resource transitions, and begins and ends every required graphics or
compute pass.

The application owns the command buffer, final color and optional depth-stencil textures, submission,
synchronization and presentation. Call `IRenderer::render` while the command buffer is recording and outside every
pass. See [[GUI Drawing]] for details.

## Examples
### A minimal raw element
```cpp
context->push_layer(1, Float2U(0.0f));

GUI::ElementHandle root = context->begin_element(1);
context->set_layout_config(root, GUI::LayoutConfig {});

GUI::ElementHandle item = context->begin_element(context->make_id("hello"));
GUI::LayoutConfig layout;
layout.width.kind = GUI::SizeKind::fixed;
layout.width.value = 220.0f;
layout.height.kind = GUI::SizeKind::fixed;
layout.height.value = 36.0f;
context->set_layout_config(item, layout);

GUI::DrawCommand text;
text.type = GUI::DrawCommandType::text;
text.rect_reference = GUI::DrawCommandRectReference::element;
text.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
text.text = "Hello GUI";
text.font = Name("default");
text.font_size = 16.0f;
text.color = Float4U(1.0f);
context->draw(text);

context->end_element();
context->end_element();
context->pop_layer();
```

### Use a high-level package
Most applications should use `Luna::EditorGUI` or another immediate API package for normal controls.

```cpp
GUI::ElementHandle row = EditorGUI::begin_h_layout(
    context, context->make_id("toolbar"), "Toolbar", toolbar_layout);

EditorGUI::text_button(context, context->make_id("build"), "Build", button_layout);
EditorGUI::text_button(context, context->make_id("run"), "Run", button_layout);

GUI::FlexLayoutDesc desc;
desc.axis = GUI::LayoutAxis::x;
desc.main_axis_gap = 8.0f;
luexp(EditorGUI::end_h_layout(context, row, desc));
```

The high-level package builds typeless GUI elements behind these calls.
