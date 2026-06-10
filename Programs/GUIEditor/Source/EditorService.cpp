/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorService.cpp
* @author JXMaster
* @date 2026/6/10
*/
#include "EditorService.hpp"
#include <Luna/Runtime/Serialization.hpp>

namespace Luna
{
    namespace GUIEditor
    {
        static constexpr const c8* name_document = "document";
        static constexpr const c8* name_node = "node";
        static constexpr const c8* name_parent = "parent";
        static constexpr const c8* name_type = "type";
        static constexpr const c8* name_label = "label";
        static constexpr const c8* name_path = "path";
        static constexpr const c8* name_enabled = "enabled";
        static constexpr const c8* name_style = "style";
        static constexpr const c8* name_key = "key";
        static constexpr const c8* name_value = "value";
        static constexpr const c8* name_index = "index";
        static constexpr const c8* name_id = "id";
        static constexpr const c8* name_dirty = "dirty";

        static R<Guid> read_guid(const Variant& data)
        {
            Guid value;
            if(data.type() == VariantType::string)
            {
                value = Guid(data.c_str());
                return value;
            }
            lutry
            {
                luexp(deserialize(value, data));
                return value;
            }
            lucatchret;
            return BasicError::bad_arguments();
        }

        static Variant write_guid(const Guid& value)
        {
            auto data = serialize(value);
            if(succeeded(data))
            {
                return data.get();
            }
            return Variant();
        }

        static u64 read_document_id(const Variant& params, u64 fallback)
        {
            return params[Name(name_document)].unum(fallback);
        }

        static R<Variant> snapshot_asset(const GA::Asset& asset)
        {
            return GA::serialize_asset(asset);
        }

        struct SnapshotEditOp : EditOp
        {
            Variant before;
            Variant after;
            Guid before_selection = Guid(0, 0);
            Guid after_selection = Guid(0, 0);

            SnapshotEditOp(const c8* op_label, Variant&& before_value, Variant&& after_value, const Guid& old_selection, const Guid& new_selection)
                : before(move(before_value))
                , after(move(after_value))
                , before_selection(old_selection)
                , after_selection(new_selection)
            {
                label = op_label ? op_label : "";
            }

            RV apply(EditorDocument& document, const Variant& data, const Guid& selection)
            {
                lutry
                {
                    lulet(asset, GA::deserialize_asset(data));
                    document.asset = asset;
                    document.selected_node = selection;
                    if(document.selected_node != Guid(0, 0) && !GA::find_node(document.asset.get(), document.selected_node))
                    {
                        document.selected_node = GA::get_root(document.asset.get());
                    }
                }
                lucatchret;
                return ok;
            }

            virtual RV undo(EditorDocument& document) override
            {
                return apply(document, before, before_selection);
            }

            virtual RV redo(EditorDocument& document) override
            {
                return apply(document, after, after_selection);
            }
        };

        static Variant document_info(const EditorDocument& document)
        {
            Variant r(VariantType::object);
            r[Name(name_id)] = document.id;
            r[Name(name_dirty)] = document.dirty;
            r[Name("has_path")] = document.has_path;
            r[Name(name_path)] = document.has_path ? document.path.encode().c_str() : "";
            r[Name("selection")] = write_guid(document.selected_node);
            r[Name("node_count")] = (u64)GA::get_node_count(document.asset.get());
            return r;
        }

        static Variant node_info(const EditorDocument& document, const GA::Node& node)
        {
            Variant r(VariantType::object);
            r[Name(name_id)] = write_guid(node.id);
            r[Name(name_parent)] = write_guid(GA::get_parent(&node));
            r[Name(name_type)] = node.type;
            r[Name(name_label)] = node.label.c_str();
            r[Name(name_enabled)] = node.enabled;
            r[Name(name_style)] = node.style;
            r[Name("child_count")] = (u64)GA::get_child_count(&node);
            r[Name("properties")] = node.properties;
            return r;
        }

        template <typename _Func>
        static RV execute_snapshot(EditorDocument& document, const c8* label, _Func&& func)
        {
            if(!document.asset)
            {
                return BasicError::bad_arguments();
            }
            lutry
            {
                lulet(before, snapshot_asset(*document.asset.get()));
                Guid before_selection = document.selected_node;
                luexp(func());
                lulet(after, snapshot_asset(*document.asset.get()));
                Guid after_selection = document.selected_node;
                UniquePtr<EditOp> op(memnew<SnapshotEditOp>(label, move(before), move(after), before_selection, after_selection));
                document.undo_stack.push_back(move(op));
                document.redo_stack.clear();
                document.dirty = true;
            }
            lucatchret;
            return ok;
        }

        RV EditorService::init()
        {
            frontend = Frontend::new_frontend();
            if(!frontend)
            {
                return BasicError::bad_platform_call();
            }
            register_frontend_functions();
            lutry
            {
                lulet(document, new_document());
                active_document_id = document->id;
            }
            lucatchret;
            return ok;
        }

        EditorDocument* EditorService::active_document()
        {
            return find_document(active_document_id);
        }

