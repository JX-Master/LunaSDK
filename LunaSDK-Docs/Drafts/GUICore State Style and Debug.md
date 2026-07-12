GUI Core provides three support systems that are shared across GUI packages: state storage, named styles and debug instrumentation.

## Designed functionality
These systems solve cross-frame and tooling problems without introducing widget classes:

1. The state store keeps typed boxed state objects by stable IDs.
2. The style system stores named style records with inheritance and schema metadata.
3. Debug output captures the current frame for in-process panels, tests and inspection tools.
4. Performance counters expose the cost of routing, state cleanup and draw command compilation.

GUI Core does not define widget-specific state or widget-specific style keys. Higher-level packages define those objects and keys.

## Concepts
### State ID
A state ID identifies one state object. The recommended form is:

```cpp
GUICore::id_t state_id = GUICore::make_state_id<StateType>(owner_id);
```

This combines the owner ID with the boxed state type GUID.

### State lifetime
`GUICore::StateLifetime` controls cleanup:

1. `current_frame`: cleared at the next `begin_frame`.
2. `next_frame`: cleared if it is not refreshed for the next frame.
3. `process`: kept until `clear_state` or process exit.
4. `persistent`: reserved for future persistent storage semantics.

### Style
`GUICore::Style` is a named set of entries and an optional parent style. Entries can:

1. Inherit from the parent.
2. Set a local value.
3. Explicitly unset an inherited value.

Only the top style in the context style stack is bound to newly created elements.

### Style value
`GUICore::StyleValue` supports:

1. `f32`
2. `f32x2`
3. `f32x3`
4. `f32x4`
5. `name`

Convenience helpers include `style_f32`, `style_f32x2`, `style_f32x3`, `style_f32x4` and `style_name`.

### Style schema
`StyleEntrySchema` describes a style entry consumed by a package. GUI Core stores schemas for tools and debug views. It does not interpret the entry as behavior.

### Debug info
`GUICore::DebugInfo` is an in-memory frame snapshot. It contains:

1. Performance counters.
2. Layers.
3. Elements.
4. Styles and style schemas.
5. Input events and deliveries.
6. Debug issues and pass records.
7. Current focus, active and capture summary.
8. Draw commands.

The snapshot is not a cross-process wire format. `LayoutConfig` can contain callback and userdata pointers, while draw commands can contain texture and shape-buffer pointers. Use the snapshot directly for same-process debug panels or tests. An external debugger must first convert it to an application-defined transport DTO that replaces or omits those runtime pointers.

The current snapshot does not include per-element `NavigationConfig` modes or navigation callback presence. Tools that need that information must inspect the live element data in-process until a portable navigation-debug record is added.

### Debug timeline
`DebugFrameTimeline` stores multiple `DebugInfo` frames so an in-process debug panel or test can step through captured frames. Pointer-valued fields are observational data only and can become stale after their backing runtime data is released.

## Programming guide
### Set and get state
State objects are boxed Runtime objects.

```cpp
struct [[Luna::struct("{EA3F570E-C41A-4B23-A70C-97656C0AB92E}")]] MyControlState
{
    f32 animation = 0.0f;
    f32 hover = 0.0f;
};

GUICore::id_t state_id = GUICore::make_state_id<MyControlState>(element.id);
Ref<MyControlState> state = new_object<MyControlState>();
state->animation = 0.5f;
luexp(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));

object_t boxed = context->get_state(state_id);
```

This state type must be declared in module code processed by LunaMetaTool so its boxed Runtime type is registered before `new_object` is called. Refresh `next_frame` state every frame while the owner remains alive.

### Clear state
```cpp
context->clear_state(state_id);
```

Use this for process lifetime state when the owning subsystem is explicitly reset.

### Define styles
```cpp
context->define_style(Name("editor.dark"));
context->define_style(Name("editor.dark.button"), Name("editor.dark"));
```

### Set style entries
```cpp
context->set_style_value(Name("editor.dark"),
    Name("gui.editor.text.color"),
    GUICore::style_f32x4(Float4U(0.90f, 0.93f, 0.96f, 1.0f)));
```

