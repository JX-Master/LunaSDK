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
#include <cstdlib>

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

            R<Variant> property_value(const PropertyEditor& editor)
            {
                switch(editor.original.type())
                {
                case VariantType::string: return Variant(editor.text.c_str());
                case VariantType::boolean: return Variant(editor.boolean);
                case VariantType::number:
                    if(editor.original.number_type() == VariantNumberType::number_i64)
                    {
                        c8* end = nullptr;
                        i64 value = (i64)strtoll(editor.text.c_str(), &end, 10);
                        if(!end || *end) return set_error(E_BAD_DATA, "Invalid signed integer property value.");
                        return Variant(value);
                    }
                    if(editor.original.number_type() == VariantNumberType::number_u64)
                    {
                        c8* end = nullptr;
                        u64 value = (u64)strtoull(editor.text.c_str(), &end, 10);
                        if(!end || *end) return set_error(E_BAD_DATA, "Invalid unsigned integer property value.");
                        return Variant(value);
                    }
                    else
                    {
                        c8* end = nullptr;
                        f64 value = strtod(editor.text.c_str(), &end);
                        if(!end || *end) return set_error(E_BAD_DATA, "Invalid floating-point property value.");
                        return Variant(value);
                    }
                default:
                    return VariantUtils::read_json(editor.text.c_str(), editor.text.size(),
                        VariantUtils::JSONReadOptions::strict());
                }
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

            bool subtree_contains(const GameGUI::Document& document, const Guid& root,
                const Guid& node)
            {
                if(root == node) return true;
                const GameGUI::NodeRecord* record = GameGUI::find_node(document, root);
                if(!record) return false;
                for(const GameGUI::ChildLink& child : record->children)
                {
                    if(subtree_contains(document, child.child, node)) return true;
                }
                return false;
            }

            bool find_parent_info(const GameGUI::Document& document, const Guid& node,
                Guid& parent, usize& sibling_index)
            {
                for(const GameGUI::NodeRecord& candidate : document.nodes)
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
