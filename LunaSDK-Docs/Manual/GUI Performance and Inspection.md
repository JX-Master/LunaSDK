GUI exposes operational performance counters in every build and direct read-only access to the current frame data for inspection tools.

## Designed functionality
These APIs provide observability without maintaining a duplicated debug snapshot:

1. `PerformanceCounters` reports frame counts and timings.
2. `get_elements` and `get_layers` expose the current dense arrays for read-only scanning.
3. `get_input_events`, `get_data_scope_stack` and `get_styles` expose the remaining frame and context collections without copying them.
4. Existing query APIs expose callback configurations, interaction state, delivered input, styles and draw commands.
5. Human-readable debug names and the high-level GUI debug panel are available in every build.

GUI does not provide issue/pass logs, input recording, input replay, frame timelines or a cross-process transport format.

## Concepts
### Performance counters
`PerformanceCounters` contains element, layer, input, state, style and draw-command counts. It also records time spent on state cleanup, input routing and draw generation.

`RendererPerformanceCounters` reports the renderer's latest recording time, backend batches and uploads. Backdrop
telemetry includes live capture count, graphics pass count, filtered pixels, blur dispatches and approximate temporary
texture bytes. A capture attachment with no visible consumer is not counted because the compiler eliminates it.
One live capture normally records one compute dispatch per effective downsample level plus horizontal and vertical
Gaussian dispatches. A sub-pixel working standard deviation omits Gaussian convolution and records only the
snapshot or requested downsample chain. Filtered-pixel and temporary-byte counters include the complete downsample
chain and use the target-derived intermediate format size.

Performance counters are operational telemetry and remain available in every build.

### Direct inspection
`IContext::get_elements`, `IContext::get_layers`, `IContext::get_input_events` and `IContext::get_data_scope_stack` return read-only spans into context-owned arrays. `IContext::get_styles` returns a const reference to the named style store. These values are views, not snapshots, and are invalidated by the corresponding collection mutation.

Tools should read the spans without mutating the tree. A tool that builds its own panel into the inspected context must first copy the small amount of presentation data it needs, then add the panel elements. Callback pointers and userdata may be inspected for presence and identity, but their package-owned semantics are not discoverable by GUI.

### Debug names
`Element` and `Layer` contain human-readable debug names. Use `set_element_debug_name` and `set_layer_debug_name` to set them.

Debug names are observational metadata. They must not participate in ID generation, layout, input routing, rendering, measurement, or widget behavior.

Debug names and inspection views are not controlled by a build option. Keeping them resident gives tools one stable API and ABI in Debug and Release builds.

## Programming guide
### Read performance counters
```cpp
GUI::PerformanceCounters counters = context->get_performance_counters();
log_info("GUI: elements=%u draw=%u callbacks=%u route=%.3f ms generate=%.3f ms",
    counters.element_count,
    counters.draw_command_count,
    counters.draw_callback_count,
    counters.input_route_ms,
    counters.draw_generate_ms);
```

Counters reflect the most recent operations that updated them.

Renderer counters are read from the renderer after recording:

```cpp
GUI::RendererPerformanceCounters rendering = renderer->get_performance_counters();
log_info("GUI renderer: %.3f ms, %u passes, %u captures, %llu filtered pixels",
    rendering.render_ms,
    rendering.render_pass_count,
    rendering.backdrop_capture_count,
    rendering.backdrop_filtered_pixel_count);
```

### Scan the element tree
```cpp
Span<const GUI::Layer> layers = context->get_layers();
Span<const GUI::Element> elements = context->get_elements();

for(u32 i = 0; i < elements.size(); ++i)
{
    const GUI::Element& element = elements[i];
    GUI::ElementHandle handle {element.id, i, context->generation()};
    GUI::LayoutCallbackConfig layout_callbacks = context->get_layout_callback_config(handle);
    GUI::NavigationConfig navigation = context->get_navigation_config(handle);
    GUI::ElementHitTestConfig hit_test = context->get_hit_test_config(handle);
    GUI::DrawConfig draw = context->get_draw_config(handle);
    GUI::BackdropBlurCaptureDesc backdrop = context->get_backdrop_blur_capture(handle);
    GUI::InteractionState interaction = context->get_interaction_state(element.id);
}
```

Topology fields such as `parent`, `first_child` and `next_sibling` are dense element indexes. Layer records provide each tree root and screen position.

### Scan context collections
```cpp
for(const GUI::InputEvent& event : context->get_input_events())
{
    // Inspect the raw event in submission order.
}

for(GUI::id_t scope : context->get_data_scope_stack())
{
    // Inspect the active scoped-ID path.
}

const HashMap<Name, GUI::Style>& styles = context->get_styles();
for(auto iter = styles.begin(); iter != styles.end(); ++iter)
{
    const Name& style_name = iter->first;
    const GUI::Style& style = iter->second;
    // Inspect style.parent and style.entries.
}
```

These are borrowed views. Do not keep their spans, references, pointers, or iterators across the corresponding context mutation.

### Inspect draw ownership
```cpp
for(const GUI::DrawCommand& command : context->get_draw_commands())
{
    if(command.element != GUI::INVALID_ELEMENT)
    {
        const GUI::Element* owner = context->get_element(command.element);
        // Inspect command and owner.
    }
}
```

Call `generate_draw_commands` first when delayed draw callbacks are used and the final command stream is required.

### Attach debug names
```cpp
context->set_layer_debug_name(layer_id, Name("Main Layer"));
context->set_element_debug_name(element, Name("Apply Button"));
```

## Examples
### Build an external inspection record
An external debugger should define its own transport-safe record and copy only the required scalar and string data while scanning the context. Do not transmit callback, userdata, texture, font or shape-buffer pointers.
