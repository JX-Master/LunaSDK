/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Instance.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "Node.hpp"
#include <Luna/Runtime/Interface.hpp>
#include "Instance.generated.hpp"

namespace Luna
{
    namespace GameGUI
    {
        //! Resolves an already loaded asset into an object usable by GameGUI.
        using ResourceResolveCallback = R<ObjRef> (*)(object_t userdata, Asset::asset_t asset);

        //! Describes a no-I/O runtime resource resolver.
        struct ResourceResolver
        {
            //! Resolution callback. Null selects the default loaded-Asset resolver.
            ResourceResolveCallback resolve = nullptr;
            //! User object retained by the instance and passed to @ref resolve.
            ObjRef userdata;
        };

        //! Describes one emitted symbolic GameGUI action.
        struct Action
        {
            //! Action name authored on the semantic node.
            Name name;
            //! Asset-local source node ID.
            Guid node;
            //! Stable mounted source identity, including the complete nested mount path.
            GUI::id_t source_id = 0;
            //! Action-specific payload.
            Variant payload;
        };

        //! Describes one semantic node generated during the current frame.
        struct GeneratedNodeInfo
        {
            //! Asset-local semantic node ID.
            Guid node;
            //! Stable mounted identity including the complete nested mount path.
            GUI::id_t source_id = 0;
            //! Stable ID of the root GUI element returned by the node generator.
            GUI::id_t root_element_id = 0;
        };

        //! Describes one GameGUI runtime instance.
        struct InstanceDesc
        {
            //! Root semantic document.
            Ref<Document> document;
            //! Optional Asset handle that identifies @ref document for dependency
            //! diagnostics.
            Asset::asset_t source_asset;
            //! Host-selected stable scope mixed into every generated GUI ID.
            GUI::id_t instance_scope = GUI::DEFAULT_DATA_SCOPE;
            //! Optional no-I/O resource resolver.
            ResourceResolver resource_resolver;
        };

        //! @interface IInstance
        //! Compiles a retained semantic tree into GUI elements and owns cross-frame
        //! node state.
        struct [[Luna::interface("{73CC35BC-7056-41D1-9DF4-8C827C79E94A}")]] IInstance : virtual Interface
        {
            //! Validates and prepares all known nodes and loaded nested
            //! dependencies.
            //! @remark This is the only phase that resolves the nested dependency graph.
            //! It performs no file I/O. Unsupported node types are diagnosed and retained
            //! as non-generated subtrees without failing this call.
            virtual RV prepare() = 0;
            //! Builds a fresh GUI element tree for the current frame.
            //! @param[in] context Begun GUI context with an active layer.
            //! @return Returns the generated root element.
            //! @remark An unsupported semantic root returns a successful invalid handle.
            //! An instance can be built at most once for each context generation.
            virtual R<GUI::ElementHandle> build(GUI::IContext* context) = 0;
            //! Resolves authored actions after @ref GUI::IContext::route_input.
            //! @param[in] context The same context and generation passed to the latest
            //! @ref build call.
            virtual RV resolve_interactions(GUI::IContext* context) = 0;
            //! Gets actions emitted by the latest resolution pass.
            //! @return Returns a view invalidated by the next build, resolution or @ref
            //! clear_actions call.
            virtual Span<const Action> get_actions() const = 0;
            //! Clears emitted actions.
            virtual void clear_actions() = 0;
            //! Checks whether the latest resolution pass requested a second layout pass.
            virtual bool relayout_requested() const = 0;
            //! Gets preparation and generation diagnostics.
            //! @return Returns a view invalidated by the next @ref prepare call.
            virtual Span<const Diagnostic> get_diagnostics() const = 0;
            //! Gets frame-local semantic-to-GUI generation mappings.
            //! @return Returns a view invalidated by the next @ref build or @ref prepare
            //! call.
            virtual Span<const GeneratedNodeInfo> get_generated_nodes() const = 0;
            //! Gets the stable GUI ID assigned to one root-document node role.
            virtual GUI::id_t make_stable_id(const Guid& node, const c8* role) const = 0;
        };

        //! Creates one retained GameGUI runtime instance.
        //! @param[in] desc Instance configuration.
        //! @return Returns the new instance.
        LUNA_GAME_GUI_API Ref<IInstance> new_instance(const InstanceDesc& desc);
    }
}
