/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Authoring.hpp
* @author JXMaster
* @date 2026/8/29
*/
#pragma once
#include <Luna/GameGUI/GameGUI.hpp>
#include "Authoring.generated.hpp"

#ifndef LUNA_GAME_GUI_EDITOR_SERVICE_API
#define LUNA_GAME_GUI_EDITOR_SERVICE_API
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        //! Current GameGUI authoring document format version.
        inline constexpr u32 CURRENT_AUTHORING_FORMAT_VERSION = 1;

        //! Gets the named data unit that stores editable GameGUI source.
        //! @return Returns `Authoring`.
        LUNA_GAME_GUI_EDITOR_SERVICE_API Name get_authoring_data_unit();

        //! Gets the Asset loader name used by the authoring data unit.
        //! @return Returns the stable authoring loader name.
        LUNA_GAME_GUI_EDITOR_SERVICE_API Name get_authoring_asset_loader();

        //! Describes one ordered child edge in an authoring document.
        struct AuthoringChildLink
        {
            //! Asset-local ID of the child node.
            Guid child;
            //! Semantic slot selected by the parent node type.
            Name slot;
            //! Parent-owned layout attachment data.
            Variant attachment;
        };

        //! Stores one losslessly editable node record.
        struct AuthoringNodeRecord
        {
            //! Stable asset-local node ID.
            Guid id;
            //! Stable registered node type ID.
            Guid type;
            //! Version of the authoring property payload.
            u32 type_version = 1;
            //! Optional author-facing node name.
            Name name;
            //! Raw authoring property payload.
            Variant properties = Variant(VariantType::object);
            //! Ordered outgoing child edges.
            Vector<AuthoringChildLink> children;
        };

        //! Stores the versioned, losslessly editable representation of one GameGUI asset.
        struct [[Luna::struct("{088762B2-DC23-4185-AD75-58C4EB1DC454}")]] AuthoringDocument
        {
            //! Authoring document format version.
            u32 format_version = CURRENT_AUTHORING_FORMAT_VERSION;
            //! Asset-local root node ID.
            Guid root;
            //! Flat canonical node records.
            Vector<AuthoringNodeRecord> nodes;
            //! Forward-compatible document payload retained by the editor.
            Variant extensions;
        };

        //! Migrates one known authoring payload between adjacent versions.
        using AuthoringNodeMigrateCallback = RV(*)(Variant& properties, u32 from_version,
            u32 to_version, object_t userdata);

        //! Describes editor-only metadata for one GameGUI node type.
        struct AuthoringNodeTypeDesc
        {
            //! Stable runtime node type ID.
            Guid type;
            //! Stable programmatic type name.
            Name name;
            //! Author-facing display name.
            Name display_name;
            //! Author-facing category.
            Name category;
            //! Current authoring property payload version.
            u32 current_version = 1;
            //! Property schema consumed by inspectors.
            Variant property_schema;
            //! Event schema consumed by editors and tools.
            Variant event_schema;
            //! Child-slot schema consumed by editors and tools.
            Variant slot_schema;
            //! Style schema consumed by editors and tools.
            Variant style_schema;
            //! Parent-layout attachment schema.
            Variant layout_schema;
            //! Default property payload for newly created nodes.
            Variant default_properties = Variant(VariantType::object);
            //! Optional adjacent-version migration callback.
            AuthoringNodeMigrateCallback migrate = nullptr;
            //! Optional user object retained by the registry.
            ObjRef userdata;
        };

        //! Registers editor-only metadata for one node type.
        //! @param[in] desc The descriptor to copy into the authoring registry.
        //! @return Returns @ref E_ALREADY_EXISTS when its type or name is already registered.
        LUNA_GAME_GUI_EDITOR_SERVICE_API RV register_authoring_node_type(
            const AuthoringNodeTypeDesc& desc);

        //! Gets editor-only metadata for one node type.
        //! @param[in] type The stable runtime node type ID.
        //! @return Returns a descriptor copy, or @ref E_NOT_FOUND.
        LUNA_GAME_GUI_EDITOR_SERVICE_API R<AuthoringNodeTypeDesc> get_authoring_node_type(
            const Guid& type);

        //! Enumerates all editor-only node type descriptors.
        //! @param[out] descriptors Descriptor copies are appended to this vector.
        LUNA_GAME_GUI_EDITOR_SERVICE_API void get_authoring_node_types(
            Vector<AuthoringNodeTypeDesc>& descriptors);

        //! Finds one node in an authoring document.
        //! @param[in] document The document to search.
        //! @param[in] id The asset-local node ID.
        //! @return Returns the matching node, or `nullptr`.
        LUNA_GAME_GUI_EDITOR_SERVICE_API const AuthoringNodeRecord* find_authoring_node(
            const AuthoringDocument& document, const Guid& id);

        //! Finds one mutable node in an authoring document.
        //! @param[in] document The document to search.
        //! @param[in] id The asset-local node ID.
        //! @return Returns the matching node, or `nullptr`.
        LUNA_GAME_GUI_EDITOR_SERVICE_API AuthoringNodeRecord* find_authoring_node(
            AuthoringDocument& document, const Guid& id);

        //! Validates authoring document topology without requiring node providers.
        //! @param[in] document The authoring document to validate.
        //! @param[out] diagnostics Optional destination for validation diagnostics.
        //! @return Returns success only when the document is a single rooted tree.
        LUNA_GAME_GUI_EDITOR_SERVICE_API RV validate_authoring_document(
            const AuthoringDocument& document, Vector<GameGUI::Diagnostic>* diagnostics = nullptr);

        //! Encodes an authoring document for JSON or Frontend transport.
        //! @param[in] document The current authoring document to encode.
        //! @return Returns its canonical Variant representation.
        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Variant> encode_authoring_document(
            const AuthoringDocument& document);

        //! Decodes and migrates known records while retaining unsupported records losslessly.
        //! @param[in] data The canonical Variant representation.
        //! @param[out] diagnostics Optional destination for migration and validation diagnostics.
        //! @return Returns the current in-memory authoring document.
        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Ref<AuthoringDocument>> decode_authoring_document(
            const Variant& data, Vector<GameGUI::Diagnostic>* diagnostics = nullptr);

        //! Cooks an authoring snapshot into the latest runtime-only document representation.
        //! @param[in] document The source authoring snapshot.
        //! @param[out] diagnostics Optional destination for cooking diagnostics.
        //! @return Returns a current runtime document, or an error if a required provider is unavailable.
        LUNA_GAME_GUI_EDITOR_SERVICE_API R<Ref<GameGUI::Document>> cook_authoring_document(
            const AuthoringDocument& document, Vector<GameGUI::Diagnostic>* diagnostics = nullptr);

        //! Adds the Authoring data-unit descriptor to an asset when it is missing.
        //! @param[in] asset The GameGUI asset to update.
        //! @return Returns errors from data-unit enumeration, insertion or metadata save.
        LUNA_GAME_GUI_EDITOR_SERVICE_API RV ensure_authoring_data_unit(Asset::asset_t asset);

        //! Cooks the loaded Authoring data unit of one asset and saves its main data unit.
        //! @param[in] asset The GameGUI asset to cook.
        //! @return Returns errors from authoring load, cooking, main-unit publication or save.
        LUNA_GAME_GUI_EDITOR_SERVICE_API RV cook_asset(Asset::asset_t asset);

        //! Registers the built-in authoring loader and built-in node metadata.
        //! @return Returns success when authoring support is ready for use.
        LUNA_GAME_GUI_EDITOR_SERVICE_API RV initialize_authoring();
    }
}
