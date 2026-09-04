/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorCommon.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            GUI::LayoutConfig fill_layout()
            {
                GUI::LayoutConfig result;
                result.width.kind = GUI::SizeKind::percent;
                result.width.value = 1.0f;
                result.height.kind = GUI::SizeKind::percent;
                result.height.value = 1.0f;
                result.flex_grow = 1.0f;
                return result;
            }

            GUI::LayoutConfig fill_width(f32 height)
            {
                GUI::LayoutConfig result;
                result.width.kind = GUI::SizeKind::percent;
                result.width.value = 1.0f;
                result.height.kind = GUI::SizeKind::fixed;
                result.height.value = height;
                return result;
            }

            GUI::LayoutConfig fixed_layout(f32 width, f32 height)
            {
                GUI::LayoutConfig result;
                result.width.kind = GUI::SizeKind::fixed;
                result.width.value = width;
                result.height.kind = GUI::SizeKind::fixed;
                result.height.value = height;
                return result;
            }

            String guid_string(const Guid& guid)
            {
                c8 buffer[GUID_STRING_LENGTH];
                RV result = encode_guid(guid, buffer, sizeof(buffer));
                luassert(succeeded(result));
                return String(buffer, sizeof(buffer));
            }

            bool decode_guid_string(const Variant& value, Guid& result)
            {
                return value.type() == VariantType::string &&
                    succeeded(decode_guid(value.c_str(), value.str().size(), result));
            }

            GUI::id_t guid_gui_id(GUI::id_t scope, const Guid& guid)
            {
                GUI::id_t result = memhash64(&guid, sizeof(guid), scope);
                return result ? result : GUI::DEFAULT_DATA_SCOPE;
            }

            GUI::id_t document_panel_id(GUI::IContext* context, u64 document_id)
            {
                return GUI::make_scoped_id(context->make_id("panel.document"), document_id);
            }

            GUI::id_t hierarchy_context_popup_id(GUI::IContext* context, u64 document_id)
            {
                return GUI::make_scoped_id(context->make_id("hierarchy.context_popup"),
                    document_id);
            }

            String property_text(const Variant& value)
            {
                switch(value.type())
                {
                case VariantType::string: return String(value.c_str());
                case VariantType::number:
                {
                    String result;
                    if(value.number_type() == VariantNumberType::number_i64)
                        strprintf(result, "%lld", (long long)value.inum());
                    else if(value.number_type() == VariantNumberType::number_u64)
                        strprintf(result, "%llu", (unsigned long long)value.unum());
                    else strprintf(result, "%.9g", value.fnum());
                    return result;
                }
                default:
                {
                    auto encoded = VariantUtils::write_json(value,
                        VariantUtils::JSONWriteOptions::strict());
                    return encoded.valid() ? encoded.get() : String("null");
                }
                }
            }

            bool decode_editing_schema(const Variant& value, EditingSchema& schema)
            {
                if(value.type() != VariantType::object ||
                    value["properties"].type() != VariantType::array)
                    return false;
                schema.properties.clear();
                for(const Variant& encoded : value["properties"].values())
                {
                    if(encoded.type() != VariantType::object) return false;
                    EditingPropertyDesc property;
                    property.id = encoded["id"].str();
                    property.alternate_id = encoded["alternate_id"].str();
                    property.display_name = encoded["display_name"].c_str();
                    property.description = encoded["description"].c_str();
                    if(property.id.empty() || property.display_name.empty()) return false;

                    Name section = encoded["section"].str();
                    if(section == Name("layout"))
                        property.section = EditingPropertySection::layout;
                    else if(section == Name("style"))
                        property.section = EditingPropertySection::style;
                    else if(section == Name("property"))
                        property.section = EditingPropertySection::property;
                    else return false;

                    Name editor = encoded["editor"].str();
                    if(editor == Name("boolean")) property.editor = EditingPropertyEditor::boolean;
                    else if(editor == Name("number")) property.editor = EditingPropertyEditor::number;
                    else if(editor == Name("string")) property.editor = EditingPropertyEditor::string;
                    else if(editor == Name("name")) property.editor = EditingPropertyEditor::name;
                    else if(editor == Name("enumeration"))
                        property.editor = EditingPropertyEditor::enumeration;
                    else if(editor == Name("float2")) property.editor = EditingPropertyEditor::float2;
                    else if(editor == Name("float4")) property.editor = EditingPropertyEditor::float4;
                    else if(editor == Name("color")) property.editor = EditingPropertyEditor::color;
                    else if(editor == Name("visual_effects"))
                        property.editor = EditingPropertyEditor::visual_effects;
                    else if(editor == Name("size")) property.editor = EditingPropertyEditor::size;
                    else if(editor == Name("asset")) property.editor = EditingPropertyEditor::asset;
                    else if(editor == Name("json")) property.editor = EditingPropertyEditor::json;
                    else return false;

                    property.has_default = encoded["has_default"].boolean();
                    if(property.has_default) property.default_value = encoded["default_value"];
                    property.optional = encoded["optional"].boolean(true);
                    property.bounded = encoded["bounded"].boolean();
                    property.minimum = encoded["minimum"].fnum();
                    property.maximum = encoded["maximum"].fnum();
                    property.step = encoded["step"].fnum(0.1);
                    property.asset_type = encoded["asset_type"].str();
                    const Variant& items = encoded["items"];
                    if(items.valid())
                    {
                        if(items.type() != VariantType::array) return false;
                        for(const Variant& encoded_item : items.values())
                        {
                            EditingEnumItemDesc item;
                            item.value = encoded_item["value"].str();
                            item.display_name = encoded_item["display_name"].c_str();
                            if(item.value.empty() || item.display_name.empty()) return false;
                            property.enumeration_items.push_back(move(item));
                        }
                    }
                    if(property.editor == EditingPropertyEditor::size &&
                        property.alternate_id.empty()) return false;
                    if(property.editor == EditingPropertyEditor::enumeration &&
                        property.enumeration_items.empty()) return false;
                    schema.properties.push_back(move(property));
                }
                return true;
            }

            Variant make_editor_vector(const f32* values, usize count)
            {
                Variant result(VariantType::array);
                for(usize i = 0; i < count; ++i) result.push_back((f64)values[i]);
                return result;
            }

            R<Variant> property_value(const PropertyEditor& editor)
            {
                switch(editor.desc.editor)
                {
                case EditingPropertyEditor::boolean: return Variant(editor.boolean);
                case EditingPropertyEditor::number: return Variant((f64)editor.number);
                case EditingPropertyEditor::string:
                case EditingPropertyEditor::name:
                case EditingPropertyEditor::asset:
                    return Variant(editor.text.c_str());
                case EditingPropertyEditor::enumeration:
                    if(editor.selected_item < 0 ||
                        (usize)editor.selected_item >= editor.desc.enumeration_items.size())
                        return E_BAD_DATA;
                    return Variant(editor.desc.enumeration_items[(usize)editor.selected_item].value);
                case EditingPropertyEditor::float2:
                    return make_editor_vector(editor.vector, 2);
                case EditingPropertyEditor::float4:
                case EditingPropertyEditor::color:
                    return make_editor_vector(editor.vector, 4);
                case EditingPropertyEditor::visual_effects:
                    return encode_visual_effects(editor.visual_effects.cspan());
                case EditingPropertyEditor::size:
                    return Variant((f64)(editor.size_mode == 2 ?
                        editor.number * 0.01f : editor.number));
                case EditingPropertyEditor::json:
                    return VariantUtils::read_json(editor.text.c_str(), editor.text.size(),
                        VariantUtils::JSONReadOptions::strict());
                }
                return E_BAD_DATA;
            }

            bool point_in_rect(const RectF& rect, const Float2U& point)
            {
                return point.x >= rect.offset_x && point.y >= rect.offset_y &&
                    point.x < rect.offset_x + rect.width &&
                    point.y < rect.offset_y + rect.height;
            }

            RectF item_screen_rect(GUI::IContext* context, const GUI::ElementHandle& item,
                bool clip_rect)
            {
                const GUI::Element* element = context->get_element(item.index);
                if(!element || element->id != item.id) return RectF();
                Span<const GUI::Layer> layers = context->get_layers();
                if(element->layer >= layers.size()) return RectF();
                RectF result = clip_rect ? element->layout_result.clip_rect :
                    element->layout_result.rect;
                result.offset_x += layers[element->layer].screen_position.x;
                result.offset_y += layers[element->layer].screen_position.y;
                return result;
            }

            bool subtree_contains(const AuthoringDocument& document, const Guid& root,
                const Guid& node)
            {
                if(root == node) return true;
                const AuthoringNodeRecord* record = find_authoring_node(document, root);
                if(!record) return false;
                for(const AuthoringChildLink& child : record->children)
                {
                    if(subtree_contains(document, child.child, node)) return true;
                }
                return false;
            }

            bool find_parent_info(const AuthoringDocument& document, const Guid& node,
                Guid& parent, usize& sibling_index)
            {
                for(const AuthoringNodeRecord& candidate : document.nodes)
                {
                    for(usize i = 0; i < candidate.children.size(); ++i)
                    {
                        if(candidate.children[i].child == node)
                        {
                            parent = candidate.id;
                            sibling_index = i;
                            return true;
                        }
                    }
                }
                return false;
            }

            Variant editing_params(const DocumentView& document)
            {
                Variant params(VariantType::object);
                params["document_id"] = document.id;
                params["expected_revision"] = document.revision;
                return params;
            }
        }
    }
}
