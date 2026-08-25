/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Document.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/Runtime/HashMap.hpp>

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            String encode_guid_string(const Guid& guid)
            {
                c8 buffer[GUID_STRING_LENGTH];
                RV result = Luna::encode_guid(guid, buffer, sizeof(buffer));
                luassert(succeeded(result));
                return String(buffer, GUID_STRING_LENGTH);
            }

            R<Guid> decode_guid_value(const Variant& value, const c8* field_name)
            {
                if(value.type() != VariantType::string)
                {
                    return set_error(E_BAD_DATA, "GameGUI %s must be a canonical GUID string.", field_name);
                }
                Guid guid;
                RV result = Luna::decode_guid(value.c_str(), value.str().size(), guid);
                if(!result.valid())
                {
                    return set_error(E_BAD_DATA, "GameGUI %s must be a canonical GUID string.", field_name);
                }
                return guid;
            }

            void add_diagnostic(Vector<Diagnostic>* diagnostics, DiagnosticSeverity severity,
                const Guid& node, const c8* message)
            {
                if(!diagnostics) return;
                Diagnostic diagnostic;
                diagnostic.severity = severity;
                diagnostic.node = node;
                diagnostic.message = message;
                diagnostics->push_back(move(diagnostic));
            }

            R<ChildLink> decode_child_link(const Variant& value, bool legacy)
            {
                ChildLink link;
                lutry
                {
                    if(legacy && value.type() == VariantType::string)
                    {
                        luset(link.child, decode_guid_value(value, "child"));
                    }
                    else
                    {
                        if(value.type() != VariantType::object)
                        {
                            luthrow(set_error(E_BAD_DATA, "GameGUI child link must be an object."));
                        }
                        luset(link.child, decode_guid_value(value["child"], "child"));
                        link.slot = value["slot"].str();
                        if(value.contains("attachment")) link.attachment = value["attachment"];
                    }
                }
                lucatchret;
                return link;
            }

            R<NodeRecord> decode_node(const Variant& value, bool legacy)
            {
                NodeRecord node;
                lutry
                {
                    if(value.type() != VariantType::object)
                    {
                        luthrow(set_error(E_BAD_DATA, "GameGUI node record must be an object."));
                    }
                    luset(node.id, decode_guid_value(value["id"], "node id"));
                    luset(node.type, decode_guid_value(value["type"], "node type"));
                    node.type_version = legacy ? 1 : (u32)value["type_version"].unum(1);
                    node.name = value["name"].str();
                    if(value.contains("properties")) node.properties = value["properties"];
                    const Variant& children = value["children"];
                    if(children.valid() && children.type() != VariantType::array)
                    {
                        luthrow(set_error(E_BAD_DATA, "GameGUI node children must be an array."));
                    }
                    for(const Variant& child : children.values())
                    {
                        lulet(link, decode_child_link(child, legacy));
                        node.children.push_back(move(link));
                    }
                }
                lucatchret;
                return node;
            }

            bool visit_node(u32 index, const Document& document, const HashMap<Guid, u32>& indices,
                Vector<u8>& colors, usize& visited, Vector<Diagnostic>* diagnostics)
            {
                if(colors[index] == 1)
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::error, document.nodes[index].id,
                        "A cycle exists in the GameGUI node tree.");
                    return false;
                }
                if(colors[index] == 2) return true;
                colors[index] = 1;
                ++visited;
                for(const ChildLink& child : document.nodes[index].children)
                {
                    auto child_index = indices.find(child.child);
                    if(child_index == indices.end()) continue;
                    if(!visit_node(child_index->second, document, indices, colors, visited, diagnostics))
                    {
                        return false;
                    }
                }
                colors[index] = 2;
                return true;
            }
        }

        LUNA_GAME_GUI_API Name get_asset_type()
        {
            return "GameGUI";
        }

        LUNA_GAME_GUI_API const NodeRecord* find_node(const Document& document, const Guid& id)
        {
            for(const NodeRecord& node : document.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        LUNA_GAME_GUI_API NodeRecord* find_node(Document& document, const Guid& id)
        {
            for(NodeRecord& node : document.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        R<Variant> encode_document(const Document& document)
        {
            Variant result(VariantType::object);
            result["format_version"] = (u64)CURRENT_DOCUMENT_FORMAT_VERSION;
            String root = encode_guid_string(document.root);
            result["root"] = root.c_str();
            Variant nodes(VariantType::array);
            for(const NodeRecord& node : document.nodes)
            {
                Variant encoded_node(VariantType::object);
                String node_id = encode_guid_string(node.id);
                String node_type = encode_guid_string(node.type);
                encoded_node["id"] = node_id.c_str();
                encoded_node["type"] = node_type.c_str();
                encoded_node["type_version"] = (u64)node.type_version;
                if(!node.name.empty()) encoded_node["name"] = node.name;
                encoded_node["properties"] = node.properties;
                Variant children(VariantType::array);
                for(const ChildLink& link : node.children)
                {
                    Variant child(VariantType::object);
                    String child_id = encode_guid_string(link.child);
                    child["child"] = child_id.c_str();
                    if(!link.slot.empty()) child["slot"] = link.slot;
                    if(link.attachment.valid()) child["attachment"] = link.attachment;
                    children.push_back(move(child));
                }
                encoded_node["children"] = move(children);
                nodes.push_back(move(encoded_node));
            }
            result["nodes"] = move(nodes);
            if(document.extensions.valid()) result["extensions"] = document.extensions;
            return result;
        }

        R<Ref<Document>> decode_document(const Variant& data,
            Vector<Diagnostic>* diagnostics)
        {
            if(data.type() != VariantType::object)
            {
                return set_error(E_BAD_DATA, "GameGUI document root must be an object.");
            }
            u32 version = (u32)data["format_version"].unum(0);
            if(version > CURRENT_DOCUMENT_FORMAT_VERSION)
            {
                return set_error(E_NOT_SUPPORTED, "GameGUI document format version %u is newer than version %u.",
                    version, CURRENT_DOCUMENT_FORMAT_VERSION);
            }
            Ref<Document> document = new_object<Document>();
            lutry
            {
                luset(document->root, decode_guid_value(data["root"], "root"));
                const Variant& nodes = data["nodes"];
                if(nodes.type() != VariantType::array)
                {
                    luthrow(set_error(E_BAD_DATA, "GameGUI document nodes must be an array."));
                }
                for(const Variant& value : nodes.values())
                {
                    lulet(node, decode_node(value, version == 0));
                    document->nodes.push_back(move(node));
                }
                if(data.contains("extensions")) document->extensions = data["extensions"];
                if(version == 0)
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::info, Guid(),
                        "Migrated GameGUI document format version 0 to version 1.");
                }
                document->format_version = CURRENT_DOCUMENT_FORMAT_VERSION;
                luexp(validate_document(*document, diagnostics));
            }
            lucatchret;
            return document;
        }

        LUNA_GAME_GUI_API RV validate_document(const Document& document,
            Vector<Diagnostic>* diagnostics)
        {
            bool valid = true;
            if(document.format_version != CURRENT_DOCUMENT_FORMAT_VERSION)
            {
                add_diagnostic(diagnostics, DiagnosticSeverity::error, Guid(),
                    "The in-memory GameGUI document format version is not current.");
                valid = false;
            }
            if(document.root == Guid())
            {
                add_diagnostic(diagnostics, DiagnosticSeverity::error, Guid(),
                    "The GameGUI document root ID is zero.");
                valid = false;
            }
            if(document.nodes.empty())
            {
                add_diagnostic(diagnostics, DiagnosticSeverity::error, Guid(),
                    "The GameGUI document contains no nodes.");
                valid = false;
            }
            HashMap<Guid, u32> indices;
            for(u32 i = 0; i < document.nodes.size(); ++i)
            {
                const NodeRecord& node = document.nodes[i];
                if(node.id == Guid() || node.type == Guid())
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::error, node.id,
                        "A GameGUI node has a zero node ID or type ID.");
                    valid = false;
                }
                if(node.properties.type() != VariantType::object)
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::error, node.id,
                        "GameGUI node properties must be an object.");
                    valid = false;
                }
                if(!indices.insert(make_pair(node.id, i)).second)
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::error, node.id,
                        "Duplicate GameGUI node ID.");
                    valid = false;
                }
            }
            auto root = indices.find(document.root);
            if(root == indices.end())
            {
                add_diagnostic(diagnostics, DiagnosticSeverity::error, document.root,
                    "The GameGUI root node does not exist.");
                valid = false;
            }
            Vector<u32> parent_counts(document.nodes.size(), 0);
            for(const NodeRecord& node : document.nodes)
            {
                for(const ChildLink& child : node.children)
                {
                    auto child_index = indices.find(child.child);
                    if(child_index == indices.end())
                    {
                        add_diagnostic(diagnostics, DiagnosticSeverity::error, node.id,
                            "A GameGUI child link references a missing node.");
                        valid = false;
                        continue;
                    }
                    if(++parent_counts[child_index->second] > 1)
                    {
                        add_diagnostic(diagnostics, DiagnosticSeverity::error, child.child,
                            "A GameGUI node has more than one parent.");
                        valid = false;
                    }
                }
            }
            if(root != indices.end() && parent_counts[root->second] != 0)
            {
                add_diagnostic(diagnostics, DiagnosticSeverity::error, document.root,
                    "The GameGUI root node has a parent.");
                valid = false;
            }
            if(root != indices.end())
            {
                Vector<u8> colors(document.nodes.size(), 0);
                usize visited = 0;
                if(!visit_node(root->second, document, indices, colors, visited, diagnostics)) valid = false;
                if(visited != document.nodes.size())
                {
                    add_diagnostic(diagnostics, DiagnosticSeverity::error, Guid(),
                        "The GameGUI document contains nodes outside the root tree.");
                    valid = false;
                }
            }
            if(!valid) return set_error(E_BAD_DATA, "GameGUI document topology validation failed.");
            return ok;
        }

        void get_direct_referred_assets(const Document& document,
            Vector<Asset::asset_t>& assets)
        {
            Vector<Asset::asset_t> collected;
            for(const NodeRecord& node : document.nodes)
            {
                auto desc = get_node_type(node.type);
                if(desc.valid() && desc.get().collect_assets)
                {
                    desc.get().collect_assets(node, collected, desc.get().userdata.get());
                }
            }
            for(Asset::asset_t asset : collected)
            {
                bool duplicate = false;
                for(Asset::asset_t existing : assets)
                {
                    if(existing == asset)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if(asset && !duplicate) assets.push_back(asset);
            }
        }
    }
}
