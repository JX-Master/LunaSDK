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
        }
    }
}
