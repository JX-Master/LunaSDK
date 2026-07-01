/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIAsset.hpp
* @author JXMaster
* @date 2026/6/10
*/
#pragma once
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Any.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/GUICore/GUICore.hpp>
#include "GUIAsset.generated.hpp"

#ifndef LUNA_GUI_ASSET_API
#define LUNA_GUI_ASSET_API
#endif

namespace Luna
{
    namespace GUIAsset
    {
        //! @addtogroup GUIAsset GUI Asset
        //! Runtime representation and generation support for visual GUI design assets.
        //! @{

        struct Node;
        struct Asset;
        struct GenerateContext;

        namespace AssetTopologyAccess
        {
            Guid root(const Asset* asset);
            void set_root(Asset* asset, const Guid& root);
            HashMap<Guid, Ref<Node>>& nodes(Asset* asset);
            const HashMap<Guid, Ref<Node>>& nodes(const Asset* asset);
            Guid parent(const Node* node);
            void set_parent(Node* node, const Guid& parent);
            Vector<Guid>& children(Node* node);
            const Vector<Guid>& children(const Node* node);
        }

        //! Callback used by one GUI asset node type to generate GUI Core elements.
        //! @param[in] context The GUI Core context to build into.
        //! @param[in,out] node The GUI asset node being generated. The node may keep transient runtime values
        //! for widgets that need stable pointers during the generated frame.
        //! @param[in] generate_context Shared generation context.
        //! @return Returns success or failure code.
        using node_generate_core_func_t = RV(*)(GUICore::IContext* context, Node& node, const GenerateContext& generate_context);

        //! Callback used by one GUI asset node type to report referred assets.
        //! @param[in] node The GUI asset node to inspect.
        //! @param[out] referred_assets Receives referred assets appended to the end of the vector.
        using node_get_referred_assets_func_t = void(*)(const Node& node, Vector<Luna::Asset::asset_t>& referred_assets);

        //! Selects the editor widget used to edit one GUI asset property.
        enum class NodePropertyKind : u8
        {
            //! UTF-8 string value.
            string,
            //! Boolean value.
            boolean,
            //! Signed integer value.
            integer,
            //! Floating-point number value.
            number,
            //! String value selected from a fixed list of entries.
            enum_string,
            //! GUI size object with `width` and `height` fields.
            size,
            //! GUI edge-insets object with `left`, `top`, `right` and `bottom` fields.
            edge_insets,
            //! Layout descriptor object with `padding` and `gap` fields.
            layout_desc,
            //! String array value.
            string_array,
            //! Number array value.
            number_array,
            //! Serialized asset reference value.
            asset
        };

        //! Design-time schema for one editable GUI asset node property.
        struct NodePropertyDesc
        {
            //! Property key stored in @ref Node::properties.
            Name key;
            //! Human-facing name shown by visual editors. If empty, @ref key is used.
            String display_name;
            //! Optional category used by visual editors to group related properties.
            String category;
            //! Editor and value kind.
            NodePropertyKind kind = NodePropertyKind::string;
            //! Default value used by editors when the property is missing from the node.
            Variant default_value;
            //! Minimum numeric value used by number editors.
            f64 min_value = 0.0;
            //! Maximum numeric value used by number editors.
            f64 max_value = 1.0;
            //! Numeric drag speed or step size.
            f32 speed = 1.0f;
            //! Valid string values for @ref NodePropertyKind::enum_string.
            Vector<String> enum_items;
        };

        //! Runtime descriptor for one registered GUI asset node type.
        struct NodeTypeDesc
        {
            //! Unique node type name stored in GUI asset JSON.
            Name type;
            //! Default property object copied into newly created nodes of this type.
            Variant default_properties;
            //! Design-time property schema used by GUI editors and external tooling.
            Vector<NodePropertyDesc> properties;
            //! Callback used to generate GUI Core elements for this node type.
            node_generate_core_func_t on_generate_core = nullptr;
            //! Optional callback used to collect assets referred by this node type.
            node_get_referred_assets_func_t on_get_referred_assets = nullptr;
        };

