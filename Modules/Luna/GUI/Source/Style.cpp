/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Style.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void register_style_schemas(GUICore::IContext* context)
        {
            luassert(context);
            auto add = [&](const c8* name, GUICore::StyleValueType type, const GUICore::StyleValue& value,
                const c8* category, const c8* description)
            {
                GUICore::StyleEntrySchema schema;
                schema.owner = Name("gui.editor");
                schema.entry = Name(name);
                schema.type = type;
                schema.default_value = value;
                schema.category = category;
                schema.description = description;
                context->register_style_entry_schema(schema);
            };

            add("gui.font", GUICore::StyleValueType::name, GUICore::style_name(Name()),
                "Text", "Registered font used by editor controls.");
            add("gui.text.color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)), "Text", "Default text color.");
            add("gui.text.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Text", "Disabled text color.");
            add("gui.text.font_size", GUICore::StyleValueType::f32, GUICore::style_f32(16.0f),
                "Text", "Default text size.");

            add("gui.button.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.12f, 0.18f, 0.27f, 1.0f)), "Button", "Button fill.");
            add("gui.button.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Button", "Hovered button fill.");
            add("gui.button.background_active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.36f, 0.58f, 1.0f)), "Button", "Active button fill.");
            add("gui.button.background_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.09f, 0.11f, 0.14f, 1.0f)), "Button", "Disabled button fill.");
            add("gui.button.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.96f, 0.97f, 0.99f, 1.0f)), "Button", "Button text color.");
            add("gui.button.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Button", "Button corner radius.");
            add("gui.button.padding", GUICore::StyleValueType::f32x2, GUICore::style_f32x2(Float2U(10.0f, 6.0f)),
                "Button", "Horizontal and vertical content padding.");

            add("gui.group.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Button Group", "Group outline.");
            add("gui.group.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "Button Group", "Group background.");
            add("gui.group.selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.35f, 0.58f, 1.0f)), "Button Group", "Selected item fill.");
            add("gui.group.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.19f, 0.27f, 1.0f)), "Button Group", "Hovered item fill.");
            add("gui.group.radius", GUICore::StyleValueType::f32, GUICore::style_f32(5.0f),
                "Button Group", "Group corner radius.");

            add("gui.choice.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "Choice", "Choice background.");
            add("gui.choice.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.19f, 0.27f, 1.0f)), "Choice", "Hovered choice fill.");
            add("gui.choice.selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.35f, 0.58f, 1.0f)), "Choice", "Selected choice fill.");
            add("gui.choice.accent", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.42f, 0.72f, 1.0f)), "Choice", "Choice accent color.");
            add("gui.choice.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.32f, 0.40f, 0.50f, 1.0f)), "Choice", "Choice outline.");
            add("gui.choice.mark", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.92f, 0.96f, 1.0f, 1.0f)), "Choice", "Choice mark color.");
            add("gui.choice.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.30f, 0.34f, 0.40f, 1.0f)), "Choice", "Disabled choice color.");
            add("gui.switch.off", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.24f, 0.29f, 1.0f)), "Choice", "Switch off track.");
            add("gui.switch.on", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.15f, 0.50f, 0.76f, 1.0f)), "Choice", "Switch on track.");

            add("gui.disclosure.header", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.19f, 0.24f, 1.0f)), "Disclosure", "Header fill.");
            add("gui.disclosure.header_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.25f, 0.32f, 1.0f)), "Disclosure", "Hovered header fill.");
            add("gui.disclosure.icon", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.90f, 0.96f, 1.0f)), "Disclosure", "Disclosure icon color.");

            add("gui.input.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "Text Input", "Input background.");
            add("gui.input.background_focused", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.11f, 0.15f, 0.21f, 1.0f)), "Text Input", "Focused input background.");
            add("gui.input.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.27f, 0.36f, 1.0f)), "Text Input", "Input border.");
            add("gui.input.border_focused", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.12f, 0.55f, 0.86f, 1.0f)), "Text Input", "Focused input border.");
            add("gui.input.selection", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.42f, 0.70f, 0.75f)), "Text Input", "Selection fill.");
            add("gui.input.cursor", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.80f, 0.92f, 1.0f, 1.0f)), "Text Input", "Input cursor.");
            add("gui.input.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Text Input", "Input corner radius.");
            add("gui.input.padding_x", GUICore::StyleValueType::f32, GUICore::style_f32(8.0f),
                "Text Input", "Input horizontal padding.");

            add("gui.slider.track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.07f, 0.09f, 0.12f, 1.0f)), "Slider", "Slider track.");
            add("gui.slider.fill", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.42f, 0.72f, 1.0f)), "Slider", "Slider fill.");
            add("gui.slider.knob", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.32f, 0.58f, 0.90f, 1.0f)), "Slider", "Slider knob.");
            add("gui.slider.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.32f, 0.36f, 0.42f, 1.0f)), "Slider", "Disabled slider color.");
            add("gui.slider.knob_size", GUICore::StyleValueType::f32, GUICore::style_f32(12.0f),
                "Slider", "Slider knob diameter.");

            add("gui.drag.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.11f, 0.15f, 0.21f, 1.0f)), "Drag", "Drag editor fill.");
            add("gui.drag.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.14f, 0.21f, 0.30f, 1.0f)), "Drag", "Hovered drag editor fill.");
            add("gui.drag.active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.32f, 0.50f, 1.0f)), "Drag", "Active drag editor fill.");
            add("gui.drag.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.09f, 0.11f, 0.14f, 1.0f)), "Drag", "Disabled drag editor fill.");

            add("gui.progress.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.07f, 0.08f, 0.10f, 1.0f)), "Progress Bar", "Progress background.");
            add("gui.progress.fill", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.15f, 0.46f, 0.76f, 1.0f)), "Progress Bar", "Progress fill.");
            add("gui.progress.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.29f, 0.35f, 1.0f)), "Progress Bar", "Progress outline.");
            add("gui.progress.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Progress Bar", "Progress corner radius.");

            add("gui.scrollbar.thumb", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.42f, 0.50f, 0.60f, 0.72f)), "Scroll View", "Scrollbar thumb.");
            add("gui.scrollbar.track", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.04f, 0.05f, 0.07f, 0.72f)), "Scroll View", "Scrollbar track.");

            add("gui.tab_bar.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "Tab Bar", "Tab bar background.");
            add("gui.tab.selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.35f, 0.58f, 1.0f)), "Tab Bar", "Selected tab fill.");
            add("gui.tab.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.19f, 0.27f, 1.0f)), "Tab Bar", "Hovered tab fill.");
            add("gui.tab.height", GUICore::StyleValueType::f32, GUICore::style_f32(32.0f),
                "Tab Bar", "Tab header height.");
            add("gui.tab.padding_x", GUICore::StyleValueType::f32, GUICore::style_f32(14.0f),
                "Tab Bar", "Tab horizontal padding.");

            add("gui.popup.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 0.98f)), "Overlay", "Popup background.");
            add("gui.popup.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Overlay", "Popup border.");
            add("gui.popup.radius", GUICore::StyleValueType::f32, GUICore::style_f32(5.0f),
                "Overlay", "Popup corner radius.");
            add("gui.popup.padding", GUICore::StyleValueType::f32, GUICore::style_f32(7.0f),
                "Overlay", "Popup content padding.");
            add("gui.popup.gap", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Overlay", "Popup item gap.");
            add("gui.tooltip.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.05f, 0.06f, 0.07f, 0.97f)), "Overlay", "Tooltip background.");
            add("gui.tooltip.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.28f, 0.33f, 0.40f, 1.0f)), "Overlay", "Tooltip border.");
            add("gui.tooltip.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Overlay", "Tooltip corner radius.");
            add("gui.tooltip.padding", GUICore::StyleValueType::f32, GUICore::style_f32(7.0f),
                "Overlay", "Tooltip content padding.");
            add("gui.tooltip.gap", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Overlay", "Tooltip item gap.");
            add("gui.combo.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)), "Combo", "Combo background.");
            add("gui.combo.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)), "Combo", "Hovered combo background.");
            add("gui.combo.background_open", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)), "Combo", "Open combo background.");
            add("gui.combo.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Combo", "Combo outline.");
            add("gui.combo.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "Combo", "Combo text color.");
            add("gui.combo.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Combo", "Combo corner radius.");
            add("gui.combo.font_size", GUICore::StyleValueType::f32, GUICore::style_f32(15.0f),
                "Combo", "Combo text size.");
            add("gui.menu_bar.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)), "Menu", "Menu bar background.");
            add("gui.menu_bar.gap", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Menu", "Gap between top-level menus.");
            add("gui.menu_item.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.14f, 0.22f, 0.32f, 1.0f)), "Menu", "Hovered menu item fill.");
            add("gui.menu_item.active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.18f, 0.36f, 0.62f, 1.0f)), "Menu", "Active menu item fill.");
            add("gui.menu_item.text", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f)), "Menu", "Menu item text.");
            add("gui.menu_item.text_disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.50f, 0.56f, 0.64f, 1.0f)), "Menu", "Disabled menu item text.");
            add("gui.menu_item.radius", GUICore::StyleValueType::f32, GUICore::style_f32(3.0f),
                "Menu", "Menu item corner radius.");
            add("gui.menu_item.font_size", GUICore::StyleValueType::f32, GUICore::style_f32(15.0f),
                "Menu", "Menu item text size.");
            add("gui.menu_separator.color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Menu", "Menu separator color.");
        }
    }
}
