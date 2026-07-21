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
            auto add_typography = [&](const c8* role, f32 font_size, const Float4U& color,
                const c8* description)
            {
                String entry;
                strprintf(entry, "gui.typography.%s.font", role);
                add(entry.c_str(), GUICore::StyleValueType::name, GUICore::style_name(Name()),
                    "Typography", description);
                strprintf(entry, "gui.typography.%s.font_size", role);
                add(entry.c_str(), GUICore::StyleValueType::f32, GUICore::style_f32(font_size),
                    "Typography", description);
                strprintf(entry, "gui.typography.%s.color", role);
                add(entry.c_str(), GUICore::StyleValueType::f32x4, GUICore::style_f32x4(color),
                    "Typography", description);
            };

            add("gui.canvas", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.922f, 0.925f, 0.918f, 1.0f)), "Palette", "Application canvas color.");
            add("gui.surface.0", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.953f, 0.953f, 0.941f, 1.0f)), "Palette", "Lowest panel surface.");
            add("gui.surface.1", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.973f, 0.973f, 0.961f, 1.0f)), "Palette", "Primary panel surface.");
            add("gui.surface.2", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.933f, 0.933f, 0.922f, 1.0f)), "Palette", "Inset control surface.");
            add("gui.surface.3", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.894f, 0.898f, 0.886f, 1.0f)), "Palette", "Raised hierarchy surface.");
            add("gui.surface.4", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.851f, 0.855f, 0.843f, 1.0f)), "Palette", "Strong hierarchy surface.");
            add("gui.surface.5", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.792f, 0.800f, 0.788f, 1.0f)), "Palette", "Highest gray surface.");
            add("gui.text.secondary", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.365f, 0.384f, 0.400f, 1.0f)), "Palette", "Secondary text color.");
            add("gui.text.muted", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.514f, 0.533f, 0.549f, 1.0f)), "Palette", "Muted metadata color.");
            add("gui.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.831f, 0.835f, 0.824f, 1.0f)), "Palette", "Default outline color.");
            add("gui.border.strong", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.765f, 0.776f, 0.761f, 1.0f)), "Palette", "Strong outline color.");
            add("gui.accent", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.890f, 0.310f, 0.349f, 1.0f)), "Accent", "Primary accent color.");
            add("gui.accent.hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.903f, 0.392f, 0.427f, 1.0f)), "Accent", "Hovered accent color.");
            add("gui.accent.active", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.748f, 0.260f, 0.293f, 1.0f)), "Accent", "Pressed accent color.");
            add("gui.accent.subtle", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.962f, 0.887f, 0.881f, 1.0f)), "Accent", "Subtle accent surface.");
            add("gui.accent.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.859f, 0.735f, 0.734f, 1.0f)), "Accent", "Disabled accent color.");
            add("gui.focus", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.684f, 0.341f, 0.373f, 1.0f)), "Accent", "Keyboard focus color.");
            add("gui.accent.ink", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.082f, 0.090f, 0.094f, 1.0f)), "Accent", "Text drawn over the accent.");
            add("gui.status.success", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.388f, 0.741f, 0.361f, 1.0f)), "Status", "Success LED color.");
            add("gui.status.busy", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.333f, 0.655f, 1.0f, 1.0f)), "Status", "Busy LED color.");
            add("gui.status.warning", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.957f, 0.678f, 0.184f, 1.0f)), "Status", "Warning LED color.");
            add("gui.status.error", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.925f, 0.275f, 0.318f, 1.0f)), "Status", "Error LED color.");
            add("gui.status.off", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.769f, 0.780f, 0.780f, 1.0f)), "Status", "Inactive LED color.");
            add("gui.control.height", GUICore::StyleValueType::f32, GUICore::style_f32(48.0f),
                "Density", "Default control and hit-target height.");
            add("gui.control.small_height", GUICore::StyleValueType::f32, GUICore::style_f32(40.0f),
                "Density", "Small control height.");
            add("gui.section.gap", GUICore::StyleValueType::f32, GUICore::style_f32(16.0f),
                "Density", "Default gap between control groups.");
            add("gui.radius.small", GUICore::StyleValueType::f32, GUICore::style_f32(7.0f),
                "Shape", "Small control corner radius.");
            add("gui.radius.medium", GUICore::StyleValueType::f32, GUICore::style_f32(12.0f),
                "Shape", "Panel corner radius.");
            add("gui.radius.large", GUICore::StyleValueType::f32, GUICore::style_f32(18.0f),
                "Shape", "Large composition corner radius.");
            add("gui.shadow.dark", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.443f, 0.467f, 0.482f, 0.20f)), "Shadow", "Dark raised shadow.");
            add("gui.shadow.light", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f, 1.0f, 1.0f, 0.96f)), "Shadow", "Light raised shadow.");
            add("gui.shadow.inset", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.471f, 0.490f, 0.506f, 0.18f)), "Shadow", "Inset shadow color.");
            add("gui.shadow.inset_light", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f, 1.0f, 1.0f, 0.90f)), "Shadow", "Inset highlight color.");
            add("gui.shadow.offset", GUICore::StyleValueType::f32x2,
                GUICore::style_f32x2(Float2U(3.0f, 3.0f)), "Shadow", "Raised shadow offset.");
            add("gui.shadow.softness", GUICore::StyleValueType::f32, GUICore::style_f32(5.0f),
                "Shadow", "Raised shadow softness.");

            add("gui.font", GUICore::StyleValueType::name, GUICore::style_name(Name()),
                "Text", "Registered font used by editor controls.");
            add("gui.text.color", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)), "Text", "Default text color.");
            add("gui.text.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.48f, 0.52f, 0.58f, 1.0f)), "Text", "Disabled text color.");
            add("gui.text.font_size", GUICore::StyleValueType::f32, GUICore::style_f32(16.0f),
                "Text", "Default text size.");
            add_typography("heading1", 32.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 1 font, size, and color.");
            add_typography("heading2", 28.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 2 font, size, and color.");
            add_typography("heading3", 24.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 3 font, size, and color.");
            add_typography("heading4", 20.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 4 font, size, and color.");
            add_typography("heading5", 18.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 5 font, size, and color.");
            add_typography("heading6", 16.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Heading level 6 font, size, and color.");
            add_typography("body", 16.0f, Float4U(0.15f, 0.16f, 0.17f, 1.0f),
                "Body font, size, and color.");
            add_typography("cite", 14.0f, Float4U(0.36f, 0.38f, 0.40f, 1.0f),
                "Citation and supporting-copy font, size, and color.");
            add_typography("code", 14.0f, Float4U(0.75f, 0.26f, 0.29f, 1.0f),
                "Code and numeric-data font, size, and color.");
            add_typography("caption", 13.0f, Float4U(0.51f, 0.53f, 0.55f, 1.0f),
                "Caption and metadata font, size, and color.");

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
            add("gui.button.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.831f, 0.835f, 0.824f, 1.0f)), "Button", "Button outline color.");
            add("gui.button.focus", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.684f, 0.341f, 0.373f, 1.0f)), "Button", "Focused button outline color.");
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
            add("gui.group.padding", GUICore::StyleValueType::f32, GUICore::style_f32(2.0f),
                "Button Group", "Inset between the group border and item fills.");
            add("gui.group.selected_radius", GUICore::StyleValueType::f32, GUICore::style_f32(8.0f),
                "Button Group", "Selected item corner radius.");

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
            add("gui.choice.indicator_size", GUICore::StyleValueType::f32, GUICore::style_f32(18.0f),
                "Choice", "Checkbox and radio indicator size.");
            add("gui.choice.disabled", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.30f, 0.34f, 0.40f, 1.0f)), "Choice", "Disabled choice color.");
            add("gui.switch.off", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.24f, 0.29f, 1.0f)), "Choice", "Switch off track.");
            add("gui.switch.on", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.15f, 0.50f, 0.76f, 1.0f)), "Choice", "Switch on track.");
            add("gui.switch.knob.off", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.96f, 0.97f, 0.99f, 1.0f)), "Choice", "Switch off-state knob fill.");
            add("gui.switch.knob.on", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f)), "Choice", "Switch on-state knob fill.");
            add("gui.switch.size", GUICore::StyleValueType::f32x2,
                GUICore::style_f32x2(Float2U(46.0f, 24.0f)), "Choice", "Switch track size.");

            add("gui.disclosure.header", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.19f, 0.24f, 1.0f)), "Disclosure", "Header fill.");
            add("gui.disclosure.header_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.20f, 0.25f, 0.32f, 1.0f)), "Disclosure", "Hovered header fill.");
            add("gui.disclosure.icon", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.86f, 0.90f, 0.96f, 1.0f)), "Disclosure", "Disclosure icon color.");
            add("gui.tree.selected", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.16f, 0.35f, 0.58f, 1.0f)), "Disclosure", "Selected tree node fill.");

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

            add("gui.color_edit.background", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)), "Color Edit", "Color preview fill.");
            add("gui.color_edit.background_hovered", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.13f, 0.19f, 0.27f, 1.0f)), "Color Edit", "Hovered color preview fill.");
            add("gui.color_edit.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Color Edit", "Color preview outline.");
            add("gui.color_edit.radius", GUICore::StyleValueType::f32, GUICore::style_f32(4.0f),
                "Color Edit", "Color preview corner radius.");
            add("gui.color_picker.border", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(0.24f, 0.30f, 0.38f, 1.0f)), "Color Edit", "Color picker outline.");
            add("gui.color_picker.cursor", GUICore::StyleValueType::f32x4,
                GUICore::style_f32x4(Float4U(1.0f, 1.0f, 1.0f, 1.0f)), "Color Edit", "Color picker cursor outline.");

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

            set_default_style(context);
        }

        namespace
        {
            Float4U color(u32 value, f32 alpha = 1.0f)
            {
                return Float4U((f32)((value >> 16) & 0xff) / 255.0f,
                    (f32)((value >> 8) & 0xff) / 255.0f, (f32)(value & 0xff) / 255.0f, alpha);
            }

            Float4U mix(const Float4U& a, const Float4U& b, f32 factor)
            {
                return a + (b - a) * clamp(factor, 0.0f, 1.0f);
            }
        }

        LUNA_GUI_API void configure_style(GUICore::IContext* context, const Name& style,
            const DefaultStyleDesc& desc)
        {
            luassert(context && !style.empty());
            context->define_style(style);
            auto set_color = [&](const c8* entry, const Float4U& value)
            {
                context->set_style_value(style, Name(entry), GUICore::style_f32x4(value));
            };
            auto set_scalar = [&](const c8* entry, f32 value)
            {
                context->set_style_value(style, Name(entry), GUICore::style_f32(value));
            };
            auto set_vector2 = [&](const c8* entry, const Float2U& value)
            {
                context->set_style_value(style, Name(entry), GUICore::style_f32x2(value));
            };
            auto set_name = [&](const c8* entry, const Name& value)
            {
                context->set_style_value(style, Name(entry), GUICore::style_name(value));
            };
            auto set_typography = [&](const c8* role, const Name& font, f32 font_size,
                const Float4U& color)
            {
                String entry;
                strprintf(entry, "gui.typography.%s.font", role);
                set_name(entry.c_str(), font);
                strprintf(entry, "gui.typography.%s.font_size", role);
                set_scalar(entry.c_str(), font_size);
                strprintf(entry, "gui.typography.%s.color", role);
                set_color(entry.c_str(), color);
            };

            const bool dark = desc.color_theme == ColorTheme::dark;
            const bool touch = desc.input_mode == InputMode::touch;
            const Float4U canvas = color(dark ? 0x17191a : 0xebecea);
            const Float4U surface0 = color(dark ? 0x1c1f20 : 0xf3f3f0);
            const Float4U surface1 = color(dark ? 0x222526 : 0xf8f8f5);
            const Float4U surface2 = color(dark ? 0x272a2b : 0xeeeeeb);
            const Float4U surface3 = color(dark ? 0x2e3233 : 0xe4e5e2);
            const Float4U surface4 = color(dark ? 0x383c3d : 0xd9dad7);
            const Float4U surface5 = color(dark ? 0x464b4c : 0xcaccc9);
            const Float4U text = color(dark ? 0xecebe7 : 0x26282b);
            const Float4U secondary = color(dark ? 0xb7b8b4 : 0x5d6266);
            const Float4U muted = color(dark ? 0x858986 : 0x83888c);
            const Float4U disabled = color(dark ? 0x666a68 : 0xafb2b3);
            const Float4U border = color(dark ? 0x333738 : 0xd4d5d2);
            const Float4U border_strong = color(dark ? 0x474b4c : 0xc3c6c2);
            Float4U accent = desc.accent;
            accent.x = clamp(accent.x, 0.0f, 1.0f);
            accent.y = clamp(accent.y, 0.0f, 1.0f);
            accent.z = clamp(accent.z, 0.0f, 1.0f);
            accent.w = 1.0f;
            const Float4U accent_hovered = mix(accent, Float4U(1.0f), 0.12f);
            const Float4U accent_active = mix(accent, Float4U(0.0f, 0.0f, 0.0f, 1.0f), 0.16f);
            const Float4U accent_subtle = mix(surface1, accent, 0.13f);
            const Float4U accent_disabled = mix(surface4, accent, 0.22f);
            const Float4U focus = mix(text, accent, dark ? 0.58f : 0.52f);
            const f32 luminance = accent.x * 0.2126f + accent.y * 0.7152f + accent.z * 0.0722f;
            const Float4U accent_ink = luminance > 0.40f ? color(0x151718) : Float4U(1.0f);
            const f32 control_height = touch ? 48.0f : 32.0f;
            const f32 small_height = touch ? 40.0f : 28.0f;
            const f32 radius_small = touch ? 7.0f : 6.0f;
            const f32 radius_medium = touch ? 12.0f : 9.0f;
            const f32 radius_large = touch ? 18.0f : 14.0f;

            set_color("gui.canvas", canvas);
            set_color("gui.surface.0", surface0);
            set_color("gui.surface.1", surface1);
            set_color("gui.surface.2", surface2);
            set_color("gui.surface.3", surface3);
            set_color("gui.surface.4", surface4);
            set_color("gui.surface.5", surface5);
            set_color("gui.text.color", text);
            set_color("gui.text.secondary", secondary);
            set_color("gui.text.muted", muted);
            set_color("gui.text.disabled", disabled);
            set_color("gui.border", border);
            set_color("gui.border.strong", border_strong);
            set_color("gui.accent", accent);
            set_color("gui.accent.hovered", accent_hovered);
            set_color("gui.accent.active", accent_active);
            set_color("gui.accent.subtle", accent_subtle);
            set_color("gui.accent.disabled", accent_disabled);
            set_color("gui.focus", focus);
            set_color("gui.accent.ink", accent_ink);
            set_color("gui.status.success", color(0x63bd5c));
            set_color("gui.status.busy", color(0x55a7ff));
            set_color("gui.status.warning", color(0xf4ad2f));
            set_color("gui.status.error", color(0xec4651));
            set_color("gui.status.off", color(dark ? 0x515554 : 0xc4c7c7));
            set_scalar("gui.control.height", control_height);
            set_scalar("gui.control.small_height", small_height);
            set_scalar("gui.section.gap", touch ? 18.0f : 13.0f);
            set_scalar("gui.radius.small", radius_small);
            set_scalar("gui.radius.medium", radius_medium);
            set_scalar("gui.radius.large", radius_large);
            set_color("gui.shadow.dark", dark ? Float4U(0.0f, 0.0f, 0.0f, 0.48f) : color(0x71777b, 0.20f));
            set_color("gui.shadow.light", dark ? Float4U(1.0f, 1.0f, 1.0f, 0.025f) : Float4U(1.0f, 1.0f, 1.0f, 0.96f));
            set_color("gui.shadow.inset", dark ? Float4U(0.0f, 0.0f, 0.0f, 0.48f) : color(0x787d81, 0.18f));
            set_color("gui.shadow.inset_light", Float4U(1.0f, 1.0f, 1.0f, dark ? 0.025f : 0.90f));
            set_vector2("gui.shadow.offset", Float2U(touch ? 3.0f : 2.0f));
            set_scalar("gui.shadow.softness", touch ? 5.0f : 4.0f);
            context->set_style_value(style, Name("gui.font"), GUICore::style_name(desc.font));
            // Open Sans has a smaller apparent x-height than the Inter face used by the HTML
            // reference. Keep the density relationship while compensating the default UI size.
            set_scalar("gui.text.font_size", touch ? 16.0f : 15.0f);
            set_typography("heading1", desc.font, touch ? 32.0f : 30.0f, text);
            set_typography("heading2", desc.font, touch ? 28.0f : 26.0f, text);
            set_typography("heading3", desc.font, touch ? 24.0f : 22.0f, text);
            set_typography("heading4", desc.font, touch ? 20.0f : 19.0f, text);
            set_typography("heading5", desc.font, touch ? 18.0f : 17.0f, text);
            set_typography("heading6", desc.font, touch ? 16.0f : 15.0f, text);
            set_typography("body", desc.font, touch ? 16.0f : 15.0f, text);
            set_typography("cite", desc.font, touch ? 14.0f : 13.0f, secondary);
            set_typography("code", desc.monospace_font, touch ? 14.0f : 13.0f, accent_active);
            set_typography("caption", desc.font, touch ? 13.0f : 12.0f, muted);

            set_color("gui.button.background", surface1);
            set_color("gui.button.background_hovered", mix(surface1, accent_subtle, 0.45f));
            set_color("gui.button.background_active", surface2);
            set_color("gui.button.background_disabled", surface2);
            set_color("gui.button.text", text);
            set_color("gui.button.border", border);
            set_color("gui.button.focus", focus);
            set_scalar("gui.button.radius", radius_small);
            set_vector2("gui.button.padding", Float2U(touch ? 16.0f : 12.0f, touch ? 14.0f : 8.0f));

            set_color("gui.group.border", border);
            set_color("gui.group.background", surface2);
            set_color("gui.group.selected", accent);
            set_color("gui.group.hovered", surface4);
            set_scalar("gui.group.radius", radius_medium);
            set_scalar("gui.group.padding", touch ? 2.0f : 3.0f);
            set_scalar("gui.group.selected_radius", touch ? 8.0f : 6.0f);

            set_color("gui.choice.background", surface0);
            set_color("gui.choice.hovered", surface2);
            set_color("gui.choice.selected", accent_subtle);
            set_color("gui.choice.accent", accent);
            set_color("gui.choice.border", border_strong);
            set_color("gui.choice.mark", accent_ink);
            set_scalar("gui.choice.indicator_size", touch ? 22.0f : 18.0f);
            set_color("gui.choice.disabled", surface4);
            set_color("gui.switch.off", surface3);
            set_color("gui.switch.on", accent);
            set_color("gui.switch.knob.off", dark ? surface1 : Float4U(1.0f));
            set_color("gui.switch.knob.on", dark ? accent_ink : Float4U(1.0f));
            set_vector2("gui.switch.size", touch ? Float2U(52.0f, 32.0f) : Float2U(46.0f, 24.0f));

            set_color("gui.disclosure.header", surface2);
            set_color("gui.disclosure.header_hovered", surface3);
            set_color("gui.disclosure.icon", secondary);
            set_color("gui.tree.selected", accent_subtle);

            set_color("gui.input.background", surface0);
            set_color("gui.input.background_focused", surface1);
            set_color("gui.input.border", border);
            set_color("gui.input.border_focused", accent);
            set_color("gui.input.selection", Float4U(accent.x, accent.y, accent.z, 0.32f));
            set_color("gui.input.cursor", accent_active);
            set_scalar("gui.input.radius", radius_small);
            set_scalar("gui.input.padding_x", touch ? 12.0f : 10.0f);

            set_color("gui.color_edit.background", surface0);
            set_color("gui.color_edit.background_hovered", surface2);
            set_color("gui.color_edit.border", border);
            set_scalar("gui.color_edit.radius", radius_small);
            set_color("gui.color_picker.border", border_strong);
            set_color("gui.color_picker.cursor", text);

            set_color("gui.slider.track", surface3);
            set_color("gui.slider.fill", accent_active);
            set_color("gui.slider.knob", accent);
            set_color("gui.slider.disabled", surface5);
            set_scalar("gui.slider.knob_size", touch ? 18.0f : 14.0f);
            set_color("gui.drag.background", surface0);
            set_color("gui.drag.hovered", surface2);
            set_color("gui.drag.active", accent_subtle);
            set_color("gui.drag.disabled", surface2);

            set_color("gui.progress.background", surface3);
            set_color("gui.progress.fill", accent);
            set_color("gui.progress.border", border);
            set_scalar("gui.progress.radius", radius_small);
            set_color("gui.scrollbar.thumb", Float4U(surface5.x, surface5.y, surface5.z, 0.82f));
            set_color("gui.scrollbar.track", Float4U(surface2.x, surface2.y, surface2.z, 0.78f));

            set_color("gui.tab_bar.background", surface1);
            set_color("gui.tab.selected", accent_subtle);
            set_color("gui.tab.hovered", surface2);
            set_scalar("gui.tab.height", small_height);
            set_scalar("gui.tab.padding_x", touch ? 16.0f : 12.0f);

            set_color("gui.popup.background", Float4U(surface1.x, surface1.y, surface1.z, 0.96f));
            set_color("gui.popup.border", border_strong);
            set_scalar("gui.popup.radius", radius_medium);
            set_scalar("gui.popup.padding", touch ? 12.0f : 7.0f);
            set_scalar("gui.popup.gap", touch ? 7.0f : 4.0f);
            set_color("gui.tooltip.background", Float4U(surface1.x, surface1.y, surface1.z, 0.97f));
            set_color("gui.tooltip.border", border_strong);
            set_scalar("gui.tooltip.radius", radius_small);
            set_scalar("gui.tooltip.padding", touch ? 10.0f : 7.0f);
            set_scalar("gui.tooltip.gap", touch ? 7.0f : 4.0f);

            set_color("gui.combo.background", surface0);
            set_color("gui.combo.background_hovered", surface2);
            set_color("gui.combo.background_open", accent_subtle);
            set_color("gui.combo.border", border);
            set_color("gui.combo.text", text);
            set_scalar("gui.combo.radius", radius_small);
            set_scalar("gui.combo.font_size", touch ? 14.0f : 13.0f);
            set_color("gui.menu_bar.background", surface1);
            set_scalar("gui.menu_bar.gap", touch ? 6.0f : 4.0f);
            set_color("gui.menu_item.hovered", accent_subtle);
            set_color("gui.menu_item.active", accent);
            set_color("gui.menu_item.text", text);
            set_color("gui.menu_item.text_disabled", disabled);
            set_scalar("gui.menu_item.radius", radius_small);
            set_scalar("gui.menu_item.font_size", touch ? 14.0f : 13.0f);
            set_color("gui.menu_separator.color", border);
        }

        LUNA_GUI_API void set_default_style(GUICore::IContext* context, const DefaultStyleDesc& desc)
        {
            configure_style(context, Name(DEFAULT_STYLE_NAME), desc);
        }
    }
}