        //! Shared generation context passed to every GUI asset node.
        struct GenerateContext
        {
            //! The owning GUI asset handle when generation originates from an Asset module asset.
            Luna::Asset::asset_t owner_asset;
            //! The asset currently being generated.
            Asset* asset = nullptr;
            //! Root rectangle used by GUI Core generation in layer coordinates.
            //! @remark When width or height is not positive, GUI Core generation falls back to the current
            //! context screen size.
            RectF core_root_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        };

        //! One editable GUI asset node stored by @ref Asset.
        //! @remark This is design-time data. It is intentionally converted to runtime GUI Core elements by @ref generate.
        struct [[Luna::struct("{3FE4777D-7310-40AD-865E-AE08C0F3D31B}")]] Node
        {
            //! Stable node ID generated when the node is created and preserved across save/load.
            Guid id = Guid(0, 0);
            //! Registered node type name.
            Name type;
            //! Human-facing label or text used by most built-in widget generators.
            String label;
            //! Type-specific editable properties.
            Variant properties = Variant(VariantType::object);
            //! Optional GUI Core layout configuration assigned before generating this node.
            GUICore::LayoutConfig layout_config;
            //! Whether @ref layout_config should be applied.
            bool has_layout_config = false;
            //! Optional canvas placement assigned before generating this node inside a canvas layout.
            GUICore::CanvasLayoutItem canvas_layout;
            //! Whether @ref canvas_layout should be applied.
            bool has_canvas_layout = false;
            //! Optional GUI style name bound before generating this node.
            Name style;
            //! Whether this node is enabled for interaction.
            bool enabled = true;
            //! Transient runtime values used by generated widgets. This field is not serialized.
            HashMap<Name, Any> runtime_values;

        private:
            //! Parent node ID, or zero for a root or detached node.
            Guid m_parent = Guid(0, 0);
            //! Ordered child node IDs.
            Vector<Guid> m_children;

            friend Guid AssetTopologyAccess::parent(const Node* node);
            friend void AssetTopologyAccess::set_parent(Node* node, const Guid& parent);
            friend Vector<Guid>& AssetTopologyAccess::children(Node* node);
            friend const Vector<Guid>& AssetTopologyAccess::children(const Node* node);
        };

        //! One GUI design asset.
        struct [[Luna::struct("{32575C1B-F418-4C1B-84AB-58B3C4FF61D6}")]] Asset
        {
            //! GUI asset schema version.
            u32 version = 1;

        private:
            //! Root editable node ID.
            Guid m_root = Guid(0, 0);
            //! All editable nodes indexed by stable node ID.
            HashMap<Guid, Ref<Node>> m_nodes;

            friend Guid AssetTopologyAccess::root(const Asset* asset);
            friend void AssetTopologyAccess::set_root(Asset* asset, const Guid& root);
            friend HashMap<Guid, Ref<Node>>& AssetTopologyAccess::nodes(Asset* asset);
            friend const HashMap<Guid, Ref<Node>>& AssetTopologyAccess::nodes(const Asset* asset);
        };

        //! Gets the Asset module type name used by GUI assets.
        //! @return Returns `GUIAsset`.
        LUNA_GUI_ASSET_API Name asset_type_name();

        //! Registers one GUI asset node type.
        //! @param[in] desc The node type descriptor. Existing descriptors with the same type name are replaced.
        LUNA_GUI_ASSET_API void register_node_type(const NodeTypeDesc& desc);

        //! Gets one registered GUI asset node type descriptor.
        //! @param[in] type The node type name.
        //! @return Returns the descriptor, or an error if no descriptor is registered for the name.
        LUNA_GUI_ASSET_API R<NodeTypeDesc> get_node_type(const Name& type);

        //! Gets all registered GUI asset node type names.
        //! @param[out] out_types Receives node type names appended to the end of the vector.
        LUNA_GUI_ASSET_API void get_node_types(Vector<Name>& out_types);

