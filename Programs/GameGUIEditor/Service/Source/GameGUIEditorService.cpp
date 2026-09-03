/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GameGUIEditorService.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_EDITOR_SERVICE_API LUNA_EXPORT
#include "../GameGUIEditorService.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Random.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace
        {
            struct HistoryEntry
            {
                Ref<AuthoringDocument> document;
                u64 state_id = 0;
                Name label;
                Name coalesce_key;
            };

            struct DocumentState
            {
                u64 id = 0;
                String title;
                Asset::asset_t asset;
                Vector<HistoryEntry> history;
                usize history_index = 0;
                u64 next_history_state = 2;
                u64 saved_state = 0;
                u64 revision = 1;
                Vector<GameGUI::Diagnostic> diagnostics;

                Ref<AuthoringDocument> document() const
                {
                    return history[history_index].document;
                }

                u64 history_state() const
                {
                    return history[history_index].state_id;
                }

                bool dirty() const
                {
                    return saved_state == 0 || history_state() != saved_state;
                }
            };

            String guid_string(const Guid& guid)
            {
                c8 buffer[GUID_STRING_LENGTH];
                RV result = encode_guid(guid, buffer, sizeof(buffer));
                luassert(succeeded(result));
                return String(buffer, sizeof(buffer));
            }

            R<Guid> guid_param(const Variant& value, const c8* field)
            {
                if(value.type() != VariantType::string)
                {
                    return set_error(E_BAD_ARGUMENTS, "%s must be a GUID string.", field);
                }
                Guid result;
                if(failed(decode_guid(value.c_str(), value.str().size(), result)))
                {
                    return set_error(E_BAD_ARGUMENTS, "%s must be a canonical GUID string.", field);
                }
                return result;
            }

            Ref<AuthoringDocument> clone_document(const AuthoringDocument& source)
            {
                Ref<AuthoringDocument> result = new_object<AuthoringDocument>();
                result->format_version = source.format_version;
                result->root = source.root;
                result->nodes = source.nodes;
                result->extensions = source.extensions;
                return result;
            }

            Variant diagnostic_variant(const GameGUI::Diagnostic& diagnostic)
            {
                Variant result(VariantType::object);
                switch(diagnostic.severity)
                {
                case GameGUI::DiagnosticSeverity::info: result["severity"] = "info"; break;
                case GameGUI::DiagnosticSeverity::warning: result["severity"] = "warning"; break;
                default: result["severity"] = "error"; break;
                }
                if(diagnostic.node != Guid()) result["node"] = guid_string(diagnostic.node).c_str();
                result["message"] = diagnostic.message.c_str();
                Variant chain(VariantType::array);
                for(const Guid& guid : diagnostic.asset_mount_chain)
                {
                    chain.push_back(guid_string(guid).c_str());
                }
                result["asset_mount_chain"] = move(chain);
                return result;
            }

            void refresh_diagnostics(DocumentState& state)
            {
                state.diagnostics.clear();
                auto cooked = cook_authoring_document(*state.document(), &state.diagnostics);
                if(!cooked.valid())
                {
                    bool has_error = false;
                    for(const GameGUI::Diagnostic& diagnostic : state.diagnostics)
                        has_error = has_error || diagnostic.severity == GameGUI::DiagnosticSeverity::error;
                    if(!has_error)
                    {
                        GameGUI::Diagnostic diagnostic;
                        diagnostic.severity = GameGUI::DiagnosticSeverity::error;
                        diagnostic.message = explain(cooked.errcode());
                        state.diagnostics.push_back(move(diagnostic));
                    }
                    return;
                }
                GameGUI::InstanceDesc desc;
                desc.document = cooked.get();
                desc.source_asset = state.asset;
                Ref<GameGUI::IInstance> instance = GameGUI::new_instance(desc);
                RV prepared = instance->prepare();
                for(const GameGUI::Diagnostic& diagnostic : instance->get_diagnostics())
                {
                    state.diagnostics.push_back(diagnostic);
                }
                if(failed(prepared) && state.diagnostics.empty())
                {
                    GameGUI::Diagnostic diagnostic;
                    diagnostic.severity = GameGUI::DiagnosticSeverity::error;
                    diagnostic.message = explain(prepared.errcode());
                    state.diagnostics.push_back(move(diagnostic));
                }
            }

            Variant metadata_variant(const DocumentState& state)
            {
                Variant result(VariantType::object);
                result["document_id"] = state.id;
                result["title"] = state.title.c_str();
                result["revision"] = state.revision;
                result["history_state"] = state.history_state();
                result["dirty"] = state.dirty();
                result["can_undo"] = state.history_index > 0;
                result["can_redo"] = state.history_index + 1 < state.history.size();
                if(state.asset)
                {
                    result["asset_guid"] = guid_string(Asset::get_asset_guid(state.asset)).c_str();
                    result["asset_path"] = Asset::get_asset_path(state.asset).encode().c_str();
                }
                Variant diagnostics(VariantType::array);
                for(const GameGUI::Diagnostic& diagnostic : state.diagnostics)
                {
                    diagnostics.push_back(diagnostic_variant(diagnostic));
                }
                result["diagnostics"] = move(diagnostics);
                return result;
            }

            R<AuthoringNodeRecord*> find_required_node(AuthoringDocument& document,
                const Variant& value)
            {
                auto id_result = guid_param(value, "command.node");
                if(!id_result.valid()) return id_result.errcode();
                Guid id = id_result.get();
                AuthoringNodeRecord* node = find_authoring_node(document, id);
                if(!node) return set_error(E_NOT_FOUND, "The GameGUI node does not exist.");
                return node;
            }

            AuthoringChildLink* find_parent_link(AuthoringDocument& document,
                const Guid& child, AuthoringNodeRecord** parent = nullptr)
            {
                for(AuthoringNodeRecord& candidate : document.nodes)
                {
                    for(AuthoringChildLink& link : candidate.children)
                    {
                        if(link.child == child)
                        {
                            if(parent) *parent = &candidate;
                            return &link;
                        }
                    }
                }
                return nullptr;
            }

            void collect_subtree(const AuthoringDocument& document, const Guid& node,
                Vector<Guid>& nodes)
            {
                nodes.push_back(node);
                const AuthoringNodeRecord* record = find_authoring_node(document, node);
                if(!record) return;
                for(const AuthoringChildLink& child : record->children)
                {
                    collect_subtree(document, child.child, nodes);
                }
            }

            bool contains_guid(const Vector<Guid>& values, const Guid& value)
            {
                for(const Guid& candidate : values)
                {
                    if(candidate == value) return true;
                }
                return false;
            }

            RV insert_child(AuthoringNodeRecord& parent, AuthoringChildLink&& link,
                usize index)
            {
                if(index > parent.children.size())
                {
                    return set_error(E_BAD_ARGUMENTS, "The child insertion index is out of range.");
                }
                parent.children.insert(parent.children.begin() + index, move(link));
                return ok;
            }

            RV apply_command(AuthoringDocument& document, const Variant& command,
                Vector<Guid>& created_nodes)
            {
                lutry
                {
                if(command.type() != VariantType::object)
                {
                    return set_error(E_BAD_ARGUMENTS, "Every GameGUI editor command must be an object.");
                }
                Name kind = command["kind"].str();
                if(kind == Name("set_name"))
                {
                    lulet(node, find_required_node(document, command["node"]));
                    if(command["name"].type() != VariantType::string)
                        return set_error(E_BAD_ARGUMENTS, "set_name.name must be a string.");
                    node->name = command["name"].str();
                    return ok;
                }
                if(kind == Name("set_property"))
                {
                    lulet(node, find_required_node(document, command["node"]));
                    if(command["property"].type() != VariantType::string ||
                        command["property"].str().empty())
                    {
                        return set_error(E_BAD_ARGUMENTS,
                            "set_property.property must be a non-empty string.");
                    }
                    node->properties[command["property"].str()] = command["value"];
                    return ok;
                }
                if(kind == Name("remove_property"))
                {
                    lulet(node, find_required_node(document, command["node"]));
                    if(command["property"].type() != VariantType::string ||
                        command["property"].str().empty())
                    {
                        return set_error(E_BAD_ARGUMENTS,
                            "remove_property.property must be a non-empty string.");
                    }
                    node->properties.erase(command["property"].str());
                    return ok;
                }
                if(kind == Name("set_properties"))
                {
                    lulet(node, find_required_node(document, command["node"]));
                    if(command["properties"].type() != VariantType::object)
                    {
                        return set_error(E_BAD_ARGUMENTS, "set_properties.properties must be an object.");
                    }
                    node->properties = command["properties"];
                    return ok;
                }
                if(kind == Name("insert_node"))
                {
                    lulet(parent_id, guid_param(command["parent"], "insert_node.parent"));
                    AuthoringNodeRecord* parent = find_authoring_node(document, parent_id);
                    if(!parent) return set_error(E_NOT_FOUND, "The insertion parent does not exist.");
                    lulet(type, guid_param(command["type"], "insert_node.type"));
                    auto descriptor = get_authoring_node_type(type);
                    if(!descriptor.valid())
                    {
                        return set_error(E_NOT_FOUND,
                            "A provider for the inserted GameGUI node type is not registered.");
                    }
                    Guid node_id;
                    if(command.contains("node"))
                    {
                        luset(node_id, guid_param(command["node"], "insert_node.node"));
                        if(node_id == Guid())
                            return set_error(E_BAD_ARGUMENTS, "insert_node.node cannot be zero.");
                    }
                    else
                    {
                        do node_id = random_guid();
                        while(find_authoring_node(document, node_id));
                    }
                    if(find_authoring_node(document, node_id))
                        return set_error(E_ALREADY_EXISTS, "The inserted GameGUI node ID already exists.");
                    AuthoringNodeRecord node;
                    node.id = node_id;
                    node.type = type;
                    node.type_version = descriptor.get().current_version;
                    node.name = command["name"].str(descriptor.get().display_name);
                    node.properties = command["properties"].type() == VariantType::object ?
                        command["properties"] : descriptor.get().default_properties;
                    if(node.properties.type() != VariantType::object)
                        node.properties = Variant(VariantType::object);
                    AuthoringChildLink link;
                    link.child = node_id;
                    link.slot = command["slot"].str();
                    if(command.contains("attachment")) link.attachment = command["attachment"];
                    usize index = command.contains("index") ?
                        (usize)command["index"].unum() : parent->children.size();
                    luexp(insert_child(*parent, move(link), index));
                    document.nodes.push_back(move(node));
                    created_nodes.push_back(node_id);
                    return ok;
                }
                if(kind == Name("set_root"))
                {
                    lulet(node, guid_param(command["node"], "set_root.node"));
                    if(node == document.root)
                        return set_error(E_BAD_ARGUMENTS, "The GameGUI node is already the root.");
                    AuthoringNodeRecord* new_root = find_authoring_node(document, node);
                    if(!new_root)
                        return set_error(E_NOT_FOUND, "The new root GameGUI node does not exist.");
                    AuthoringNodeRecord* old_parent = nullptr;
                    AuthoringChildLink* old_link = find_parent_link(document, node,
                        &old_parent);
                    if(!old_link || !old_parent)
                    {
                        return set_error(E_NOT_FOUND,
                            "The new root GameGUI node is not attached to the document tree.");
                    }
                    for(usize i = 0; i < old_parent->children.size(); ++i)
                    {
                        if(old_parent->children[i].child == node)
                        {
                            old_parent->children.erase(old_parent->children.begin() + i);
                            break;
                        }
                    }
                    AuthoringChildLink old_root_link;
                    old_root_link.child = document.root;
                    new_root->children.push_back(move(old_root_link));
                    document.root = node;
                    return ok;
                }
                if(kind == Name("remove_node"))
                {
                    lulet(node, guid_param(command["node"], "remove_node.node"));
                    if(node == document.root)
                        return set_error(E_BAD_ARGUMENTS, "The root GameGUI node cannot be removed.");
                    AuthoringNodeRecord* parent = nullptr;
                    AuthoringChildLink* link = find_parent_link(document, node, &parent);
                    if(!link || !parent)
                        return set_error(E_NOT_FOUND, "The removed GameGUI node does not exist.");
                    for(usize i = 0; i < parent->children.size(); ++i)
                    {
                        if(parent->children[i].child == node)
                        {
                            parent->children.erase(parent->children.begin() + i);
                            break;
                        }
                    }
                    Vector<Guid> removed;
                    collect_subtree(document, node, removed);
                    Vector<AuthoringNodeRecord> kept;
                    kept.reserve(document.nodes.size() - removed.size());
                    for(AuthoringNodeRecord& record : document.nodes)
                    {
                        if(!contains_guid(removed, record.id)) kept.push_back(move(record));
                    }
                    document.nodes = move(kept);
                    return ok;
                }
                if(kind == Name("move_node"))
                {
                    lulet(node, guid_param(command["node"], "move_node.node"));
                    lulet(new_parent_id, guid_param(command["parent"], "move_node.parent"));
                    if(node == document.root)
                        return set_error(E_BAD_ARGUMENTS, "The root GameGUI node cannot be moved.");
                    AuthoringNodeRecord* new_parent = find_authoring_node(document, new_parent_id);
                    if(!new_parent)
                        return set_error(E_NOT_FOUND, "The new GameGUI parent does not exist.");
                    AuthoringNodeRecord* old_parent = nullptr;
                    AuthoringChildLink* old_link = find_parent_link(document, node, &old_parent);
                    if(!old_link || !old_parent)
                        return set_error(E_NOT_FOUND, "The moved GameGUI node does not exist.");
                    AuthoringChildLink link = *old_link;
                    if(command.contains("slot")) link.slot = command["slot"].str();
                    if(command.contains("attachment")) link.attachment = command["attachment"];
                    for(usize i = 0; i < old_parent->children.size(); ++i)
                    {
                        if(old_parent->children[i].child == node)
                        {
                            old_parent->children.erase(old_parent->children.begin() + i);
                            break;
                        }
                    }
                    usize index = command.contains("index") ?
                        (usize)command["index"].unum() : new_parent->children.size();
                    return insert_child(*new_parent, move(link), index);
                }
                if(kind == Name("set_attachment"))
                {
                    lulet(node, guid_param(command["node"], "set_attachment.node"));
                    AuthoringChildLink* link = find_parent_link(document, node);
                    if(!link)
                        return set_error(E_NOT_FOUND, "The attached GameGUI node does not exist.");
                    if(command.contains("slot")) link->slot = command["slot"].str();
                    link->attachment = command["attachment"];
                    return ok;
                }
                return set_error(E_BAD_ARGUMENTS, "Unknown GameGUI editor command kind `%s`.",
                    kind.c_str());
                }
                lucatchret;
                return E_FAILURE;
            }

            const c8* editing_section_name(EditingPropertySection section)
            {
                switch(section)
                {
                case EditingPropertySection::layout: return "layout";
                case EditingPropertySection::style: return "style";
                case EditingPropertySection::property: return "property";
                }
                return "property";
            }

            const c8* editing_editor_name(EditingPropertyEditor editor)
            {
                switch(editor)
                {
                case EditingPropertyEditor::boolean: return "boolean";
                case EditingPropertyEditor::number: return "number";
                case EditingPropertyEditor::string: return "string";
                case EditingPropertyEditor::name: return "name";
                case EditingPropertyEditor::enumeration: return "enumeration";
                case EditingPropertyEditor::float2: return "float2";
                case EditingPropertyEditor::float4: return "float4";
                case EditingPropertyEditor::color: return "color";
                case EditingPropertyEditor::size: return "size";
                case EditingPropertyEditor::asset: return "asset";
                case EditingPropertyEditor::json: return "json";
                }
                return "json";
            }

            Variant editing_schema_variant(const EditingSchema& schema)
            {
                Variant result(VariantType::object);
                Variant properties(VariantType::array);
                for(const EditingPropertyDesc& desc : schema.properties)
                {
                    Variant property(VariantType::object);
                    property["id"] = desc.id;
                    if(!desc.alternate_id.empty()) property["alternate_id"] = desc.alternate_id;
                    property["display_name"] = desc.display_name.c_str();
                    if(!desc.description.empty()) property["description"] = desc.description.c_str();
                    property["section"] = editing_section_name(desc.section);
                    property["editor"] = editing_editor_name(desc.editor);
                    property["has_default"] = desc.has_default;
                    if(desc.has_default) property["default_value"] = desc.default_value;
                    property["optional"] = desc.optional;
                    property["bounded"] = desc.bounded;
                    if(desc.bounded)
                    {
                        property["minimum"] = desc.minimum;
                        property["maximum"] = desc.maximum;
                    }
                    property["step"] = desc.step;
                    if(!desc.enumeration_items.empty())
                    {
                        Variant items(VariantType::array);
                        for(const EditingEnumItemDesc& item_desc : desc.enumeration_items)
                        {
                            Variant item(VariantType::object);
                            item["value"] = item_desc.value;
                            item["display_name"] = item_desc.display_name.c_str();
                            items.push_back(move(item));
                        }
                        property["items"] = move(items);
                    }
                    if(!desc.asset_type.empty()) property["asset_type"] = desc.asset_type;
                    properties.push_back(move(property));
                }
                result["properties"] = move(properties);
                return result;
            }

            Variant node_type_variant(const AuthoringNodeTypeDesc& desc)
            {
                Variant result(VariantType::object);
                result["type"] = guid_string(desc.type).c_str();
                result["name"] = desc.name;
                result["display_name"] = desc.display_name;
                result["category"] = desc.category;
                result["current_version"] = (u64)desc.current_version;
                result["property_schema"] = editing_schema_variant(desc.property_schema);
                result["child_attachment_schema"] =
                    editing_schema_variant(desc.child_attachment_schema);
                result["slot_schema"] = desc.slot_schema;
                result["default_properties"] = desc.default_properties;
                auto runtime = GameGUI::get_node_type(desc.type);
                result["nested_document"] = runtime.valid() && runtime.get().nested_document;
                return result;
            }
        }

        class ServiceImpl
        {
        public:
            Ref<Frontend::IFrontend> frontend;
            HashMap<u64, UniquePtr<DocumentState>> documents;
            HashMap<Guid, u64> asset_documents;
            Vector<u64> document_order;
            u64 next_document_id = 1;
            u64 next_untitled_id = 1;

            R<DocumentState*> get_document(const Variant& params)
            {
                if(params.type() != VariantType::object ||
                    params["document_id"].type() != VariantType::number)
                {
                    return set_error(E_BAD_ARGUMENTS, "document_id must be provided as an integer.");
                }
                u64 id = params["document_id"].unum();
                auto iter = documents.find(id);
                if(iter == documents.end())
                    return set_error(E_NOT_FOUND, "The GameGUI editor document is not open.");
                return iter->second.get();
            }

            RV check_revision(const DocumentState& document, const Variant& params)
            {
                if(params["expected_revision"].type() != VariantType::number)
                {
                    return set_error(E_BAD_ARGUMENTS, "expected_revision must be provided as an integer.");
                }
                u64 expected = params["expected_revision"].unum();
                if(expected != document.revision)
                {
                    return set_error(E_BUSY,
                        "The GameGUI editor document revision is stale (expected %llu, current %llu).",
                        (unsigned long long)expected, (unsigned long long)document.revision);
                }
                return ok;
            }

            DocumentState* add_document(const Ref<AuthoringDocument>& source, const c8* title,
                Asset::asset_t asset, bool saved)
            {
                UniquePtr<DocumentState> state(memnew<DocumentState>());
                state->id = next_document_id++;
                state->title = title;
                state->asset = asset;
                HistoryEntry entry;
                entry.document = clone_document(*source);
                entry.state_id = 1;
                entry.label = "Initial";
                state->history.push_back(move(entry));
                state->saved_state = saved ? 1 : 0;
                refresh_diagnostics(*state);
                u64 id = state->id;
                DocumentState* result = state.get();
                documents.insert(make_pair(id, move(state)));
                document_order.push_back(id);
                if(asset) asset_documents.insert_or_assign(Asset::get_asset_guid(asset), id);
                return result;
            }

            R<Variant> create(const Variant& params)
            {
                AuthoringNodeRecord root;
                root.id = random_guid();
                root.type = GameGUI::get_flex_node_type();
                auto descriptor = get_authoring_node_type(root.type);
                if(!descriptor.valid()) return descriptor.errcode();
                root.type_version = descriptor.get().current_version;
                root.name = "Root";
                root.properties = descriptor.get().default_properties;
                Ref<AuthoringDocument> document = new_object<AuthoringDocument>();
                document->root = root.id;
                document->nodes.push_back(move(root));
                String title;
                strprintf(title, "Untitled %llu", (unsigned long long)next_untitled_id++);
                DocumentState* state = add_document(document, title.c_str(), Asset::asset_t(), false);
                return metadata_variant(*state);
            }

            R<Variant> open(const Variant& params)
            {
                lutry
                {
                    if(params.type() != VariantType::object)
                        luthrow(set_error(E_BAD_ARGUMENTS, "Open parameters must be an object."));
                    Asset::asset_t asset;
                    if(params["asset_guid"].type() == VariantType::string)
                    {
                        lulet(guid, guid_param(params["asset_guid"], "asset_guid"));
                        auto existing = asset_documents.find(guid);
                        if(existing != asset_documents.end())
                            return metadata_variant(*documents.find(existing->second)->second);
                        asset = Asset::get_asset(guid);
                    }
                    else if(params["path"].type() == VariantType::string)
                    {
                        Path path(params["path"].c_str());
                        lulet(path_asset, Asset::get_asset_by_path(path));
                        asset = path_asset;
                        Guid guid = Asset::get_asset_guid(asset);
                        auto existing = asset_documents.find(guid);
                        if(existing != asset_documents.end())
                            return metadata_variant(*documents.find(existing->second)->second);
                    }
                    else
                    {
                        luthrow(set_error(E_BAD_ARGUMENTS,
                            "Open requires either asset_guid or path."));
                    }
                    auto main_state = Asset::get_asset_data_unit_state(asset, Name());
                    if(!main_state.valid() ||
                        main_state.get() == Asset::AssetDataUnitState::unregistered)
                        luthrow(set_error(E_NOT_FOUND, "The GameGUI asset is not registered."));
                    if(Asset::get_asset_type(asset) != GameGUI::get_asset_type())
                        luthrow(set_error(E_BAD_ARGUMENTS, "The selected asset is not a GameGUI document."));
                    luexp(ensure_authoring_data_unit(asset));
                    luexp(Asset::load_asset_data_unit(asset, get_authoring_data_unit()));
                    lulet(source, Asset::get_asset_data_unit_object<AuthoringDocument>(asset,
                        get_authoring_data_unit()));
                    if(!source) luthrow(set_error(E_BAD_DATA, "The GameGUI asset has no document data."));
                    DocumentState* state = add_document(source,
                        Asset::get_asset_name(asset).c_str(), asset, true);
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> list(const Variant& params)
            {
                Variant result(VariantType::array);
                for(u64 id : document_order)
                {
                    auto iter = documents.find(id);
                    if(iter != documents.end()) result.push_back(metadata_variant(*iter->second));
                }
                return result;
            }

            R<Variant> snapshot(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    Variant result = metadata_variant(*state);
                    lulet(document, encode_authoring_document(*state->document()));
                    result["document"] = move(document);
                    return result;
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> apply(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    const Variant& commands = params["commands"];
                    if(commands.type() != VariantType::array || commands.empty())
                        luthrow(set_error(E_BAD_ARGUMENTS, "commands must be a non-empty array."));
                    Ref<AuthoringDocument> edited = clone_document(*state->document());
                    Vector<Guid> created;
                    for(const Variant& command : commands.values())
                    {
                        luexp(apply_command(*edited, command, created));
                    }
                    luexp(validate_authoring_document(*edited));
                    Name coalesce_key = params["coalesce_key"].str();
                    bool coalesce = !coalesce_key.empty() && state->history_index > 0 &&
                        state->history_index + 1 == state->history.size() &&
                        state->history[state->history_index].coalesce_key == coalesce_key &&
                        state->history_state() != state->saved_state;
                    if(coalesce)
                    {
                        state->history[state->history_index].document = edited;
                    }
                    else
                    {
                        if(state->history_index + 1 < state->history.size())
                        {
                            state->history.erase(state->history.begin() + state->history_index + 1,
                                state->history.end());
                        }
                        HistoryEntry entry;
                        entry.document = edited;
                        entry.state_id = state->next_history_state++;
                        entry.label = params["label"].str("Edit");
                        entry.coalesce_key = coalesce_key;
                        state->history.push_back(move(entry));
                        state->history_index = state->history.size() - 1;
                    }
                    ++state->revision;
                    refresh_diagnostics(*state);
                    Variant result = metadata_variant(*state);
                    Variant created_result(VariantType::array);
                    for(const Guid& id : created)
                        created_result.push_back(guid_string(id).c_str());
                    result["created_nodes"] = move(created_result);
                    return result;
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> undo(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(state->history_index == 0)
                        luthrow(set_error(E_BAD_CALLING_TIME, "The GameGUI editor document cannot be undone."));
                    --state->history_index;
                    ++state->revision;
                    refresh_diagnostics(*state);
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> redo(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(state->history_index + 1 >= state->history.size())
                        luthrow(set_error(E_BAD_CALLING_TIME, "The GameGUI editor document cannot be redone."));
                    ++state->history_index;
                    ++state->revision;
                    refresh_diagnostics(*state);
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> save(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(!state->asset)
                        luthrow(set_error(E_BAD_CALLING_TIME,
                            "An untitled GameGUI document must be saved with SaveAs."));
                    luexp(ensure_authoring_data_unit(state->asset));
                    luexp(Asset::set_asset_data_unit_object(state->asset,
                        get_authoring_data_unit(), state->document().object()));
                    luexp(Asset::save_asset_data_unit(state->asset, get_authoring_data_unit()));
                    state->saved_state = state->history_state();
                    ++state->revision;
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> save_as(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(params["path"].type() != VariantType::string ||
                        params["path"].str().empty())
                    {
                        luthrow(set_error(E_BAD_ARGUMENTS, "SaveAs path must be a non-empty string."));
                    }
                    Path path(params["path"].c_str());
                    lulet(asset, Asset::new_asset(path, GameGUI::get_asset_type(), true));
                    if(Asset::get_asset_type(asset) != GameGUI::get_asset_type())
                        luthrow(set_error(E_BAD_ARGUMENTS, "The SaveAs path belongs to another asset type."));
                    Guid guid = Asset::get_asset_guid(asset);
                    auto open_document = asset_documents.find(guid);
                    if(open_document != asset_documents.end() && open_document->second != state->id)
                        luthrow(set_error(E_ALREADY_EXISTS,
                            "The SaveAs asset is already open in another document."));
                    luexp(ensure_authoring_data_unit(asset));
                    luexp(Asset::set_asset_data_unit_object(asset, get_authoring_data_unit(),
                        state->document().object()));
                    luexp(Asset::save_asset_data_unit(asset, get_authoring_data_unit()));
                    if(state->asset) asset_documents.erase(Asset::get_asset_guid(state->asset));
                    state->asset = asset;
                    asset_documents.insert_or_assign(guid, state->id);
                    state->title = Asset::get_asset_name(asset).c_str();
                    state->saved_state = state->history_state();
                    ++state->revision;
                    refresh_diagnostics(*state);
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> close(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(state->dirty() && !params["discard"].boolean(false))
                    {
                        luthrow(set_error(E_BUSY,
                            "The GameGUI editor document has unsaved changes; explicit discard is required."));
                    }
                    u64 id = state->id;
                    if(state->asset) asset_documents.erase(Asset::get_asset_guid(state->asset));
                    documents.erase(id);
                    for(usize i = 0; i < document_order.size(); ++i)
                    {
                        if(document_order[i] == id)
                        {
                            document_order.erase(document_order.begin() + i);
                            break;
                        }
                    }
                    Variant result(VariantType::object);
                    result["closed"] = true;
                    result["document_id"] = id;
                    return result;
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> cook(const Variant& params)
            {
                lutry
                {
                    lulet(state, get_document(params));
                    luexp(check_revision(*state, params));
                    if(!state->asset)
                        luthrow(set_error(E_BAD_CALLING_TIME,
                            "An untitled GameGUI document must be saved before cooking."));
                    Vector<GameGUI::Diagnostic> diagnostics;
                    lulet(cooked, cook_authoring_document(*state->document(), &diagnostics));
                    luexp(Asset::set_asset_data_unit_object(state->asset, Name(), cooked.object()));
                    luexp(Asset::save_asset_data_unit(state->asset, Name()));
                    return metadata_variant(*state);
                }
                lucatchret;
                return E_FAILURE;
            }

            R<Variant> node_types(const Variant& params)
            {
                Vector<AuthoringNodeTypeDesc> descriptors;
                get_authoring_node_types(descriptors);
                Variant result(VariantType::array);
                for(const AuthoringNodeTypeDesc& descriptor : descriptors)
                    result.push_back(node_type_variant(descriptor));
                return result;
            }

            RV install()
            {
                frontend = Frontend::new_frontend();
                lutry
                {
                    luexp(frontend->set_resource_function(CREATE_DOCUMENT_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return create(params); }));
                    luexp(frontend->set_resource_function(OPEN_DOCUMENT_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return open(params); }));
                    luexp(frontend->set_resource_function(LIST_DOCUMENTS_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return list(params); }));
                    luexp(frontend->set_resource_function(GET_SNAPSHOT_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return snapshot(params); }));
                    luexp(frontend->set_resource_function(APPLY_COMMANDS_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return apply(params); }));
                    luexp(frontend->set_resource_function(UNDO_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return undo(params); }));
                    luexp(frontend->set_resource_function(REDO_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return redo(params); }));
                    luexp(frontend->set_resource_function(SAVE_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return save(params); }));
                    luexp(frontend->set_resource_function(SAVE_AS_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return save_as(params); }));
                    luexp(frontend->set_resource_function(COOK_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return cook(params); }));
                    luexp(frontend->set_resource_function(CLOSE_DOCUMENT_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return close(params); }));
                    luexp(frontend->set_resource_function(GET_NODE_TYPES_URL,
                        [this](Frontend::IFrontend*, const Variant& params) { return node_types(params); }));
                }
                lucatchret;
                return ok;
            }
        };

        Service::Service() : m_impl(memnew<ServiceImpl>()) {}

        Service::~Service()
        {
            m_impl->frontend.reset();
            memdelete(m_impl);
        }

        RV Service::init()
        {
            return m_impl->install();
        }

        Frontend::IFrontend* Service::frontend() const
        {
            return m_impl->frontend;
        }

        LUNA_GAME_GUI_EDITOR_SERVICE_API R<UniquePtr<Service>> new_service()
        {
            RV authoring_result = initialize_authoring();
            if(failed(authoring_result)) return authoring_result.errcode();
            void* memory = memalloc(sizeof(Service), alignof(Service));
            UniquePtr<Service> service(new (memory) Service());
            RV result = service->init();
            if(failed(result)) return result.errcode();
            return service;
        }
    }
}
