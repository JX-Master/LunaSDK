/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Authoring.cpp
* @author JXMaster
* @date 2026/8/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_EDITOR_SERVICE_API LUNA_EXPORT
#include "../Authoring.hpp"
#include "GameGUIEditorService.meta.generated.hpp"
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VFS/VFS.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace
        {
            SpinLock g_authoring_registry_lock;
            HashMap<Guid, AuthoringNodeTypeDesc> g_authoring_node_types;
            bool g_authoring_initialized = false;

            String guid_string(const Guid& guid)
            {
                c8 buffer[GUID_STRING_LENGTH];
                RV result = encode_guid(guid, buffer, sizeof(buffer));
                luassert(succeeded(result));
                return String(buffer, sizeof(buffer));
            }

            R<Guid> decode_guid_value(const Variant& value, const c8* field_name)
            {
                if(value.type() != VariantType::string)
                    return set_error(E_BAD_DATA, "GameGUI authoring %s must be a GUID string.", field_name);
                Guid guid;
                RV result = decode_guid(value.c_str(), value.str().size(), guid);
                if(failed(result))
                    return set_error(E_BAD_DATA, "GameGUI authoring %s must be a GUID string.", field_name);
                return guid;
            }

            void add_diagnostic(Vector<GameGUI::Diagnostic>* diagnostics,
                GameGUI::DiagnosticSeverity severity, const Guid& node, const c8* message)
            {
                if(!diagnostics) return;
                GameGUI::Diagnostic diagnostic;
                diagnostic.severity = severity;
                diagnostic.node = node;
                diagnostic.message = message;
                diagnostics->push_back(move(diagnostic));
            }

            R<AuthoringChildLink> decode_child_link(const Variant& value, bool legacy)
            {
                AuthoringChildLink link;
                lutry
                {
                    if(legacy && value.type() == VariantType::string)
                    {
                        luset(link.child, decode_guid_value(value, "child"));
                    }
                    else
                    {
                        if(value.type() != VariantType::object)
                            luthrow(set_error(E_BAD_DATA, "GameGUI authoring child link must be an object."));
                        luset(link.child, decode_guid_value(value["child"], "child"));
                        link.slot = value["slot"].str();
                        if(value.contains("attachment")) link.attachment = value["attachment"];
                    }
                }
                lucatchret;
                return link;
            }

            R<AuthoringNodeRecord> decode_node(const Variant& value, bool legacy)
            {
                AuthoringNodeRecord node;
                lutry
                {
                    if(value.type() != VariantType::object)
                        luthrow(set_error(E_BAD_DATA, "GameGUI authoring node must be an object."));
                    luset(node.id, decode_guid_value(value["id"], "node id"));
                    luset(node.type, decode_guid_value(value["type"], "node type"));
                    node.type_version = legacy ? 1 : (u32)value["type_version"].unum(1);
                    if(!node.type_version)
                        luthrow(set_error(E_BAD_DATA, "GameGUI authoring node version cannot be zero."));
                    node.name = value["name"].str();
                    if(value.contains("properties")) node.properties = value["properties"];
                    const Variant& children = value["children"];
                    if(children.valid() && children.type() != VariantType::array)
                        luthrow(set_error(E_BAD_DATA, "GameGUI authoring children must be an array."));
                    for(const Variant& child : children.values())
                    {
                        lulet(link, decode_child_link(child, legacy));
                        node.children.push_back(move(link));
                    }
                }
                lucatchret;
                return node;
            }

            Ref<AuthoringDocument> clone_authoring_document(const AuthoringDocument& source)
            {
                Ref<AuthoringDocument> result = new_object<AuthoringDocument>();
                result->format_version = source.format_version;
                result->root = source.root;
                result->nodes = source.nodes;
                result->extensions = source.extensions;
                return result;
            }

            RV migrate_known_nodes(AuthoringDocument& document,
                Vector<GameGUI::Diagnostic>* diagnostics)
            {
                for(AuthoringNodeRecord& node : document.nodes)
                {
                    auto desc_result = get_authoring_node_type(node.type);
                    if(!desc_result.valid())
                    {
                        add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::warning, node.id,
                            "The authoring node provider is unavailable; the raw record was retained.");
                        continue;
                    }
                    AuthoringNodeTypeDesc desc = move(desc_result.get());
                    if(node.type_version > desc.current_version)
                    {
                        add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::warning, node.id,
                            "The authoring node payload is newer than its installed provider; the raw record was retained.");
                        continue;
                    }
                    while(node.type_version < desc.current_version)
                    {
                        if(!desc.migrate)
                            return set_error(E_NOT_SUPPORTED,
                                "GameGUI authoring node `%s` requires a migration callback.", desc.name.c_str());
                        u32 from_version = node.type_version;
                        lutry
                        {
                            luexp(desc.migrate(node.properties, from_version, from_version + 1,
                                desc.userdata.get()));
                        }
                        lucatchret;
                        ++node.type_version;
                    }
                }
                return ok;
            }

            R<ObjRef> load_authoring(object_t userdata, Asset::asset_t asset,
                const Name& data_unit, const Path& path)
            {
                lutry
                {
                    Path source_path = path;
                    source_path.append_extension("json");
                    lulet(file, VFS::open_file(source_path, FileOpenFlag::read,
                        FileCreationMode::open_existing));
                    lulet(data, VariantUtils::read_json(file,
                        VariantUtils::JSONReadOptions::strict()));
                    lulet(document, decode_authoring_document(data));
                    return ObjRef(document.object());
                }
                lucatchret;
                return E_FAILURE;
            }

            R<ObjRef> load_default_authoring(object_t userdata, Asset::asset_t asset,
                const Name& data_unit)
            {
                return ObjRef(new_object<AuthoringDocument>().object());
            }

            RV save_authoring(object_t userdata, Asset::asset_t asset, const Name& data_unit,
                const Path& path, object_t data)
            {
                AuthoringDocument* document = cast_object<AuthoringDocument>(data);
                if(!document) return E_BAD_ARGUMENTS;
                lutry
                {
                    luexp(validate_authoring_document(*document));
                    lulet(encoded, encode_authoring_document(*document));
                    Path source_path = path;
                    source_path.append_extension("json");
                    lulet(file, VFS::open_file(source_path, FileOpenFlag::write,
                        FileCreationMode::create_always));
                    VariantUtils::JSONWriteOptions options;
                    options.indent = true;
                    options.encode_blobs = false;
                    options.allow_non_finite_numbers = false;
                    luexp(VariantUtils::write_json(file, encoded, options));
                }
                lucatchret;
                return ok;
            }

            RV set_authoring(object_t userdata, Asset::asset_t asset, const Name& data_unit,
                object_t data)
            {
                return data && !cast_object<AuthoringDocument>(data) ? RV(E_BAD_ARGUMENTS) : ok;
            }

            void get_authoring_references(object_t userdata, Asset::asset_t asset,
                const Name& data_unit, Vector<Asset::asset_t>& assets)
            {
                auto document = Asset::get_asset_data_unit_object<AuthoringDocument>(asset, data_unit);
                if(!document.valid() || !document.get()) return;
                Vector<Asset::asset_t> collected;
                for(const AuthoringNodeRecord& source : document.get()->nodes)
                {
                    auto runtime_type = GameGUI::get_node_type(source.type);
                    if(!runtime_type.valid() || !runtime_type.get().collect_assets) continue;
                    GameGUI::NodeRecord node;
                    node.id = source.id;
                    node.type = source.type;
                    node.name = source.name;
                    node.properties = source.properties;
                    for(const AuthoringChildLink& child : source.children)
                    {
                        GameGUI::ChildLink runtime_child;
                        runtime_child.child = child.child;
                        runtime_child.slot = child.slot;
                        runtime_child.attachment = child.attachment;
                        node.children.push_back(move(runtime_child));
                    }
                    runtime_type.get().collect_assets(node, collected,
                        runtime_type.get().userdata.get());
                }
                for(Asset::asset_t referred : collected)
                {
                    bool duplicate = false;
                    for(Asset::asset_t existing : assets)
                    {
                        if(existing == referred)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if(referred && !duplicate) assets.push_back(referred);
                }
            }

            Variant make_slot_schema(const c8* kind)
            {
                Variant schema(VariantType::object);
                schema["kind"] = kind;
                return schema;
            }

            Variant make_float2(f64 x, f64 y)
            {
                Variant value(VariantType::array);
                value.push_back(x);
                value.push_back(y);
                return value;
            }

            Variant make_float4(f64 x, f64 y, f64 z, f64 w)
            {
                Variant value(VariantType::array);
                value.push_back(x);
                value.push_back(y);
                value.push_back(z);
                value.push_back(w);
                return value;
            }

            EditingPropertyDesc make_property(const c8* id, const c8* display_name,
                EditingPropertySection section, EditingPropertyEditor editor)
            {
                EditingPropertyDesc property;
                property.id = id;
                property.display_name = display_name;
                property.section = section;
                property.editor = editor;
                return property;
            }

            void set_default(EditingPropertyDesc& property, Variant&& value)
            {
                property.default_value = move(value);
                property.has_default = true;
            }

            void add_common_layout_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("width", "Width",
                    EditingPropertySection::layout, EditingPropertyEditor::size);
                property.alternate_id = "width_percent";
                property.step = 1.0;
                property.description = "Controls automatic, fixed-pixel or parent-relative width.";
                schema.properties.push_back(move(property));

                property = make_property("height", "Height",
                    EditingPropertySection::layout, EditingPropertyEditor::size);
                property.alternate_id = "height_percent";
                property.step = 1.0;
                property.description = "Controls automatic, fixed-pixel or parent-relative height.";
                schema.properties.push_back(move(property));

                property = make_property("margin", "Margin",
                    EditingPropertySection::layout, EditingPropertyEditor::float4);
                set_default(property, make_float4(0.0, 0.0, 0.0, 0.0));
                property.step = 1.0;
                property.description = "Left, top, right and bottom outer spacing.";
                schema.properties.push_back(move(property));

                property = make_property("padding", "Padding",
                    EditingPropertySection::layout, EditingPropertyEditor::float4);
                set_default(property, make_float4(0.0, 0.0, 0.0, 0.0));
                property.step = 1.0;
                property.description = "Left, top, right and bottom inner spacing.";
                schema.properties.push_back(move(property));

                property = make_property("flex_grow", "Flex Grow",
                    EditingPropertySection::layout, EditingPropertyEditor::number);
                set_default(property, Variant(0.0));
                property.step = 0.1;
                property.description = "Relative share of positive free space.";
                schema.properties.push_back(move(property));

                property = make_property("flex_shrink", "Flex Shrink",
                    EditingPropertySection::layout, EditingPropertyEditor::number);
                set_default(property, Variant(1.0));
                property.step = 0.1;
                property.description = "Relative share of negative free space.";
                schema.properties.push_back(move(property));
            }

            void add_flex_layout_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("axis", "Axis",
                    EditingPropertySection::layout, EditingPropertyEditor::enumeration);
                set_default(property, Variant("y"));
                property.enumeration_items.push_back({"y", "Vertical"});
                property.enumeration_items.push_back({"x", "Horizontal"});
                schema.properties.push_back(move(property));

                property = make_property("reverse", "Reverse",
                    EditingPropertySection::layout, EditingPropertyEditor::boolean);
                set_default(property, Variant(false));
                schema.properties.push_back(move(property));

                property = make_property("wrap", "Wrap",
                    EditingPropertySection::layout, EditingPropertyEditor::enumeration);
                set_default(property, Variant("none"));
                property.enumeration_items.push_back({"none", "No Wrap"});
                property.enumeration_items.push_back({"wrap", "Wrap"});
                property.enumeration_items.push_back({"wrap_reverse", "Wrap Reverse"});
                property.description = "Controls whether children wrap onto additional lines.";
                schema.properties.push_back(move(property));

                property = make_property("main_alignment", "Main Alignment",
                    EditingPropertySection::layout, EditingPropertyEditor::enumeration);
                set_default(property, Variant("start"));
                property.enumeration_items.push_back({"start", "Start"});
                property.enumeration_items.push_back({"center", "Center"});
                property.enumeration_items.push_back({"end", "End"});
                property.enumeration_items.push_back({"space_between", "Space Between"});
                property.enumeration_items.push_back({"space_around", "Space Around"});
                property.enumeration_items.push_back({"space_evenly", "Space Evenly"});
                property.description = "Distributes children along the primary layout axis.";
                schema.properties.push_back(move(property));

                property = make_property("cross_alignment", "Cross Alignment",
                    EditingPropertySection::layout, EditingPropertyEditor::enumeration);
                set_default(property, Variant("stretch"));
                property.enumeration_items.push_back({"start", "Start"});
                property.enumeration_items.push_back({"center", "Center"});
                property.enumeration_items.push_back({"end", "End"});
                property.enumeration_items.push_back({"stretch", "Stretch"});
                property.description = "Aligns children across each flex line.";
                schema.properties.push_back(move(property));

                property = make_property("line_alignment", "Line Alignment",
                    EditingPropertySection::layout, EditingPropertyEditor::enumeration);
                set_default(property, Variant("start"));
                property.enumeration_items.push_back({"start", "Start"});
                property.enumeration_items.push_back({"center", "Center"});
                property.enumeration_items.push_back({"end", "End"});
                property.enumeration_items.push_back({"stretch", "Stretch"});
                property.enumeration_items.push_back({"space_between", "Space Between"});
                property.enumeration_items.push_back({"space_around", "Space Around"});
                property.enumeration_items.push_back({"space_evenly", "Space Evenly"});
                property.description = "Distributes wrapped lines along the cross axis.";
                schema.properties.push_back(move(property));

                property = make_property("gap", "Gap",
                    EditingPropertySection::layout, EditingPropertyEditor::number);
                set_default(property, Variant(0.0));
                property.step = 1.0;
                schema.properties.push_back(move(property));

                property = make_property("line_gap", "Line Gap",
                    EditingPropertySection::layout, EditingPropertyEditor::number);
                set_default(property, Variant(0.0));
                property.step = 1.0;
                schema.properties.push_back(move(property));

                property = make_property("clip_children", "Clip Children",
                    EditingPropertySection::layout, EditingPropertyEditor::boolean);
                set_default(property, Variant(false));
                schema.properties.push_back(move(property));
            }

            void add_canvas_layout_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("clip_children", "Clip Children",
                    EditingPropertySection::layout, EditingPropertyEditor::boolean);
                set_default(property, Variant(false));
                schema.properties.push_back(move(property));
            }

            void add_canvas_attachment_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("anchor_min", "Anchor Min",
                    EditingPropertySection::layout, EditingPropertyEditor::float2);
                set_default(property, make_float2(0.0, 0.0));
                property.step = 0.01;
                property.description = "Normalized minimum anchor in the parent canvas.";
                schema.properties.push_back(move(property));

                property = make_property("anchor_max", "Anchor Max",
                    EditingPropertySection::layout, EditingPropertyEditor::float2);
                set_default(property, make_float2(0.0, 0.0));
                property.step = 0.01;
                property.description = "Normalized maximum anchor in the parent canvas.";
                schema.properties.push_back(move(property));

                property = make_property("offset", "Offset",
                    EditingPropertySection::layout, EditingPropertyEditor::float4);
                set_default(property, make_float4(0.0, 0.0, 0.0, 0.0));
                property.step = 1.0;
                property.description = "Left, top, right and bottom offsets from the anchors.";
                schema.properties.push_back(move(property));

                property = make_property("pivot", "Pivot",
                    EditingPropertySection::layout, EditingPropertyEditor::float2);
                set_default(property, make_float2(0.0, 0.0));
                property.step = 0.01;
                property.description = "Normalized pivot used by canvas placement.";
                schema.properties.push_back(move(property));
            }

            void add_panel_style_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("color", "Color",
                    EditingPropertySection::style, EditingPropertyEditor::color);
                set_default(property, make_float4(0.22, 0.22, 0.22, 1.0));
                schema.properties.push_back(move(property));

                property = make_property("radius", "Corner Radius",
                    EditingPropertySection::style, EditingPropertyEditor::number);
                set_default(property, Variant(0.0));
                property.bounded = true;
                property.minimum = 0.0;
                property.maximum = 1024.0;
                property.step = 1.0;
                schema.properties.push_back(move(property));
            }

            void add_text_style_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("text_color", "Text Color",
                    EditingPropertySection::style, EditingPropertyEditor::color);
                set_default(property, make_float4(1.0, 1.0, 1.0, 1.0));
                schema.properties.push_back(move(property));

                property = make_property("font", "Font",
                    EditingPropertySection::style, EditingPropertyEditor::name);
                set_default(property, Variant(""));
                schema.properties.push_back(move(property));

                property = make_property("font_size", "Font Size",
                    EditingPropertySection::style, EditingPropertyEditor::number);
                set_default(property, Variant(16.0));
                property.bounded = true;
                property.minimum = 1.0;
                property.maximum = 512.0;
                property.step = 1.0;
                schema.properties.push_back(move(property));
            }

            void add_visual_effects_schema(EditingSchema& schema)
            {
                EditingPropertyDesc property = make_property("visual_effects", "Visual Effects",
                    EditingPropertySection::style, EditingPropertyEditor::visual_effects);
                set_default(property, Variant(VariantType::array));
                property.description = "Ordered static visuals emitted before or after child elements.";
                schema.properties.push_back(move(property));
            }

            Variant make_default_layout(f32 height = -1.0f)
            {
                Variant properties(VariantType::object);
                if(height >= 0.0f) properties["height"] = height;
                return properties;
            }

            RV register_builtin_authoring_types()
            {
                lutry
                {
                    AuthoringNodeTypeDesc desc;
                    desc.type = GameGUI::get_flex_node_type();
                    desc.name = "Flex";
                    desc.display_name = "Flex Layout";
                    desc.category = "Layout";
                    add_common_layout_schema(desc.property_schema);
                    add_flex_layout_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    desc.slot_schema = make_slot_schema("ordered_children");
                    desc.default_properties = make_default_layout();
                    luexp(register_authoring_node_type(desc));

                    desc = AuthoringNodeTypeDesc();
                    desc.type = GameGUI::get_canvas_node_type();
                    desc.name = "Canvas";
                    desc.display_name = "Canvas Layout";
                    desc.category = "Layout";
                    add_common_layout_schema(desc.property_schema);
                    add_canvas_layout_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    add_canvas_attachment_schema(desc.child_attachment_schema);
                    desc.slot_schema = make_slot_schema("canvas_children");
                    desc.default_properties = make_default_layout();
                    luexp(register_authoring_node_type(desc));

                    desc = AuthoringNodeTypeDesc();
                    desc.type = GameGUI::get_panel_node_type();
                    desc.name = "Panel";
                    desc.display_name = "Panel";
                    desc.category = "Visual";
                    add_common_layout_schema(desc.property_schema);
                    add_panel_style_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    desc.slot_schema = make_slot_schema("ordered_children");
                    desc.default_properties = make_default_layout();
                    luexp(register_authoring_node_type(desc));

                    desc = AuthoringNodeTypeDesc();
                    desc.type = GameGUI::get_text_node_type();
                    desc.name = "Text";
                    desc.display_name = "Text";
                    desc.category = "Visual";
                    add_common_layout_schema(desc.property_schema);
                    add_text_style_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    {
                        EditingPropertyDesc property = make_property("text", "Text",
                            EditingPropertySection::property, EditingPropertyEditor::string);
                        set_default(property, Variant(""));
                        desc.property_schema.properties.push_back(move(property));
                    }
                    desc.default_properties = make_default_layout(24.0f);
                    desc.default_properties["text"] = "Text";
                    luexp(register_authoring_node_type(desc));

                    desc = AuthoringNodeTypeDesc();
                    desc.type = GameGUI::get_button_node_type();
                    desc.name = "Button";
                    desc.display_name = "Button";
                    desc.category = "Input";
                    add_common_layout_schema(desc.property_schema);
                    add_panel_style_schema(desc.property_schema);
                    add_text_style_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    {
                        EditingPropertyDesc property = make_property("text", "Text",
                            EditingPropertySection::property, EditingPropertyEditor::string);
                        set_default(property, Variant(""));
                        desc.property_schema.properties.push_back(move(property));

                        property = make_property("action", "Action",
                            EditingPropertySection::property, EditingPropertyEditor::name);
                        set_default(property, Variant(""));
                        desc.property_schema.properties.push_back(move(property));

                        property = make_property("action_payload", "Action Payload",
                            EditingPropertySection::property, EditingPropertyEditor::json);
                        property.has_default = true;
                        property.default_value = Variant();
                        desc.property_schema.properties.push_back(move(property));
                    }
                    desc.default_properties = make_default_layout(32.0f);
                    desc.default_properties["text"] = "Button";
                    luexp(register_authoring_node_type(desc));

                    desc = AuthoringNodeTypeDesc();
                    desc.type = GameGUI::get_asset_instance_node_type();
                    desc.name = "AssetInstance";
                    desc.display_name = "Asset Instance";
                    desc.category = "Composition";
                    add_common_layout_schema(desc.property_schema);
                    add_visual_effects_schema(desc.property_schema);
                    {
                        EditingPropertyDesc property = make_property("asset", "Asset",
                            EditingPropertySection::property, EditingPropertyEditor::asset);
                        property.asset_type = GameGUI::get_asset_type();
                        property.optional = false;
                        property.description = "GUID of the nested GameGUI asset.";
                        desc.property_schema.properties.push_back(move(property));
                    }
                    desc.default_properties = make_default_layout();
                    luexp(register_authoring_node_type(desc));
                }
                lucatchret;
                return ok;
            }
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API Name get_authoring_data_unit()
        {
            return "Authoring";
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API Name get_authoring_asset_loader()
        {
            return "Luna.GameGUIEditor.Authoring";
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API RV register_authoring_node_type(
            const AuthoringNodeTypeDesc& desc)
        {
            if(desc.type == Guid() || desc.name.empty() || desc.current_version == 0 ||
                desc.default_properties.type() != VariantType::object)
                return E_BAD_ARGUMENTS;
            const EditingSchema* schemas[] = {&desc.property_schema,
                &desc.child_attachment_schema};
            for(const EditingSchema* schema : schemas)
            {
                for(usize i = 0; i < schema->properties.size(); ++i)
                {
                    const EditingPropertyDesc& property = schema->properties[i];
                    if(property.id.empty() || property.display_name.empty())
                        return set_error(E_BAD_ARGUMENTS,
                            "GameGUI editing properties require an ID and display name.");
                    if(property.editor == EditingPropertyEditor::size &&
                        property.alternate_id.empty())
                    {
                        return set_error(E_BAD_ARGUMENTS,
                            "GameGUI size editors require an alternate percentage property.");
                    }
                    if(property.editor == EditingPropertyEditor::enumeration &&
                        property.enumeration_items.empty())
                    {
                        return set_error(E_BAD_ARGUMENTS,
                            "GameGUI enumeration editors require at least one item.");
                    }
                    if(property.bounded && property.maximum < property.minimum)
                        return set_error(E_BAD_ARGUMENTS,
                            "GameGUI editing property bounds are reversed.");
                    for(usize j = i + 1; j < schema->properties.size(); ++j)
                    {
                        if(property.id == schema->properties[j].id)
                            return set_error(E_ALREADY_EXISTS,
                                "A GameGUI editing schema contains duplicate property IDs.");
                    }
                }
            }
            LockGuard guard(g_authoring_registry_lock);
            if(g_authoring_node_types.find(desc.type) != g_authoring_node_types.end())
                return E_ALREADY_EXISTS;
            for(const auto& existing : g_authoring_node_types)
            {
                if(existing.second.name == desc.name) return E_ALREADY_EXISTS;
            }
            g_authoring_node_types.insert(make_pair(desc.type, desc));
            return ok;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API R<AuthoringNodeTypeDesc> get_authoring_node_type(
            const Guid& type)
        {
            LockGuard guard(g_authoring_registry_lock);
            auto iter = g_authoring_node_types.find(type);
            if(iter == g_authoring_node_types.end()) return E_NOT_FOUND;
            return iter->second;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API void get_authoring_node_types(
            Vector<AuthoringNodeTypeDesc>& descriptors)
        {
            LockGuard guard(g_authoring_registry_lock);
            for(const auto& desc : g_authoring_node_types) descriptors.push_back(desc.second);
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API const AuthoringNodeRecord* find_authoring_node(
            const AuthoringDocument& document, const Guid& id)
        {
            for(const AuthoringNodeRecord& node : document.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API AuthoringNodeRecord* find_authoring_node(
            AuthoringDocument& document, const Guid& id)
        {
            for(AuthoringNodeRecord& node : document.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API RV validate_authoring_document(
            const AuthoringDocument& document, Vector<GameGUI::Diagnostic>* diagnostics)
        {
            if(document.format_version != CURRENT_AUTHORING_FORMAT_VERSION)
                return set_error(E_BAD_DATA, "The GameGUI authoring document is not current in memory.");
            Ref<GameGUI::Document> topology = new_object<GameGUI::Document>();
            topology->root = document.root;
            for(const AuthoringNodeRecord& source : document.nodes)
            {
                GameGUI::NodeRecord node;
                node.id = source.id;
                node.type = source.type;
                node.name = source.name;
                node.properties = source.properties;
                for(const AuthoringChildLink& child : source.children)
                {
                    GameGUI::ChildLink link;
                    link.child = child.child;
                    link.slot = child.slot;
                    link.attachment = child.attachment;
                    node.children.push_back(move(link));
                }
                topology->nodes.push_back(move(node));
            }
            return GameGUI::validate_document(*topology, diagnostics);
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Variant> encode_authoring_document(
            const AuthoringDocument& document)
        {
            Variant result(VariantType::object);
            result["format_version"] = (u64)CURRENT_AUTHORING_FORMAT_VERSION;
            result["root"] = guid_string(document.root).c_str();
            Variant nodes(VariantType::array);
            for(const AuthoringNodeRecord& node : document.nodes)
            {
                Variant encoded(VariantType::object);
                encoded["id"] = guid_string(node.id).c_str();
                encoded["type"] = guid_string(node.type).c_str();
                encoded["type_version"] = (u64)node.type_version;
                if(!node.name.empty()) encoded["name"] = node.name;
                encoded["properties"] = node.properties;
                Variant children(VariantType::array);
                for(const AuthoringChildLink& link : node.children)
                {
                    Variant child(VariantType::object);
                    child["child"] = guid_string(link.child).c_str();
                    if(!link.slot.empty()) child["slot"] = link.slot;
                    if(link.attachment.valid()) child["attachment"] = link.attachment;
                    children.push_back(move(child));
                }
                encoded["children"] = move(children);
                nodes.push_back(move(encoded));
            }
            result["nodes"] = move(nodes);
            if(document.extensions.valid()) result["extensions"] = document.extensions;
            return result;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Ref<AuthoringDocument>> decode_authoring_document(
            const Variant& data, Vector<GameGUI::Diagnostic>* diagnostics)
        {
            if(data.type() != VariantType::object)
                return set_error(E_BAD_DATA, "GameGUI authoring document root must be an object.");
            u32 version = (u32)data["format_version"].unum(0);
            if(version > CURRENT_AUTHORING_FORMAT_VERSION)
                return set_error(E_NOT_SUPPORTED, "GameGUI authoring document version %u is newer than %u.",
                    version, CURRENT_AUTHORING_FORMAT_VERSION);
            Ref<AuthoringDocument> document = new_object<AuthoringDocument>();
            lutry
            {
                luset(document->root, decode_guid_value(data["root"], "root"));
                const Variant& nodes = data["nodes"];
                if(nodes.type() != VariantType::array)
                    luthrow(set_error(E_BAD_DATA, "GameGUI authoring nodes must be an array."));
                for(const Variant& value : nodes.values())
                {
                    lulet(node, decode_node(value, version == 0));
                    document->nodes.push_back(move(node));
                }
                if(data.contains("extensions")) document->extensions = data["extensions"];
                document->format_version = CURRENT_AUTHORING_FORMAT_VERSION;
                if(version == 0)
                    add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::info, Guid(),
                        "Migrated GameGUI authoring document version 0 to version 1.");
                luexp(validate_authoring_document(*document, diagnostics));
                luexp(migrate_known_nodes(*document, diagnostics));
            }
            lucatchret;
            return document;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Ref<GameGUI::Document>> cook_authoring_document(
            const AuthoringDocument& source, Vector<GameGUI::Diagnostic>* diagnostics)
        {
            lutry
            {
                Ref<AuthoringDocument> document = clone_authoring_document(source);
                luexp(validate_authoring_document(*document, diagnostics));
                luexp(migrate_known_nodes(*document, diagnostics));
                Ref<GameGUI::Document> cooked = new_object<GameGUI::Document>();
                cooked->root = document->root;
                for(const AuthoringNodeRecord& source_node : document->nodes)
                {
                    auto authoring_type_result = get_authoring_node_type(source_node.type);
                    if(!authoring_type_result.valid())
                    {
                        add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::error,
                            source_node.id,
                            "The authoring node provider is unavailable, so this document cannot be cooked.");
                        luthrow(set_error(E_NOT_FOUND,
                            "A GameGUI authoring node provider required for cooking is unavailable."));
                    }
                    AuthoringNodeTypeDesc authoring_type = move(authoring_type_result.get());
                    if(source_node.type_version != authoring_type.current_version)
                    {
                        add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::error,
                            source_node.id,
                            "The authoring node payload version is unsupported by this cooker.");
                        luthrow(set_error(E_NOT_SUPPORTED,
                            "GameGUI node `%s` cannot be cooked by this editor.",
                            authoring_type.name.c_str()));
                    }
                    auto runtime_type_result = GameGUI::get_node_type(source_node.type);
                    if(!runtime_type_result.valid())
                    {
                        add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::error,
                            source_node.id,
                            "The runtime node provider required for cooking is unavailable.");
                        luthrow(set_error(E_NOT_FOUND,
                            "A GameGUI runtime node provider required for cooking is unavailable."));
                    }
                    GameGUI::NodeTypeDesc runtime_type = move(runtime_type_result.get());
                    GameGUI::NodeRecord node;
                    node.id = source_node.id;
                    node.type = source_node.type;
                    node.name = source_node.name;
                    node.properties = source_node.properties;
                    for(const AuthoringChildLink& source_child : source_node.children)
                    {
                        GameGUI::ChildLink child;
                        child.child = source_child.child;
                        child.slot = source_child.slot;
                        child.attachment = source_child.attachment;
                        node.children.push_back(move(child));
                    }
                    if(runtime_type.validate)
                    {
                        RV validation = runtime_type.validate(node, runtime_type.userdata.get());
                        if(failed(validation))
                        {
                            add_diagnostic(diagnostics, GameGUI::DiagnosticSeverity::error,
                                source_node.id, explain(validation.errcode()));
                            luthrow(validation.errcode());
                        }
                    }
                    cooked->nodes.push_back(move(node));
                }
                luexp(GameGUI::validate_document(*cooked, diagnostics));
                return cooked;
            }
            lucatchret;
            return E_FAILURE;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API RV ensure_authoring_data_unit(Asset::asset_t asset)
        {
            Vector<Asset::AssetDataUnitDesc> data_units;
            lutry
            {
                luexp(Asset::get_asset_data_units(asset, data_units));
                for(const Asset::AssetDataUnitDesc& desc : data_units)
                {
                    if(desc.id == get_authoring_data_unit())
                    {
                        if(desc.loader != get_authoring_asset_loader())
                            luthrow(set_error(E_BAD_DATA,
                                "The GameGUI Authoring data unit uses an unexpected loader."));
                        return ok;
                    }
                }
                Asset::AssetDataUnitDesc desc;
                desc.id = get_authoring_data_unit();
                desc.loader = get_authoring_asset_loader();
                luexp(Asset::add_asset_data_unit(asset, desc));
                luexp(Asset::save_asset_meta(asset));
            }
            lucatchret;
            return ok;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API RV initialize_authoring()
        {
            if(g_authoring_initialized) return ok;
            Meta::register_GameGUIEditorService_types();
            register_boxed_type<AuthoringDocument>();
            Asset::AssetLoaderDesc loader;
            loader.name = get_authoring_asset_loader();
            loader.on_load_asset_data_unit = load_authoring;
            loader.on_load_asset_data_unit_default_data = load_default_authoring;
            loader.on_save_asset_data_unit = save_authoring;
            loader.on_set_asset_data_unit = set_authoring;
            loader.on_get_referred_assets = get_authoring_references;
            Asset::register_asset_loader(loader);
            lutry
            {
                luexp(register_builtin_authoring_types());
            }
            lucatchret;
            g_authoring_initialized = true;
            return ok;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API RV cook_asset(Asset::asset_t asset)
        {
            if(!asset || Asset::get_asset_type(asset) != GameGUI::get_asset_type())
                return E_BAD_ARGUMENTS;
            lutry
            {
                luexp(ensure_authoring_data_unit(asset));
                luexp(Asset::load_asset_data_unit(asset, get_authoring_data_unit(), true));
                lulet(authoring, Asset::get_asset_data_unit_object<AuthoringDocument>(asset,
                    get_authoring_data_unit()));
                if(!authoring) luthrow(E_BAD_DATA);
                lulet(cooked, cook_authoring_document(*authoring));
                luexp(Asset::set_asset_data_unit_object(asset, Name(), cooked.object()));
                luexp(Asset::save_asset_data_unit(asset, Name()));
            }
            lucatchret;
            return ok;
        }
    }
}
