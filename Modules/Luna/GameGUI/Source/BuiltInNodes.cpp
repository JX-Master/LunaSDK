/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file BuiltInNodes.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            constexpr Guid FLEX_NODE_TYPE("{DA52CB84-F9D5-4F70-89E9-9110A28E8F90}");
            constexpr Guid CANVAS_NODE_TYPE("{4D9DD1E5-05B8-4578-9C2B-04990333EBA8}");
            constexpr Guid PANEL_NODE_TYPE("{3422E8A6-A381-4E37-8EB0-AEA8CC179F5B}");
            constexpr Guid TEXT_NODE_TYPE("{2A517F8A-4316-4E77-8A4F-4FFB2A32026B}");
            constexpr Guid BUTTON_NODE_TYPE("{88E839CE-7731-4B93-86E6-B7A80FD6DCE5}");
            constexpr Guid ASSET_INSTANCE_NODE_TYPE("{D13F9808-6F27-4B43-B7B0-C771E5850AB1}");

            RV decode_asset_guid(const Variant& value, Guid& guid)
            {
                if(value.type() != VariantType::string)
                {
                    return set_error(E_BAD_DATA,
                        "GameGUI AssetInstance asset must be a canonical GUID string.");
                }
                RV result = decode_guid(value.c_str(), value.str().size(), guid);
                if(!result.valid())
                {
                    return set_error(E_BAD_DATA,
                        "GameGUI AssetInstance asset must be a canonical GUID string.");
                }
                return ok;
            }

            f32 get_number(const Variant& properties, const c8* key, f32 default_value)
            {
                const Variant& value = properties[Name(key)];
                return value.type() == VariantType::number ? (f32)value.fnum() : default_value;
            }

            bool get_boolean(const Variant& properties, const c8* key, bool default_value)
            {
                const Variant& value = properties[Name(key)];
                return value.type() == VariantType::boolean ? value.boolean() : default_value;
            }

            Float2U get_float2(const Variant& value, const Float2U& default_value)
            {
                if(value.type() != VariantType::array || value.size() != 2) return default_value;
                return Float2U((f32)value[0].fnum(default_value.x), (f32)value[1].fnum(default_value.y));
            }

            Float4U get_float4(const Variant& value, const Float4U& default_value)
            {
                if(value.type() != VariantType::array || value.size() != 4) return default_value;
                return Float4U((f32)value[0].fnum(default_value.x), (f32)value[1].fnum(default_value.y),
                    (f32)value[2].fnum(default_value.z), (f32)value[3].fnum(default_value.w));
            }

            GUI::LayoutConfig get_layout(const Variant& properties)
            {
                GUI::LayoutConfig layout;
                if(properties["width"].type() == VariantType::number)
                {
                    layout.width.kind = GUI::SizeKind::fixed;
                    layout.width.value = (f32)properties["width"].fnum();
                }
                else if(properties["width_percent"].type() == VariantType::number)
                {
                    layout.width.kind = GUI::SizeKind::percent;
                    layout.width.value = (f32)properties["width_percent"].fnum();
                }
                if(properties["height"].type() == VariantType::number)
                {
                    layout.height.kind = GUI::SizeKind::fixed;
                    layout.height.value = (f32)properties["height"].fnum();
                }
                else if(properties["height_percent"].type() == VariantType::number)
                {
                    layout.height.kind = GUI::SizeKind::percent;
                    layout.height.value = (f32)properties["height_percent"].fnum();
                }
                layout.margin = get_float4(properties["margin"], Float4U(0.0f));
                layout.padding = get_float4(properties["padding"], Float4U(0.0f));
                layout.flex_grow = get_number(properties, "flex_grow", 0.0f);
                layout.flex_shrink = get_number(properties, "flex_shrink", 1.0f);
                return layout;
            }

            void set_common_element_data(BuildContext& context, const GUI::ElementHandle& element,
                const NodeRecord& node)
            {
                context.gui()->set_layout_config(element, get_layout(node.properties));
                if(!node.name.empty()) context.gui()->set_element_debug_name(element, node.name);
            }

            RV validate_properties_object(const NodeRecord& node, object_t userdata)
            {
                if(node.properties.valid() && node.properties.type() != VariantType::object)
                {
                    return set_error(E_BAD_DATA, "GameGUI built-in node properties must be an object.");
                }
                return ok;
            }

            R<GUI::ElementHandle> build_flex(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                GUI::FlexLayoutDesc layout;
                layout.axis = node.properties["axis"].str() == Name("x") ?
                    GUI::LayoutAxis::x : GUI::LayoutAxis::y;
                layout.reverse = get_boolean(node.properties, "reverse", false);
                layout.main_axis_gap = get_number(node.properties, "gap", 0.0f);
                layout.cross_axis_gap = get_number(node.properties, "line_gap", 0.0f);
                layout.clip_children = get_boolean(node.properties, "clip_children", false);
                context.set_flex_layout(element, layout);
                RV children_result = context.build_children();
                context.gui()->end_element();
                if(!children_result.valid()) return children_result.errcode();
                return element;
            }

            GUI::CanvasLayoutItem decode_canvas_item(BuildContext& context, const ChildLink& link)
            {
                GUI::CanvasLayoutItem item;
                item.element_id = context.make_child_id(link.child, "element");
                const Variant& attachment = link.attachment;
                item.anchor_min = get_float2(attachment["anchor_min"], Float2U(0.0f));
                item.anchor_max = get_float2(attachment["anchor_max"], item.anchor_min);
                item.offset = get_float4(attachment["offset"], Float4U(0.0f));
                item.pivot = get_float2(attachment["pivot"], Float2U(0.0f));
                return item;
            }

            R<GUI::ElementHandle> build_canvas(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                Vector<GUI::CanvasLayoutItem> items;
                for(const ChildLink& link : node.children) items.push_back(decode_canvas_item(context, link));
                context.set_canvas_layout(element,
                    Span<const GUI::CanvasLayoutItem>(items.data(), items.size()),
                    get_boolean(node.properties, "clip_children", false));
                RV children_result = context.build_children();
                context.gui()->end_element();
                if(!children_result.valid()) return children_result.errcode();
                return element;
            }

            void draw_panel(BuildContext& context, const GUI::ElementHandle& element,
                const Variant& properties, const Float4U& default_color = Float4U(1.0f))
            {
                GUI::DrawCommand draw;
                draw.type = GUI::DrawCommandType::rounded_rect;
                draw.rect_reference = GUI::DrawCommandRectReference::element;
                draw.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
                draw.color = get_float4(properties["color"], default_color);
                draw.radius = get_number(properties, "radius", 0.0f);
                context.gui()->draw_for_element(element, draw);
            }

            R<GUI::ElementHandle> build_panel(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                GUI::FlexLayoutDesc layout;
                context.set_flex_layout(element, layout);
                draw_panel(context, element, node.properties, Float4U(0.22f, 0.22f, 0.22f, 1.0f));
                RV children_result = context.build_children();
                context.gui()->end_element();
                if(!children_result.valid()) return children_result.errcode();
                return element;
            }

            void draw_text(BuildContext& context, const GUI::ElementHandle& element,
                const Variant& properties)
            {
                GUI::DrawCommand draw;
                draw.type = GUI::DrawCommandType::text;
                draw.rect_reference = GUI::DrawCommandRectReference::element;
                draw.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
                draw.color = get_float4(properties["text_color"], Float4U(1.0f));
                draw.font = properties["font"].str();
                draw.font_size = get_number(properties, "font_size", 16.0f);
                draw.text = properties["text"].c_str();
                context.gui()->draw_for_element(element, draw);
            }

            R<GUI::ElementHandle> build_text(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                draw_text(context, element, node.properties);
                context.gui()->end_element();
                return element;
            }

            R<GUI::ElementHandle> build_button(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                GUI::Interactable interactable;
                interactable.flags = GUI::InteractableFlag::hoverable |
                    GUI::InteractableFlag::activatable | GUI::InteractableFlag::focusable;
                interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                context.gui()->set_interactable(element, interactable);
                draw_panel(context, element, node.properties, Float4U(0.22f, 0.22f, 0.22f, 1.0f));
                draw_text(context, element, node.properties);
                context.gui()->end_element();
                return element;
            }

            RV resolve_button(ResolveContext& context, const NodeRecord& node, object_t userdata)
            {
                GUI::InteractionState interaction = context.gui()->get_interaction_state(
                    context.make_id("element"));
                if(interaction.clicked)
                {
                    Name action = node.properties["action"].str();
                    if(!action.empty())
                    {
                        Variant& state = context.state();
                        state["click_count"] = state["click_count"].unum() + 1;
                        context.emit_action(action, node.properties["action_payload"]);
                    }
                }
                return ok;
            }

            RV validate_asset_instance(const NodeRecord& node, object_t userdata)
            {
                if(!node.children.empty())
                {
                    return set_error(E_BAD_DATA, "GameGUI AssetInstance nodes cannot have local children.");
                }
                Guid asset_guid;
                RV result = decode_asset_guid(node.properties["asset"], asset_guid);
                if(!result.valid()) return result;
                if(asset_guid == Guid())
                {
                    return set_error(E_BAD_DATA, "GameGUI AssetInstance requires a non-zero asset GUID.");
                }
                return ok;
            }

            R<Any> prepare_asset_instance(const NodeRecord& node, object_t userdata)
            {
                Guid asset_guid;
                RV result = decode_asset_guid(node.properties["asset"], asset_guid);
                if(!result.valid()) return result.errcode();
                Any prepared;
                prepared.emplace<Asset::asset_t>(Asset::get_asset(asset_guid));
                return prepared;
            }

            void collect_asset_instance(const NodeRecord& node, Vector<Asset::asset_t>& assets,
                object_t userdata)
            {
                Guid asset_guid;
                RV result = decode_asset_guid(node.properties["asset"], asset_guid);
                if(result.valid() && asset_guid != Guid()) assets.push_back(Asset::get_asset(asset_guid));
            }

            R<GUI::ElementHandle> build_asset_instance(BuildContext& context, const NodeRecord& node,
                object_t userdata)
            {
                const Asset::asset_t* asset = context.prepared_data().as<Asset::asset_t>();
                if(!asset || !*asset) return E_BAD_DATA;
                GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
                set_common_element_data(context, element, node);
                GUI::FlexLayoutDesc layout;
                context.set_flex_layout(element, layout);
                auto nested = context.build_nested(*asset);
                context.gui()->end_element();
                if(!nested.valid()) return nested.errcode();
                return element;
            }

            RV register_if_missing(const NodeTypeDesc& desc)
            {
                auto existing = get_node_type(desc.type);
                if(existing.valid()) return existing.get().name == desc.name ? ok : RV(E_ALREADY_EXISTS);
                return register_node_type(desc);
            }
        }

        LUNA_GAME_GUI_API Guid get_flex_node_type() { return FLEX_NODE_TYPE; }
        LUNA_GAME_GUI_API Guid get_canvas_node_type() { return CANVAS_NODE_TYPE; }
        LUNA_GAME_GUI_API Guid get_panel_node_type() { return PANEL_NODE_TYPE; }
        LUNA_GAME_GUI_API Guid get_text_node_type() { return TEXT_NODE_TYPE; }
        LUNA_GAME_GUI_API Guid get_button_node_type() { return BUTTON_NODE_TYPE; }
        LUNA_GAME_GUI_API Guid get_asset_instance_node_type() { return ASSET_INSTANCE_NODE_TYPE; }

        LUNA_GAME_GUI_API RV register_builtin_node_types()
        {
            lutry
            {
                NodeTypeDesc desc;
                desc.type = FLEX_NODE_TYPE;
                desc.name = "Flex";
                desc.validate = validate_properties_object;
                desc.build = build_flex;
                luexp(register_if_missing(desc));

                desc = NodeTypeDesc();
                desc.type = CANVAS_NODE_TYPE;
                desc.name = "Canvas";
                desc.validate = validate_properties_object;
                desc.build = build_canvas;
                luexp(register_if_missing(desc));

                desc = NodeTypeDesc();
                desc.type = PANEL_NODE_TYPE;
                desc.name = "Panel";
                desc.validate = validate_properties_object;
                desc.build = build_panel;
                luexp(register_if_missing(desc));

                desc = NodeTypeDesc();
                desc.type = TEXT_NODE_TYPE;
                desc.name = "Text";
                desc.validate = validate_properties_object;
                desc.build = build_text;
                luexp(register_if_missing(desc));

                desc = NodeTypeDesc();
                desc.type = BUTTON_NODE_TYPE;
                desc.name = "Button";
                desc.validate = validate_properties_object;
                desc.build = build_button;
                desc.resolve = resolve_button;
                luexp(register_if_missing(desc));

                desc = NodeTypeDesc();
                desc.type = ASSET_INSTANCE_NODE_TYPE;
                desc.name = "AssetInstance";
                desc.validate = validate_asset_instance;
                desc.prepare = prepare_asset_instance;
                desc.build = build_asset_instance;
                desc.collect_assets = collect_asset_instance;
                desc.nested_document = true;
                luexp(register_if_missing(desc));
            }
            lucatchret;
            return ok;
        }
    }
}