        //! Creates one node of a registered type and initializes it with stable ID and default properties.
        //! @param[in] type The registered node type name.
        //! @param[in] label Optional node label.
        //! @return Returns the created node.
        LUNA_GUI_ASSET_API R<Ref<Node>> new_node(const Name& type, const c8* label = nullptr);

        //! Gets the stable GUI Core element ID normally used when generating one GUI asset node.
        //! @param[in] node The GUI asset node.
        //! @return Returns the nonzero GUI Core ID derived from @ref Node::id.
        //! @remark Custom node generators should use this helper instead of duplicating the ID mapping algorithm.
        LUNA_GUI_ASSET_API GUICore::id_t node_core_id(const Node& node);

        //! Creates one GUI asset with a default vertical layout root.
        //! @return Returns the created asset.
        LUNA_GUI_ASSET_API Ref<Asset> new_asset();

        //! Gets the root node ID of one asset.
        //! @param[in] asset The asset to inspect.
        //! @return Returns the root node ID, or zero if the asset is `nullptr` or has no root.
        LUNA_GUI_ASSET_API Guid get_root(const Asset* asset);

        //! Gets the number of nodes stored in one asset.
        //! @param[in] asset The asset to inspect.
        //! @return Returns the node count.
        LUNA_GUI_ASSET_API usize get_node_count(const Asset* asset);

        //! Gets the parent node ID of one node.
        //! @param[in] node The node to inspect.
        //! @return Returns the parent node ID, or zero if the node is `nullptr`, root or detached.
        LUNA_GUI_ASSET_API Guid get_parent(const Node* node);

        //! Gets the ordered child node IDs of one node.
        //! @param[in] node The node to inspect.
        //! @return Returns a read-only span of child node IDs.
        LUNA_GUI_ASSET_API Span<const Guid> get_children(const Node* node);

        //! Gets the number of children of one node.
        //! @param[in] node The node to inspect.
        //! @return Returns the child count.
        LUNA_GUI_ASSET_API usize get_child_count(const Node* node);

        //! Gets one child node ID by index.
        //! @param[in] node The node to inspect.
        //! @param[in] index The child index.
        //! @return Returns the child node ID, or zero if the index is out of range.
        LUNA_GUI_ASSET_API Guid get_child(const Node* node, usize index);

        //! Finds one node in an asset by stable ID.
        //! @param[in] asset The asset to search.
        //! @param[in] id The node ID.
        //! @return Returns the node, or `nullptr` if the node does not exist.
        LUNA_GUI_ASSET_API Ref<Node> find_node(Asset* asset, const Guid& id);

        //! Finds one node in an asset by stable ID.
        //! @param[in] asset The asset to search.
        //! @param[in] id The node ID.
        //! @return Returns the node, or `nullptr` if the node does not exist.
        LUNA_GUI_ASSET_API Ref<Node> find_node(const Asset* asset, const Guid& id);

        //! Adds a node to an asset and optionally inserts it under a parent.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] node The node to add. If its ID is zero, a new ID is generated.
        //! @param[in] parent The parent node ID. Passing zero inserts a detached node, or the root if the asset has no root.
        //! @param[in] index The child insertion index. Passing @ref USIZE_MAX appends to the end.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV add_node(Asset* asset, Ref<Node> node, const Guid& parent = Guid(0, 0), usize index = USIZE_MAX);

        //! Removes a node and all of its descendants from an asset.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] id The node ID to remove.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV remove_node(Asset* asset, const Guid& id);

        //! Makes one existing node the root node of the asset.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] id The node ID to set as root.
        //! @return Returns success or failure code.
        //! @remark The new root is detached from its previous parent. The old root remains in the asset as a
        //! detached node unless it is also an ancestor of the new root.
        LUNA_GUI_ASSET_API RV set_root(Asset* asset, const Guid& id);

