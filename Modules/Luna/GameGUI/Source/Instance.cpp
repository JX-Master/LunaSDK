/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Instance.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include "InstanceInternal.hpp"

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            bool contains_document(Span<const object_t> documents, object_t document)
            {
                for (object_t candidate : documents)
                {
                    if (candidate == document)
                        return true;
                }
                return false;
            }
        }

        using namespace Internal;

        GUI::id_t Internal::Instance::node_identity(GUI::id_t mount_scope, const Guid& node) const
        {
            GUI::id_t result = GUI::make_scoped_id(mount_scope, node.high);
            return GUI::make_scoped_id(result, node.low);
        }

        GUI::id_t Internal::Instance::role_id(GUI::id_t identity, const c8* role) const
        {
            return GUI::make_scoped_id(identity, role ? role : "");
        }

        Variant& Internal::Instance::state(GUI::id_t identity, PreparedNode* node)
        {
            auto iter = states.find(identity);
            if (iter == states.end())
            {
                Variant initial_state = node->type.create_state ? node->type.create_state(node->type.userdata.get())
                                                                : Variant(VariantType::object);
                iter = states.insert(make_pair(identity, move(initial_state))).first;
            }
            return iter->second;
        }

        void Internal::Instance::add_diagnostic(DiagnosticSeverity severity, const Guid& node, const c8* message,
                                                Span<const Guid> mount_chain)
        {
            Diagnostic diagnostic;
            diagnostic.severity = severity;
            diagnostic.node = node;
            diagnostic.message = message;
            diagnostic.asset_mount_chain.assign(mount_chain.begin(), mount_chain.end());
            diagnostics.push_back(move(diagnostic));
        }

        R<ObjRef> Internal::Instance::resolve_asset(Asset::asset_t asset) const
        {
            if (!asset)
                return E_BAD_ARGUMENTS;
            if (desc.resource_resolver.resolve)
            {
                return desc.resource_resolver.resolve(desc.resource_resolver.userdata.get(), asset);
            }
            if (Asset::get_asset_state(asset) != Asset::AssetState::loaded)
            {
                return set_error(E_NOT_FOUND, "GameGUI resource resolution requires already loaded asset data.");
            }
            ObjRef data = Asset::get_asset_data(asset);
            if (!data)
                return E_NOT_FOUND;
            return data;
        }

        R<PreparedDocument*> Internal::Instance::prepare_document(const Ref<Document>& source, const Guid& asset_guid,
                                                                  Vector<object_t>& active_documents,
                                                                  Vector<Guid>& active_mounts)
        {
            object_t source_object = source.object();
            if (contains_document(Span<const object_t>(active_documents.data(), active_documents.size()),
                                  source_object))
            {
                Vector<Guid> cycle_chain = active_mounts;
                if (asset_guid != Guid())
                    cycle_chain.push_back(asset_guid);
                add_diagnostic(DiagnosticSeverity::error, Guid(),
                               "A cycle exists in the nested GameGUI asset mount chain.",
                               Span<const Guid>(cycle_chain.data(), cycle_chain.size()));
                return set_error(E_LOOP, "Cycling GameGUI asset dependency detected.");
            }
            auto existing = prepared_by_object.find(source_object);
            if (existing != prepared_by_object.end())
            {
                if (asset_guid != Guid())
                    prepared_by_asset.insert_or_assign(asset_guid, existing->second);
                return existing->second;
            }

            lutry
            {
                luexp(validate_document(*source, &diagnostics));
                UniquePtr<PreparedDocument> owned(memnew<PreparedDocument>());
                PreparedDocument* prepared_document = owned.get();
                prepared_document->source = source;
                prepared_documents.push_back(move(owned));
                prepared_by_object.insert(make_pair(source_object, prepared_document));
                if (asset_guid != Guid())
                    prepared_by_asset.insert_or_assign(asset_guid, prepared_document);
                active_documents.push_back(source_object);
                if (asset_guid != Guid())
                    active_mounts.push_back(asset_guid);

                for (const NodeRecord& serialized_node : source->nodes)
                {
                    PreparedNode prepared_node;
                    prepared_node.record = serialized_node;
                    auto type_result = get_node_type(serialized_node.type);
                    if (!type_result.valid())
                    {
                        add_diagnostic(DiagnosticSeverity::error, serialized_node.id,
                                       "The GameGUI node type provider is unavailable.");
                        u32 node_index = (u32)prepared_document->nodes.size();
                        prepared_document->node_indices.insert(make_pair(serialized_node.id, node_index));
                        prepared_document->nodes.push_back(move(prepared_node));
                        continue;
                    }
                    NodeTypeDesc type = move(type_result.get());
                    if (serialized_node.type_version > type.current_version)
                    {
                        add_diagnostic(DiagnosticSeverity::error, serialized_node.id,
                                       "The GameGUI node payload is newer than its registered provider.");
                        prepared_node.type = move(type);
                        u32 node_index = (u32)prepared_document->nodes.size();
                        prepared_document->node_indices.insert(make_pair(serialized_node.id, node_index));
                        prepared_document->nodes.push_back(move(prepared_node));
                        continue;
                    }
                    prepared_node.type = move(type);
                    if (prepared_node.record.type_version < prepared_node.type.current_version)
                    {
                        if (!prepared_node.type.migrate)
                        {
                            add_diagnostic(DiagnosticSeverity::error, serialized_node.id,
                                           "The GameGUI node requires a migration callback.");
                            u32 node_index = (u32)prepared_document->nodes.size();
                            prepared_document->node_indices.insert(make_pair(serialized_node.id, node_index));
                            prepared_document->nodes.push_back(move(prepared_node));
                            continue;
                        }
                        while (prepared_node.record.type_version < prepared_node.type.current_version)
                        {
                            u32 from_version = prepared_node.record.type_version;
                            luexp(prepared_node.type.migrate(prepared_node.record.properties, from_version,
                                                             from_version + 1, prepared_node.type.userdata.get()));
                            ++prepared_node.record.type_version;
                        }
                    }
                    if (prepared_node.type.validate)
                    {
                        luexp(prepared_node.type.validate(prepared_node.record, prepared_node.type.userdata.get()));
                    }
                    if (prepared_node.type.prepare)
                    {
                        luset(prepared_node.prepared_data,
                              prepared_node.type.prepare(prepared_node.record, prepared_node.type.userdata.get()));
                    }
                    prepared_node.supported = true;
                    u32 node_index = (u32)prepared_document->nodes.size();
                    prepared_document->node_indices.insert(make_pair(prepared_node.record.id, node_index));
                    prepared_document->nodes.push_back(move(prepared_node));
                }

                for (PreparedNode& node : prepared_document->nodes)
                {
                    if (!node.supported || !node.type.nested_document || !node.type.collect_assets)
                        continue;
                    Vector<Asset::asset_t> nested_assets;
                    node.type.collect_assets(node.record, nested_assets, node.type.userdata.get());
                    for (Asset::asset_t nested_asset : nested_assets)
                    {
                        lulet(resource, resolve_asset(nested_asset));
                        Ref<Document> nested_document(resource);
                        if (!nested_document)
                        {
                            add_diagnostic(DiagnosticSeverity::error, node.record.id,
                                           "A nested GameGUI asset does not contain a GameGUI document.");
                            luthrow(E_BAD_DATA);
                        }
                        Guid nested_guid = Asset::get_asset_guid(nested_asset);
                        lulet(unused, prepare_document(nested_document, nested_guid, active_documents, active_mounts));
                        (void)unused;
                    }
                }
                active_documents.pop_back();
                if (asset_guid != Guid())
                    active_mounts.pop_back();
                return prepared_document;
            }
            lucatch
            {
                if (!active_documents.empty() && active_documents.back() == source_object)
                {
                    active_documents.pop_back();
                    if (asset_guid != Guid() && !active_mounts.empty())
                        active_mounts.pop_back();
                }
                return luerr;
            }
        }

        R<GUI::ElementHandle> Internal::Instance::build_node(PreparedDocument* document, PreparedNode* node,
                                                             GUI::id_t mount_scope, const ChildLink* parent_link)
        {
            if (!node->supported)
                return GUI::ElementHandle();
            UniquePtr<FrameNodeData> owned_frame(memnew<FrameNodeData>());
            FrameNodeData* node_frame_data = owned_frame.get();
            frame_data.push_back(move(owned_frame));
            GUI::id_t identity = node_identity(mount_scope, node->record.id);
            GeneratedNode generated;
            generated.node = node;
            generated.identity = identity;
            generated_nodes.push_back(generated);
            GeneratedNodeInfo info;
            info.node = node->record.id;
            info.source_id = identity;
            usize info_index = generated_node_info.size();
            generated_node_info.push_back(info);

            BuildContextData data;
            data.instance = this;
            data.document = document;
            data.node = node;
            data.parent_link = parent_link;
            data.mount_scope = mount_scope;
            data.identity = identity;
            data.frame_data = node_frame_data;
            BuildContext context(&data);
            auto result = node->type.build(context, node->record, node->type.userdata.get());
            if (result.valid())
                generated_node_info[info_index].root_element_id = result.get().id;
            return result;
        }

        RV Internal::Instance::build_children(BuildContextData& context)
        {
            lutry
            {
                for (const ChildLink& link : context.node->record.children)
                {
                    auto index = context.document->node_indices.find(link.child);
                    if (index == context.document->node_indices.end())
                    {
                        luthrow(E_BAD_DATA);
                    }
                    lulet(unused, build_node(context.document, &context.document->nodes[index->second],
                                             context.mount_scope, &link));
                    (void)unused;
                }
            }
            lucatchret;
            return ok;
        }

        R<GUI::ElementHandle> Internal::Instance::build_nested(BuildContextData& context, Asset::asset_t asset)
        {
            Guid asset_guid = Asset::get_asset_guid(asset);
            auto nested = prepared_by_asset.find(asset_guid);
            if (nested == prepared_by_asset.end())
            {
                return set_error(E_BAD_CALLING_TIME,
                                 "Nested GameGUI asset was not resolved during instance preparation.");
            }
            PreparedDocument* document = nested->second;
            auto root = document->node_indices.find(document->source->root);
            if (root == document->node_indices.end())
                return E_BAD_DATA;
            GUI::id_t nested_mount_scope = GUI::make_scoped_id(context.identity, asset_guid.high);
            nested_mount_scope = GUI::make_scoped_id(nested_mount_scope, asset_guid.low);
            return build_node(document, &document->nodes[root->second], nested_mount_scope, nullptr);
        }

        LUNA_GAME_GUI_API GUI::IContext* BuildContext::gui() const
        {
            return ((BuildContextData*)m_impl)->instance->gui;
        }

        LUNA_GAME_GUI_API GUI::id_t BuildContext::make_id(const c8* role) const
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            return data->instance->role_id(data->identity, role);
        }

        LUNA_GAME_GUI_API GUI::id_t BuildContext::make_child_id(const Guid& child, const c8* role) const
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            GUI::id_t identity = data->instance->node_identity(data->mount_scope, child);
            return data->instance->role_id(identity, role);
        }

        LUNA_GAME_GUI_API Variant& BuildContext::state()
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            return data->instance->state(data->identity, data->node);
        }

        LUNA_GAME_GUI_API const Any& BuildContext::prepared_data() const
        {
            return ((BuildContextData*)m_impl)->node->prepared_data;
        }

        LUNA_GAME_GUI_API RV BuildContext::build_children()
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            return data->instance->build_children(*data);
        }

        LUNA_GAME_GUI_API const ChildLink* BuildContext::parent_link() const
        {
            return ((BuildContextData*)m_impl)->parent_link;
        }

        LUNA_GAME_GUI_API R<GUI::ElementHandle> BuildContext::build_nested(Asset::asset_t asset)
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            return data->instance->build_nested(*data, asset);
        }

        LUNA_GAME_GUI_API R<ObjRef> BuildContext::resolve_resource(Asset::asset_t asset) const
        {
            return ((BuildContextData*)m_impl)->instance->resolve_asset(asset);
        }

        LUNA_GAME_GUI_API void BuildContext::set_flex_layout(const GUI::ElementHandle& element,
                                                             const GUI::FlexLayoutDesc& desc)
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            data->frame_data->flex = desc;
            GUI::LayoutCallbackConfig config;
            config.algorithm = "GameGUI.Flex";
            config.measure_callback = GUI::measure_flex;
            config.callback = GUI::layout_flex;
            config.userdata = &data->frame_data->flex;
            gui()->set_layout_callback_config(element, config);
        }

        LUNA_GAME_GUI_API void BuildContext::set_canvas_layout(const GUI::ElementHandle& element,
                                                               Span<const GUI::CanvasLayoutItem> items,
                                                               bool clip_children)
        {
            BuildContextData* data = (BuildContextData*)m_impl;
            data->frame_data->canvas_items.assign(items.begin(), items.end());
            data->frame_data->canvas.items = Span<const GUI::CanvasLayoutItem>(data->frame_data->canvas_items.data(),
                                                                               data->frame_data->canvas_items.size());
            data->frame_data->canvas.clip_children = clip_children;
            GUI::LayoutCallbackConfig config;
            config.algorithm = "GameGUI.Canvas";
            config.callback = GUI::layout_canvas;
            config.userdata = &data->frame_data->canvas;
            gui()->set_layout_callback_config(element, config);
        }

        LUNA_GAME_GUI_API GUI::IContext* ResolveContext::gui() const
        {
            return ((ResolveContextData*)m_impl)->instance->gui;
        }

        LUNA_GAME_GUI_API GUI::id_t ResolveContext::make_id(const c8* role) const
        {
            ResolveContextData* data = (ResolveContextData*)m_impl;
            return data->instance->role_id(data->identity, role);
        }

        LUNA_GAME_GUI_API Variant& ResolveContext::state()
        {
            ResolveContextData* data = (ResolveContextData*)m_impl;
            return data->instance->state(data->identity, data->node);
        }

        LUNA_GAME_GUI_API const Any& ResolveContext::prepared_data() const
        {
            return ((ResolveContextData*)m_impl)->node->prepared_data;
        }

        LUNA_GAME_GUI_API void ResolveContext::emit_action(const Name& name, const Variant& payload)
        {
            ResolveContextData* data = (ResolveContextData*)m_impl;
            Action action;
            action.name = name;
            action.node = data->node->record.id;
            action.source_id = data->identity;
            action.payload = payload;
            data->instance->actions.push_back(move(action));
        }

        LUNA_GAME_GUI_API void ResolveContext::request_relayout()
        {
            ((ResolveContextData*)m_impl)->instance->relayout = true;
        }

        RV Internal::Instance::prepare()
        {
            if (!desc.document)
                return E_BAD_ARGUMENTS;
            prepared = false;
            prepared_documents.clear();
            prepared_by_object.clear();
            prepared_by_asset.clear();
            states.clear();
            frame_data.clear();
            generated_nodes.clear();
            generated_node_info.clear();
            root_document = nullptr;
            build_generation = 0;
            diagnostics.clear();
            Vector<object_t> active_documents;
            Vector<Guid> active_mounts;
            Guid root_asset_guid = desc.source_asset ? Asset::get_asset_guid(desc.source_asset) : Guid();
            auto root = prepare_document(desc.document, root_asset_guid, active_documents, active_mounts);
            if (!root.valid())
                return root.errcode();
            root_document = root.get();
            prepared = true;
            return ok;
        }

        R<GUI::ElementHandle> Internal::Instance::build(GUI::IContext* context)
        {
            if (!context)
                return E_BAD_ARGUMENTS;
            if (context->generation() == 0)
            {
                return set_error(E_BAD_CALLING_TIME, "GameGUI instance build requires a begun GUI context frame.");
            }
            if (context == gui && context->generation() == build_generation)
            {
                return set_error(E_BAD_CALLING_TIME,
                                 "A GameGUI instance can only be built once per GUI context frame.");
            }
            if (!prepared)
            {
                RV result = prepare();
                if (!result.valid())
                    return result.errcode();
            }
            gui = context;
            build_generation = context->generation();
            frame_data.clear();
            generated_nodes.clear();
            generated_node_info.clear();
            actions.clear();
            relayout = false;
            PreparedDocument* document = root_document;
            auto root = document->node_indices.find(document->source->root);
            if (root == document->node_indices.end())
                return E_BAD_DATA;
            return build_node(document, &document->nodes[root->second], desc.instance_scope, nullptr);
        }

        RV Internal::Instance::resolve_interactions(GUI::IContext* context)
        {
            if (!context || context != gui)
                return E_BAD_ARGUMENTS;
            if (context->generation() != build_generation)
                return E_BAD_CALLING_TIME;
            actions.clear();
            relayout = false;
            lutry
            {
                for (const GeneratedNode& generated : generated_nodes)
                {
                    if (!generated.node->type.resolve)
                        continue;
                    ResolveContextData data;
                    data.instance = this;
                    data.node = generated.node;
                    data.identity = generated.identity;
                    ResolveContext resolve_context(&data);
                    luexp(generated.node->type.resolve(resolve_context, generated.node->record,
                                                       generated.node->type.userdata.get()));
                }
            }
            lucatchret;
            return ok;
        }

        Span<const Action> Internal::Instance::get_actions() const
        {
            return Span<const Action>(actions.data(), actions.size());
        }

        void Internal::Instance::clear_actions() { actions.clear(); }

        bool Internal::Instance::relayout_requested() const { return relayout; }

        Span<const Diagnostic> Internal::Instance::get_diagnostics() const
        {
            return Span<const Diagnostic>(diagnostics.data(), diagnostics.size());
        }

        Span<const GeneratedNodeInfo> Internal::Instance::get_generated_nodes() const
        {
            return Span<const GeneratedNodeInfo>(generated_node_info.data(), generated_node_info.size());
        }

        GUI::id_t Internal::Instance::make_stable_id(const Guid& node, const c8* role) const
        {
            return role_id(node_identity(desc.instance_scope, node), role);
        }

        LUNA_GAME_GUI_API Ref<IInstance> new_instance(const InstanceDesc& desc)
        {
            Ref<Internal::Instance> instance = new_object<Internal::Instance>();
            instance->desc = desc;
            return instance;
        }
    }
}
