/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorVisualEffects.cpp
* @author JXMaster
* @date 2026/9/3
*/
#include "EditorApp.hpp"

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            namespace
            {
                constexpr const c8* EFFECT_TYPE_NAMES[] = {
                    "rect", "gradient_rect", "rounded_rect", "rounded_rect_stroke", "shadow"
                };
                constexpr const c8* EFFECT_TYPE_LABELS[] = {
                    "Rectangle", "Gradient Rectangle", "Rounded Rectangle",
                    "Rounded Rectangle Stroke", "Shadow"
                };
                constexpr usize EFFECT_TYPE_COUNT =
                    sizeof(EFFECT_TYPE_NAMES) / sizeof(EFFECT_TYPE_NAMES[0]);
                constexpr const c8* EFFECT_PHASE_NAMES[] = {
                    "before_children", "after_children"
                };
                constexpr const c8* EFFECT_PHASE_LABELS[] = {
                    "Before Children", "After Children"
                };

                i32 find_name(const Name& name, const c8* const* names, usize count)
                {
                    for(usize i = 0; i < count; ++i)
                    {
                        if(name == Name(names[i])) return (i32)i;
                    }
                    return -1;
                }

                void decode_float_array(const Variant& value, f32* output, usize count)
                {
                    if(value.type() != VariantType::array || value.size() != count) return;
                    for(usize i = 0; i < count; ++i)
                    {
                        if(value[i].type() == VariantType::number)
                            output[i] = (f32)value[i].fnum();
                    }
                }

                Variant make_float_array(const f32* values, usize count)
                {
                    Variant result(VariantType::array);
                    for(usize i = 0; i < count; ++i) result.push_back((f64)values[i]);
                    return result;
                }

                void erase_known_effect_fields(Variant& value)
                {
                    const c8* fields[] = {
                        "phase", "type", "inset", "color", "color_top_right",
                        "color_bottom_right", "color_bottom_left", "radius", "line_width",
                        "shadow_offset", "shadow_softness", "shadow_spread", "shadow_mode"
                    };
                    for(const c8* field : fields) value.erase(Name(field));
                }

                GUI::LayoutConfig fixed_no_shrink(f32 width, f32 height)
                {
                    GUI::LayoutConfig result = fixed_layout(width, height);
                    result.flex_shrink = 0.0f;
                    return result;
                }

                void add_action(UIHandles& handles, usize property_index, usize effect_index,
                    VisualEffectAction action, const GUI::ElementHandle& element)
                {
                    VisualEffectActionHit hit;
                    hit.property_index = property_index;
                    hit.effect_index = effect_index;
                    hit.action = action;
                    hit.element = element;
                    handles.visual_effect_actions.push_back(hit);
                }

                GUI::ElementHandle labeled_number(EditorApp& app, GUI::id_t scope,
                    const c8* id, const c8* label, f32* value, f32 minimum = 0.0f,
                    f32 maximum = 0.0f)
                {
                    GUI::ElementHandle row = EditorGUI::begin_h_layout(app.gui,
                        GUI::make_scoped_id(scope, id), label, fill_width(28.0f));
                    EditorGUI::text(app.gui, GUI::make_scoped_id(scope, label), label,
                        fixed_no_shrink(104.0f, 28.0f));
                    EditorGUI::DragDesc desc;
                    desc.speed = 1.0f;
                    String value_id = id;
                    value_id.append(".value");
                    GUI::ElementHandle result = EditorGUI::drag_float(app.gui,
                        GUI::make_scoped_id(scope, value_id.c_str()), value,
                        minimum, maximum, fill_width(28.0f), desc);
                    EditorGUI::end_h_layout(app.gui, row);
                    return result;
                }

                GUI::ElementHandle labeled_color(EditorApp& app, GUI::id_t scope,
                    const c8* id, const c8* label, f32* value)
                {
                    GUI::ElementHandle row = EditorGUI::begin_h_layout(app.gui,
                        GUI::make_scoped_id(scope, id), label, fill_width(30.0f));
                    EditorGUI::text(app.gui, GUI::make_scoped_id(scope, label), label,
                        fixed_no_shrink(104.0f, 28.0f));
                    String value_id = id;
                    value_id.append(".value");
                    GUI::ElementHandle result = EditorGUI::color_edit4(app.gui,
                        GUI::make_scoped_id(scope, value_id.c_str()),
                        label, value, fill_width(28.0f));
                    EditorGUI::end_h_layout(app.gui, row);
                    return result;
                }

                void labeled_combo(EditorApp& app, GUI::id_t scope, const c8* id,
                    const c8* label, i32* selected, const c8* const* items, usize count)
                {
                    GUI::ElementHandle row = EditorGUI::begin_h_layout(app.gui,
                        GUI::make_scoped_id(scope, id), label, fill_width(28.0f));
                    EditorGUI::text(app.gui, GUI::make_scoped_id(scope, label), label,
                        fixed_no_shrink(104.0f, 28.0f));
                    String value_id = id;
                    value_id.append(".value");
                    Vector<const c8*> choices;
                    choices.reserve(count);
                    for(usize i = 0; i < count; ++i) choices.push_back(items[i]);
                    EditorGUI::combo(app.gui,
                        GUI::make_scoped_id(scope, value_id.c_str()), label,
                        selected, Span<const c8*>(choices.data(), choices.size()),
                        fill_width(28.0f));
                    EditorGUI::end_h_layout(app.gui, row);
                }

                f32 effect_height(const VisualEffectEditor& effect)
                {
                    if(!effect.supported) return 76.0f;
                    f32 result = 28.0f + 28.0f + 28.0f + 24.0f + 4.0f * 28.0f;
                    switch(effect.type)
                    {
                    case 0: result += 30.0f; break;
                    case 1: result += 4.0f * 30.0f; break;
                    case 2: result += 30.0f + 28.0f; break;
                    case 3: result += 30.0f + 2.0f * 28.0f; break;
                    case 4: result += 30.0f + 5.0f * 28.0f + 28.0f; break;
                    default: result += 48.0f; break;
                    }
                    return result;
                }
            }

            void decode_visual_effects(const Variant& value,
                Vector<VisualEffectEditor>& effects)
            {
                effects.clear();
                if(value.type() != VariantType::array) return;
                effects.reserve(value.size());
                for(const Variant& source : value.values())
                {
                    VisualEffectEditor effect;
                    effect.source = source;
                    if(source.type() != VariantType::object)
                    {
                        effect.supported = false;
                        effects.push_back(move(effect));
                        continue;
                    }
                    effect.phase = find_name(source["phase"].str(), EFFECT_PHASE_NAMES, 2);
                    effect.type = find_name(source["type"].str(), EFFECT_TYPE_NAMES,
                        EFFECT_TYPE_COUNT);
                    if(effect.phase < 0 || effect.type < 0)
                    {
                        effect.supported = false;
                        effects.push_back(move(effect));
                        continue;
                    }
                    if(effect.type == 4)
                    {
                        effect.color[0] = 0.0f;
                        effect.color[1] = 0.0f;
                        effect.color[2] = 0.0f;
                        effect.color[3] = 0.5f;
                    }
                    decode_float_array(source["inset"], effect.inset, 4);
                    decode_float_array(source["color"], effect.color, 4);
                    decode_float_array(source["color_top_right"], effect.gradient_colors[0], 4);
                    decode_float_array(source["color_bottom_right"], effect.gradient_colors[1], 4);
                    decode_float_array(source["color_bottom_left"], effect.gradient_colors[2], 4);
                    effect.radius = (f32)source["radius"].fnum();
                    effect.line_width = (f32)source["line_width"].fnum(1.0);
                    decode_float_array(source["shadow_offset"], effect.shadow_offset, 2);
                    effect.shadow_softness = (f32)source["shadow_softness"].fnum();
                    effect.shadow_spread = (f32)source["shadow_spread"].fnum();
                    effect.shadow_mode = source["shadow_mode"].str() == Name("inner") ? 1 : 0;
                    effects.push_back(move(effect));
                }
            }

            Variant encode_visual_effects(Span<const VisualEffectEditor> effects)
            {
                Variant result(VariantType::array);
                for(const VisualEffectEditor& effect : effects)
                {
                    if(!effect.supported)
                    {
                        result.push_back(effect.source);
                        continue;
                    }
                    Variant encoded = effect.source.type() == VariantType::object ?
                        effect.source : Variant(VariantType::object);
                    erase_known_effect_fields(encoded);
                    encoded["phase"] = EFFECT_PHASE_NAMES[clamp(effect.phase, 0, 1)];
                    i32 type = clamp(effect.type, 0, (i32)EFFECT_TYPE_COUNT - 1);
                    encoded["type"] = EFFECT_TYPE_NAMES[type];
                    encoded["inset"] = make_float_array(effect.inset, 4);
                    encoded["color"] = make_float_array(effect.color, 4);
                    switch(type)
                    {
                    case 1:
                        encoded["color_top_right"] = make_float_array(
                            effect.gradient_colors[0], 4);
                        encoded["color_bottom_right"] = make_float_array(
                            effect.gradient_colors[1], 4);
                        encoded["color_bottom_left"] = make_float_array(
                            effect.gradient_colors[2], 4);
                        break;
                    case 2:
                        encoded["radius"] = (f64)effect.radius;
                        break;
                    case 3:
                        encoded["radius"] = (f64)effect.radius;
                        encoded["line_width"] = (f64)effect.line_width;
                        break;
                    case 4:
                        encoded["radius"] = (f64)effect.radius;
                        encoded["shadow_offset"] = make_float_array(effect.shadow_offset, 2);
                        encoded["shadow_softness"] = (f64)effect.shadow_softness;
                        encoded["shadow_spread"] = (f64)effect.shadow_spread;
                        encoded["shadow_mode"] = effect.shadow_mode == 1 ? "inner" : "outer";
                        break;
                    default: break;
                    }
                    result.push_back(move(encoded));
                }
                return result;
            }

            f32 visual_effects_editor_height(const PropertyEditor& property)
            {
                f32 result = 24.0f + 28.0f;
                for(const VisualEffectEditor& effect : property.visual_effects)
                    result += effect_height(effect);
                return result;
            }

            GUI::ElementHandle EditorApp::build_visual_effects_editor(PropertyEditor& property,
                usize property_index, GUI::id_t property_id, UIHandles& handles)
            {
                GUI::ElementHandle add = EditorGUI::text_button(gui,
                    GUI::make_scoped_id(property_id, "add"), "Add Effect", fill_width(28.0f));
                add_action(handles, property_index, 0, VisualEffectAction::add, add);
                for(usize i = 0; i < property.visual_effects.size(); ++i)
                {
                    VisualEffectEditor& effect = property.visual_effects[i];
                    GUI::id_t effect_id = GUI::make_scoped_id(property_id, (u64)i + 1);
                    GUI::ElementHandle toolbar = EditorGUI::begin_h_layout(gui,
                        GUI::make_scoped_id(effect_id, "toolbar"), "Effect Actions",
                        fill_width(28.0f));
                    String title;
                    if(effect.supported && effect.type >= 0 &&
                        effect.type < (i32)EFFECT_TYPE_COUNT)
                    {
                        strprintf(title, "Effect %u: %s", (u32)i + 1,
                            EFFECT_TYPE_LABELS[effect.type]);
                    }
                    else strprintf(title, "Effect %u: Unsupported", (u32)i + 1);
                    EditorGUI::text(gui, GUI::make_scoped_id(effect_id, "title"),
                        title.c_str(), fill_width(28.0f));
                    if(i > 0)
                    {
                        GUI::ElementHandle up = EditorGUI::text_button(gui,
                            GUI::make_scoped_id(effect_id, "up"), "Up",
                            fixed_no_shrink(40.0f, 26.0f));
                        add_action(handles, property_index, i,
                            VisualEffectAction::move_up, up);
                    }
                    if(i + 1 < property.visual_effects.size())
                    {
                        GUI::ElementHandle down = EditorGUI::text_button(gui,
                            GUI::make_scoped_id(effect_id, "down"), "Down",
                            fixed_no_shrink(52.0f, 26.0f));
                        add_action(handles, property_index, i,
                            VisualEffectAction::move_down, down);
                    }
                    GUI::ElementHandle remove = EditorGUI::text_button(gui,
                        GUI::make_scoped_id(effect_id, "remove"), "Delete",
                        fixed_no_shrink(58.0f, 26.0f));
                    add_action(handles, property_index, i, VisualEffectAction::remove, remove);
                    EditorGUI::end_h_layout(gui, toolbar);

                    if(!effect.supported)
                    {
                        EditorGUI::text(gui, GUI::make_scoped_id(effect_id, "unsupported"),
                            "This effect uses unsupported or malformed data.", fill_width(48.0f));
                        continue;
                    }

                    labeled_combo(*this, effect_id, "phase", "Phase", &effect.phase,
                        EFFECT_PHASE_LABELS, 2);
                    labeled_combo(*this, effect_id, "type", "Type", &effect.type,
                        EFFECT_TYPE_LABELS, EFFECT_TYPE_COUNT);
                    EditorGUI::text(gui, GUI::make_scoped_id(effect_id, "insets"), "Insets",
                        fill_width(24.0f));
                    const c8* side_ids[] = {"left", "top", "right", "bottom"};
                    const c8* side_labels[] = {"Left", "Top", "Right", "Bottom"};
                    for(usize side = 0; side < 4; ++side)
                    {
                        labeled_number(*this, effect_id, side_ids[side], side_labels[side],
                            effect.inset + side);
                    }

                    switch(effect.type)
                    {
                    case 0:
                        labeled_color(*this, effect_id, "color", "Color", effect.color);
                        break;
                    case 1:
                        labeled_color(*this, effect_id, "color_top_left", "Top Left",
                            effect.color);
                        labeled_color(*this, effect_id, "color_top_right", "Top Right",
                            effect.gradient_colors[0]);
                        labeled_color(*this, effect_id, "color_bottom_right", "Bottom Right",
                            effect.gradient_colors[1]);
                        labeled_color(*this, effect_id, "color_bottom_left", "Bottom Left",
                            effect.gradient_colors[2]);
                        break;
                    case 2:
                        labeled_color(*this, effect_id, "color", "Color", effect.color);
                        labeled_number(*this, effect_id, "radius", "Corner Radius",
                            &effect.radius, 0.0f, 1024.0f);
                        break;
                    case 3:
                        labeled_color(*this, effect_id, "color", "Color", effect.color);
                        labeled_number(*this, effect_id, "radius", "Corner Radius",
                            &effect.radius, 0.0f, 1024.0f);
                        labeled_number(*this, effect_id, "line_width", "Line Width",
                            &effect.line_width, 0.0f, 1024.0f);
                        break;
                    case 4:
                    {
                        labeled_color(*this, effect_id, "color", "Color", effect.color);
                        labeled_number(*this, effect_id, "radius", "Corner Radius",
                            &effect.radius, 0.0f, 1024.0f);
                        labeled_number(*this, effect_id, "offset_x", "Offset X",
                            effect.shadow_offset);
                        labeled_number(*this, effect_id, "offset_y", "Offset Y",
                            effect.shadow_offset + 1);
                        labeled_number(*this, effect_id, "softness", "Softness",
                            &effect.shadow_softness, 0.0f, 1024.0f);
                        labeled_number(*this, effect_id, "spread", "Spread",
                            &effect.shadow_spread);
                        const c8* modes[] = {"Outer", "Inner"};
                        labeled_combo(*this, effect_id, "shadow_mode", "Mode",
                            &effect.shadow_mode, modes, 2);
                        break;
                    }
                    default:
                        EditorGUI::text(gui, GUI::make_scoped_id(effect_id, "invalid_type"),
                            "Unsupported effect type.", fill_width(48.0f));
                        break;
                    }
                }
                return add;
            }

            bool EditorApp::process_visual_effect_actions(DocumentView& document,
                const UIHandles& handles)
            {
                for(const VisualEffectActionHit& hit : handles.visual_effect_actions)
                {
                    if(!EditorGUI::is_item_clicked(gui, hit.element) ||
                        hit.property_index >= document.property_editors.size()) continue;
                    PropertyEditor& property = document.property_editors[hit.property_index];
                    if(property.desc.editor != EditingPropertyEditor::visual_effects) continue;
                    switch(hit.action)
                    {
                    case VisualEffectAction::add:
                    {
                        VisualEffectEditor effect;
                        effect.type = 4;
                        effect.color[0] = 0.0f;
                        effect.color[1] = 0.0f;
                        effect.color[2] = 0.0f;
                        effect.color[3] = 0.5f;
                        effect.shadow_offset[1] = 4.0f;
                        effect.shadow_softness = 8.0f;
                        property.visual_effects.push_back(move(effect));
                        break;
                    }
                    case VisualEffectAction::move_up:
                        if(!hit.effect_index || hit.effect_index >= property.visual_effects.size())
                            continue;
                        swap(property.visual_effects[hit.effect_index - 1],
                            property.visual_effects[hit.effect_index]);
                        break;
                    case VisualEffectAction::move_down:
                        if(hit.effect_index + 1 >= property.visual_effects.size()) continue;
                        swap(property.visual_effects[hit.effect_index],
                            property.visual_effects[hit.effect_index + 1]);
                        break;
                    case VisualEffectAction::remove:
                        if(hit.effect_index >= property.visual_effects.size()) continue;
                        property.visual_effects.erase(
                            property.visual_effects.begin() + hit.effect_index);
                        break;
                    }
                    apply_inspector_changes(document);
                    return true;
                }
                return false;
            }
        }
    }
}
