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

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            constexpr const c8* RUNTIME_ASSET_LOADER = "Luna.GameGUI.Runtime";

            R<ObjRef> load_asset(object_t userdata, Asset::asset_t asset,
                const Name& data_unit, const Path& path)
            {
                lutry
                {
                    Path document_path = path;
                    document_path.append_extension("cooked");
                    lulet(file, VFS::open_file(document_path, FileOpenFlag::read,
                        FileCreationMode::open_existing));
                    lulet(document, read_cooked_document(file));
                    return ObjRef(document.object());
                }
                lucatch
                {
                    return luerr;
                }
                return E_FAILURE;
            }

            R<ObjRef> load_default_asset(object_t userdata, Asset::asset_t asset,
                const Name& data_unit)
            {
                return ObjRef(new_object<Document>().object());
            }

            RV save_asset(object_t userdata, Asset::asset_t asset, const Name& data_unit,
                const Path& path, object_t data)
            {
                Document* document = cast_object<Document>(data);
                if (!document)
                    return E_BAD_ARGUMENTS;
                lutry
                {
                    luexp(validate_document(*document));
                    Path document_path = path;
                    document_path.append_extension("cooked");
                    lulet(file, VFS::open_file(document_path, FileOpenFlag::write,
                        FileCreationMode::create_always));
                    luexp(write_cooked_document(file, *document));
                }
                lucatchret;
                return ok;
            }

            RV set_asset_data(object_t userdata, Asset::asset_t asset, const Name& data_unit,
                object_t data)
            {
                if (data && !cast_object<Document>(data))
                    return E_BAD_ARGUMENTS;
                return ok;
            }

            void get_referred_assets(object_t userdata, Asset::asset_t asset,
                const Name& data_unit, Vector<Asset::asset_t>& referred_assets)
            {
                auto document = Asset::get_asset_data_unit_object<Document>(asset, Name());
                if(document.valid() && document.get())
                    get_direct_referred_assets(*document.get(), referred_assets);
            }

            struct GameGUIModule : Module
            {
                virtual const c8* get_name() override { return "GameGUI"; }

                virtual RV on_register() override
                {
                    return add_dependency_modules(
                        this, {module_asset(), GUI::module_gui(), module_vfs()});
                }

                virtual RV on_init() override
                {
                    Meta::register_GameGUI_types();
                    register_boxed_type<Document>();
                    lutry
                    {
                        luexp(register_builtin_node_types());
                        Asset::AssetLoaderDesc loader;
                        loader.name = RUNTIME_ASSET_LOADER;
                        loader.on_load_asset_data_unit = load_asset;
                        loader.on_load_asset_data_unit_default_data = load_default_asset;
                        loader.on_save_asset_data_unit = save_asset;
                        loader.on_set_asset_data_unit = set_asset_data;
                        loader.on_get_referred_assets = get_referred_assets;
                        Asset::register_asset_loader(loader);
                        Asset::AssetTypeDesc type;
                        type.name = get_asset_type();
                        type.main_data_unit_loader = RUNTIME_ASSET_LOADER;
                        Asset::register_asset_type(type);
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
