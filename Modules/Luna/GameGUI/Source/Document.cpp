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
                        return false;
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

        LUNA_GAME_GUI_API RV validate_document(const Document& document,
            Vector<Diagnostic>* diagnostics)
        {
            bool valid = true;
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
                    desc.get().collect_assets(node, collected, desc.get().userdata.get());
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
