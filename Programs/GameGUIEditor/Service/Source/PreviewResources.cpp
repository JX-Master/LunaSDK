/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PreviewResources.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "PreviewResources.hpp"
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/GameGUI/Node.hpp>

namespace Luna::GameGUIEditor
{
    namespace
    {
        R<ObjRef> resolve_preview_resource(object_t userdata, Asset::asset_t asset)
        {
            auto resources = cast_object<PreviewResources>(userdata);
            Guid guid = Asset::get_asset_guid(asset);
            auto found = resources->objects.find(guid);
            if(found != resources->objects.end() && found->second) return found->second;
            auto error = resources->errors.find(guid);
            return set_error(E_NOT_FOUND, "%s", error == resources->errors.end() ?
                "The preview dependency is unavailable in the opened working directories." : error->second.c_str());
        }
    }

    R<Ref<AuthoringDocument>> load_editor_authoring(Asset::asset_t asset)
    {
        lutry
        {
            auto loader = Asset::get_asset_data_unit_loader(asset, get_authoring_data_unit());
            if(!loader.valid() || loader.get() != get_authoring_asset_loader())
                luthrow(set_error(E_NOT_SUPPORTED, "This GameGUI asset has no compatible Authoring data unit."));
            luexp(Asset::load_asset_data_unit(asset, get_authoring_data_unit()));
            lulet(source, Asset::get_asset_data_unit_object<AuthoringDocument>(asset, get_authoring_data_unit()));
            if(!source) luthrow(set_error(E_BAD_DATA, "The GameGUI Authoring data is missing."));
            return source;
        }
        lucatchret;
        return E_FAILURE;
    }

    R<GameGUI::InstanceDesc> prepare_editor_preview(const AuthoringDocument& source,
        Asset::asset_t asset, WorkingDirectories& directories,
        const Function<Ref<AuthoringDocument>(Asset::asset_t)>& working_copy)
    {
        GameGUI::InstanceDesc desc;
        lutry
        {
            luset(desc.document, cook_authoring_document(source));
            desc.source_asset = asset;
            auto resources = new_object<PreviewResources>();
            desc.resource_resolver.userdata = resources.object();
            desc.resource_resolver.resolve = resolve_preview_resource;
            if(asset) resources->objects.insert_or_assign(Asset::get_asset_guid(asset), desc.document.object());
            Vector<Ref<GameGUI::Document>> pending;
            pending.push_back(desc.document);
            for(usize i = 0; i < pending.size(); ++i)
            {
                Vector<Asset::asset_t> references;
                for(const auto& node : pending[i]->nodes)
                {
                    auto type = GameGUI::get_node_type(node.type);
                    if(type.valid() && type.get().collect_assets)
                        type.get().collect_assets(node, references, type.get().userdata.get());
                }
                for(Asset::asset_t reference : references)
                {
                    if(!reference) continue;
                    Guid guid = Asset::get_asset_guid(reference);
                    if(resources->objects.contains(guid)) continue;
                    resources->objects.insert_or_assign(guid, ObjRef());
                    auto load = [&]() -> RV
                    {
                        lutry
                        {
                            luexp(directories.owner(Asset::get_asset_path(reference)));
                            if(Asset::get_asset_type(reference) == GameGUI::get_asset_type())
                            {
                                Ref<AuthoringDocument> authoring = working_copy(reference);
                                if(!authoring)
                                {
                                    // Loading an existing unit must never add or save metadata.
                                    luset(authoring, load_editor_authoring(reference));
                                }
                                if(!authoring) luthrow(E_BAD_DATA);
                                lulet(cooked, cook_authoring_document(*authoring));
                                resources->objects.insert_or_assign(guid, cooked.object());
                                pending.push_back(cooked);
                            }
                            else
                            {
                                luexp(Asset::load_asset_data_unit(reference, Name()));
                                lulet(object, Asset::get_asset_data_unit_object(reference, Name()));
                                resources->objects.insert_or_assign(guid, object);
                            }
                        }
                        lucatchret;
                        return ok;
                    };
                    RV loaded = load();
                    if(failed(loaded)) resources->errors.insert_or_assign(guid, String(explain(loaded.errcode())));
                }
            }
        }
        lucatchret;
        return desc;
    }
}
