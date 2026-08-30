/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Node.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "Document.hpp"
#include <Luna/GUI/GUI.hpp>
#include <Luna/Runtime/Any.hpp>

namespace Luna
{
    namespace GameGUI
    {
        namespace Internal
        {
            struct Instance;
        }

        //! Provides controlled services to node build callbacks.
        class BuildContext
        {
        public:
            //! Gets the target GUI context.
            LUNA_GAME_GUI_API GUI::IContext* gui() const;
            //! Gets a deterministic GUI ID for one semantic role on the current node.
            LUNA_GAME_GUI_API GUI::id_t make_id(const c8* role) const;
            //! Gets a deterministic GUI ID for one direct child semantic role.
            LUNA_GAME_GUI_API GUI::id_t make_child_id(const Guid& child, const c8* role) const;
            //! Gets persistent per-instance state for the current mounted node.
            LUNA_GAME_GUI_API Variant& state();
            //! Gets optional typed immutable data compiled during instance preparation.
            LUNA_GAME_GUI_API const Any& prepared_data() const;
            //! Builds all direct child links in authored order.
            LUNA_GAME_GUI_API RV build_children();
            //! Gets the incoming parent-owned child link, or `nullptr` for an instance root.
            LUNA_GAME_GUI_API const ChildLink* parent_link() const;
            //! Builds one nested GameGUI document asset at the current mount point.
            //! @param[in] asset The already loaded GameGUI document asset.
            //! @return Returns the nested generated root element.
            LUNA_GAME_GUI_API R<GUI::ElementHandle> build_nested(Asset::asset_t asset);
            //! Resolves one already loaded resource using the instance resolver.
            LUNA_GAME_GUI_API R<ObjRef> resolve_resource(Asset::asset_t asset) const;
            //! Installs a Flex layout descriptor whose storage remains valid for the current frame.
            LUNA_GAME_GUI_API void set_flex_layout(const GUI::ElementHandle& element,
                const GUI::FlexLayoutDesc& desc);
            //! Installs a Canvas layout descriptor whose item storage remains valid for the current frame.
            LUNA_GAME_GUI_API void set_canvas_layout(const GUI::ElementHandle& element,
                Span<const GUI::CanvasLayoutItem> items, bool clip_children = false);

        private:
            friend struct Internal::Instance;
            explicit BuildContext(void* impl) : m_impl(impl) {}
            void* m_impl;
        };

        //! Provides controlled services to node interaction resolution callbacks.
        class ResolveContext
        {
        public:
            //! Gets the target GUI context.
            LUNA_GAME_GUI_API GUI::IContext* gui() const;
            //! Gets the generated GUI ID for one semantic role on the current node.
            LUNA_GAME_GUI_API GUI::id_t make_id(const c8* role) const;
            //! Gets persistent per-instance state for the current mounted node.
            LUNA_GAME_GUI_API Variant& state();
            //! Gets optional typed immutable data compiled during instance preparation.
            LUNA_GAME_GUI_API const Any& prepared_data() const;
            //! Emits one symbolic action.
            LUNA_GAME_GUI_API void emit_action(const Name& name, const Variant& payload = Variant());
            //! Requests one host-driven layout pass before draw generation.
            LUNA_GAME_GUI_API void request_relayout();

        private:
            friend struct Internal::Instance;
            explicit ResolveContext(void* impl) : m_impl(impl) {}
            void* m_impl;
        };

        //! Validates the payload of one known node.
        using NodeValidateCallback = RV(*)(const NodeRecord& node, object_t userdata);
        //! Compiles one known raw node payload into optional typed immutable data.
        //! @remark Types stored in the returned @ref Any must be registered with Runtime.
        using NodePrepareCallback = R<Any>(*)(const NodeRecord& node, object_t userdata);
        //! Constructs persistent per-instance state for one mounted node.
        using NodeStateCreateCallback = Variant(*)(object_t userdata);
        //! Builds one known semantic node into GUI elements.
        using NodeBuildCallback = R<GUI::ElementHandle>(*)(BuildContext& context, const NodeRecord& node,
            object_t userdata);
        //! Resolves interactions for one known semantic node after GUI input routing.
        using NodeResolveCallback = RV(*)(ResolveContext& context, const NodeRecord& node,
            object_t userdata);
        //! Enumerates direct asset references stored by one known semantic node.
        using NodeCollectAssetsCallback = void(*)(const NodeRecord& node, Vector<Asset::asset_t>& assets,
            object_t userdata);

        //! Describes one registered GameGUI semantic node type.
        struct NodeTypeDesc
        {
            //! Stable type ID serialized in node records.
            Guid type;
            //! Stable programmatic type name.
            Name name;
            //! Optional known-node validation callback.
            NodeValidateCallback validate = nullptr;
            //! Optional typed immutable cache construction callback.
            NodePrepareCallback prepare = nullptr;
            //! Optional per-instance state construction callback.
            NodeStateCreateCallback create_state = nullptr;
            //! Required runtime generation callback.
            NodeBuildCallback build = nullptr;
            //! Optional interaction resolution callback.
            NodeResolveCallback resolve = nullptr;
            //! Optional direct asset reference enumerator.
            NodeCollectAssetsCallback collect_assets = nullptr;
            //! Whether references enumerated by @ref collect_assets mount nested GameGUI documents.
            bool nested_document = false;
            //! User object retained by the registry and passed to callbacks.
            ObjRef userdata;
        };

        //! Registers one semantic node type.
        //! @param[in] desc Descriptor copied into the registry.
        //! @return Returns @ref E_ALREADY_EXISTS if the type ID or name is already registered.
        LUNA_GAME_GUI_API RV register_node_type(const NodeTypeDesc& desc);

        //! Unregisters one semantic node type.
        //! @param[in] type Type ID to remove.
        //! @return Returns @ref E_NOT_FOUND if the type is not registered.
        LUNA_GAME_GUI_API RV unregister_node_type(const Guid& type);

        //! Gets one semantic node descriptor.
        //! @param[in] type Type ID to query.
        //! @return Returns a descriptor copy or @ref E_NOT_FOUND.
        LUNA_GAME_GUI_API R<NodeTypeDesc> get_node_type(const Guid& type);

        //! Enumerates all registered semantic node descriptors.
        //! @param[out] descriptors Descriptor copies are appended to this vector.
        LUNA_GAME_GUI_API void get_node_types(Vector<NodeTypeDesc>& descriptors);
    }
}
