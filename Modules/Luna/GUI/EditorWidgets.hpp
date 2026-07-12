/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorWidgets.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include "Base.hpp"
#include "EditorState.hpp"
#include <Luna/GUICore/GUICore.hpp>

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Registers style entry schema records used by the default editor-style GUI package when writing directly to GUI Core.
        LUNA_GUI_API void register_editor_style_schemas(GUICore::IContext* context);

        //! @name Direct GUI Core item queries
        //! @{
        //! Checks whether an element handle refers to an element in the current GUI Core frame.
        //! @param[in] context The GUI Core context that should own @p handle.
        //! @param[in] handle The element handle to validate.
        //! @return Returns `true` if @p handle belongs to @p context, matches the current context generation and points to the same element ID.
        LUNA_GUI_API bool is_item_valid(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element was clicked by the primary pointer button during the latest input routing pass.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if the element was clicked.
        LUNA_GUI_API bool is_item_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element received a secondary pointer-button release during the latest input routing pass.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if a routed right-button release was delivered to the element.
        LUNA_GUI_API bool is_item_right_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element was double-clicked during the latest input routing pass.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if the element was double-clicked.
        LUNA_GUI_API bool is_item_double_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element is currently hovered.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if the element is hovered.
        LUNA_GUI_API bool is_item_hovered(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element is currently active through pointer capture.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if the element is active.
        LUNA_GUI_API bool is_item_active(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Checks whether an element currently has keyboard focus.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns `true` if the element is focused.
        LUNA_GUI_API bool is_item_focused(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Gets the arranged rectangle of an element in layer coordinates.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns the element rectangle, or an empty rectangle if @p handle is invalid.
        LUNA_GUI_API RectF get_item_rect(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! Gets the clip rectangle of an element in layer coordinates.
        //! @param[in] context The GUI Core context.
        //! @param[in] handle The element handle to query.
        //! @return Returns the element clip rectangle, or an empty rectangle if @p handle is invalid.
        LUNA_GUI_API RectF get_item_clip_rect(GUICore::IContext* context, const GUICore::ElementHandle& handle);
        //! @}

        //! @name Direct GUI Core debug views
        //! @{
        //! Adds an editor-style debug panel for one GUI Core debug snapshot.
        //! @param[in] context The GUI Core context receiving the debug panel elements.
        //! @param[in] id Stable debug panel element ID.
        //! @param[in] info The debug snapshot to visualize.
        //! @param[in] layout Layout input for the debug panel root.
        //! @return Returns the debug panel root element.
        //! @remark The debug panel is a high-level editor-style view. It consumes @ref GUICore::DebugInfo but does not
        //! inspect or mutate GUI Core private runtime storage.
        LUNA_GUI_API GUICore::ElementHandle show_debug_info(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::DebugInfo& info, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! @}

        //! @name Direct GUI Core editor widgets
        //! @{
        //! Begins a horizontal linear layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_h_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current horizontal linear layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            GUICore::FlexLayoutDesc desc);
        //! Ends the current horizontal linear layout and applies GUI Core linear layout to its direct children.
        LUNA_GUI_API RV end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::FlexLayoutDesc desc = GUICore::FlexLayoutDesc());
        //! Begins a vertical linear layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_v_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current vertical linear layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            GUICore::FlexLayoutDesc desc);
        //! Ends the current vertical linear layout and applies GUI Core linear layout to its direct children.
        LUNA_GUI_API RV end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, GUICore::FlexLayoutDesc desc = GUICore::FlexLayoutDesc());
        //! Begins a focus scope directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_focus_scope(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current direct GUI Core focus scope.
        LUNA_GUI_API void end_focus_scope(GUICore::IContext* context);
        //! Begins a row-major grid layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_grid_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current grid layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::GridLayoutDesc& desc);
        //! Ends the current grid layout and applies GUI Core grid layout to its direct children.
        LUNA_GUI_API RV end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::GridLayoutDesc& desc = GUICore::GridLayoutDesc());
        //! Begins an anchor-based canvas layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_canvas_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current canvas layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::CanvasLayoutDesc& desc);
        //! Ends the current canvas layout and applies GUI Core canvas layout to its direct children.
        LUNA_GUI_API RV end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::CanvasLayoutDesc& desc = GUICore::CanvasLayoutDesc());
        //! Begins a dock space directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_dock_space(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Replaces the layout descriptor stored for a direct GUI Core dock space.
        LUNA_GUI_API void set_dockspace_layout(GUICore::IContext* context, GUICore::id_t dock_space,
            const DockSpaceLayoutDesc& desc);
        //! Ends a direct GUI Core dock space and arranges submitted panels.
        LUNA_GUI_API RV end_dock_space(GUICore::IContext* context, const GUICore::ElementHandle& dock_space,
            const RectF& rect);
        //! Begins a dock panel inside the current direct GUI Core dock space.
        LUNA_GUI_API bool begin_dock_panel(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool* open = nullptr, const DockPanelStyle& style = DockPanelStyle(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), GUICore::ElementHandle* out_handle = nullptr);
        //! Ends the current direct GUI Core dock panel.
        LUNA_GUI_API void end_dock_panel(GUICore::IContext* context);
        //! Begins a scroll viewport layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_scroll_viewport(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current scroll viewport and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc = GUICore::ScrollViewportLayoutDesc());
        //! Ends the current scroll viewport and applies GUI Core scroll viewport layout to its direct children.
        LUNA_GUI_API RV end_scroll_viewport(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc = GUICore::ScrollViewportLayoutDesc());
        //! Begins an editor-style scroll view directly in a GUI Core context.
        //! @remark The scroll view manages wheel input, offset clamping and package-level scroll state, then delegates
        //! clipping and child translation to @ref GUICore::layout_scroll_viewport.
        LUNA_GUI_API GUICore::ElementHandle begin_scroll_view(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current scroll view and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::ScrollViewportLayoutDesc& desc = GUICore::ScrollViewportLayoutDesc());
        //! Ends the current scroll view and applies GUI Core scroll viewport layout to its direct children.
        LUNA_GUI_API RV end_scroll_view(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::ScrollViewportLayoutDesc& desc = GUICore::ScrollViewportLayoutDesc());
        //! Begins a table track layout directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_table_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Sets column tracks for the current direct GUI Core table layout.
        //! @param[in] context The GUI Core context.
        //! @param[in] columns Column track descriptors copied into the current table build scope.
        //! @remark If no columns are provided, @ref end_table_layout creates fit-content columns from the widest row.
        LUNA_GUI_API void set_table_columns(GUICore::IContext* context, Span<const GUICore::TableTrackDesc> columns);
        //! Sets the gap between columns and rows for the current direct GUI Core table layout.
        LUNA_GUI_API void set_table_gap(GUICore::IContext* context, const Float2U& gap);
        //! Sets padding assigned to cells collected by subsequent direct GUI Core table rows.
        LUNA_GUI_API void set_table_cell_padding(GUICore::IContext* context, const Float4U& padding);
        //! Sets whether the current direct GUI Core table clips its children.
        LUNA_GUI_API void set_table_clip_children(GUICore::IContext* context, bool clip_children);
        //! Begins a row in the current direct GUI Core table layout.
        //! @param[in] context The GUI Core context.
        //! @param[in] row Row track descriptor for the submitted row.
        //! @return Returns `true` when the caller should submit row cells. Future virtualized table modes may return `false`
        //! while still reserving row layout space.
        LUNA_GUI_API bool begin_table_row(GUICore::IContext* context,
            const GUICore::TableTrackDesc& row = GUICore::TableTrackDesc());
        //! Ends the current direct GUI Core table row and maps newly submitted direct children to table cells.
        LUNA_GUI_API void end_table_row(GUICore::IContext* context);
        //! Ends the current table layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout);
        //! Ends the current table layout and applies GUI Core table layout to its direct children.
        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect);
        //! Ends the current table layout and defers arranging its direct children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const GUICore::TableLayoutDesc& desc);
        //! Ends the current table layout and applies GUI Core table layout to its direct children.
        LUNA_GUI_API RV end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& layout,
            const RectF& rect, const GUICore::TableLayoutDesc& desc);
        //! Applies all deferred editor-style layout requests under one root element.
        //! @param[in] context The GUI Core context that owns @p root.
        //! @param[in] root The root element of the subtree to arrange.
        //! @param[in] rect The root rectangle in layer coordinates.
        //! @return Returns success or failure code.
        //! @remark This lets high-level immediate code build a subtree first, then perform layout in one pass from
        //! a known root rectangle. It does not add widget semantics to GUI Core.
        LUNA_GUI_API RV layout_editor_tree(GUICore::IContext* context, const GUICore::ElementHandle& root,
            const RectF& rect);

        //! Adds a text element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds an image element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, GUICore::id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), ImageFlag flags = ImageFlag::none);
        //! Adds a shape element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, GUICore::id_t id, const GUICore::ShapeDesc& shape,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds an invisible hit-test region directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds an absolute-positioned rectangle draw element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle draw_rect(GUICore::IContext* context, GUICore::id_t id,
            const RectF& rect, const Float4U& color, f32 radius = 0.0f);
        //! Adds an absolute-positioned circle draw element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle draw_circle(GUICore::IContext* context, GUICore::id_t id,
            const Float2U& center, f32 radius, const Float4U& color);
        //! Adds an absolute-positioned line draw element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle draw_line(GUICore::IContext* context, GUICore::id_t id,
            const Float2U& begin, const Float2U& end, const Float4U& color, f32 width = 1.0f);
        //! Adds an absolute-positioned text draw element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle draw_text(GUICore::IContext* context, GUICore::id_t id,
            const RectF& rect, const c8* text, const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            TextAlignment horizontal_alignment = TextAlignment::begin,
            TextAlignment vertical_alignment = TextAlignment::center);
        //! Adds an absolute-positioned image draw element directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle draw_image(GUICore::IContext* context, GUICore::id_t id,
            RHI::ITexture* texture, const RectF& rect, const Float4U& color = Float4U(1.0f), ImageFlag flags = ImageFlag::none);

        //! Begins an interactive button container directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_button(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Ends the current GUI Core button container.
        LUNA_GUI_API void end_button(GUICore::IContext* context);
        //! Adds a text button directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle text_button(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a shape icon button directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle shape_button(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const GUICore::ShapeDesc& shape, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), f32 padding = 6.0f,
            bool enabled = true);
        //! Adds a selectable item directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle selectable(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool selected,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a checkbox directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool checked,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a checkbox bound to a boolean value directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a radio button directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool selected,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a radio button bound to a boolean value directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds one radio button bound to an integer selection directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, i32* value,
            i32 button_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds an animated switch directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool checked,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds an animated switch bound to a boolean value directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true);
        //! Adds a single-selection segmented button group directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, GUICore::id_t id,
            i32* current_item, Span<const c8*> items, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            bool enabled = true);
        //! Adds a multi-selection segmented button group directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, GUICore::id_t id,
            Span<bool> selected, Span<const c8*> items, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            bool enabled = true);
        //! Adds an editable single-line UTF-8 text input directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, GUICore::id_t id, String& value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), bool enabled = true, bool readonly = false);
        //! Adds a collapsing header directly to a GUI Core context.
        LUNA_GUI_API bool collapsing_header(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool default_open = true, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            GUICore::ElementHandle* out_handle = nullptr);
        //! Adds a tree node directly to a GUI Core context.
        LUNA_GUI_API bool tree_node(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            TreeNodeFlag flags = TreeNodeFlag::none, u32 indent_depth = 0,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), GUICore::ElementHandle* out_handle = nullptr);
        //! Adds a progress bar directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, GUICore::id_t id, f32 fraction,
            const c8* overlay = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! Adds a single-value floating-point slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component floating-point slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float2(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component floating-point slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float3(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component floating-point slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_float4(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a single-value integer slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component integer slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int2(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component integer slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int3(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component integer slider directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle slider_int4(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a single-value floating-point drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_float(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component floating-point drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_float2(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component floating-point drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_float3(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component floating-point drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_float4(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a single-value integer drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_int(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a two-component integer drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_int2(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a three-component integer drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_int3(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a four-component integer drag editor directly to a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle drag_int4(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! Begins a tab bar directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_tab_bar(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, TabBarFlag flags = TabBarFlag::fitting_shrink,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a direct GUI Core tab bar and defers arranging its headers and selected content until @ref layout_editor_tree.
        LUNA_GUI_API RV end_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar);
        //! Ends a direct GUI Core tab bar and lays out its headers and selected content.
        LUNA_GUI_API RV end_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar, const RectF& rect);
        //! Applies editor-style tab bar layout to an already built tab bar.
        LUNA_GUI_API RV layout_tab_bar(GUICore::IContext* context, const GUICore::ElementHandle& tab_bar, const RectF& rect);
        //! Adds one tab header directly to a GUI Core tab bar.
        LUNA_GUI_API bool begin_tab_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool* open = nullptr, TabItemFlag flags = TabItemFlag::none, GUICore::ElementHandle* out_handle = nullptr);
        //! Ends the selected direct GUI Core tab item content block.
        LUNA_GUI_API void end_tab_item(GUICore::IContext* context);

        //! Requests a direct GUI Core popup to open.
        LUNA_GUI_API void open_popup(GUICore::IContext* context, GUICore::id_t id);
        //! Closes a direct GUI Core popup.
        LUNA_GUI_API void close_popup(GUICore::IContext* context, GUICore::id_t id);
        //! Checks whether a direct GUI Core popup is open.
        LUNA_GUI_API bool is_popup_open(GUICore::IContext* context, GUICore::id_t id);
        //! Begins a popup layer directly in a GUI Core context.
        LUNA_GUI_API bool begin_popup(GUICore::IContext* context, GUICore::id_t id, const PopupDesc& desc,
            GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a direct GUI Core popup layer and lays out its direct children vertically.
        LUNA_GUI_API RV end_popup(GUICore::IContext* context, const GUICore::ElementHandle& popup, const RectF& rect);
        //! Begins a tooltip layer for an owner element directly in a GUI Core context.
        LUNA_GUI_API bool begin_tooltip(GUICore::IContext* context, GUICore::id_t id, const GUICore::ElementHandle& owner,
            const TooltipDesc& desc = TooltipDesc(), GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a direct GUI Core tooltip layer and lays out its direct children vertically.
        LUNA_GUI_API RV end_tooltip(GUICore::IContext* context, const GUICore::ElementHandle& tooltip, const RectF& rect);
        //! Adds a simple text tooltip for an owner element directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle set_item_tooltip(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::ElementHandle& owner, const c8* content, const TooltipDesc& desc = TooltipDesc());
        //! Begins a menu bar directly in a GUI Core context.
        LUNA_GUI_API GUICore::ElementHandle begin_menu_bar(GUICore::IContext* context, GUICore::id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a direct GUI Core menu bar and defers arranging its menu children until @ref layout_editor_tree.
        LUNA_GUI_API RV end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar);
        //! Ends a direct GUI Core menu bar and lays out its direct menu children horizontally.
        LUNA_GUI_API RV end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar, const RectF& rect);
        //! Applies editor-style menu bar layout to an already built menu bar.
        LUNA_GUI_API RV layout_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar, const RectF& rect);
        //! Begins a menu or submenu directly in a GUI Core context.
        LUNA_GUI_API bool begin_menu(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool enabled = true,
            GUICore::ElementHandle* out_handle = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a direct GUI Core menu or submenu popup layer.
        LUNA_GUI_API RV end_menu(GUICore::IContext* context, const RectF& rect);
        //! Adds one direct GUI Core menu item.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const c8* shortcut = nullptr, bool selected = false, bool enabled = true,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds one direct GUI Core checkable menu item.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const c8* shortcut, bool* selected, bool enabled = true,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds one direct GUI Core menu separator.
        LUNA_GUI_API GUICore::ElementHandle menu_separator(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());

        //! @}

        //! @}
    }
}
