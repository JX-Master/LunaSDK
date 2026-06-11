/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Context.hpp"
#include "State.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! @name Build stack helpers
        //! @{
        //! Pushes an integer value onto the widget ID stack.
        LUNA_GUI_API void push_id(IContext* context, u64 id);
        //! Pushes a pointer value onto the widget ID stack.
        LUNA_GUI_API void push_id(IContext* context, const void* ptr);
        //! Pushes a string value onto the widget ID stack.
        LUNA_GUI_API void push_id(IContext* context, const c8* str);
        //! Pops the current widget ID stack entry.
        LUNA_GUI_API void pop_id(IContext* context);
        //! Pushes a new layer and makes it the current build target.
        LUNA_GUI_API void push_layer(IContext* context, id_t id, const Float2U& screen_position = Float2U(0.0f));
        //! Pops the current layer.
        LUNA_GUI_API void pop_layer(IContext* context);
        //! Pushes a user clip rectangle for subsequently created nodes.
        LUNA_GUI_API void push_clip_rect(IContext* context, const RectF& rect);
        //! Pops the current user clip rectangle.
        LUNA_GUI_API void pop_clip_rect(IContext* context);
        //! @}

        //! @name Style helpers
        //! @{
        //! Defines a named style.
        LUNA_GUI_API void define_style(IContext* context, const Name& name, const Name& parent = Name());
        //! Sets the parent style of a named style.
        LUNA_GUI_API void set_style_parent(IContext* context, const Name& name, const Name& parent);
        //! Sets one style entry to an explicit value.
        LUNA_GUI_API void set_style_value(IContext* context, const Name& style, const Name& entry, const StyleValue& value);
        //! Sets one scalar f32 style entry.
        LUNA_GUI_API void set_style_f32(IContext* context, const Name& style, const Name& entry, f32 value);
        //! Sets one f32x2 style entry.
        LUNA_GUI_API void set_style_f32x2(IContext* context, const Name& style, const Name& entry, const Float2U& value);
        //! Sets one f32x3 style entry.
        LUNA_GUI_API void set_style_f32x3(IContext* context, const Name& style, const Name& entry, const Float3U& value);
        //! Sets one f32x4 style entry.
        LUNA_GUI_API void set_style_f32x4(IContext* context, const Name& style, const Name& entry, const Float4U& value);
        //! Sets one name style entry.
        LUNA_GUI_API void set_style_name(IContext* context, const Name& style, const Name& entry, const Name& value);
        //! Removes a local style entry so the value is inherited from the parent style.
        LUNA_GUI_API void inherit_style_entry(IContext* context, const Name& style, const Name& entry);
        //! Explicitly unsets a style entry and hides inherited values.
        LUNA_GUI_API void unset_style_entry(IContext* context, const Name& style, const Name& entry);
        //! Resolves a style value with a fallback.
        LUNA_GUI_API StyleValue get_style_value(IContext* context, const Name& style, const Name& entry, const StyleValue& default_value);
        //! Pushes a style onto the build style stack.
        LUNA_GUI_API void push_style(IContext* context, const Name& style);
        //! Pops the current build style.
        LUNA_GUI_API void pop_style(IContext* context);
        //! @}

        //! @name Font helpers
        //! @{
        //! Registers a font file in the context.
        LUNA_GUI_API RV register_font(IContext* context, const Name& id, Font::IFontFile* font, u32 font_index = 0);
        //! Gets a registered font by ID.
        LUNA_GUI_API FontDesc get_font(IContext* context, const Name& id);
        //! @}

        //! @name Tree and custom node helpers
        //! @{
        //! Pushes the most recently created tree node as a parent for subsequent widgets.
        LUNA_GUI_API void tree_push(IContext* context);
        //! Pushes the specified tree node as a parent for subsequent widgets.
        LUNA_GUI_API void tree_push(IContext* context, ItemHandle node);
        //! Pops the current tree parent.
        LUNA_GUI_API void tree_pop(IContext* context);
        //! Adds a custom node object to the current description.
        LUNA_GUI_API ItemHandle custom_node(IContext* context, Ref<Node> node, const c8* label = nullptr, bool interactive = false);
        //! @}

        //! @name Next item modifiers
        //! @{
        //! Assigns layout style to the next item.
        LUNA_GUI_API void set_next_item_layout(IContext* context, const LayoutStyle& style);
        //! Assigns enabled state to the next item.
        LUNA_GUI_API void set_next_item_enabled(IContext* context, bool enabled);
        //! Pushes an enabled state for subsequently created items.
        LUNA_GUI_API void push_enabled(IContext* context, bool enabled);
        //! Pops the current enabled state.
        LUNA_GUI_API void pop_enabled(IContext* context);
        //! Assigns canvas placement to the next item created inside a canvas layout.
        LUNA_GUI_API void set_next_canvas_item_layout(IContext* context, const CanvasItemLayout& layout);
        //! Assigns dock panel style and optional open state to the next dock panel item.
        LUNA_GUI_API void set_next_dock_panel_style(IContext* context, const DockPanelStyle& style, bool* open = nullptr);
        //! Assigns a render proxy to the next item.
        LUNA_GUI_API void set_next_item_render_proxy(IContext* context, const RenderProxyDesc& proxy);
        //! @}

        //! @name Layout containers
        //! @{
        //! Begins a horizontal linear layout.
        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        //! Begins an absolute-positioned horizontal linear layout.
        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        //! Ends the current horizontal layout.
        LUNA_GUI_API void end_h_layout(IContext* context);
        //! Begins a vertical linear layout.
        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        //! Begins an absolute-positioned vertical linear layout.
        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        //! Ends the current vertical layout.
        LUNA_GUI_API void end_v_layout(IContext* context);
        //! Begins a table layout.
        LUNA_GUI_API ItemHandle begin_table_layout(IContext* context, const c8* label, const TableDesc& desc);
        //! Ends the current table layout.
        LUNA_GUI_API void end_table_layout(IContext* context);
        //! Begins one table row.
        //! @param[in] context The GUI context.
        //! @return Returns `true` if cells for this row should be submitted. Returns `false` when fixed-height
        //! table virtualization skips this row; in that case, do not call @ref end_table_row.
        LUNA_GUI_API bool begin_table_row(IContext* context);
        //! Ends the current table row.
        LUNA_GUI_API void end_table_row(IContext* context);
        //! Assigns a color override to the next table cell.
        LUNA_GUI_API void set_next_table_cell_color(IContext* context, const Float4U& color);
        //! Begins a row-major grid layout.
        LUNA_GUI_API ItemHandle begin_grid_layout(IContext* context, const c8* label, const GridLayoutDesc& desc);
        //! Ends the current grid layout.
        LUNA_GUI_API void end_grid_layout(IContext* context);
        //! Begins a canvas layout.
        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label = nullptr, const Size& size = Size(), const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        //! Begins an absolute-positioned canvas layout.
        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label, const RectF& rect, const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        //! Ends the current canvas layout.
        LUNA_GUI_API void end_canvas_layout(IContext* context);
        //! Begins a dock space that manages dock panels.
        LUNA_GUI_API ItemHandle begin_dock_space(IContext* context, const c8* label, const Size& size = Size());
        //! Replaces the stored layout for a dock space by ID.
        LUNA_GUI_API void set_dockspace_layout(IContext* context, id_t dock_space, const DockSpaceLayoutDesc& desc);
        //! Replaces the stored layout for a dock space handle.
        LUNA_GUI_API void set_dockspace_layout(IContext* context, ItemHandle dock_space, const DockSpaceLayoutDesc& desc);
        //! Ends the current dock space.
        LUNA_GUI_API void end_dock_space(IContext* context);
        //! Begins a dock panel inside the current dock space.
        LUNA_GUI_API ItemHandle begin_dock_panel(IContext* context, const c8* label, bool* open = nullptr, const DockPanelStyle& style = DockPanelStyle(), const LayoutDesc& desc = LayoutDesc());
        //! Ends the current dock panel.
        LUNA_GUI_API void end_dock_panel(IContext* context);
        //! Begins a scroll view.
        LUNA_GUI_API ItemHandle begin_scroll_view(IContext* context, const c8* label, const Size& size, const ScrollViewDesc& desc = ScrollViewDesc());
        //! Ends the current scroll view.
        LUNA_GUI_API void end_scroll_view(IContext* context);
        //! Begins a top-level window-like layout helper.
        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, const Size& size = Size());
        //! Begins a top-level window-like layout helper with an open flag.
        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, bool* open, const Size& size = Size());
        //! Ends the current window helper.
        LUNA_GUI_API void end_window(IContext* context);
        //! @}

        //! @name Popup, tooltip, menu and tab helpers
        //! @{
        //! Begins a popup layer at the specified screen position.
        LUNA_GUI_API bool begin_popup(IContext* context, const c8* label, const Float2U& position, const Size& size = Size(), ItemHandle* out_handle = nullptr);
        //! Begins a popup layer using a descriptor.
        LUNA_GUI_API bool begin_popup(IContext* context, const c8* label, const PopupDesc& desc, ItemHandle* out_handle = nullptr);
        //! Ends the current popup layer.
        LUNA_GUI_API void end_popup(IContext* context);
        //! Requests a popup with the specified label to open.
        LUNA_GUI_API void open_popup(IContext* context, const c8* label);
        //! Requests a popup with the specified handle to open.
        LUNA_GUI_API void open_popup(IContext* context, ItemHandle popup);
        //! Closes a popup with the specified label.
        LUNA_GUI_API void close_popup(IContext* context, const c8* label);
        //! Closes a popup with the specified handle.
        LUNA_GUI_API void close_popup(IContext* context, ItemHandle popup);
        //! Closes the innermost currently building popup.
        LUNA_GUI_API void close_current_popup(IContext* context);
        //! Closes all popups in the current popup stack.
        LUNA_GUI_API void close_all_popups(IContext* context);
        //! Checks whether a popup with the specified label is open.
        LUNA_GUI_API bool is_popup_open(IContext* context, const c8* label);
        //! Checks whether a popup with the specified handle is open.
        LUNA_GUI_API bool is_popup_open(IContext* context, ItemHandle popup);
        //! Begins a tooltip layer for an owner item.
        LUNA_GUI_API ItemHandle begin_tooltip(IContext* context, ItemHandle owner, const c8* label = nullptr, const TooltipDesc& desc = TooltipDesc());
        //! Ends the current tooltip layer.
        LUNA_GUI_API void end_tooltip(IContext* context);
        //! Creates a simple text tooltip for an owner item.
        LUNA_GUI_API ItemHandle set_item_tooltip(IContext* context, ItemHandle owner, const c8* text, const TooltipDesc& desc = TooltipDesc());
        //! Begins a menu bar.
        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        //! Begins an absolute-positioned menu bar.
        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        //! Ends the current menu bar.
        LUNA_GUI_API void end_menu_bar(IContext* context);
        //! Begins a menu or submenu.
        LUNA_GUI_API bool begin_menu(IContext* context, const c8* label, bool enabled = true, ItemHandle* out_handle = nullptr);
        //! Ends the current menu or submenu.
        LUNA_GUI_API void end_menu(IContext* context);
        //! Adds a menu item with optional shortcut and selected marker.
        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut = nullptr, bool selected = false, bool enabled = true);
        //! Adds a menu item that toggles a boolean selected value when clicked.
        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool* selected, bool enabled = true);
        //! Adds a menu separator.
        LUNA_GUI_API ItemHandle menu_separator(IContext* context);
        //! Begins a tab bar.
        LUNA_GUI_API ItemHandle begin_tab_bar(IContext* context, const c8* label, TabBarFlag flags = TabBarFlag::fitting_shrink);
        //! Ends the current tab bar.
        LUNA_GUI_API void end_tab_bar(IContext* context);
        //! Begins a tab item and returns whether its content should be built.
        LUNA_GUI_API bool begin_tab_item(IContext* context, const c8* label, bool* open = nullptr, TabItemFlag flags = TabItemFlag::none);
        //! Ends the current tab item.
        LUNA_GUI_API void end_tab_item(IContext* context);
        //! Adds a tab-strip button.
        LUNA_GUI_API ItemHandle tab_item_button(IContext* context, const c8* label, TabItemFlag flags = TabItemFlag::none);
        //! Marks the tab item with the specified label as closed.
        LUNA_GUI_API void set_tab_item_closed(IContext* context, const c8* label);
        //! @}

        //! @name Basic widgets
        //! @{
        //! Begins an interactive button container.
        //! @param[in] context The GUI context.
        //! @param[in] label The button label used for ID generation and debug inspection.
        //! @param[in] size Optional fixed size. Width or height set to 0 uses the button content's preferred size.
        //! @return Returns the created button item handle.
        //! @remark The button itself draws the button chrome. The label is only used for ID generation and debug
        //! inspection; it is not displayed automatically. Child widgets submitted before @ref end_button are
        //! arranged inside the button. Use @ref text_button for a simple text button helper.
        LUNA_GUI_API ItemHandle begin_button(IContext* context, const c8* label, const Size& size = Size());
        //! Ends the current button container.
        //! @param[in] context The GUI context.
        LUNA_GUI_API void end_button(IContext* context);
        //! Adds a text button helper.
        //! @param[in] context The GUI context.
        //! @param[in] text The button text. This is also used for ID generation and debug inspection.
        //! @return Returns the created button item handle.
        //! @remark This creates a button container and inserts one text child using the default button-label renderer.
        LUNA_GUI_API ItemHandle text_button(IContext* context, const c8* text);
        //! Adds an absolute-positioned text button helper.
        //! @param[in] context The GUI context.
        //! @param[in] text The button text. This is also used for ID generation and debug inspection.
        //! @param[in] rect The button rectangle in layer coordinates.
        //! @return Returns the created button item handle.
        //! @remark This creates a button container and inserts one text child using the default button-label renderer.
        LUNA_GUI_API ItemHandle text_button(IContext* context, const c8* text, const RectF& rect);
        //! Adds a shape button helper.
        //! @param[in] context The GUI context.
        //! @param[in] label The button label used for ID generation and debug inspection.
        //! @param[in] shape The shape drawn as the button content.
        //! @param[in] size The requested shape content size.
        //! @return Returns the created button item handle.
        //! @remark This creates a button container and inserts one shape child.
        LUNA_GUI_API ItemHandle shape_button(IContext* context, const c8* label, ShapeDesc& shape, const Size& size);
        //! Adds a progress bar.
        //! @param[in] label The widget label used for ID generation and debug inspection.
        //! @param[in] fraction The progress fraction in the 0-1 range. Values outside the range are clamped.
        //! @param[in] size Optional fixed size. Width or height set to 0 uses the progress bar's default layout behavior.
        //! @param[in] overlay Optional text drawn over the bar. When `nullptr`, the bar displays a percentage.
        //! @return Returns the created progress bar item handle.
        LUNA_GUI_API ItemHandle progress_bar(IContext* context, const c8* label, f32 fraction, const Size& size = Size(), const c8* overlay = nullptr);
        //! Adds a selectable item.
        LUNA_GUI_API ItemHandle selectable(IContext* context, const c8* label, bool selected = false);
        //! Adds a text label.
        LUNA_GUI_API ItemHandle text(IContext* context, const c8* text);
        //! Adds a checkbox bound to a boolean value.
        LUNA_GUI_API ItemHandle checkbox(IContext* context, const c8* label, bool* value);
        //! Adds a radio button with explicit selected state.
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool selected);
        //! Adds a radio button bound to a boolean value.
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool* value);
        //! Adds a radio button bound to an integer selection value.
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, i32* value, i32 button_value);
        //! Adds an animated switch bound to a boolean value.
        LUNA_GUI_API ItemHandle toggle_switch(IContext* context, const c8* label, bool* value);
        //! Adds an editable UTF-8 text input.
        LUNA_GUI_API ItemHandle input_text(IContext* context, const c8* label, String& value);
        //! Adds an image item.
        LUNA_GUI_API ItemHandle image(IContext* context, RHI::ITexture* texture, const Size& size, ImageFlag flags = ImageFlag::none);
        //! Adds a vector shape item.
        //! @param[in] context The GUI context.
        //! @param[in] shape The VG shape description. The pointed shape buffer and texture are retained by the node.
        //! @param[in] size The requested item size.
        //! @return Returns the created shape item handle.
        LUNA_GUI_API ItemHandle shape(IContext* context, ShapeDesc& shape, const Size& size);
        //! Adds a collapsing header.
        LUNA_GUI_API ItemHandle collapsing_header(IContext* context, const c8* label);
        //! Adds a tree node.
        LUNA_GUI_API ItemHandle tree_node(IContext* context, const c8* label, TreeNodeFlag flags = TreeNodeFlag::none);
        //! Adds a single-selection button group.
        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, i32* current_item, Span<const c8*> items);
        //! Adds a multi-selection button group.
        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, Span<bool> selected, Span<const c8*> items);
        //! @}

        //! @name Numeric widgets
        //! @{
        //! Adds a scalar float slider.
        LUNA_GUI_API ItemHandle slider_float(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a two-component float slider.
        LUNA_GUI_API ItemHandle slider_float2(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a three-component float slider.
        LUNA_GUI_API ItemHandle slider_float3(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a four-component float slider.
        LUNA_GUI_API ItemHandle slider_float4(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        //! Adds a scalar integer slider.
        LUNA_GUI_API ItemHandle slider_int(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a two-component integer slider.
        LUNA_GUI_API ItemHandle slider_int2(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a three-component integer slider.
        LUNA_GUI_API ItemHandle slider_int3(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a four-component integer slider.
        LUNA_GUI_API ItemHandle slider_int4(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        //! Adds a scalar float drag editor.
        LUNA_GUI_API ItemHandle drag_float(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a two-component float drag editor.
        LUNA_GUI_API ItemHandle drag_float2(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a three-component float drag editor.
        LUNA_GUI_API ItemHandle drag_float3(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a four-component float drag editor.
        LUNA_GUI_API ItemHandle drag_float4(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a scalar integer drag editor.
        LUNA_GUI_API ItemHandle drag_int(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a two-component integer drag editor.
        LUNA_GUI_API ItemHandle drag_int2(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a three-component integer drag editor.
        LUNA_GUI_API ItemHandle drag_int3(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! Adds a four-component integer drag editor.
        LUNA_GUI_API ItemHandle drag_int4(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        //! @}

        //! @name Input state queries
        //! @{
        //! Adds an invisible hit-test rectangle.
        LUNA_GUI_API ItemHandle hit_box(IContext* context, const c8* label, const RectF& rect);
        //! Gets the current pointer position in screen coordinates.
        LUNA_GUI_API Float2U get_pointer_position(IContext* context);
        //! Checks whether a pointer button is currently down.
        LUNA_GUI_API bool is_pointer_button_down(IContext* context, PointerButton button);
        //! Checks whether a key is currently down.
        LUNA_GUI_API bool is_key_down(IContext* context, KeyCode key);
        //! Gets current keyboard modifiers.
        LUNA_GUI_API KeyModifierFlag get_key_modifiers(IContext* context);
        //! Gets the current frame descriptor.
        LUNA_GUI_API FrameDesc get_frame_desc(IContext* context);
        //! Gets the pointer delta for the current frame.
        LUNA_GUI_API Float2U get_pointer_delta(IContext* context);
        //! @}

        //! @name Drawing helpers
        //! @{
        //! Adds a custom rectangle drawing node.
        LUNA_GUI_API ItemHandle draw_rect(IContext* context, const RectF& rect, const Float4U& color, f32 radius = 0.0f);
        //! Adds a custom circle drawing node.
        LUNA_GUI_API ItemHandle draw_circle(IContext* context, const Float2U& center, f32 radius, const Float4U& color);
        //! Adds a custom line drawing node.
        LUNA_GUI_API ItemHandle draw_line(IContext* context, const Float2U& begin, const Float2U& end, const Float4U& color, f32 width = 1.0f);
        //! Adds a custom text drawing node.
        LUNA_GUI_API ItemHandle draw_text(IContext* context, const RectF& rect, const c8* text, const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            TextAlignment horizontal_alignment = TextAlignment::begin,
            TextAlignment vertical_alignment = TextAlignment::center);
        //! Adds a custom image drawing node.
        LUNA_GUI_API ItemHandle draw_image(IContext* context, RHI::ITexture* texture, const RectF& rect, const Float4U& color = Float4U(1.0f), ImageFlag flags = ImageFlag::none);
        //! @}

        //! @}
    }
}

#include "Views.hpp"
