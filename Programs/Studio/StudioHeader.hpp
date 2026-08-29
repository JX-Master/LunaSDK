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
#include "StudioEnv.hpp"
#include <Luna/HID/HID.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/Image/DDSImage.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/ObjLoader/ObjLoader.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include "StudioHeader.generated.hpp"

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
    inline R<ObjRef> load_json_asset(object_t userdata, Asset::asset_t asset, const Name& data_unit, const Path& path)
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
    inline R<ObjRef> create_default_object(object_t userdata, Asset::asset_t asset, const Name& data_unit)
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
    inline RV save_json_asset(object_t userdata, Asset::asset_t asset, const Name& data_unit, const Path& path, object_t data)
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
    
    void async_load_asset(Asset::asset_t asset, const Name& data_unit);

    template <typename _Ty>
    inline Ref<_Ty> get_asset_or_async_load_if_not_ready(Asset::asset_t asset, const Name& data_unit)
    {
        if(!asset)
        {
            return nullptr;
        }
        auto state = Asset::get_asset_data_unit_state(asset, data_unit);
        if(succeeded(state) && state.get() == Asset::AssetDataUnitState::unloaded)
        {
            async_load_asset(asset, data_unit);
        }
        auto data = Asset::get_asset_data_unit_object(asset, data_unit);
        if(failed(data))
        {
            return nullptr;
        }
        return Ref<_Ty>(data.get());
    }

}
