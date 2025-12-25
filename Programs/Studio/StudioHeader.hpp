/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StudioHeader.hpp
* @author JXMaster
* @date 2020/4/20
*/
#pragma once
#include <Luna/HID/HID.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/ImGui/ImGui.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/Image/DDSImage.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/ObjLoader/ObjLoader.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    template <typename _Ty>
    inline RV load_object_from_json_file(_Ty& dst, const Path& path)
    {
        lutry
        {
            lulet(file, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
            lulet(file_data, VariantUtils::read_json(file));
            luexp(deserialize(dst, file_data));
        }
        lucatchret;
        return ok;
    }

    template <typename _Ty>
    inline R<ObjRef> load_json_asset(object_t userdata, Asset::asset_t asset, const Path& path)
    {
        ObjRef ret;
        lutry
        {
            Path file_path = path;
            file_path.append_extension("json");
            Ref<_Ty> obj = new_object<_Ty>();
            luexp(load_object_from_json_file(*obj.get(), file_path));
            ret = obj;
        }
        lucatchret;
        return ret;
    }

    template <typename _Ty>
    inline R<ObjRef> create_default_object(object_t userdata, Asset::asset_t asset)
    {
        return ObjRef(new_object<_Ty>().object());
    }

    template <typename _Ty>
    inline RV save_object_to_json_file(const _Ty& src, const Path& path)
    {
        lutry
        {
            lulet(file, VFS::open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
            lulet(file_data, serialize(src));
            auto file_data_json = VariantUtils::write_json(file_data);
            luexp(file->write(file_data_json.data(), file_data_json.size()));
        }
        lucatchret;
        return ok;
    }

    template <typename _Ty>
    inline RV save_json_asset(object_t userdata, Asset::asset_t asset, const Path& path, object_t data)
    {
        lutry
        {
            Path file_path = path;
            file_path.append_extension("json");
            Ref<_Ty> obj = ObjRef(data);
            luexp(save_object_to_json_file(*obj.get(), file_path));
        }
        lucatchret;
        return ok;
    }
    
    void async_load_asset(Asset::asset_t asset);

    template <typename _Ty>
    inline Ref<_Ty> get_asset_or_async_load_if_not_ready(Asset::asset_t asset)
    {
        if(asset && Asset::get_asset_state(asset) == Asset::AssetState::unloaded)
        {
            async_load_asset(asset);
        }
        return Asset::get_asset_data<_Ty>(asset);
    }

    //! @interface IAssetEditor
    //! Represents a window of the editor.
    struct IAssetEditor : virtual Interface
    {
        luiid("{410f7868-38b5-4e3f-b291-8e58d2cb7372}");

        virtual void on_render() = 0;
        virtual bool closed() = 0;
    };

    struct AssetEditorDesc
    {
        ObjRef userdata;
        //! Called when the tile is going to be drawn in asset browser.
        void (*on_draw_tile)(object_t userdata, Asset::asset_t asset, const RectF& draw_rect);
        //! Called when a new editor is requested to be open for the specified asset.
        Ref<IAssetEditor>(*new_editor)(object_t userdata, Asset::asset_t editing_asset);
    };

    struct AssetImporterDesc
    {
        //! Called when a new importer is requested to be open for the specified asset.
        Ref<IAssetEditor>(*new_importer)(const Path& create_dir);
    };

    struct AppEnv
    {
        HashSet<Name> new_asset_types; // Displayed on the "New" tab of asset browser.
        HashMap<Name, AssetImporterDesc> importer_types;

        HashMap<Name, AssetEditorDesc> editor_types;

        HashSet<typeinfo_t> component_types;
        HashSet<typeinfo_t> scene_component_types;

        Ref<RHI::IDevice> device;

        u32 graphics_queue;
        u32 async_compute_queue;
        u32 async_copy_queue;

        void register_asset_importer_type(const Name& name, const AssetImporterDesc& desc)
        {
            importer_types.insert(Pair<Name, AssetImporterDesc>(name, desc));
        }

        void register_asset_editor_type(const Name& name, const AssetEditorDesc& desc)
        {
            editor_types.insert(Pair<Name, AssetEditorDesc>(name, desc));
        }
    };

    extern AppEnv* g_env;
}
