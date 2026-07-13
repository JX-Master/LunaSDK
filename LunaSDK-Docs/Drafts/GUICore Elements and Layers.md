Elements and layers are the structural data model of GUI Core. They describe what exists in the submitted frame and where each subtree belongs in Z order.

## Designed functionality
The element and layer system provides a widget-free representation of one GUI frame:

1. Elements store typeless records for submitted GUI primitives.
2. Layers separate independent GUI surfaces such as normal content, popups and overlays.
3. Stable IDs connect the current frame to state, interaction data and debug output.
4. Dense indexes provide efficient traversal during layout, input routing and draw command compilation.

The element tree is the core-level ground truth for the current frame. If an element is not submitted in a frame, it is not part of that frame's GUI.

## Concepts
### Stable ID
`GUICore::id_t` is used for elements, layers and state IDs. Stable IDs should remain consistent across frames for the same logical item.

Use `IContext::make_id` or `GUICore::make_scoped_id` to build IDs from local numbers or strings. Use data scopes to isolate repeated structures.

```cpp
context->push_data_scope(context->make_id("asset-list"));
GUICore::id_t item_id = context->make_id(asset_guid_string);
context->pop_data_scope();
```

### Element
`GUICore::Element` stores:

1. Stable ID.
2. Layer and topology indexes.
3. Bound style name.
4. Layout config and result.
5. Interactable data.
6. Sparse layout, navigation, hit-test and draw callback indexes.
7. Human-readable debug name.

It does not store a widget kind. A text label, hit-test box, layout container or image region is still the same `Element` type.

Draw commands store their owning dense element index. Delayed callbacks and `draw_for_element` can interleave commands owned by different elements. Call `IContext::generate_draw_commands()` first, then filter `IContext::get_draw_commands()` by `DrawCommand::element` when the actual command set is needed.

### Element handle
`GUICore::ElementHandle` is returned by `begin_element`. It stores stable ID, dense index and generation. Handles are frame-local. Do not keep a handle for use after the next `begin_frame`; keep the stable ID instead. The stable ID is intentionally kept in the handle so APIs can validate that the dense index still points at the same element and so callers can use the returned handle as a stable interaction/state key without another lookup.

### Data scope
A data scope participates in ID generation but does not affect topology. It is used by immediate APIs to avoid repeated child labels producing the same ID.

### Layer
`GUICore::Layer` stores:

1. Stable layer ID.
2. Screen-space layer origin.
3. Root element index.
4. Ordered draw command indexes.
5. Human-readable debug name.

The first element created after `push_layer` becomes the root of that layer. Every element must belong to one layer.

Layer-local positions use the layer origin as `(0, 0)`. Screen positions are converted by adding the layer origin. `Layer::draw_command_indices` is the authoritative command order for a layer.

## Programming guide
### Begin a layer
Call `push_layer` before creating elements.

```cpp
GUICore::id_t popup_layer_id = context->make_id("popup-layer");
context->push_layer(popup_layer_id, popup_screen_pos);
GUICore::ElementHandle popup_root = context->begin_element(context->make_id("popup-root"));

context->set_layer_debug_name(popup_layer_id, Name("Popup"));
context->set_element_debug_name(popup_root, Name("Popup Root"));
```

Call `pop_layer` after the root subtree is complete.

```cpp
context->end_element();
context->pop_layer();
```

Layers are appended in bottom-to-top order. Later layers render above earlier layers and receive input first.

### Build element topology
`begin_element` creates a child of the current element, or the layer root if the layer has no root yet. `end_element` returns to the parent.

```cpp
GUICore::ElementHandle panel = context->begin_element(context->make_id("panel"));
context->set_layout_config(panel, panel_layout);

GUICore::ElementHandle label = context->begin_element(context->make_id("title"));
context->set_layout_config(label, label_layout);
context->end_element();

context->end_element();
```

This produces:

```text
panel
  title
```

### Attach layout data
Each element can receive a dense `LayoutConfig`. Elements that measure content or arrange children additionally receive a sparse `LayoutCallbackConfig`. Layout algorithms later write `LayoutResult`.

```cpp
GUICore::LayoutConfig layout;
layout.width.kind = GUICore::SizeKind::percent;
layout.width.value = 1.0f;
layout.height.kind = GUICore::SizeKind::fixed;
layout.height.value = 32.0f;
layout.padding = Float4U(8.0f, 4.0f, 8.0f, 4.0f);
context->set_layout_config(element, layout);
```

Attach callbacks separately only when the element needs content measurement or child arrangement:

```cpp
GUICore::LayoutCallbackConfig callbacks;
callbacks.algorithm = Name("example.layout");
callbacks.measure_callback = measure_example;
callbacks.callback = layout_example;
callbacks.userdata = frame_arena_data;
context->set_layout_callback_config(element, callbacks);
```

The context stores callback-bearing records in per-frame sparse arrays. `begin_frame` clears the logical records but
retains reusable vector capacity. Do not use callback userdata as cross-frame state; use the context state store for
that purpose.

### Attach interaction data
Use `set_interactable` when an element should participate in input routing.

```cpp
GUICore::Interactable interactable;
interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
context->set_interactable(button_element, interactable);
```

See [[GUICore Input and Interaction]] for routing behavior.

### Bind style
`begin_element` automatically binds the current top style-stack entry to the new element. Use `bind_style` only to override that inherited binding or clear it with an empty name.

```cpp
context->bind_style(element, Name("editor.button"));
```

### Find elements
Use stable IDs for queries:

```cpp
const GUICore::Element* element = context->find_element(id);
GUICore::ElementHandle handle = context->find_element_handle(id);
```

`get_element(index)` is useful for dense traversal when implementing systems or debug views.

## Examples
### Popup as a layer
```cpp
if(open_popup)
{
    context->push_layer(context->make_id("file-popup"), popup_screen_pos);

    GUICore::ElementHandle root = context->begin_element(context->make_id("file-popup-root"));
    context->set_layout_config(root, popup_layout);

    // Add menu item elements here.

    context->end_element();
    context->pop_layer();
}
```

This keeps popup input and rendering naturally above the default layer.

### Repeated list with data scopes
```cpp
GUICore::ElementHandle list = context->begin_element(context->make_id("tracks"));
context->push_data_scope(list.id);

for(usize i = first_visible; i < last_visible; ++i)
{
    context->push_data_scope(GUICore::make_scoped_id(list.id, track_ids[i]));
    GUICore::ElementHandle row = context->begin_element(context->make_id("row"));
    context->set_layout_config(row, row_layout);
    context->end_element();
    context->pop_data_scope();
}

context->pop_data_scope();
context->end_element();
```

The local `"row"` ID is reused safely because each row pushes a different data scope.
