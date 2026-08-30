/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Document.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include <Luna/Asset/Asset.hpp>
#include <Luna/Runtime/Variant.hpp>

#ifndef LUNA_GAME_GUI_API
#define LUNA_GAME_GUI_API
#endif

namespace Luna
{
    namespace GameGUI
    {
        //! Describes the severity of one document or instance diagnostic.
        enum class DiagnosticSeverity : u8
        {
            //! Informational diagnostic.
            info,
            //! Recoverable warning.
            warning,
            //! Error that prevents validation or generation.
            error
        };

        //! Describes one document or instance diagnostic.
        struct Diagnostic
        {
            //! Diagnostic severity.
            DiagnosticSeverity severity = DiagnosticSeverity::error;
            //! Node associated with the diagnostic, or zero when the diagnostic is document-wide.
            Guid node;
            //! Human-readable diagnostic text.
            String message;
            //! Nested asset GUID mount chain associated with this diagnostic.
            Vector<Guid> asset_mount_chain;
        };

        //! Describes one ordered child edge in a GameGUI semantic tree.
        struct ChildLink
        {
            //! Asset-local ID of the child node.
            Guid child;
            //! Semantic slot selected by the parent node type.
            Name slot;
            //! Parent-owned attachment data, such as Canvas placement.
            Variant attachment;
        };

        //! Stores one node in the canonical flat document representation.
        struct NodeRecord
        {
            //! Stable asset-local node ID.
            Guid id;
            //! Stable registered node type ID.
            Guid type;
            //! Optional author-facing node name.
            Name name;
            //! Current cooked property payload.
            Variant properties = Variant(VariantType::object);
            //! Ordered outgoing child edges.
            Vector<ChildLink> children;
        };

        //! Stores one current cooked GameGUI semantic node tree.
        //! @remark Runtime element handles, instance state and prepared caches are never stored in this object.
        struct Document
        {
            lustruct("Luna::GameGUI::Document", "{318C604E-37FB-453E-BF7E-FA5532EEFF27}");

            //! Asset-local root node ID.
            Guid root;
            //! Flat canonical node records.
            Vector<NodeRecord> nodes;
        };

        //! Gets the Asset type name used by GameGUI documents.
        //! @return Returns `GameGUI`.
        LUNA_GAME_GUI_API Name get_asset_type();

        //! Finds one node in a document.
        //! @param[in] document The document to search.
        //! @param[in] id Asset-local node ID.
        //! @return Returns the node, or `nullptr` when no node matches.
        LUNA_GAME_GUI_API const NodeRecord* find_node(const Document& document, const Guid& id);

        //! Finds one mutable node in a document.
        //! @param[in] document The document to search.
        //! @param[in] id Asset-local node ID.
        //! @return Returns the node, or `nullptr` when no node matches.
        LUNA_GAME_GUI_API NodeRecord* find_node(Document& document, const Guid& id);

        //! Validates document topology and canonical record invariants.
        //! @param[in] document The document to validate.
        //! @param[out] diagnostics Optional destination for all validation errors.
        //! @return Returns success only when the document is a single rooted tree.
        LUNA_GAME_GUI_API RV validate_document(const Document& document,
            Vector<Diagnostic>* diagnostics = nullptr);

    }
}
