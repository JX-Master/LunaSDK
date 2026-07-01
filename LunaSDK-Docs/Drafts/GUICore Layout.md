GUI Core layout algorithms operate on typeless element data. They  read `LayoutConfig` from parent and child elements, then write `LayoutResult` back to the context.

## Designed functionality
The layout system provides reusable layout primitives for higher-level GUI packages and applications:

1. Resolve fixed, fit, percentage, ratio and expand sizes.
2. Arrange direct children without depending on widget types.
3. Write layer-coordinate rectangles and clip rectangles.
4. Keep parent-owned placement data, such as canvas anchors and table cells, outside the child element type.

GUI Core layout helpers are intentionally primitive. A high-level package may combine them with widget-specific measurement, scrollbars, table headers, virtualization or editor metadata.

## Concepts
### Layout config
`GUICore::LayoutConfig` is attached to each element. It contains:

1. Width and height `SizeValue`.
2. Margin.
3. Padding.

`SizeValue::kind` controls how the axis is resolved:

1. `fit`: use content-driven size.
2. `fit_largest`: use the largest measured content size among siblings on that axis.
3. `pixels`: use an absolute logical pixel size.
4. `percent`: use a percentage of the parent size.
5. `ratio`: consume remaining space using a weighted ratio.
6. `expand`: expand to the available size.

`min` and `max` constrain the final size. A negative `max` means no maximum.

### Layout result
`GUICore::LayoutResult` stores:

1. `rect`: element rectangle in layer coordinates.
2. `clip_rect`: element clip rectangle in layer coordinates.
3. `content_size`: measured content size.

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

### Run a linear layout
`layout_linear` arranges direct children along one axis. Use `LayoutAxis::x` for rows and `LayoutAxis::y` for columns.

```cpp
GUICore::LinearLayoutDesc desc;
desc.axis = GUICore::LayoutAxis::x;
desc.gap = 8.0f;
desc.clip_children = true;
luexp(GUICore::layout_linear(context, row, row_rect, desc));
```

### Run a grid layout
`layout_grid` arranges direct children in row-major order. It does not provide scrolling by itself.

```cpp
GUICore::GridLayoutDesc desc;
desc.mode = GUICore::GridLayoutMode::fixed_cell_size;
desc.cell_size = Float2U(96.0f, 80.0f);
desc.gap = Float2U(8.0f, 8.0f);
luexp(GUICore::layout_grid(context, grid, grid_rect, desc));
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
luexp(GUICore::layout_stack(context, stack, stack_rect, desc));
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
luexp(GUICore::layout_canvas(context, canvas, canvas_rect, desc));
```

If `anchor_min` and `anchor_max` differ on an axis, the child stretches between those anchored edges. If they are equal, the child uses its layout size and pivot on that axis.

### Run a scroll viewport layout
`layout_scroll_viewport` translates and clips direct children by a scroll offset. It does not implement scrollbar rendering, scroll input handling or offset clamping.

```cpp
GUICore::ScrollViewportLayoutDesc desc;
desc.scroll_offset = current_scroll_offset;
luexp(GUICore::layout_scroll_viewport(context, viewport, viewport_rect, desc));
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
luexp(GUICore::layout_table(context, table, table_rect, desc));
```

Explicit cell attachments make the primitive compatible with virtualized rows and editor-authored table descriptions.

## Examples
### Full-screen root layout
```cpp
GUICore::LayoutResult root_result;
root_result.rect = RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y);
root_result.clip_rect = root_result.rect;
root_result.content_size = frame.screen_size;
context->set_layout_result(root, root_result);
```

After the root result is written, run child layout algorithms from the root down.

### Vertical tool panel
```cpp
GUICore::ElementHandle panel = context->begin_element(context->make_id("inspector"), Name("Inspector"));
context->set_layout_config(panel, panel_layout);

// Build direct children here.

context->end_element();

GUICore::LinearLayoutDesc desc;
desc.axis = GUICore::LayoutAxis::y;
desc.gap = 6.0f;
luexp(GUICore::layout_linear(context, panel, panel_rect, desc));
```

### Virtualized table usage
GUI Core table layout can place only the rows that were submitted. A higher-level package can compute visible row range, submit only those children, then provide row tracks and cell attachments for that range.

```cpp
for(u32 row = first_visible_row; row < last_visible_row; ++row)
{
    // Submit row cells and fill TableLayoutCell records for this row.
}

luexp(GUICore::layout_table(context, table, table_rect, table_desc));
```

The total scrollable size and scrollbar behavior still belong to the higher-level scroll/table package.