        EditorDocument* EditorService::find_document(u64 id)
        {
            for(UniquePtr<EditorDocument>& document : documents)
            {
                if(document && document->id == id)
                {
                    return document.get();
                }
            }
            return nullptr;
        }

        R<EditorDocument*> EditorService::new_document()
        {
            UniquePtr<EditorDocument> document(memnew<EditorDocument>());
            document->id = next_document_id++;
            document->asset = GA::new_asset();
            if(!document->asset)
            {
                return BasicError::bad_platform_call();
            }
            document->selected_node = GA::get_root(document->asset.get());
            EditorDocument* ptr = document.get();
            documents.push_back(move(document));
            active_document_id = ptr->id;
            last_status = "Created new GUI asset.";
            return ptr;
        }

        R<EditorDocument*> EditorService::open_document(const Path& path)
        {
            lutry
            {
                lulet(asset, GA::load_asset_from_json_file(path));
                UniquePtr<EditorDocument> document(memnew<EditorDocument>());
                document->id = next_document_id++;
                document->asset = asset;
                document->path = path;
                document->has_path = true;
                document->selected_node = GA::get_root(document->asset.get());
                EditorDocument* ptr = document.get();
                documents.push_back(move(document));
                active_document_id = ptr->id;
                last_status = "Opened GUI asset.";
                return ptr;
            }
            lucatchret;
            return BasicError::bad_arguments();
        }

