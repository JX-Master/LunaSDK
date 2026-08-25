/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InstanceInternal.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "../Instance.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include "InstanceInternal.generated.hpp"

namespace Luna
{
    namespace GameGUI
    {
        namespace Internal
        {
            struct PreparedNode
            {
                NodeRecord record;
                NodeTypeDesc type;
                Any prepared_data;
                bool supported = false;
            };

            struct PreparedDocument
            {
                Ref<Document> source;
                Vector<PreparedNode> nodes;
                HashMap<Guid, u32> node_indices;
            };

            struct FrameNodeData
            {
                GUI::FlexLayoutDesc flex;
                Vector<GUI::CanvasLayoutItem> canvas_items;
                GUI::CanvasLayoutDesc canvas;
            };

            struct GeneratedNode
            {
                PreparedNode* node = nullptr;
                GUI::id_t identity = 0;
            };

            struct BuildContextData
            {
                Instance* instance = nullptr;
                PreparedDocument* document = nullptr;
                PreparedNode* node = nullptr;
                const ChildLink* parent_link = nullptr;
                GUI::id_t mount_scope = 0;
                GUI::id_t identity = 0;
                FrameNodeData* frame_data = nullptr;
            };

            struct ResolveContextData
            {
                Instance* instance = nullptr;
                PreparedNode* node = nullptr;
                GUI::id_t identity = 0;
            };

            struct [[Luna::struct("{3663D89A-F8A5-4266-931C-DC5C0E47C12C}")]] Instance : IInstance
            {
                luiimpl();

                InstanceDesc desc;
                GUI::IContext* gui = nullptr;
                Vector<UniquePtr<PreparedDocument>> prepared_documents;
                HashMap<object_t, PreparedDocument*> prepared_by_object;
                HashMap<Guid, PreparedDocument*> prepared_by_asset;
                PreparedDocument* root_document = nullptr;
                HashMap<GUI::id_t, Variant> states;
                Vector<UniquePtr<FrameNodeData>> frame_data;
                Vector<GeneratedNode> generated_nodes;
                Vector<GeneratedNodeInfo> generated_node_info;
                Vector<Action> actions;
                Vector<Diagnostic> diagnostics;
                u32 build_generation = 0;
                bool prepared = false;
                bool relayout = false;

                GUI::id_t node_identity(GUI::id_t mount_scope, const Guid& node) const;
                GUI::id_t role_id(GUI::id_t identity, const c8* role) const;
                Variant& state(GUI::id_t identity, PreparedNode* node);
                void add_diagnostic(DiagnosticSeverity severity, const Guid& node, const c8* message,
                                    Span<const Guid> mount_chain = Span<const Guid>());
                R<ObjRef> resolve_asset(Asset::asset_t asset) const;
                R<PreparedDocument*> prepare_document(const Ref<Document>& source, const Guid& asset_guid,
                                                      Vector<object_t>& active_documents, Vector<Guid>& active_mounts);
                R<GUI::ElementHandle> build_node(PreparedDocument* document, PreparedNode* node, GUI::id_t mount_scope,
                                                 const ChildLink* parent_link);
                RV build_children(BuildContextData& context);
                R<GUI::ElementHandle> build_nested(BuildContextData& context, Asset::asset_t asset);

                virtual RV prepare() override;
                virtual R<GUI::ElementHandle> build(GUI::IContext* context) override;
                virtual RV resolve_interactions(GUI::IContext* context) override;
                virtual Span<const Action> get_actions() const override;
                virtual void clear_actions() override;
                virtual bool relayout_requested() const override;
                virtual Span<const Diagnostic> get_diagnostics() const override;
                virtual Span<const GeneratedNodeInfo> get_generated_nodes() const override;
                virtual GUI::id_t make_stable_id(const Guid& node, const c8* role) const override;
            };
        }
    }
}
