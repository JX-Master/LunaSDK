GUI Core layout algorithms operate on typeless element data. They read `LayoutConfig` from parent and child elements, measure child size requirements when needed, then write `LayoutResult` back to the context.

## Designed functionality
The layout system provides reusable layout primitives for higher-level GUI packages and applications:

1. Resolve fixed, fit and percentage sizes with explicit minimum and maximum constraints.
2. Let higher-level packages provide measurement callbacks for content-driven sizing such as text.
3. Arrange direct children without depending on widget types.
4. Write layer-coordinate rectangles and clip rectangles.
5. Keep parent-owned placement data, such as canvas anchors and table cells, outside the child element type.

GUI Core layout helpers are intentionally primitive. A high-level package may combine them with widget-specific measurement, scrollbars, table headers, virtualization or editor metadata.

## Concepts
### Layout config
`GUICore::LayoutConfig` is attached to each element. It contains:

1. Width and height `SizeValue`.
2. Margin.
3. Padding.
4. Flex grow and shrink factors.
5. Optional measure, arrange and finalize callbacks.
6. Callback userdata owned by the caller that installs the callback.

`SizeValue::kind` controls how the axis is resolved:

1. `fit`: use the content size returned by `LayoutConfig::measure_callback`. If no measure callback is installed, content minimum and desired size are zero, so the resolved desired size is only padding plus explicit constraints.
2. `fixed`: use an absolute logical-coordinate size.
3. `percent`: use a percentage of the available parent content size.

`min` and `max` constrain the final element box size on the axis. A negative `max` means no maximum.

### Measure callback
`LayoutConfig::measure_callback` computes content-box size for one element. It returns a `GUICore::MeasureResult`:

1. `minimum`: minimum content size.
2. `desired`: preferred content size.
3. `maximum`: maximum content size, or `F32_MAX` for an unbounded axis.

GUI Core adds padding, applies `SizeValue::min` and `SizeValue::max`, and resolves the final element-box measurement. Margin belongs to the parent layout algorithm and is not included in `MeasureResult`.

This keeps drawing and measurement separate. Text may live only in `DrawCommand` records; GUI Core will not scan draw commands to infer text size. A higher-level package that wants text hug sizing should install a text measure callback and pass the text, font and font size through its own callback userdata.

Use `IContext::measure_element` inside layout algorithms when a child element's resolved minimum, desired and maximum sizes are needed. The returned sizes are element-box sizes excluding margin.

### Layout callbacks
`LayoutConfig::callback` arranges one element's direct children. `IContext::apply_layout` writes the root layout result, then walks the element tree from parent to child and calls these callbacks.

`LayoutConfig::finalize_callback` is called after child subtrees have been arranged. Use it when a parent needs to derive final data from arranged children.

### Layout result
`GUICore::LayoutResult` stores:

1. `rect`: element rectangle in layer coordinates.
2. `clip_rect`: element clip rectangle in layer coordinates.
3. `content_size`: measured content size written by the layout algorithm. For scroll and table containers, this may be larger than the visible rectangle.

`clip_rect` participates in hit testing. Drawing is clipped only by explicit draw commands such as `push_clip` and `pop_clip`.

### Parent-owned placement
Some layout data belongs to the parent algorithm instead of the child:

1. `CanvasLayoutItem` maps child IDs to anchors and offsets.
2. `TableLayoutCell` maps child IDs to row and column cells.

This keeps child elements typeless and allows one child to be placed by different parent algorithms in different contexts.

## Programming guide
### Attach layout config
```cpp
GUICore::LayoutConfig config;
config.width.kind = GUICore::SizeKind::percent;
config.width.value = 1.0f;
config.height.kind = GUICore::SizeKind::fixed;
config.height.value = 40.0f;
config.margin = Float4U(4.0f, 2.0f, 4.0f, 2.0f);
config.padding = Float4U(8.0f, 4.0f, 8.0f, 4.0f);
context->set_layout_config(element, config);
```

### Run layout from a root
After building elements, call `IContext::apply_layout` once for each root that should be arranged.

