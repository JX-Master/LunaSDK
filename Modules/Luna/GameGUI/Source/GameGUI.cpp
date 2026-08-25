/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GameGUI.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include "InstanceInternal.hpp"
#include "GameGUI.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            R<ObjRef> load_asset(object_t userdata, Asset::asset_t asset, const Path& path)
            {
                lutry
                {
                    Path document_path = path;
                    document_path.append_extension("json");
                    lulet(file, VFS::open_file(document_path, FileOpenFlag::read,
                        FileCreationMode::open_existing));
                    lulet(data, VariantUtils::read_json(file,
                        VariantUtils::JSONReadOptions::strict()));
                    lulet(document, decode_document(data));
                    return ObjRef(document.object());
                }
                lucatch
                {
                    return luerr;
                }
            }

            R<ObjRef> load_default_asset(object_t userdata, Asset::asset_t asset)
            {
                return ObjRef(new_object<Document>().object());
            }

            RV save_asset(object_t userdata, Asset::asset_t asset, const Path& path, object_t data)
            {
                Document* document = cast_object<Document>(data);
                if (!document)
                    return E_BAD_ARGUMENTS;
                lutry
                {
                    luexp(validate_document(*document));
                    lulet(encoded, encode_document(*document));
                    Path document_path = path;
                    document_path.append_extension("json");
                    lulet(file, VFS::open_file(document_path, FileOpenFlag::write,
                        FileCreationMode::create_always));
                    VariantUtils::JSONWriteOptions options;
                    options.indent = true;
                    options.encode_blobs = false;
                    options.allow_non_finite_numbers = false;
                    luexp(VariantUtils::write_json(file, encoded, options));
                }
                lucatchret;
                return ok;
            }

            RV set_asset_data(object_t userdata, Asset::asset_t asset, object_t data)
            {
                if (data && !cast_object<Document>(data))
                    return E_BAD_ARGUMENTS;
                return ok;
            }

            void get_referred_assets(object_t userdata, Asset::asset_t asset, Vector<Asset::asset_t>& referred_assets)
            {
                Ref<Document> document = Asset::get_asset_data<Document>(asset);
                if (document)
                    get_direct_referred_assets(*document, referred_assets);
            }

            struct GameGUIModule : Module
            {
                virtual const c8* get_name() override { return "GameGUI"; }

                virtual RV on_register() override
                {
                    return add_dependency_modules(
                        this, {module_asset(), GUI::module_gui(), module_variant_utils(), module_vfs()});
                }

                virtual RV on_init() override
                {
                    Meta::register_GameGUI_types();
                    register_boxed_type<Document>();
                    lutry
                    {
                        luexp(register_builtin_node_types());
                        Asset::AssetTypeDesc desc;
                        desc.name = get_asset_type();
                        desc.on_load_asset = load_asset;
                        desc.on_load_asset_default_data = load_default_asset;
                        desc.on_save_asset = save_asset;
                        desc.on_set_asset_data = set_asset_data;
                        desc.on_get_referred_assets = get_referred_assets;
                        Asset::register_asset_type(desc);
                    }
                    lucatchret;
                    return ok;
                }

                virtual void on_close() override { close_node_registry(); }
            };
        }

        LUNA_GAME_GUI_API Module* module_game_gui()
        {
            static GameGUIModule module;
            return &module;
        }
    }
}