### Inherit or unset entries
```cpp
context->inherit_style_entry(Name("editor.dark.button"), Name("gui.editor.text.color"));
context->unset_style_entry(Name("editor.disabled"), Name("gui.editor.button.accent"));
```

`inherit_style_entry` removes a local override. `unset_style_entry` hides an inherited value.

### Use the style stack
```cpp
context->push_style(Name("editor.dark.button"));
GUICore::ElementHandle button = context->begin_element(button_id, Name("Button"));
context->end_element();
context->pop_style();
```

`begin_element` itself binds the current style-stack top to each newly created element. Call `bind_style` only when a package needs to replace that binding or clear it with an empty style name.

### Resolve style values
```cpp
GUICore::StyleValue fallback = GUICore::style_f32x4(Float4U(1.0f));
GUICore::StyleValue value = context->get_style_value(
    context->current_style(),
    Name("gui.editor.text.color"),
    fallback);
```

Packages should always provide a default value in case the style entry is missing or unset.

### Register style schemas
```cpp
GUICore::StyleEntrySchema schema;
schema.owner = Name("gui.editor");
schema.entry = Name("gui.editor.button.background");
schema.type = GUICore::StyleValueType::f32x4;
schema.default_value = GUICore::style_f32x4(Float4U(0.12f, 0.18f, 0.26f, 1.0f));
schema.category = "Button";
schema.description = "Button background color.";
context->register_style_entry_schema(schema);
```

GUI Editor and debug panels use schemas to show meaningful editable style values.

### Read performance counters
```cpp
GUICore::PerformanceCounters counters = context->get_performance_counters();
log_info("GUICore: elements=%u draw=%u callbacks=%u route=%.3f ms generate=%.3f ms compile=%.3f ms",
    counters.element_count,
    counters.draw_command_count,
    counters.draw_callback_count,
    counters.input_route_ms,
    counters.draw_generate_ms,
    counters.draw_compile_ms);
```

Counters reflect the most recent operations that updated them.

### Dump debug info
```cpp
GUICore::DebugInfo info = context->dump_debug_info();
```

Use the snapshot directly in a debug panel or compare it in tests. For external tooling, convert it to a transport-safe representation that removes callback, userdata, texture, and shape-buffer pointers.

### Log issues and passes
```cpp
context->log_debug_issue(GUICore::DebugIssueSeverity::warning,
    Name("layout"), "Child has no matching table cell.", child_id);

context->log_debug_pass(GUICore::DebugPassKind::layout,
    Name("layout_table"), Name("table_rows_changed"), table_id,
    "Table row count changed.", duration_ms);
```

These records are frame-local and appear in `dump_debug_info`.

### Capture a debug timeline
```cpp
GUICore::DebugFrameTimeline timeline;
GUICore::push_debug_frame(timeline, context->dump_debug_info(), 120);

const GUICore::DebugInfo* frame = GUICore::current_debug_frame(timeline);
GUICore::step_debug_frame(timeline, -1);
GUICore::clear_debug_frames(timeline);
```

## Examples
### Animate state owned by an element
```cpp
GUICore::id_t state_id = GUICore::make_state_id<MyControlState>(element.id);
Ref<MyControlState> state = cast_object<MyControlState>(context->get_state(state_id));
if(!state)
{
    state = new_object<MyControlState>();
}

GUICore::InteractionState interaction = context->get_interaction_state(element.id);
f32 target = interaction.hovered ? 1.0f : 0.0f;
state->hover = lerp(state->hover, target, min(frame.delta_time * 12.0f, 1.0f));

luexp(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
```

### Debug element style resolution
```cpp
GUICore::DebugInfo info = context->dump_debug_info();
for(const GUICore::DebugElementInfo& element : info.elements)
{
    if(element.id == selected_element_id)
    {
        for(const GUICore::DebugResolvedStyleEntryInfo& entry : element.resolved_style)
        {
            // Show entry.owner, entry.entry, entry.value and entry.defaulted.
        }
    }
}
```

This is the preferred way for GUI tooling to inspect which style entries a package declared and which values resolved for a specific element.
