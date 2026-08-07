/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/RHI/RHI.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! Editor-oriented immediate GUI package implemented on GUI Core.
        //! @{

        //! Stable identifier used by GUI package elements and package-owned state.
        using id_t = GUICore::id_t;

        //! Horizontal or vertical text alignment inside an element.
        enum class TextAlignment : u8
        {
            //! Aligns content to the beginning edge.
            begin,
            //! Aligns content to the center.
            center,
            //! Aligns content to the ending edge.
            end
        };

        //! Selects a semantic typography role resolved from the bound Style.
        enum class TypographyRole : u8
        {
            //! Largest page or document heading.
            heading1,
            //! Second-level section heading.
            heading2,
            //! Third-level section heading.
            heading3,
            //! Fourth-level section heading.
            heading4,
            //! Fifth-level section heading.
            heading5,
            //! Smallest semantic heading.
            heading6,
            //! Primary prose and ordinary application copy.
            body,
            //! Secondary quotation, attribution, or supporting copy.
            cite,
            //! Monospaced code and numeric data.
            code,
            //! Compact metadata and captions.
            caption
        };

        //! Bit flags controlling image rendering.
        enum class ImageFlag : u8
        {
            //! Uses default image sampling.
            none = 0x00,
            //! Flips texture coordinates vertically.
            flip_y = 0x01,
            //! Uses nearest-neighbor texture sampling.
            nearest = 0x02
        };

        //! Controls scrollbar presentation in a scroll view.
        enum class ScrollBarMode : u8
        {
            //! Scrollbars fade in while scrolling or interacting and overlay content.
            dynamic_overlay,
            //! Scrollbars remain visible and reserve space beside the content viewport.
            always_visible
        };

        //! Describes text presentation.
        struct TextDesc
        {
            //! Semantic typography role used when font, size, or color is not explicitly overridden.
            TypographyRole typography = TypographyRole::body;
            //! Horizontal text alignment.
            TextAlignment horizontal_alignment = TextAlignment::begin;
            //! Vertical text alignment.
            TextAlignment vertical_alignment = TextAlignment::center;
            //! Optional text color override. A negative alpha uses the bound style value.
            Float4U color = Float4U(0.0f, 0.0f, 0.0f, -1.0f);
            //! Optional font size override. Non-positive values use the bound style value.
            f32 font_size = 0.0f;
            //! Optional registered GUI Core font ID. Empty names use the bound style value.
            Name font;
        };

        //! Describes a button container.
        struct ButtonDesc
        {
            //! Whether the button accepts input.
            bool enabled = true;
        };

        //! Describes a single-selection button group.
        struct ButtonGroupDesc
        {
            //! Whether group items accept input.
            bool enabled = true;
            //! Minimum width assigned to every item.
            f32 item_min_width = 64.0f;
        };

        //! Describes a shape element.
        struct ShapeWidgetDesc
        {
            //! Shape tint color.
            Float4U tint = Float4U(1.0f);
        };

        //! Describes a shape button.
        struct ShapeButtonDesc
        {
            //! Whether the button accepts input.
            bool enabled = true;
            //! Empty space between the shape and the button bounds.
            f32 padding = 6.0f;
            //! Shape tint color. A negative alpha uses the button text color.
            Float4U tint = Float4U(0.0f, 0.0f, 0.0f, -1.0f);
        };

        //! Describes an invisible hit-test element.
        struct HitBoxDesc
        {
            //! Whether the element accepts input.
            bool enabled = true;
        };

        //! Describes a selectable, checkbox, radio button, or toggle switch.
        struct ChoiceDesc
        {
            //! Whether the control accepts input.
            bool enabled = true;
        };

        //! Describes a disclosure control.
        struct DisclosureDesc
        {
            //! Whether the control accepts input.
            bool enabled = true;
            //! Initial state used when no persistent state exists.
            bool default_open = true;
        };

        //! Describes a draggable numeric editor.
        struct DragDesc
        {
            //! Whether the editor accepts input.
            bool enabled = true;
            //! Value change produced by one logical unit of horizontal pointer movement.
            f32 speed = 0.01f;
        };

        //! Flags controlling tree node presentation and behavior.
        enum class TreeNodeFlag : u8
        {
            //! Uses the default collapsible tree node behavior.
            none = 0x00,
            //! Renders a leaf node that cannot be expanded.
            leaf = 0x01,
            //! Keeps the node open and omits disclosure interaction.
            always_open = 0x02,
            //! Renders the node using the selected tree-item presentation.
            selected = 0x04,
            //! Toggles the open state only when the disclosure arrow is clicked.
            open_on_arrow = 0x08
        };

        //! Controls how a tab bar handles headers that exceed its available width.
        enum class TabBarFittingMode : u8
        {
            //! Keeps every header at its natural width. Overflow is clipped by the tab bar.
            none,
            //! Proportionally shrinks headers so the full strip fits the tab bar width.
            shrink
        };

        //! Describes one tab item.
        struct TabItemDesc
        {
            //! Requests this item as the selected tab during the current build.
            //! @remark When multiple items request selection, the last submitted item wins.
            bool selected = false;
        };

        //! Describes image rendering.
        struct ImageDesc
        {
            //! Image sampling flags.
            ImageFlag flags = ImageFlag::none;
            //! Image tint color.
            Float4U tint = Float4U(1.0f);
            //! Minimum texture coordinate.
            Float2U min_texcoord = Float2U(0.0f);
            //! Maximum texture coordinate.
            Float2U max_texcoord = Float2U(1.0f);
        };

        //! Describes a single-line text input.
        struct TextInputDesc
        {
            //! Whether the input accepts interaction.
            bool enabled = true;
            //! Whether editing is disabled while focus and selection remain available.
            bool read_only = false;
            //! Optional placeholder displayed while the value is empty.
            const c8* placeholder = nullptr;
        };

        //! Describes scalar slider interaction.
        struct SliderDesc
        {
            //! Whether the slider accepts interaction.
            bool enabled = true;
            //! Smallest normalized change produced by keyboard navigation.
            f32 navigation_step = 0.01f;
        };

        //! Describes progress bar presentation.
        struct ProgressBarDesc
        {
            //! Optional overlay text. Passing `nullptr` formats the percentage automatically.
            const c8* overlay = nullptr;
            //! Whether overlay text is displayed.
            bool show_overlay = true;
        };

        //! Describes a color-edit preview and its picker popup.
        struct ColorEditDesc
        {
            //! Whether the preview and picker controls accept interaction.
            bool enabled = true;
            //! Popup width in logical units. Non-positive values use the editor default.
            f32 popup_width = 0.0f;
        };

        //! Describes package-level scroll view behavior.
        struct ScrollViewDesc
        {
            //! Scrollbar display mode.
            ScrollBarMode scrollbar_mode = ScrollBarMode::dynamic_overlay;
            //! Maximum expected per-frame scroll displacement used by GUI Core visible-range queries.
            Float2U max_scroll_delta = Float2U(80.0f);
            //! Logical units moved for one unit of wheel input.
            f32 wheel_scale = 40.0f;
            //! Whether horizontal scrolling is enabled.
            bool horizontal = true;
            //! Whether vertical scrolling is enabled.
            bool vertical = true;
        };

        //! Describes a tab bar.
        struct TabBarDesc
        {
            //! Whether tab headers accept interaction.
            bool enabled = true;
            //! Header fitting behavior when the natural tab strip is wider than the bar.
            TabBarFittingMode fitting_mode = TabBarFittingMode::none;
        };

        //! Describes package-level table layout behavior.
        struct TableDesc
        {
            //! Gap between adjacent columns and rows.
            Float2U gap = Float2U(0.0f);
            //! Padding applied inside every submitted cell.
            Float4U cell_padding = Float4U(0.0f);
            //! Whether table children are clipped to the table content rectangle.
            //! @remark Disabled by default to match @ref GUICore::TableLayoutDesc::clip_children.
            bool clip_children = false;
            //! Whether all rows use @ref fixed_row_height instead of their submitted row track descriptors.
            bool fixed_row_height_mode = false;
            //! Row height used when @ref fixed_row_height_mode is enabled.
            f32 fixed_row_height = 24.0f;
            //! Whether fixed-height rows outside the previous-frame visible range may skip child submission.
            //! @remark This option is ignored unless @ref fixed_row_height_mode is enabled. The table must be
            //! rebuilt inside a clipped viewport for the optimization to reject off-screen rows.
            bool virtualize_fixed_rows = false;
            //! Whether separators between absolute-size columns can be dragged to resize the preceding column.
            bool resizable_columns = false;
            //! Interactive width of each resizable column separator.
            f32 resize_handle_width = 8.0f;
        };

        //! Bit flags controlling popup lifetime and input behavior.
        enum class PopupFlag : u8
        {
            //! Uses the default popup behavior.
            none = 0x00,
            //! Closes the popup when the primary pointer is pressed outside this popup and its descendants.
            close_on_outside_click = 0x01,
            //! Closes the popup when Escape is pressed.
            close_on_escape = 0x02
        };

        //! Describes a popup layer.
        struct PopupDesc
        {
            //! Popup top-left position in screen logical coordinates.
            Float2U position = Float2U(0.0f);
            //! Requested popup root layout.
            GUICore::LayoutConfig layout;
            //! Popup lifetime and input behavior.
            PopupFlag flags = PopupFlag::close_on_outside_click | PopupFlag::close_on_escape;
        };

        //! Describes a tooltip layer.
        struct TooltipDesc
        {
            //! Offset from the pointer position to the tooltip layer origin.
            Float2U offset = Float2U(14.0f, 18.0f);
            //! Requested tooltip root layout.
            GUICore::LayoutConfig layout;
            //! Continuous hover duration required before the tooltip appears.
            f32 delay = 0.35f;
            //! Maximum width used by simple text tooltips.
            f32 max_width = 360.0f;
        };

        //! Describes a combo box.
        struct ComboDesc
        {
            //! Whether the combo box accepts interaction.
            bool enabled = true;
            //! Popup width. Non-positive values use a package default.
            f32 popup_width = 0.0f;
            //! Maximum popup height.
            f32 popup_max_height = 320.0f;
        };

        //! Describes a menu bar.
        struct MenuBarDesc
        {
            //! Gap between top-level menu items. A negative value uses the current Style value.
            f32 gap = -1.0f;
        };

        //! Describes a menu or menu item.
        struct MenuItemDesc
        {
            //! Whether the item accepts interaction.
            bool enabled = true;
            //! Optional shortcut text displayed at the trailing edge.
            const c8* shortcut = nullptr;
        };

        //! Reports package work performed after GUI Core input routing.
        struct ResolveResult
        {
            //! Whether one or more bound application values changed.
            bool value_changed = false;
            //! Whether layout must be applied again before drawing.
            bool relayout_requested = false;
        };

        //! Gets the GUI package module object.
        LUNA_GUI_API Module* module_gui();

        //! Resolves current-frame widget actions after @ref GUICore::IContext::route_input.
        //! @param[in] context The GUI Core context containing the current element tree and routed input.
        //! @return Returns value and relayout changes produced by package controls.
        //! @remark Call this after input routing and before final draw command generation. If relayout is requested,
        //! apply layout to the layer root again before generating draw commands.
        //! Values bound to widgets by pointer or reference must remain valid through this call and the subsequent
        //! GUI Core draw-command generation for the current frame.
        LUNA_GUI_API ResolveResult resolve_interactions(GUICore::IContext* context);

        //! Applies layout to one GUI subtree.
        //! @param[in] context The GUI Core context.
        //! @param[in] root Root element to arrange.
        //! @param[in] rect Root rectangle in layer coordinates.
        //! @return Returns success or failure code.
        LUNA_GUI_API RV layout_tree(GUICore::IContext* context, const GUICore::ElementHandle& root, const RectF& rect);

        //! Checks whether an element handle is valid in the current frame.
        LUNA_GUI_API bool is_item_valid(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element was clicked by the primary pointer.
        LUNA_GUI_API bool is_item_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element received a secondary-pointer click during the current frame.
        LUNA_GUI_API bool is_item_right_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element was double-clicked by the primary pointer during the current frame.
        LUNA_GUI_API bool is_item_double_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element is hovered.
        LUNA_GUI_API bool is_item_hovered(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element is active.
        LUNA_GUI_API bool is_item_active(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Checks whether an element has keyboard focus.
        LUNA_GUI_API bool is_item_focused(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Gets the arranged rectangle of an element in layer coordinates.
        //! @return Returns an empty rectangle when @p item is invalid.
        LUNA_GUI_API RectF get_item_rect(GUICore::IContext* context, const GUICore::ElementHandle& item);
        //! Gets the effective clip rectangle of an element in layer coordinates.
        //! @return Returns an empty rectangle when @p item is invalid.
        LUNA_GUI_API RectF get_item_clip_rect(GUICore::IContext* context, const GUICore::ElementHandle& item);

        //! @}
    }
}