        //! Detaches one existing node from its parent while keeping the node and its descendants in the asset.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] id The node ID to detach.
        //! @return Returns success or failure code.
        //! @remark Detaching the root node is a no-op.
        LUNA_GUI_ASSET_API RV detach_node(Asset* asset, const Guid& id);

        //! Moves one existing node under another parent node.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] id The node ID to move.
        //! @param[in] new_parent The new parent node ID.
        //! @param[in] index The new child insertion index. Passing @ref USIZE_MAX appends to the end.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV move_node(Asset* asset, const Guid& id, const Guid& new_parent, usize index = USIZE_MAX);

        //! Reorders one existing node among its siblings.
        //! @param[in,out] asset The asset to modify.
        //! @param[in] id The node ID to reorder.
        //! @param[in] index The new child insertion index. Passing @ref USIZE_MAX moves the node to the end.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV reorder_node(Asset* asset, const Guid& id, usize index = USIZE_MAX);

        //! Generates GUI Core elements from one GUI asset.
        //! @param[in] context The GUI Core context to build into.
        //! @param[in,out] asset The GUI asset to generate.
        //! @param[in] generate_context Optional generation context.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV generate(GUICore::IContext* context, Asset* asset, const GenerateContext& generate_context = GenerateContext());

        //! Generates GUI Core elements from one GUI asset node.
        //! @param[in] context The GUI Core context to build into.
        //! @param[in,out] node The GUI asset node to generate.
        //! @param[in] generate_context Shared generation context.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV generate_node(GUICore::IContext* context, Node& node, const GenerateContext& generate_context = GenerateContext());

        //! Generates GUI Core elements for all children of one GUI asset node.
        //! @param[in] context The GUI Core context to build into.
        //! @param[in,out] node The parent node whose children should be generated.
        //! @param[in] generate_context Shared generation context.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV generate_children(GUICore::IContext* context, Node& node, const GenerateContext& generate_context = GenerateContext());

        //! Serializes one GUI asset node to a schema-controlled variant object.
        //! @param[in] node The node to serialize.
        //! @return Returns the serialized variant object.
        LUNA_GUI_ASSET_API R<Variant> serialize_node(const Node& node);

        //! Deserializes one GUI asset node from a variant object.
        //! @param[in] data The serialized node object.
        //! @return Returns the loaded node.
        LUNA_GUI_ASSET_API R<Ref<Node>> deserialize_node(const Variant& data);

        //! Serializes one GUI asset to a schema-controlled variant object.
        //! @param[in] asset The asset to serialize.
        //! @return Returns the serialized variant object.
        LUNA_GUI_ASSET_API R<Variant> serialize_asset(const Asset& asset);

        //! Deserializes one GUI asset from a variant object.
        //! @param[in] data The serialized asset object.
        //! @return Returns the loaded asset.
        LUNA_GUI_ASSET_API R<Ref<Asset>> deserialize_asset(const Variant& data);

        //! Saves one GUI asset to a JSON file.
        //! @param[in] asset The asset to save.
        //! @param[in] path The VFS path of the JSON file.
        //! @return Returns success or failure code.
        LUNA_GUI_ASSET_API RV save_asset_to_json_file(const Asset& asset, const Path& path);

        //! Loads one GUI asset from a JSON file.
        //! @param[in] path The VFS path of the JSON file.
        //! @return Returns the loaded asset.
        LUNA_GUI_ASSET_API R<Ref<Asset>> load_asset_from_json_file(const Path& path);

        //! Collects assets referred by one GUI asset.
        //! @param[in] asset The GUI asset to inspect.
        //! @param[out] referred_assets Receives referred assets appended to the end of the vector.
        LUNA_GUI_ASSET_API void get_referred_assets(const Asset& asset, Vector<Luna::Asset::asset_t>& referred_assets);

        //! Gets the GUIAsset module object.
        //! @return Returns the module object.
        LUNA_GUI_ASSET_API Module* module_gui_asset();

        //! @}
    }
}