        RV EditorService::save_document(u64 document_id, const Path* path)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || !document->asset)
            {
                return BasicError::bad_arguments();
            }
            if(path)
            {
                document->path = *path;
                document->has_path = true;
            }
            if(!document->has_path)
            {
                return set_error(BasicError::bad_arguments(), "GUIEditor document does not have a save path.");
            }
            lutry
            {
                luexp(GA::save_asset_to_json_file(*document->asset.get(), document->path));
                document->dirty = false;
                last_status = "Saved GUI asset.";
            }
            lucatchret;
            return ok;
        }

        RV EditorService::create_node(u64 document_id, const Guid& parent, const Name& type, const c8* label, usize index)
        {
            EditorDocument* document = find_document(document_id);
            if(!document)
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Create Node", [&]() -> RV {
                lutry
                {
                    lulet(node, GA::new_node(type, label));
                    Guid parent_id = parent == Guid(0, 0) ? GA::get_root(document->asset.get()) : parent;
                    luexp(GA::add_node(document->asset.get(), node, parent_id, index));
                    document->selected_node = node->id;
                    last_status = "Created node.";
                }
                lucatchret;
                return ok;
            });
        }

        RV EditorService::remove_node(u64 document_id, const Guid& node)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || node == Guid(0, 0) || node == GA::get_root(document->asset.get()))
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Remove Node", [&]() -> RV {
                Ref<GA::Node> target = GA::find_node(document->asset.get(), node);
                if(!target)
                {
                    return BasicError::not_found();
                }
                Guid parent = GA::get_parent(target.get());
                RV remove_result = GA::remove_node(document->asset.get(), node);
                if(failed(remove_result))
                {
                    return remove_result;
                }
                document->selected_node = parent != Guid(0, 0) ? parent : GA::get_root(document->asset.get());
                last_status = "Removed node.";
                return ok;
            });
        }

        RV EditorService::move_node(u64 document_id, const Guid& node, const Guid& parent, usize index)
        {
            EditorDocument* document = find_document(document_id);
            if(!document)
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Move Node", [&]() -> RV {
                RV move_result = GA::move_node(document->asset.get(), node, parent, index);
                if(failed(move_result))
                {
                    return move_result;
                }
                document->selected_node = node;
                last_status = "Moved node.";
                return ok;
            });
        }

        RV EditorService::reorder_node(u64 document_id, const Guid& node, usize index)
        {
            EditorDocument* document = find_document(document_id);
            if(!document)
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Reorder Node", [&]() -> RV {
                RV reorder_result = GA::reorder_node(document->asset.get(), node, index);
                if(failed(reorder_result))
                {
                    return reorder_result;
                }
                document->selected_node = node;
                last_status = "Reordered node.";
                return ok;
            });
        }

        RV EditorService::set_node_common(u64 document_id, const Guid& node, const c8* label, bool enabled, const Name& style)
        {
            EditorDocument* document = find_document(document_id);
            if(!document)
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Set Node Common", [&]() -> RV {
                Ref<GA::Node> target = GA::find_node(document->asset.get(), node);
                if(!target)
                {
                    return BasicError::not_found();
                }
                target->label = label ? label : "";
                target->enabled = enabled;
                target->style = style;
                document->selected_node = node;
                last_status = "Updated node common fields.";
                return ok;
            });
        }

        RV EditorService::set_node_property(u64 document_id, const Guid& node, const Name& key, Variant&& value)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || key.empty())
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Set Node Property", [&]() -> RV {
                Ref<GA::Node> target = GA::find_node(document->asset.get(), node);
                if(!target)
                {
                    return BasicError::not_found();
                }
                if(target->properties.type() != VariantType::object)
                {
                    target->properties = Variant(VariantType::object);
                }
                target->properties[key] = move(value);
                document->selected_node = node;
                last_status = "Updated node property.";
                return ok;
            });
        }

        RV EditorService::erase_node_property(u64 document_id, const Guid& node, const Name& key)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || key.empty())
            {
                return BasicError::bad_arguments();
            }
            return execute_snapshot(*document, "Erase Node Property", [&]() -> RV {
                Ref<GA::Node> target = GA::find_node(document->asset.get(), node);
                if(!target)
                {
                    return BasicError::not_found();
                }
                target->properties.erase(key);
                document->selected_node = node;
                last_status = "Erased node property.";
                return ok;
            });
        }

        RV EditorService::set_selection(u64 document_id, const Guid& node)
        {
            EditorDocument* document = find_document(document_id);
            if(!document)
            {
                return BasicError::bad_arguments();
            }
            if(node != Guid(0, 0) && !GA::find_node(document->asset.get(), node))
            {
                return BasicError::not_found();
            }
            document->selected_node = node;
            return ok;
        }

        RV EditorService::undo(u64 document_id)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || document->undo_stack.empty())
            {
                return BasicError::bad_arguments();
            }
            UniquePtr<EditOp> op = move(document->undo_stack.back());
            document->undo_stack.pop_back();
            RV undo_result = op->undo(*document);
            if(failed(undo_result))
            {
                return undo_result;
            }
            document->redo_stack.push_back(move(op));
            document->dirty = true;
            last_status = "Undo.";
            return ok;
        }

        RV EditorService::redo(u64 document_id)
        {
            EditorDocument* document = find_document(document_id);
            if(!document || document->redo_stack.empty())
            {
                return BasicError::bad_arguments();
            }
            UniquePtr<EditOp> op = move(document->redo_stack.back());
            document->redo_stack.pop_back();
            RV redo_result = op->redo(*document);
            if(failed(redo_result))
            {
                return redo_result;
            }
            document->undo_stack.push_back(move(op));
            document->dirty = true;
            last_status = "Redo.";
            return ok;
        }

        Variant EditorService::invoke(const Name& method, const Variant& params)
        {
            return frontend ? frontend->invoke(method, params) : Frontend::make_error_response(
                Frontend::make_frontend_error("GUIEditor", "not_initialized", "Editor service is not initialized."));
        }

        void EditorService::register_frontend_functions()
        {
            (void)frontend->set_resource_function("/gui_editor/new", [this](Frontend::IFrontend*, const Variant&) -> R<Variant> {
                lutry
                {
                    lulet(document, new_document());
                    return document_info(*document);
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/open", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    lulet(document, open_document(Path(params[Name(name_path)].c_str())));
                    return document_info(*document);
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/save", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    Path path(params[Name(name_path)].c_str());
                    const Path* path_ptr = params[Name(name_path)].valid() ? &path : nullptr;
                    luexp(save_document(id, path_ptr));
                    EditorDocument* document = find_document(id);
                    return document ? document_info(*document) : Variant();
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/document", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                EditorDocument* document = find_document(read_document_id(params, active_document_id));
                if(!document)
                {
                    return BasicError::not_found();
                }
                return document_info(*document);
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/get", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    EditorDocument* document = find_document(read_document_id(params, active_document_id));
                    if(!document)
                    {
                        return BasicError::not_found();
                    }
                    lulet(id, read_guid(params[Name(name_node)]));
                    Ref<GA::Node> node = GA::find_node(document->asset.get(), id);
                    if(!node)
                    {
                        return BasicError::not_found();
                    }
                    return node_info(*document, *node.get());
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/create", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    Guid parent = Guid(0, 0);
                    if(params[Name(name_parent)].valid())
                    {
                        lulet(parsed_parent, read_guid(params[Name(name_parent)]));
                        parent = parsed_parent;
                    }
                    usize index = (usize)params[Name(name_index)].unum(USIZE_MAX);
                    luexp(create_node(id, parent, params[Name(name_type)].str(), params[Name(name_label)].c_str("New Node"), index));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/remove", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    lulet(node, read_guid(params[Name(name_node)]));
                    luexp(remove_node(id, node));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/set_common", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    lulet(node, read_guid(params[Name(name_node)]));
                    luexp(set_node_common(id, node, params[Name(name_label)].c_str(), params[Name(name_enabled)].boolean(true), params[Name(name_style)].str()));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/set_property", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    lulet(node, read_guid(params[Name(name_node)]));
                    Variant value = params[Name(name_value)];
                    luexp(set_node_property(id, node, params[Name(name_key)].str(), move(value)));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/node/erase_property", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    lulet(node, read_guid(params[Name(name_node)]));
                    luexp(erase_node_property(id, node, params[Name(name_key)].str()));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/undo", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    luexp(undo(id));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
            (void)frontend->set_resource_function("/gui_editor/redo", [this](Frontend::IFrontend*, const Variant& params) -> R<Variant> {
                lutry
                {
                    u64 id = read_document_id(params, active_document_id);
                    luexp(redo(id));
                    return document_info(*find_document(id));
                }
                lucatchret;
                return BasicError::bad_arguments();
            }, true);
        }
    }
}