```cpp
RectF screen_rect(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y);
luexp(context->apply_layout(root, screen_rect));
```

### Provide custom measurement
Use a measure callback when `fit` axes should be driven by package-owned content.

```cpp
struct TextMeasureData
{
    const c8* text;
    Font::IFontFile* font;
    u32 font_index;
    f32 font_size;
};

GUICore::MeasureResult measure_text(
    GUICore::IContext* context,
    const GUICore::ElementHandle& element,
    const Float2U& available_content_size,
    void* userdata)
{
    TextMeasureData* data = (TextMeasureData*)userdata;
    GUICore::MeasureResult result;

    // Arrange or otherwise measure package-owned text here.
    result.desired = measure_text_bounds(
        data->text, data->font, data->font_index, data->font_size, available_content_size);
    result.maximum = Float2U(F32_MAX, F32_MAX);
    return result;
}

GUICore::LayoutConfig label_layout;
label_layout.width.kind = GUICore::SizeKind::fit;
label_layout.height.kind = GUICore::SizeKind::fit;
label_layout.padding = Float4U(8.0f, 4.0f, 8.0f, 4.0f);
label_layout.measure_callback = measure_text;
label_layout.userdata = text_measure_data;
context->set_layout_config(label, label_layout);
```

The callback userdata must remain valid until layout has finished. Immediate API packages commonly store this data in a per-frame arena and reset the arena after the frame.

### Run a flex layout
`layout_flex` arranges direct children with CSS-like flex rules. Use `LayoutAxis::x` for rows and `LayoutAxis::y` for columns.

Install both `measure_flex` and `layout_flex` when the flex container itself may be measured by an ancestor.

```cpp
GUICore::FlexLayoutDesc desc;
desc.axis = GUICore::LayoutAxis::x;
desc.main_axis_gap = 8.0f;
desc.cross_alignment = GUICore::FlexAlignment::stretch;

GUICore::LayoutConfig row_layout;
row_layout.width.kind = GUICore::SizeKind::percent;
row_layout.width.value = 1.0f;
row_layout.height.kind = GUICore::SizeKind::fit;
row_layout.measure_callback = GUICore::measure_flex;
row_layout.callback = GUICore::layout_flex;
row_layout.userdata = &desc;
context->set_layout_config(row, row_layout);
```

Flex layout measures children first, groups them into lines, distributes free or missing main-axis space with `flex_grow` and `flex_shrink`, then arranges child subtrees.

### Run a grid layout
`layout_grid` arranges direct children in row-major order. It does not provide scrolling by itself.

```cpp
GUICore::GridLayoutDesc desc;
desc.mode = GUICore::GridLayoutMode::fixed_cell_size;
desc.cell_size = Float2U(96.0f, 80.0f);
desc.gap = Float2U(8.0f, 8.0f);

GUICore::LayoutConfig grid_layout;
grid_layout.callback = GUICore::layout_grid;
grid_layout.userdata = &desc;
context->set_layout_config(grid, grid_layout);
```

Use `fixed_column_count` when the caller wants a stable number of columns and a derived cell width.

```cpp
desc.mode = GUICore::GridLayoutMode::fixed_column_count;
desc.column_count = 4;
desc.cell_size.y = 80.0f;
```

### Run a stack layout
`layout_stack` overlays all direct children in the same parent content rectangle.

```cpp
GUICore::StackLayoutDesc desc;
desc.alignment = Float2U(0.5f, 0.5f);

GUICore::LayoutConfig stack_layout;
stack_layout.callback = GUICore::layout_stack;
stack_layout.userdata = &desc;
context->set_layout_config(stack, stack_layout);
```

This is useful for overlays, visual chrome and simple popup bodies.

### Run a canvas layout
`layout_canvas` uses Unity-style normalized anchors and offsets.

