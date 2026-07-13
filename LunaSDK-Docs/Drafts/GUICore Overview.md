GUI Core is the low-level, data-oriented foundation used by LunaSDK GUI packages. It sits between higher-level immediate GUI APIs and the vector graphics backend.

Related pages:

1. [[GUICore Elements and Layers]]
2. [[GUICore Layout]]
3. [[GUICore Input and Interaction]]
4. [[GUICore Drawing]]
5. [[GUICore State and Style]]
6. [[GUICore Performance and Inspection]]

## Designed functionality
GUI Core solves the part of GUI work that is common to multiple GUI packages:

1. Store the submitted frame as a typeless element tree.
2. Route host-independent input through layers and interactable elements.
3. Store reusable state objects and named styles.
4. Run primitive layout algorithms on element data.
5. Generate GUI-level draw commands after layout and input, then compile them to VG draw lists.
6. Expose performance counters and direct read-only frame inspection.

GUI Core does not provide high-level widgets. Controls such as buttons, checkboxes, menus, sliders, color editors, dock panels and inspectors belong to a higher-level package such as `Luna::GUI`. Those packages build GUI Core elements, attach input data, run layout helpers and install package-owned draw callbacks or static draw commands.

This split keeps GUI Core small and orthogonal. Applications can use only the systems they need, and different immediate API packages can coexist because they submit into the same `GUICore::IContext`.

## Concepts
### Context
`GUICore::IContext` owns the per-frame element tree, layer list, sparse callback records, queued input events, routed interaction state, state store, style records and generated draw commands.

A context is explicit. There is no global current GUI context. Pass the `IContext*` to the higher-level API or directly to GUI Core functions.

### Frame
Each frame starts with `IContext::begin_frame`. The host provides a `GUICore::FrameDesc` with logical screen size, framebuffer size, DPI scale and delta time.

Logical screen coordinates use a top-left origin, X to the right and Y downward. They are the coordinates used by input, layers and layout. The renderer later maps the final VG draw list to the target render pass.

### Element tree
The element tree is typeless. Every node is a `GUICore::Element` record with the same storage shape. Behavior is defined by data attached to the element:

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
An immediate API package is a higher-level library that implements concrete controls by calling GUI Core APIs. The default `Luna::GUI` package is the editor-style package used by tools and Studio.

Such packages own visual policy and widget behavior. GUI Core owns shared storage, routing, layout, draw command compilation, style storage and direct read-only inspection.

## Programming guide
### Initialize modules
Applications normally initialize GUI Core together with Runtime, RHI, VG and Font. A window-backed application also initializes `Window`, `GUIWindow` and the chosen high-level GUI package.

```cpp
luexp(add_modules({
    module_rhi(),
    module_font(),
    module_vg(),
    GUICore::module_gui_core(),
    GUI::module_gui()
}));
luexp(init_modules());
```

### Create a context
Create a context with `GUICore::new_context`. Register fonts and any style schemas required by higher-level packages.

```cpp
Ref<GUICore::IContext> context = GUICore::new_context();
luexp(context->register_font(Name("default"), Font::get_default_font()));
GUI::register_editor_style_schemas(context);
```

### Build one frame
The usual frame order is:

1. Begin the frame.
2. Feed input events.
3. Push one or more layers.
4. Build elements and attach layout, interaction and draw data.
5. Run layout.
6. Route input.
7. Resolve higher-level package state and generate draw commands.
8. Compile draw commands to VG.
9. Render the VG draw list.

```cpp
GUICore::FrameDesc frame;
frame.screen_size = Float2U((f32)window_width, (f32)window_height);
frame.framebuffer_size = UInt2U(framebuffer_width, framebuffer_height);
frame.dpi_scale = dpi_scale;
frame.delta_time = delta_time;

context->begin_frame(frame);
context->add_input_events(input_events);

context->push_layer(1, Float2U(0.0f));
GUICore::ElementHandle root = context->begin_element(1);

// Higher-level GUI package calls, or direct GUI Core element construction.

context->end_element();
context->pop_layer();

luexp(context->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
context->route_input();
luexp(context->generate_draw_commands());
luexp(context->compile_draw_commands(vg_draw_list));
```

High-level packages attach dense `LayoutConfig` input and optional sparse `LayoutCallbackConfig` records while they build elements. `IContext::apply_layout` owns the top-down tree traversal and invokes those callbacks. GUI Core exposes the primitive helpers used by those callbacks in [[GUICore Layout]].

### Render
GUI Core records static commands and delayed element draw callbacks. `IContext::generate_draw_commands` evaluates callbacks against final layout and interaction data, and `IContext::compile_draw_commands` translates the resulting command stream into a `VG::IShapeDrawList`.

The application still owns the RHI render pass, command buffer, swapchain or render target. See [[GUICore Drawing]] for details.

## Examples
### A minimal raw element
```cpp
context->push_layer(1, Float2U(0.0f));

GUICore::ElementHandle root = context->begin_element(1);
context->set_layout_config(root, GUICore::LayoutConfig {});

GUICore::ElementHandle item = context->begin_element(context->make_id("hello"));
GUICore::LayoutConfig layout;
layout.width.kind = GUICore::SizeKind::fixed;
layout.width.value = 220.0f;
layout.height.kind = GUICore::SizeKind::fixed;
layout.height.value = 36.0f;
context->set_layout_config(item, layout);

GUICore::DrawCommand text;
text.type = GUICore::DrawCommandType::text;
text.rect_reference = GUICore::DrawCommandRectReference::element;
text.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
text.text = "Hello GUI Core";
text.font = Name("default");
text.font_size = 16.0f;
text.color = Float4U(1.0f);
context->draw(text);

context->end_element();
context->end_element();
context->pop_layer();
```

### Use a high-level package
Most applications should use `Luna::GUI` or another immediate API package for normal controls.

```cpp
GUICore::ElementHandle row = GUI::begin_h_layout(
    context, context->make_id("toolbar"), "Toolbar", toolbar_layout);

GUI::text_button(context, context->make_id("build"), "Build", button_layout);
GUI::text_button(context, context->make_id("run"), "Run", button_layout);

GUICore::FlexLayoutDesc desc;
desc.axis = GUICore::LayoutAxis::x;
desc.main_axis_gap = 8.0f;
luexp(GUI::end_h_layout(context, row, desc));
```

The high-level package builds typeless GUI Core elements behind these calls.
