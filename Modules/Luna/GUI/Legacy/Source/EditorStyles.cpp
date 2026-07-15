/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorStyles.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/Legacy/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void register_editor_style_schemas(GUICore::IContext* context)
        {
            luassert(context);
            auto add = [&](const c8* entry, GUICore::StyleValueType type, const GUICore::StyleValue& default_value,
                const c8* category, const c8* description) {
                GUICore::StyleEntrySchema schema;
                schema.owner = Name("gui.editor");
                schema.entry = Name(entry);
                schema.type = type;
                schema.default_value = default_value;
                schema.category = category ? category : "";
                schema.description = description ? description : "";
                context->register_style_entry_schema(schema);
            };
            add("gui.editor.text.color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)), "Text", "Default text color.");
            add("gui.editor.text.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Text", "Disabled text color.");
            add("gui.editor.text.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(16.0f), "Text", "Default text font size.");
            add("gui.editor.button.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.12f, 0.18f, 0.27f, 1.0f)), "Button", "Button background color.");
            add("gui.editor.button.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Button", "Hovered button background color.");
            add("gui.editor.button.background_active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.36f, 0.58f, 1.0f)), "Button", "Active button background color.");
            add("gui.editor.button.background_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.09f, 0.11f, 0.14f, 1.0f)), "Button", "Disabled button background color.");
            add("gui.editor.button.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Button", "Button text color.");
            add("gui.editor.button.text_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Button", "Disabled button text color.");
            add("gui.editor.button.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(16.0f), "Button", "Button text font size.");
            add("gui.editor.button.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(5.0f), "Button", "Button corner radius.");
            add("gui.editor.shape_button.icon", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Button", "Shape button icon color.");
            add("gui.editor.shape_button.icon_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Button", "Disabled shape button icon color.");
            add("gui.editor.selection.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Selection", "Selected item background color.");
            add("gui.editor.selection.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Selection", "Selected item text color.");
            add("gui.editor.selection.text_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Selection", "Disabled selected item text color.");
            add("gui.editor.check.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.55f, 0.64f, 0.76f, 1.0f)), "Check", "Checkbox and radio border color.");
            add("gui.editor.check.fill", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.42f, 0.72f, 1.0f)), "Check", "Checked fill color.");
            add("gui.editor.check.mark", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Check", "Checked mark color.");
            add("gui.editor.check.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.34f, 0.38f, 0.44f, 1.0f)), "Check", "Disabled checkbox and radio color.");
            add("gui.editor.switch.animation_speed", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Switch", "Switch animation speed.");
            add("gui.editor.switch.track_size", GUICore::StyleValueType::f32x2,
                GUICore::style_f32x2(Float2U(44.0f, 22.0f)), "Switch", "Switch track size.");
            add("gui.editor.switch.knob_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(18.0f), "Switch", "Switch knob size.");
            add("gui.editor.switch.knob_margin", GUICore::StyleValueType::f32,
                GUICore::style_f32(2.0f), "Switch", "Switch knob margin.");
            add("gui.editor.switch.off_track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.12f, 0.14f, 0.16f, 1.0f)), "Switch", "Switch off-track color.");
            add("gui.editor.switch.on_track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.55f, 0.32f, 1.0f)), "Switch", "Switch on-track color.");
            add("gui.editor.switch.off_knob", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.78f, 0.80f, 0.84f, 1.0f)), "Switch", "Switch off-knob color.");
            add("gui.editor.switch.on_knob", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Switch", "Switch on-knob color.");
            add("gui.editor.switch.disabled_track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.15f, 0.18f, 1.0f)), "Switch", "Disabled switch track color.");
            add("gui.editor.switch.disabled_knob", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.42f, 0.46f, 0.52f, 1.0f)), "Switch", "Disabled switch knob color.");
            add("gui.editor.switch.label_offset", GUICore::StyleValueType::f32,
                GUICore::style_f32(56.0f), "Switch", "Switch label offset.");
            add("gui.editor.switch.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(16.0f), "Switch", "Switch label font size.");
            add("gui.editor.input_text.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "InputText", "Text input background color.");
            add("gui.editor.input_text.background_focused", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.12f, 0.16f, 0.22f, 1.0f)), "InputText", "Focused text input background color.");
            add("gui.editor.input_text.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.24f, 0.32f, 1.0f)), "InputText", "Text input border color.");
            add("gui.editor.input_text.border_focused", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.55f, 0.86f, 1.0f)), "InputText", "Focused text input border color.");
            add("gui.editor.input_text.border_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "InputText", "Text input border size.");
            add("gui.editor.input_text.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "InputText", "Text input corner radius.");
            add("gui.editor.input_text.padding_x", GUICore::StyleValueType::f32,
                GUICore::style_f32(8.0f), "InputText", "Text input horizontal padding.");
            add("gui.editor.input_text.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "InputText", "Text input text color.");
            add("gui.editor.input_text.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(16.0f), "InputText", "Text input font size.");
            add("gui.editor.input_text.cursor", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.75f, 0.88f, 1.0f, 1.0f)), "InputText", "Text input cursor color.");
            add("gui.editor.collapsing_header.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.19f, 0.24f, 1.0f)), "Collapsing Header", "Collapsing header background color.");
            add("gui.editor.collapsing_header.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.25f, 0.32f, 1.0f)), "Collapsing Header", "Hovered collapsing header background color.");
            add("gui.editor.collapsing_header.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Collapsing Header", "Collapsing header corner radius.");
            add("gui.editor.collapsing_header.icon_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.90f, 0.96f, 1.0f)), "Collapsing Header", "Collapsing header disclosure icon color.");
            add("gui.editor.collapsing_header.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Collapsing Header", "Collapsing header text color.");
            add("gui.editor.collapsing_header.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(16.0f), "Collapsing Header", "Collapsing header text font size.");
            add("gui.editor.tree.selected_background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Tree", "Selected tree node background color.");
            add("gui.editor.tree.hovered_background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.17f, 0.22f, 1.0f)), "Tree", "Hovered tree node background color.");
            add("gui.editor.tree.icon_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Tree", "Tree node disclosure icon color.");
            add("gui.editor.tree.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Tree", "Tree node text color.");
            add("gui.editor.tree.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(15.0f), "Tree", "Tree node text font size.");
            add("gui.editor.progress.border_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Progress", "Progress bar border size.");
            add("gui.editor.progress.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(5.0f), "Progress", "Progress bar corner radius.");
            add("gui.editor.progress.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.25f, 0.29f, 0.35f, 1.0f)), "Progress", "Progress bar border color.");
            add("gui.editor.progress.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.07f, 0.08f, 0.10f, 1.0f)), "Progress", "Progress bar background color.");
            add("gui.editor.progress.fill", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.36f, 0.62f, 1.0f)), "Progress", "Progress bar fill color.");
            add("gui.editor.progress.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Progress", "Progress bar overlay font size.");
            add("gui.editor.progress.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Progress", "Progress bar overlay text color.");
            add("gui.editor.slider.track_width", GUICore::StyleValueType::f32,
                GUICore::style_f32(2.0f), "Slider", "Slider track line width.");
            add("gui.editor.slider.fill_width", GUICore::StyleValueType::f32,
                GUICore::style_f32(2.0f), "Slider", "Slider filled track line width.");
            add("gui.editor.slider.knob_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(12.0f), "Slider", "Slider knob size.");
            add("gui.editor.slider.track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.07f, 0.09f, 0.12f, 1.0f)), "Slider", "Slider track color.");
            add("gui.editor.slider.fill", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.42f, 0.72f, 1.0f)), "Slider", "Slider fill color.");
            add("gui.editor.slider.knob", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.32f, 0.55f, 0.88f, 1.0f)), "Slider", "Slider knob color.");
            add("gui.editor.drag.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)), "Drag", "Drag editor background color.");
            add("gui.editor.drag.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)), "Drag", "Hovered drag editor background color.");
            add("gui.editor.drag.background_active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.27f, 0.42f, 1.0f)), "Drag", "Active drag editor background color.");
            add("gui.editor.drag.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.25f, 0.34f, 1.0f)), "Drag", "Drag editor border color.");
            add("gui.editor.drag.border_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Drag", "Drag editor border size.");
            add("gui.editor.drag.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Drag", "Drag editor corner radius.");
            add("gui.editor.drag.padding_x", GUICore::StyleValueType::f32,
                GUICore::style_f32(8.0f), "Drag", "Drag editor horizontal text padding.");
            add("gui.editor.drag.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "Drag", "Drag editor text color.");
            add("gui.editor.drag.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Drag", "Drag editor font size.");
            add("gui.editor.tab_bar.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.75f)), "Tab", "Tab bar background color.");
            add("gui.editor.tab_bar.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Tab", "Tab bar corner radius.");
            add("gui.editor.tab_bar.header_height", GUICore::StyleValueType::f32,
                GUICore::style_f32(30.0f), "Tab", "Tab bar header height.");
            add("gui.editor.tab_bar.content_gap", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Tab", "Tab bar content gap.");
            add("gui.editor.tab.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.10f, 0.12f, 0.16f, 1.0f)), "Tab", "Tab header background color.");
            add("gui.editor.tab.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.25f, 1.0f)), "Tab", "Hovered tab header background color.");
            add("gui.editor.tab.background_selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Tab", "Selected tab header background color.");
            add("gui.editor.tab.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.72f, 0.78f, 0.86f, 1.0f)), "Tab", "Tab header text color.");
            add("gui.editor.tab.text_selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Tab", "Selected tab header text color.");
            add("gui.editor.tab.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Tab", "Tab header corner radius.");
            add("gui.editor.tab.padding_x", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Tab", "Tab header horizontal padding.");
            add("gui.editor.tab.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(15.0f), "Tab", "Tab header font size.");
            add("gui.editor.popup.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.98f)), "Popup", "Popup background color.");
            add("gui.editor.popup.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Popup", "Popup border color.");
            add("gui.editor.popup.border_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Popup", "Popup border size.");
            add("gui.editor.popup.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(5.0f), "Popup", "Popup corner radius.");
            add("gui.editor.popup.padding", GUICore::StyleValueType::f32,
                GUICore::style_f32(6.0f), "Popup", "Popup content padding.");
            add("gui.editor.popup.gap", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Popup", "Popup child gap.");
            add("gui.editor.tooltip.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.05f, 0.06f, 0.07f, 0.97f)), "Tooltip", "Tooltip background color.");
            add("gui.editor.tooltip.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.28f, 0.33f, 0.40f, 1.0f)), "Tooltip", "Tooltip border color.");
            add("gui.editor.tooltip.border_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Tooltip", "Tooltip border size.");
            add("gui.editor.tooltip.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Tooltip", "Tooltip corner radius.");
            add("gui.editor.tooltip.padding", GUICore::StyleValueType::f32,
                GUICore::style_f32(6.0f), "Tooltip", "Tooltip content padding.");
            add("gui.editor.tooltip.gap", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Tooltip", "Tooltip child gap.");
            add("gui.editor.combo.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)), "Combo", "Combo preview background color.");
            add("gui.editor.combo.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)), "Combo", "Hovered combo preview background color.");
            add("gui.editor.combo.background_open", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Combo", "Open combo preview background color.");
            add("gui.editor.combo.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.25f, 0.34f, 1.0f)), "Combo", "Combo preview border color.");
            add("gui.editor.combo.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "Combo", "Combo preview text color.");
            add("gui.editor.combo.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Combo", "Combo preview corner radius.");
            add("gui.editor.combo.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(15.0f), "Combo", "Combo preview font size.");
            add("gui.editor.menu_bar.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.75f)), "Menu", "Menu bar background color.");
            add("gui.editor.menu_bar.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.27f, 0.36f, 1.0f)), "Menu", "Menu bar bottom border color.");
            add("gui.editor.menu_bar.border_width", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Menu", "Menu bar bottom border width.");
            add("gui.editor.menu_bar.gap", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Menu", "Menu bar item gap.");
            add("gui.editor.menu_item.text_color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "Menu", "Menu item text color.");
            add("gui.editor.menu_item.text_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.50f, 0.56f, 0.64f, 1.0f)), "Menu", "Disabled menu item text color.");
            add("gui.editor.menu_item.background_active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.36f, 0.62f, 1.0f)), "Menu", "Active menu item background color.");
            add("gui.editor.menu_item.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.14f, 0.22f, 0.32f, 1.0f)), "Menu", "Hovered menu item background color.");
            add("gui.editor.menu_item.top_level_radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(4.0f), "Menu", "Top-level menu item corner radius.");
            add("gui.editor.menu_item.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(3.0f), "Menu", "Popup menu item corner radius.");
            add("gui.editor.menu_item.check_width", GUICore::StyleValueType::f32,
                GUICore::style_f32(30.0f), "Menu", "Popup menu item checkmark column width.");
            add("gui.editor.menu_item.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(15.0f), "Menu", "Menu item font size.");
            add("gui.editor.menu_item.shortcut_font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Menu", "Menu item shortcut font size.");
            add("gui.editor.menu_item.shortcut_alpha", GUICore::StyleValueType::f32,
                GUICore::style_f32(0.72f), "Menu", "Menu item shortcut alpha multiplier.");
            add("gui.editor.menu_separator.padding", GUICore::StyleValueType::f32,
                GUICore::style_f32(8.0f), "Menu", "Menu separator horizontal padding.");
            add("gui.editor.menu_separator.color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Menu", "Menu separator color.");
            add("gui.editor.menu_separator.width", GUICore::StyleValueType::f32,
                GUICore::style_f32(1.0f), "Menu", "Menu separator line width.");
            add("gui.button_group.radius", GUICore::StyleValueType::f32,
                GUICore::style_f32(5.0f), "Button Group", "Segmented button group corner radius.");
            add("gui.button_group.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.28f, 0.36f, 0.46f, 1.0f)), "Button Group", "Segmented button group border color.");
            add("gui.button_group.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.07f, 0.09f, 0.12f, 1.0f)), "Button Group", "Segmented button group background color.");
            add("gui.button_group.selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.31f, 0.50f, 1.0f)), "Button Group", "Selected segment color.");
            add("gui.button_group.selected_hot", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.40f, 0.64f, 1.0f)), "Button Group", "Hovered selected segment color.");
            add("gui.button_group.selected_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.15f, 0.18f, 0.22f, 1.0f)), "Button Group", "Disabled selected segment color.");
            add("gui.button_group.hover", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.14f, 0.20f, 0.28f, 1.0f)), "Button Group", "Hovered segment color.");
            add("gui.button_group.separator", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.28f, 0.36f, 0.46f, 1.0f)), "Button Group", "Segment separator color.");
            add("gui.button_group.text_selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Button Group", "Selected segment text color.");
            add("gui.button_group.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.72f, 0.78f, 0.86f, 1.0f)), "Button Group", "Segment text color.");
            add("gui.button_group.text_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Button Group", "Disabled segment text color.");
            add("gui.button_group.font_size", GUICore::StyleValueType::f32,
                GUICore::style_f32(15.0f), "Button Group", "Segment text font size.");
            add("gui.button_group.animation_speed", GUICore::StyleValueType::f32,
                GUICore::style_f32(14.0f), "Button Group", "Segment animation speed.");
        }

    }
}
