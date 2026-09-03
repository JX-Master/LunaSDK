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

        //! Identifies the Inspector section that contains one editable property.
        enum class EditingPropertySection : u8
        {
            //! Geometry and layout behavior.
            layout,
            //! Visual appearance.
            style,
            //! Node-specific content, resources and actions.
            property
        };

        //! Identifies the editor used to manipulate one authoring property.
        enum class EditingPropertyEditor : u8
        {
            //! Boolean checkbox.
            boolean,
            //! Scalar floating-point drag editor.
            number,
            //! UTF-8 text input.
            string,
            //! Interned-name text input.
            name,
            //! Single-selection enumeration.
            enumeration,
            //! Two-component floating-point editor.
            float2,
            //! Four-component floating-point editor.
            float4,
            //! Four-component normalized color editor.
            color,
            //! Auto, fixed or percentage size editor backed by two mutually exclusive properties.
            size,
            //! Asset GUID editor.
            asset,
            //! Raw JSON value editor.
            json
        };

        //! Describes one enumeration item exposed by an editing property.
        struct EditingEnumItemDesc
        {
            //! Value stored in the authoring document.
            Name value;
            //! Human-readable item label.
            String display_name;
        };

        //! Describes one field in an editor-only property schema.
        struct EditingPropertyDesc
        {
            //! Property name in the target Variant object.
            Name id;
            //! Secondary property used by compound editors such as percentage size.
            Name alternate_id;
            //! Human-readable field label.
            String display_name;
            //! Optional tooltip or help text.
            String description;
            //! Inspector section containing this field.
            EditingPropertySection section = EditingPropertySection::property;
            //! Editor used to manipulate the field.
            EditingPropertyEditor editor = EditingPropertyEditor::json;
            //! Value displayed when the property is absent.
            Variant default_value;
            //! Whether @ref default_value is defined, including an explicit null default.
            bool has_default = false;
            //! Whether the property may be absent from the authoring document.
            bool optional = true;
            //! Whether numeric values are clamped to @ref minimum and @ref maximum.
            bool bounded = false;
            //! Inclusive numeric minimum when @ref bounded is true.
            f64 minimum = 0.0;
            //! Inclusive numeric maximum when @ref bounded is true.
            f64 maximum = 0.0;
            //! Suggested drag increment for numeric editors.
            f64 step = 0.1;
            //! Items available to an enumeration editor.
            Vector<EditingEnumItemDesc> enumeration_items;
            //! Required asset type for an asset editor. An empty name accepts any type.
            Name asset_type;
        };

        //! Ordered collection of fields that edit one Variant object.
        struct EditingSchema
        {
            //! Fields displayed by the Inspector in registration order.
            Vector<EditingPropertyDesc> properties;
        };

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
            //! Schema for values stored in @ref AuthoringNodeRecord::properties.
            EditingSchema property_schema;
            //! Schema for child-link attachments owned by nodes of this type.
            EditingSchema child_attachment_schema;
            //! Child-slot schema consumed by editors and tools.
            Variant slot_schema;
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