```cpp
GUICore::CanvasLayoutItem item;
item.element_id = child.id;
item.anchor_min = Float2U(0.5f, 0.0f);
item.anchor_max = Float2U(0.5f, 0.0f);
item.offset = Float4U(0.0f, 12.0f, 0.0f, 0.0f);
item.pivot = Float2U(0.5f, 0.0f);

GUICore::CanvasLayoutDesc desc;
desc.items = Span<const GUICore::CanvasLayoutItem>(&item, 1);

GUICore::LayoutConfig canvas_layout;
canvas_layout.callback = GUICore::layout_canvas;
canvas_layout.userdata = &desc;
context->set_layout_config(canvas, canvas_layout);
```

If `anchor_min` and `anchor_max` differ on an axis, the child stretches between those anchored edges. If they are equal, the child uses its layout size and pivot on that axis.

### Run a scroll viewport layout
`layout_scroll_viewport` translates and clips direct children by a scroll offset. It does not implement scrollbar rendering, scroll input handling or offset clamping.

```cpp
GUICore::ScrollViewportLayoutDesc desc;
desc.scroll_offset = current_scroll_offset;

GUICore::LayoutConfig viewport_layout;
viewport_layout.callback = GUICore::layout_scroll_viewport;
viewport_layout.userdata = &desc;
context->set_layout_config(viewport, viewport_layout);
```

Higher-level packages should implement scroll state, scrollbars, clamping and virtualized child submission on top of this primitive.

### Run a table layout
`layout_table` uses explicit column tracks, row tracks and cell attachments.

```cpp
GUICore::TableTrackDesc columns[3];
columns[0].kind = GUICore::TableTrackSizeKind::pixels;
columns[0].value = 120.0f;
columns[1].kind = GUICore::TableTrackSizeKind::ratio;
columns[1].value = 1.0f;
columns[2].kind = GUICore::TableTrackSizeKind::pixels;
columns[2].value = 180.0f;

GUICore::TableLayoutCell cells[2];
cells[0].element_id = name_element.id;
cells[0].row = 0;
cells[0].column = 0;
cells[1].element_id = value_element.id;
cells[1].row = 0;
cells[1].column = 1;

GUICore::TableLayoutDesc desc;
desc.columns = Span<const GUICore::TableTrackDesc>(columns, 3);
desc.rows = Span<const GUICore::TableTrackDesc>(rows, row_count);
desc.cells = Span<const GUICore::TableLayoutCell>(cells, cell_count);
desc.gap = Float2U(4.0f, 2.0f);

GUICore::LayoutConfig table_layout;
table_layout.callback = GUICore::layout_table;
table_layout.userdata = &desc;
context->set_layout_config(table, table_layout);
```

Explicit cell attachments make the primitive compatible with virtualized rows and editor-authored table descriptions.

## Examples
### Full-screen root layout
```cpp
RectF screen_rect(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y);
luexp(context->apply_layout(root, screen_rect));
```

### Vertical tool panel
```cpp
GUICore::FlexLayoutDesc desc;
desc.axis = GUICore::LayoutAxis::y;
desc.main_axis_gap = 6.0f;

GUICore::LayoutConfig panel_layout;
panel_layout.width.kind = GUICore::SizeKind::fixed;
panel_layout.width.value = 280.0f;
panel_layout.height.kind = GUICore::SizeKind::percent;
panel_layout.height.value = 1.0f;
panel_layout.measure_callback = GUICore::measure_flex;
panel_layout.callback = GUICore::layout_flex;
panel_layout.userdata = &desc;

GUICore::ElementHandle panel = context->begin_element(context->make_id("inspector"), Name("Inspector"));
context->set_layout_config(panel, panel_layout);

// Build direct children here.

context->end_element();
```

### Virtualized table usage
GUI Core table layout can place only the rows that were submitted. A higher-level package can compute visible row range, submit only those children, then provide row tracks and cell attachments for that range.

```cpp
for(u32 row = first_visible_row; row < last_visible_row; ++row)
{
    // Submit row cells and fill TableLayoutCell records for this row.
}

context->set_layout_config(table, table_layout);
```

The total scrollable size and scrollbar behavior still belong to the higher-level scroll/table package.
